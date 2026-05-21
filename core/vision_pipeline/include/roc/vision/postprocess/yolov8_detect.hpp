#pragma once

// =============================================================================
// roc::vision::postprocess::Yolov8DetectPostprocessor
//
// Decoder for the classic YOLOv8 detection head (Ultralytics export, 2023).
// Distinct from `Yolo26DetectPostprocessor` for two reasons:
//
//   1. Class scores are **already sigmoid-activated** in the exported graph
//      (Ultralytics YOLOv8 `forward` returns class probabilities in [0, 1]
//      after the Detect head's `cls_pw` sigmoid).  Re-applying sigmoid in
//      the postprocessor — which is what `yolo26_detect` does for raw
//      logits — would crush the dynamic range and break thresholding.
//   2. YOLOv8 retains many highly-overlapping anchors per object (its
//      training uses Task-Aligned Assignment with k>1), so external
//      **NMS is required** to deduplicate.  YOLO26's Consistent Dual
//      Assignment removes that requirement.
//
// Tensor layout (channel-major, single batch), supports both:
//
//   * [1, 5,  N]  — single-class detector (4 bbox + 1 fused score).
//                   The "fused score" is `obj_conf × cls_conf` already
//                   sigmoid-multiplied at export time.  argmax degenerates
//                   to "class 0 with score = channel 4".
//
//   * [1, 84, N]  — full COCO 80-class detector (4 bbox + 80 class probs).
//                   argmax picks the winning class per anchor.
//
// Both shapes share the same channel-major stride (channel stride == N).
// The decoder accepts rank-2 [F, N] as well as rank-3 [1, F, N] so that
// either Ultralytics-default exports or NCNN-style rank-2 dumps load.
//
// Per-frame cost @ N = 8400 :
//   * Single-class: ~50 K float ops (5 reads × 8400 + score compare).
//   * 80-class:     ~750 K float ops (84 reads × 8400 + argmax).
// Plus the NMS pass, which is O(K log K + K²) on the gate-survivors K (~50).
//
// Inverse letterbox is applied AFTER NMS (cheaper — survives the threshold +
// suppression gates first).  Boxes are projected from model-input pixels to
// the capture-frame pixel system using the same `unletterbox` static helper
// the YOLO26 decoder uses (§4.2 in `phase_1_universal_ep_and_manifest.md`).
//
// Algorithmic source:
//   Ultralytics `yolov8/ultralytics/utils/ops.py::non_max_suppression`,
//   conf_thres = score_threshold, iou_thres = iou_threshold; class-agnostic
//   NMS by default (matches the Ultralytics default `agnostic=False` →
//   per-class NMS; we honour `agnostic=False` semantics here too).
// =============================================================================

#include "roc/vision/postprocessor.hpp"

namespace roc {
namespace vision {

class Yolov8DetectPostprocessor : public Postprocessor {
public:
    Yolov8DetectPostprocessor() = default;

    std::vector<Detection>
    decode_detections(const std::vector<Ort::Value>& outputs,
                      const PostprocessContext&      ctx) const override;

    // ReID heads return empty + logged warning (same contract as yolo26).
    std::vector<Embedding>
    decode_embeddings(const std::vector<Ort::Value>& /*outputs*/,
                      const PostprocessContext&      /*ctx*/) const override {
        return {};
    }

    const char* head_name() const noexcept override { return "yolov8_detect"; }
};

}  // namespace vision
}  // namespace roc
