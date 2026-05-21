#pragma once

// =============================================================================
// roc::vision::OpticalFlowSampler — sparse LK + rigid 3-DOF bg odometry (§13.3).
// =============================================================================

#include <opencv2/core/mat.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace roc {
namespace vision {

struct BgOdometrySample {
    std::uint32_t frame_id   = 0;
    std::int64_t  ts_ns      = 0;
    float         dx_px      = 0.0f;
    float         dy_px      = 0.0f;
    float         theta_rad  = 0.0f;
    float         scale      = 1.0f;
    std::uint16_t feature_count = 0;
    std::uint16_t inlier_count  = 0;
    float         confidence    = 0.0f;
    bool          valid_estimate = false;
};

class OpticalFlowSampler {
public:
    struct Config {
        std::size_t max_features            = 200;
        double      quality_level           = 0.01;
        double      min_distance_px         = 10.0;
        float       center_mask_w_frac      = 0.30f;
        float       center_mask_h_frac      = 0.30f;
        std::size_t feature_renewal_frames  = 30;
        float       ransac_reproj_threshold_px = 3.0f;
    };

    OpticalFlowSampler();
    explicit OpticalFlowSampler(const Config& cfg);

    [[nodiscard]] std::optional<BgOdometrySample> sample(
        const cv::Mat& frame_bgr,
        std::uint32_t  frame_id,
        std::int64_t   ts_ns);

private:
    void ensure_buffers(const cv::Mat& frame_bgr);
    void renew_features(const cv::Mat& gray);
    void build_center_mask(int width, int height);

    Config                        cfg_;
    cv::Mat                       prev_gray_;
    cv::Mat                       curr_gray_;
    cv::Mat                       center_mask_;
    std::vector<cv::Point2f>      prev_features_;
    std::vector<cv::Point2f>      curr_features_;
    std::vector<unsigned char>    lk_status_;
    std::vector<float>            lk_err_;
    std::vector<cv::Point2f>      src_pts_;
    std::vector<cv::Point2f>      dst_pts_;
    std::size_t                   frames_since_renewal_ = 0;
    bool                          warmed_up_          = false;
    int                           last_w_             = 0;
    int                           last_h_             = 0;
};

}  // namespace vision
}  // namespace roc
