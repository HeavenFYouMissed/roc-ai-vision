// =============================================================================
// roc::vision::PostprocessorRegistry — string-keyed strategy table.
// =============================================================================

#include "roc/vision/postprocessor_registry.hpp"

#include "roc/vision/postprocess/osnet_reid.hpp"
#include "roc/vision/postprocess/post_nms.hpp"
#include "roc/vision/postprocess/yolo26_detect.hpp"
#include "roc/vision/postprocess/yolov8_detect.hpp"

namespace roc {
namespace vision {

PostprocessorRegistry::PostprocessorRegistry() {
    entries_.emplace("yolo26_detect",
                     std::unique_ptr<Postprocessor>(new Yolo26DetectPostprocessor()));
    entries_.emplace("osnet_reid",
                     std::unique_ptr<Postprocessor>(new OsnetReidPostprocessor()));
    entries_.emplace("yolov8_detect",
                     std::unique_ptr<Postprocessor>(new Yolov8DetectPostprocessor()));
    entries_.emplace("post_nms",
                     std::unique_ptr<Postprocessor>(new PostNmsPostprocessor()));
}

bool PostprocessorRegistry::register_postprocessor(std::string head_name,
                                                   std::unique_ptr<Postprocessor> p) {
    if (!p) return false;
    auto it = entries_.find(head_name);
    if (it != entries_.end()) return false;
    entries_.emplace(std::move(head_name), std::move(p));
    return true;
}

Postprocessor* PostprocessorRegistry::find(const std::string& head_name) const noexcept {
    auto it = entries_.find(head_name);
    if (it == entries_.end()) return nullptr;
    return it->second.get();
}

}  // namespace vision
}  // namespace roc
