#include "roc/vision/optical_flow_sampler.hpp"

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#include <algorithm>
#include <cmath>

namespace roc {
namespace vision {

OpticalFlowSampler::OpticalFlowSampler() : cfg_() {}

OpticalFlowSampler::OpticalFlowSampler(const Config& cfg) : cfg_(cfg) {
    prev_features_.reserve(cfg_.max_features);
    curr_features_.reserve(cfg_.max_features);
    lk_status_.reserve(cfg_.max_features);
    lk_err_.reserve(cfg_.max_features);
    src_pts_.reserve(cfg_.max_features);
    dst_pts_.reserve(cfg_.max_features);
}

void OpticalFlowSampler::build_center_mask(int width, int height) {
    center_mask_ = cv::Mat(height, width, CV_8UC1, cv::Scalar(255));
    const int mx = static_cast<int>(cfg_.center_mask_w_frac * width);
    const int my = static_cast<int>(cfg_.center_mask_h_frac * height);
    const int x0 = (width - mx) / 2;
    const int y0 = (height - my) / 2;
    cv::rectangle(center_mask_, cv::Rect(x0, y0, mx, my), cv::Scalar(0), cv::FILLED);
}

void OpticalFlowSampler::ensure_buffers(const cv::Mat& frame_bgr) {
    if (frame_bgr.empty()) return;
    if (frame_bgr.cols != last_w_ || frame_bgr.rows != last_h_) {
        last_w_ = frame_bgr.cols;
        last_h_ = frame_bgr.rows;
        prev_gray_.create(last_h_, last_w_, CV_8UC1);
        curr_gray_.create(last_h_, last_w_, CV_8UC1);
        build_center_mask(last_w_, last_h_);
        warmed_up_ = false;
        prev_features_.clear();
        frames_since_renewal_ = 0;
    }
}

void OpticalFlowSampler::renew_features(const cv::Mat& gray) {
    prev_features_.clear();
    cv::goodFeaturesToTrack(
        gray,
        prev_features_,
        static_cast<int>(cfg_.max_features),
        cfg_.quality_level,
        cfg_.min_distance_px,
        center_mask_);
    frames_since_renewal_ = 0;
}

std::optional<BgOdometrySample> OpticalFlowSampler::sample(
        const cv::Mat& frame_bgr,
        const std::uint32_t frame_id,
        const std::int64_t ts_ns) {
    if (frame_bgr.empty()) return std::nullopt;

    ensure_buffers(frame_bgr);
    cv::cvtColor(frame_bgr, curr_gray_, cv::COLOR_BGR2GRAY);

    if (!warmed_up_) {
        curr_gray_.copyTo(prev_gray_);
        renew_features(prev_gray_);
        warmed_up_ = true;
        return std::nullopt;
    }

    if (prev_features_.empty()
        || frames_since_renewal_ >= cfg_.feature_renewal_frames) {
        renew_features(prev_gray_);
    }

    curr_features_.clear();
    lk_status_.clear();
    lk_err_.clear();

    if (!prev_features_.empty()) {
        cv::calcOpticalFlowPyrLK(
            prev_gray_,
            curr_gray_,
            prev_features_,
            curr_features_,
            lk_status_,
            lk_err_,
            cv::Size(21, 21),
            3);
    }

    src_pts_.clear();
    dst_pts_.clear();
    for (std::size_t i = 0; i < prev_features_.size() && i < curr_features_.size();
         ++i) {
        if (lk_status_[i] == 0) continue;
        src_pts_.push_back(prev_features_[i]);
        dst_pts_.push_back(curr_features_[i]);
    }

    BgOdometrySample out{};
    out.frame_id = frame_id;
    out.ts_ns    = ts_ns;
    out.feature_count =
        static_cast<std::uint16_t>(std::min(src_pts_.size(), std::size_t{65535}));

    const std::size_t min_features = 8;
    if (src_pts_.size() < min_features) {
        out.valid_estimate = false;
        out.confidence     = 0.0f;
        curr_gray_.copyTo(prev_gray_);
        prev_features_ = std::move(curr_features_);
        ++frames_since_renewal_;
        return out;
    }

    cv::Mat affine = cv::estimateAffinePartial2D(
        src_pts_,
        dst_pts_,
        cv::noArray(),
        cv::RANSAC,
        static_cast<double>(cfg_.ransac_reproj_threshold_px));

    if (affine.empty() || affine.rows != 2 || affine.cols != 3) {
        out.valid_estimate = false;
        out.confidence     = 0.0f;
    } else {
        out.dx_px     = static_cast<float>(affine.at<double>(0, 2));
        out.dy_px     = static_cast<float>(affine.at<double>(1, 2));
        const double a = affine.at<double>(0, 0);
        const double b = affine.at<double>(0, 1);
        out.theta_rad = static_cast<float>(std::atan2(b, a));
        out.scale     = static_cast<float>(std::sqrt(a * a + b * b));
        out.valid_estimate = true;

        const float inlier_ratio =
            static_cast<float>(src_pts_.size())
            / static_cast<float>(std::max<std::size_t>(prev_features_.size(), 1));
        const float count_norm =
            static_cast<float>(src_pts_.size())
            / static_cast<float>(cfg_.max_features);
        out.inlier_count =
            static_cast<std::uint16_t>(std::min(src_pts_.size(), std::size_t{65535}));
        out.confidence = std::min(1.0f, inlier_ratio * count_norm);

        if (out.inlier_count < static_cast<std::uint16_t>(0.5f * out.feature_count)) {
            out.valid_estimate = false;
        }
    }

    curr_gray_.copyTo(prev_gray_);
    if (curr_features_.size() >= min_features) {
        prev_features_ = std::move(curr_features_);
    } else {
        renew_features(prev_gray_);
    }
    ++frames_since_renewal_;

    return out;
}

}  // namespace vision
}  // namespace roc
