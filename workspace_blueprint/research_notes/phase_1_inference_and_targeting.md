# ROC AI Vision — Research Notes

> Technical reference for the core mathematical and engineering protocols used across the pipeline.
> This document is updated as each phase introduces new subsystems.

---

## 1. Dynamic ONNX Model Metadata Querying

### Protocol

When the ROS 2 vision node loads an `.onnx` file, the **first** operation is to interrogate the model's input layer for its expected shape vector using the ONNX Runtime C++ API.

### API Sequence

```cpp
Ort::Session session(env, model_path, session_options);

// Query input node count and names
size_t num_inputs = session.GetInputCount();
Ort::AllocatorWithDefaultOptions allocator;
auto input_name = session.GetInputNameAllocated(0, allocator);

// Extract the shape vector
auto type_info = session.GetInputTypeInfo(0);
auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
std::vector<int64_t> input_shape = tensor_info.GetShape();
```

### Shape Vector Format

$$\text{Shape} = [\text{Batch},\ \text{Channels},\ \text{Height},\ \text{Width}]$$

Standard vision models return shapes such as:
- `[1, 3, 640, 640]` — YOLOv8 default
- `[1, 3, 1080, 1920]` — Full HD custom model

The engine must handle **any** shape dynamically. No hardcoded dimensions.

### Output Node Querying

The same pattern applies to output nodes. The engine queries `GetOutputCount()`, `GetOutputNameAllocated()`, and the output tensor shape to determine how to parse the raw inference results (e.g., `[1, 84, 8400]` for YOLOv8 detection output).

---

## 2. Letterbox Preprocessing — Linear Scaling Math

### Purpose

Scale an arbitrary capture frame to match the ONNX model's expected input dimensions while preserving the original aspect ratio. Dead space is padded with a flat color.

### Scale Factor Calculation

Given source frame dimensions $(src_w, src_h)$ and target model dimensions $(dst_w, dst_h)$:

$$s = \min\left(\frac{dst_w}{src_w},\ \frac{dst_h}{src_h}\right)$$

### Resized Dimensions

$$new_w = \lfloor src_w \times s \rfloor$$
$$new_h = \lfloor src_h \times s \rfloor$$

### Padding Offsets

$$pad_x = \frac{dst_w - new_w}{2}$$
$$pad_y = \frac{dst_h - new_h}{2}$$

Padding color: `(114, 114, 114)` — the standard gray value used by YOLO-family models to represent neutral background.

### Pixel Normalization

After letterboxing, the pixel buffer is converted from integer byte range to floating point:

$$pixel_{norm} = \frac{pixel_{raw}}{255.0}$$

Channel order: BGR (OpenCV native) → RGB (model expected). The conversion is performed during the `cv::Mat` to tensor copy.

### Inverse Transform

To map detection coordinates back to the original frame:

$$x_{orig} = \frac{x_{det} - pad_x}{s}$$
$$y_{orig} = \frac{y_{det} - pad_y}{s}$$

---

## 3. Target Segment Feature Tracking Maps

### Entity Hierarchy

The system tracks humanoid entities as a hierarchical structure:

| Segment | Role | Tracking Characteristics |
|---------|------|--------------------------|
| `person` | Parent entity | Global bounding box encompassing the full human structure. Used as the root association node. |
| `head` / `face` | Primary anchor | High-precision tracking point. Used for facial re-identification, orientation detection (gaze direction relative to camera), and precision sensor pointing. Exhibits higher positional variance due to head rotation. |
| `upper_torso` | Stable center | Chest and shoulder region. Most stable tracking point in the hierarchy — minimal erratic movement. Used for smooth path trajectory calculation via physical center-of-mass estimation. |
| `accessories` | Appearance features | Hats, vests, backpacks, carried objects. Contribute to the DeepSORT appearance embedding vector for persistent ID maintenance across occlusions and path crossings. |

### Sub-Feature Association

Sub-features are grouped to their parent `person` entity using two criteria:

1. **Spatial proximity:** The centroid of the sub-feature bounding box must fall within a configurable radius of the parent entity's centroid.
2. **Bounding box overlap:** The sub-feature bounding box must have a minimum IoU (Intersection over Union) threshold with the parent entity's global bounding box.

### Actuator Delta Mapping

The system calculates different actuator response profiles depending on the selected tracking target:

- **Head tracking:** Higher angular velocity deltas due to rapid head movement. Requires predictive intercept compensation.
- **Torso tracking:** Smoother, lower-frequency deltas. Preferred for stable long-duration tracking.

The translation vector for reference point alignment:

$$\vec{d} = \begin{bmatrix} dx \\ dy \end{bmatrix} = \begin{bmatrix} t_x - c_x \\ t_y - c_y \end{bmatrix}$$

Where $(t_x, t_y)$ is the target's predicted position from the EKF state vector and $(c_x, c_y)$ is the screen-center reference point.

### Selection Matrix Weights

The TargetSelector evaluates candidates using a weighted scoring function:

$$score_i = w_{prox} \cdot \frac{1}{\|\vec{d}_i\| + \epsilon} + w_{conf} \cdot conf_i + w_{stab} \cdot stab_i$$

Where:
- $w_{prox}$ — Weight for proximity to the reference point
- $w_{conf}$ — Weight for detection confidence
- $w_{stab}$ — Weight for tracking stability (inverse of EKF innovation covariance)
- $\epsilon$ — Small constant to prevent division by zero
