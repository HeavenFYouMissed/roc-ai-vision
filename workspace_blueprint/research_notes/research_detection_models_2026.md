# Detection Model Architecture Research — ROC AI Vision

**Date:** 2026-05-17  
**Author:** Lead CV Research Scientist  
**Scope:** State-of-the-art detection architectures (2024–2026) for real-time humanoid entity tracking  
**Target classes:** `person`, `head`/`face`, `upper_torso`, `accessories` (hat, vest, backpack)  
**Deployment:** ONNX Runtime C++ API, Windows x64, CUDA EP / TensorRT EP / DirectML EP  
**Constraint:** Detection latency must fit within a 1000 Hz EKF tracking loop budget

---

## Table of Contents

1. [YOLO Family Comparison (v8 → v9 → v10 → v11 → YOLO26 → YOLO-World)](#1-yolo-family-comparison)
2. [RT-DETR / RT-DETRv2 / DEIM — Transformer-Based NMS-Free Detection](#2-rt-detr-family)
3. [Co-DETR and DINO-Based Detectors — Accuracy Ceiling](#3-co-detr-and-dino)
4. [Multi-Class Hierarchical Detection Strategy](#4-hierarchical-detection)
5. [Custom Training Considerations — Datasets & Pipelines](#5-custom-training)
6. [ONNX Export Best Practices](#6-onnx-export)
7. [Benchmark Comparison Table](#7-benchmark-table)
8. [Recommendation for ROC AI Vision](#8-recommendation)
9. [References](#9-references)

---

## 1. YOLO Family Comparison

### 1.1 YOLOv8 (Ultralytics, Jan 2023)

**Architecture:**
- CSPDarknet backbone with C2f (Cross-Stage Partial with two convolutions) blocks
- Decoupled detection head separating classification and regression branches
- Fully anchor-free design — predicts normalized box parameters directly
- PANet-style neck for multi-scale feature fusion
- Distribution Focal Loss (DFL) for bounding-box regression

**Key metrics (COCO val2017, 640px):**

| Variant | Params | GFLOPs | mAP@50-95 |
|---------|--------|--------|-----------|
| v8n     | 3.2M   | 8.7    | 37.3      |
| v8s     | 11.2M  | 28.6   | 44.9      |
| v8m     | 25.9M  | 78.9   | 50.2      |
| v8l     | 43.7M  | 165.2  | 52.9      |
| v8x     | 68.2M  | 257.8  | 53.9      |

**ONNX maturity:** Excellent. First-class export via `model.export(format='onnx', dynamic=True, simplify=True)`. Well-tested with ONNX Runtime C++ (official `examples/YOLOv8-ONNXRuntime-CPP/`). Opset 12–13 recommended.

**Dynamic batch:** Supported via `dynamic=True`. **Critical bug:** when `nms=True` and batch > 1, only batch index 0 produces valid detections (GitHub #23647). Workaround: export without NMS and run NMS externally, or use batch=1.

**ROC relevance:** Mature, battle-tested, extensive C++ deployment examples. However, NMS adds non-deterministic latency (0.5–3 ms depending on detection density). 22% more parameters than YOLO11 at equivalent accuracy.

---

### 1.2 YOLOv9 (Chien-Yao Wang et al., Feb 2024)

**Architecture — Two key innovations:**

1. **PGI (Programmable Gradient Information):** Pairs the main computational branch with an auxiliary reversible branch during training. Invertible transformations preserve intermediate feature information, countering information bottleneck problems. The auxiliary branch is discarded at inference — zero additional cost.

2. **GELAN (Generalized Efficient Layer Aggregation Network):** Partitions input feature maps into multiple pathways, applies heterogeneous 1×1 and 3×3 convolutions, aggregates at several depths, concatenates followed by 1×1 projection. Achieves better parameter utilization than depthwise convolution methods using only conventional convolution operators.

**Key metrics (COCO val2017):**

| Variant    | Params | GFLOPs | mAP@50-95 |
|------------|--------|--------|-----------|
| v9-t       | 2.0M   | 7.7    | 38.3      |
| v9-s       | 7.2M   | 26.7   | 46.8      |
| v9-m       | 20.1M  | 76.8   | 51.4      |
| v9-c       | 25.5M  | 102.8  | 53.0      |
| v9-e       | 58.1M  | 192.5  | 55.6      |

**ONNX maturity:** Supported. Available via `yolov9-onnx` PyPI package (v0.8.0). MIT and official variants (gelan-c, gelan-e, yolov9-c, yolov9-e) available. Export via Google Colab notebooks with configurable input sizes.

**Dynamic batch:** Not well-documented. Likely follows standard PyTorch ONNX export patterns but lacks Ultralytics-level tooling.

**ROC relevance:** Strongest small-object accuracy improvements among CNN-only YOLO variants thanks to PGI's information preservation. Not maintained by Ultralytics — community-driven, which means slower bug fixes and export improvements. GELAN is inference-friendly (only standard conv ops).

**Key paper:** Wang et al., "YOLOv9: Learning What You Want to Learn Using Programmable Gradient Information," arXiv:2402.13616

---

### 1.3 YOLOv10 (Tsinghua University, May 2024)

**Architecture — First YOLO with NMS-free inference:**

- **Consistent Dual Assignments:** Uses one-to-many head during training (standard YOLO assignment) and one-to-one head during inference. This eliminates NMS at test time while maintaining strong training signal.
- Spatial-channel decoupled downsampling
- Rank-guided block design for efficient parameter allocation
- Large-kernel convolutions in deeper stages

**Key metrics (COCO val2017):**

| Variant | Params | GFLOPs | mAP@50-95 | Latency (T4, TRT) |
|---------|--------|--------|-----------|--------------------|
| v10-n   | 2.3M   | 6.7    | 38.5      | 1.84 ms            |
| v10-s   | 7.2M   | 21.6   | 46.3      | 2.49 ms            |
| v10-m   | 15.4M  | 59.1   | 51.1      | 4.74 ms            |
| v10-b   | 19.1M  | 92.0   | 52.5      | 5.74 ms            |
| v10-l   | 24.4M  | 120.3  | 53.2      | 7.28 ms            |
| v10-x   | 29.5M  | 160.4  | 54.4      | 10.70 ms           |

**ONNX maturity:** Supported but less tested than v8. The dual-head architecture exports cleanly since only the one-to-one head is active at inference.

**Critical limitation:** The one-to-one assignment during inference can miss overlapping objects — reported lower recall in crowded scenes compared to NMS-based variants. For surveillance with dense person crowds, this is a concern.

**ROC relevance:** Deterministic latency (no NMS) is extremely attractive for a 1000 Hz EKF loop. However, the accuracy regression on overlapping detections (heads inside person boxes, accessories overlapping torso) is problematic for hierarchical multi-class detection. Not Ultralytics-maintained.

**Key paper:** Wang et al., "YOLOv10: Real-Time End-to-End Object Detection," NeurIPS 2024

---

### 1.4 YOLO11 (Ultralytics, Sep 2024)

**Architecture — Efficiency-focused refinements:**

- **C3K2 blocks** (Cross-Stage Partial with 3×3 kernels) replacing C2f modules — lighter, better channel interaction
- **Enhanced SPPF** (Spatial Pyramid Pooling-Fast) for multi-scale features
- **C2PSA** (Cross-Stage Partial Spatial Attention) — spatial attention mechanism targeting small-object fidelity
- Hybrid task-aware label assignment: classification, localization, and auxiliary tasks jointly optimized
- 22% fewer parameters than YOLOv8 at equivalent mAP

**Key metrics (COCO val2017, 640px):**

| Variant | Params | GFLOPs | mAP@50-95 | CPU ONNX (ms) | T4 TRT FP16 (ms) |
|---------|--------|--------|-----------|---------------|-------------------|
| 11n     | 2.6M   | 6.5    | 39.5      | 56.1          | 1.5               |
| 11s     | 9.4M   | 21.5   | 47.0      | —             | 2.5               |
| 11m     | 20.1M  | 68.0   | 51.5      | —             | 4.7               |
| 11l     | 25.3M  | 86.9   | 53.4      | —             | 6.2               |
| 11x     | 56.9M  | 194.9  | 54.7      | —             | 10.7              |

**ONNX maturity:** Excellent. Same Ultralytics export pipeline as v8. Supports detection, segmentation, pose, OBB, classification. Full C++ inference examples available.

**Dynamic batch:** Same `dynamic=True` export. Same NMS batch bug as v8 (GitHub #23647). YOLO26 addresses this.

**ROC relevance:** Best parameter-efficiency in the YOLO family (pre-YOLO26). C2PSA attention mechanism helps with crowded head detection. Still requires NMS, adding latency variance. Strong Ultralytics ecosystem support.

---

### 1.5 YOLO26 (Ultralytics, Sep 2025) — **PRIMARY CANDIDATE**

**Architecture — Edge-first NMS-free redesign:**

1. **NMS-Free End-to-End Inference:** Native end-to-end predictor removes NMS entirely. No IoU/score threshold tuning. Deterministic latency critical for 1000 Hz EKF.

2. **DFL Removal:** Distribution Focal Loss eliminated. Lighter bounding-box regression without distributional decoding. Exports cleanly to ONNX/TensorRT/CoreML/TFLite with fewer custom kernels.

3. **ProgLoss (Progressive Loss Balancing):** Adaptive reweighting of classification, localization, and auxiliary loss terms across training epochs. Prevents domination by easy negatives or large objects in later training.

4. **STAL (Small-Target-Aware Label Assignment):** Adjusts assignment priors and spatial tolerance for tiny/occluded instances. Directly improves recall for head and accessory detection.

5. **MuSGD Optimizer:** Hybrid SGD + Muon-style curvature/momentum. Faster convergence, reduced late-epoch oscillations. Inspired by Moonshot AI's Kimi K2 breakthroughs.

**Key metrics (COCO val2017, 640px):**

| Variant | Params | GFLOPs | mAP@50-95 | CPU ONNX (ms) | T4 TRT10 FP16 (ms) |
|---------|--------|--------|-----------|---------------|---------------------|
| 26n     | 2.4M   | 5.4    | 40.9      | 38.9          | 1.7                 |
| 26s     | 9.5M   | 20.7   | 48.6      | 87.2          | 2.5                 |
| 26m     | 20.4M  | 68.2   | 53.1      | 220.0         | 4.7                 |
| 26l     | 24.8M  | 86.4   | 55.0      | 286.2         | 6.2                 |
| 26x     | 55.7M  | 193.9  | 57.5      | 525.8         | 11.8                |

**Improvement over YOLO11:**
- 26n: +1.4 mAP, −7.7% params, −17% GFLOPs, −31% CPU latency
- 26l: +1.6 mAP, −2.0% params, −0.6% GFLOPs, same T4 latency

**Multi-task:** Object detection, instance segmentation, pose/keypoints, oriented bounding boxes, classification — all from the same backbone/neck with task-specific heads.

**ONNX export:** Simplified operator graph (no DFL, no NMS) maps cleanly with minimal custom kernels. INT8 and FP16 quantization with minimal accuracy loss. No `nms=True` export flag — the batch>1 bug that afflicted v8/v11 does not exist.

**ROC relevance:** **Strongest candidate.** NMS-free = deterministic latency for EKF integration. STAL directly addresses head/accessory small-object detection. Clean ONNX export for C++ deployment. First Ultralytics model to natively unify all five detection tasks. Up to 43% faster CPU inference.

**Key papers:**
- Sapkota et al., "YOLO26: An Analysis of NMS-Free End to End Framework for Real-Time Object Detection," arXiv:2601.12882
- Sapkota et al., "YOLO26: Key Architectural Enhancements and Performance Benchmarking," arXiv:2509.25164
- Sapkota & Karkee, "Ultralytics YOLO Evolution: An Overview of YOLO26, YOLO11, YOLOv8 and YOLOv5," arXiv:2510.09653

---

### 1.6 YOLO-World (Tencent AILab, Jan 2024) — Open-Vocabulary Detection

**Architecture:**
- CNN-based (not Transformer) for speed — built on YOLOv8 backbone
- **Re-parameterizable Vision-Language PAN (RepVL-PAN):** Fuses text embeddings into multi-scale visual features
- **Prompt-then-Detect strategy:** Text prompts are computed offline as vocabulary embeddings, stored as static tensors. At inference, no language model runs — pure CNN detection.
- Pre-trained on large-scale datasets (Objects365, GoldG, CC3M)

**Key metrics (LVIS minival, zero-shot):**

| Variant     | Params | AP   | AP_rare |
|-------------|--------|------|---------|
| YOLO-World-S | 13M   | 24.3 | 16.6    |
| YOLO-World-M | 29M   | 28.6 | 19.7    |
| YOLO-World-L | 48M   | 32.5 | 23.2    |

**ONNX export:** Supported via official `deploy/export_onnx.py`. Options: `--without-nms` for external NMS, `--without-bbox-decoder` for INT8 quantization. ONNX Runtime Python deployment available (ibaiGorordo/ONNX-YOLO-World-Open-Vocabulary-Object-Detection).

**ROC relevance:** Interesting for rapid prototyping — could detect "person wearing hat," "backpack," "safety vest" without custom training via text prompts. However, significantly lower accuracy than closed-vocabulary models on specific classes. Zero-shot AP on LVIS (~28) vs COCO-trained models (~53 mAP). Not competitive for production surveillance where per-class accuracy matters. Best used as a labeling/annotation aid.

---

## 2. RT-DETR Family — Transformer-Based NMS-Free Detection

### 2.1 RT-DETR (Baidu, Apr 2023)

**Architecture — First real-time DETR:**
- Hybrid encoder: CNN backbone (ResNet-18/34/50/101 or HGNetv2) + Transformer encoder for multi-scale feature processing
- Object queries decoded in a Transformer decoder
- End-to-end — **no NMS** by design (set prediction with Hungarian matching)
- Deterministic latency: no post-processing hyperparameters

**Key metrics (COCO val2017, T4 GPU, TRT FP16):**

| Variant      | Backbone   | Params | mAP@50-95 | FPS (T4 TRT) | Latency |
|--------------|------------|--------|-----------|---------------|---------|
| RT-DETR-R18  | ResNet-18  | 20M    | 46.5      | 217           | 4.6 ms  |
| RT-DETR-R50  | ResNet-50  | 42M    | 53.1      | 108           | 9.3 ms  |
| RT-DETR-R101 | ResNet-101 | 76M    | 54.3      | 74            | 13.5 ms |
| RT-DETR-L    | HGNetv2    | 32M    | 53.0      | 114           | 8.8 ms  |
| RT-DETR-X    | HGNetv2    | 67M    | 54.8      | 74            | 13.5 ms |

**ONNX Runtime benchmark (RTX 5080):**
- RT-DETR (ONNX Runtime): 39.35 ms latency, 25.41 FPS
- RT-DETR (TensorRT): 24.15 ms latency, 41.40 FPS
- TensorRT provides ~3× speedup over PyTorch, ~1.6× over ONNX Runtime

**Key advantage:** Detects ~62% more objects per frame (24.48 vs 15.11) compared to YOLOv8 in dense scenes. The Hungarian matching naturally handles overlapping objects (person + head + accessories in same region).

**Key paper:** Zhao et al., "DETRs Beat YOLOs on Real-time Object Detection," CVPR 2024, arXiv:2304.08069

---

### 2.2 RT-DETRv2 (Baidu, Jul 2024)

**Improvements over v1:**
- **Discrete sampling operator:** Replaces `grid_sample` to remove deployment constraints on resource-constrained devices
- **Selective multi-scale feature extraction:** Distinct sampling points for features at different scales in deformable attention
- **Dynamic data augmentation** and scale-adaptive hyperparameters without speed sacrifice

**Key metrics:**

| Variant       | Backbone  | mAP@50-95 | FPS (T4 TRT) |
|---------------|-----------|-----------|---------------|
| RT-DETRv2-S   | ResNet-18 | 48.1      | 217           |
| RT-DETRv2-M   | ResNet-34 | 49.9      | 161           |
| RT-DETRv2-L   | HGNetv2   | 53.4      | 114           |
| RT-DETRv2-X   | HGNetv2   | 54.3      | 74            |

**ONNX support:** Recently added to HuggingFace Optimum (PR #2201, merged Feb 2025). FP16 ONNX has known issues being addressed in the Transformers library. C++ ONNX Runtime deployment available (CVHub520/rtdetr-onnxruntime-deploy with CMake build, CPU + CUDA).

**ROC relevance:** NMS-free with improved deployment story. The discrete sampling operator makes ONNX export cleaner than v1. However, FP16 ONNX bugs and less mature C++ tooling vs YOLO make this riskier for immediate production.

---

### 2.3 DEIM (CVPR 2025) — Faster DETR Training

**Key innovations:**
- **Dense O2O Matching:** Increases positive samples per image using data augmentation during one-to-one matching
- **Matchability-Aware Loss (MAL):** Handles low-quality matches introduced by Dense O2O
- 50% training time reduction when integrated with RT-DETR / D-FINE

**Performance:**

| Model         | mAP@50-95 | FPS (T4) |
|---------------|-----------|----------|
| DEIM-D-FINE-L | 54.7      | 124      |
| DEIM-D-FINE-X | 56.5      | 78       |

**PINTO model zoo:** DEIMv2-Wholebody34 variant exists with 34 body-part classes (body, head, face, eye, nose, mouth, ear, hand-L, hand-R, foot, etc.) — directly relevant to ROC's hierarchical detection needs.

---

## 3. Co-DETR and DINO-Based Detectors — Accuracy Ceiling

### 3.1 Co-DETR (Sense-X, 2023–2024)

**Architecture:**
- Collaborative hybrid assignments that combine one-to-one and one-to-many training
- Builds on DINO (DETR with Improved deNoising anchOr boxes)
- Achieves the highest accuracy on COCO among end-to-end detectors

**Key metrics (COCO test-dev):**

| Model                      | Backbone  | AP   |
|----------------------------|-----------|------|
| Co-DINO (ResNet-50)        | R50       | 55.4 |
| Co-DINO (Swin-L)           | Swin-L    | 60.7 |
| Co-DETR (Swin-L, O365 PT)  | Swin-L    | 64.8 |
| Co-DETR (ViT-L)            | ViT-L     | 66.0 |

**TensorRT deployment (Co-DETR-TensorRT, RTX 4090):**
- 4× inference runtime improvement over PyTorch FP32
- Custom C++ TensorRT plugin for Multi-Scale Deformable Attention
- Full TensorRT compilation possible with hybrid PyTorch/TensorRT execution

**ONNX export:** Supported with opset_version=16. ONNX-to-TensorRT conversion can hit memory constraints on the deformable attention operator.

**LVIS performance:** 72.0 box AP, 59.7 mask AP on LVIS minival — exceptional for rare/fine-grained categories.

**ROC relevance:** Sets the accuracy ceiling (66 AP) but at extreme computational cost. ViT-L backbone is far too heavy for real-time (>50 ms per frame even with TensorRT). Co-DINO R50 at 55.4 AP is more realistic but still slower than YOLO26-l (55.0 AP) by a significant margin. **Not recommended for production** — useful as an accuracy reference and for offline dataset annotation.

**Key paper:** Zong et al., "DETRs with Collaborative Hybrid Assignments Training," ICCV 2023

---

### 3.2 LightlyTrain DINOv3 / LTDETR

More practical DINO-derived option from Lightly AI:
- DINOv3/ConvNeXt-large: 60.0 mAP@50-95 on COCO val, 42.3 ms latency on T4
- Self-supervised pre-training significantly boosts domain adaptation

---

## 4. Multi-Class Hierarchical Detection Strategy

### 4.1 The Core Question

Can a single model detect `person` + `head` + `upper_torso` + `accessories` simultaneously, or is a two-stage cascade (detect person → crop → detect sub-features) better?

### 4.2 Evidence: Single-Model Multi-Class Detection Works

The PINTO model zoo (github.com/PINTO0309/PINTO_model_zoo) provides extensive evidence that **single-model hierarchical detection is viable and practical:**

**YOLOX-Body-Head-Hand (Model 426):**
- 3 classes: body, head, hand
- 14,667 images, 66,903 labels (30,729 body, 26,268 head, 18,087 hand)
- All images completely re-annotated by hand ("not miss a single object, not compromise on a single pixel")
- Available in ONNX: N, T, S, M, L, X variants
- Strong resistance to blur, occlusion, varying lighting, darkness, halation
- Both integrated NMS and external NMS ONNX variants

**YOLOX-WholeBody12 (Model 449):**
- 12 classes: Body, BodyWithWheelchair, Head, Face, Eye, Nose, Mouth, Ear, Hand, Hand-Left, Hand-Right, Foot
- Enhanced noise robustness (motion blur, gaussian noise, contrast noise, backlighting)

**YOLOX-Body-Head-Face-HandLR-Foot-Dist (Model 445):**
- Body, head, face, left/right hands, feet, plus distance estimation

**YOLOv9-Wholebody25 / Wholebody28:**
- 25/28 body-part classes in a single YOLOv9 model
- Classes include body, head, face, eye, nose, mouth, ear, hand variants, foot

**RT-DETRv2-Wholebody25:**
- NMS-free transformer variant with 25 body-part classes

**DEIMv2-Wholebody34:**
- Latest CVPR 2025 DEIM with 34 classes, NMS-free

### 4.3 Single-Stage vs Two-Stage Analysis

| Criterion                      | Single-Stage (Flat Multi-Class) | Two-Stage (Person → Crop → Sub-features) |
|-------------------------------|----------------------------------|-------------------------------------------|
| Latency                       | 1 forward pass (~2–6 ms)        | 2+ passes (~4–15 ms with N crops)         |
| Complexity                    | Simple pipeline                  | Crop management, ROI alignment            |
| Accuracy on small parts       | Good with STAL/attention         | Better — cropped region has higher resolution |
| GPU memory                    | One model loaded                 | Two models, more VRAM                     |
| Annotation effort             | One unified dataset              | Separate datasets or cross-referencing    |
| Occlusion handling            | Natural — sees global context    | Crops may miss context                    |
| Batch efficiency              | Full frame batching              | Variable crops per frame                  |
| ONNX deployment complexity    | 1 session                        | 2+ sessions, dynamic crop batching        |

### 4.4 Recommendation for ROC

**Use a single-model approach** for the following reasons:

1. **Latency:** A two-stage approach with N person crops creates O(N) additional inference passes. With 5–15 persons in a surveillance frame, this adds 10–45 ms — unacceptable for the EKF loop.

2. **Proven viability:** PINTO's YOLOX-Body-Head-Hand demonstrates robust multi-class body-part detection in a single model, already available in ONNX.

3. **YOLO26 + STAL:** YOLO26's Small-Target-Aware Label Assignment specifically addresses the small-object challenge (heads, accessories) that was the main argument for two-stage detection.

4. **Simplified ONNX pipeline:** One session, one forward pass, one output tensor to parse. No dynamic crop batching logic.

**Fallback option:** If single-model accuracy on accessories (hat, vest, backpack) proves insufficient, use a lightweight secondary classifier on head/torso crops — but only after exhausting single-model training improvements.

---

## 5. Custom Training Considerations

### 5.1 Datasets for Head / Torso / Accessory Detection

#### 5.1.1 CrowdHuman (Megvii, 2018)

- **Scale:** 15,000 training, 4,370 validation, 5,000 test images
- **Density:** ~470K human instances, average 23 persons/image
- **Annotations:** Three bounding boxes per instance:
  - `fbox` — full body
  - `vbox` — visible region
  - `hbox` — head bounding box
- **Format:** `.odgt` (JSON per line) with occlusion and visibility attributes
- **Conversion:** CrowdHuman → YOLO format converters exist (github.com/yakhyo/yolov8-crowdhuman)
- **ROC relevance:** **Primary source** for head detection supervision in crowded scenes. The `hbox` annotations are exactly what ROC needs for the `head` class.

**Key paper:** Shao et al., "CrowdHuman: A Benchmark for Detecting Human in a Crowd," arXiv:1805.00123

#### 5.1.2 COCO-WholeBody (ECCV 2020)

- **Scale:** Extension of COCO with 133 dense keypoints per person
  - 68 face landmarks, 42 hand landmarks, 23 body/feet keypoints
- **Bounding boxes:** Face box, left/right hand boxes, body box derived from keypoints
- **ROC relevance:** Can derive `head` (face bbox expanded) and `upper_torso` (shoulder keypoints → bbox) annotations programmatically. Useful for generating torso bounding boxes from keypoint data.

**Key paper:** Jin et al., "Whole-Body Human Pose Estimation in the Wild," ECCV 2020, arXiv:2007.11858

#### 5.1.3 MOT17 / MOT20 (MOTChallenge)

- **MOT17:** Diverse pedestrian tracking sequences (intersections, malls, streets) with person-level bounding box annotations at 25 FPS
- **MOT20:** Crowded scenes — 8 sequences, 2,332 tracks, 1,336,920 annotated boxes in training
- **ROC relevance:** Excellent for `person` class training with temporal consistency. No head/torso annotations — must be augmented with other datasets.

#### 5.1.4 PPE / Accessory Datasets

| Dataset | Images | Classes | mAP (best) | Source |
|---------|--------|---------|------------|--------|
| Construction-PPE | ~5,000 | 11 (vest, helmet, etc.) | — | Ultralytics curated |
| PPE Detection (HF) | ~5,000 | 6 (vest, helmet, mask, goggles, gloves, shoes) | — | 51ddhesh/PPE_Detection |
| YOLOv5-PPE | 9,675 | 6 (person, helmet, vest, gloves, glasses, mask) | 0.841 mAP50 | DarthRegicid1 |
| Hard-hat-vest (HF) | — | 5 (hat, nohat, vest, novest, person) | 0.809 mAP50 | wesjos, YOLO11m |

**For ROC's accessory classes** (hat, vest, backpack):
- PPE datasets cover hat/helmet and vest well
- Backpack detection requires additional data — consider Objects365 (backpack class exists), Open Images V7, or custom annotation

#### 5.1.5 PINTO Re-Annotated COCO-Hand

- 14,667 images completely hand-re-annotated
- Extreme annotation quality — "not miss a single object, not compromise on a single pixel"
- 30,729 body, 26,268 head, 18,087 hand labels
- **Directly usable** as pre-training foundation for ROC's body+head classes

### 5.2 Recommended Training Pipeline

```
Phase 1: Dataset Construction
├── CrowdHuman (hbox → head, fbox → person)
├── COCO-WholeBody (face bbox → head, keypoint-derived → upper_torso)
├── PINTO COCO-Hand (body + head, pre-annotated)
├── PPE datasets (hat, vest → accessories)
├── Objects365 / Open Images (backpack, bag classes)
└── Merge → Unified YOLO-format dataset
    Classes: [person, head, upper_torso, hat, vest, backpack]

Phase 2: Pre-training
├── Start from YOLO26m or YOLO26l COCO checkpoint
├── Freeze backbone, train neck + head on merged dataset
├── 50 epochs, batch=16, img=640, augmentation=heavy
└── Validate on held-out CrowdHuman val + custom accessory val

Phase 3: Fine-tuning
├── Unfreeze all layers
├── Lower LR (1e-4 → 1e-5)
├── 100 epochs with ProgLoss + STAL enabled
├── Focus on hard negatives: occluded heads, small accessories
└── Export best checkpoint to ONNX

Phase 4: Quantization
├── Post-Training Quantization: FP16 → validate mAP drop < 0.5
├── INT8 calibration with representative surveillance frames
├── TensorRT engine build + cache for target GPU
└── Latency validation: must be < 3 ms per frame (640px)
```

---

## 6. ONNX Export Best Practices

### 6.1 Dynamic Axes Configuration

```python
# YOLO26 export (recommended)
from ultralytics import YOLO
model = YOLO('yolo26m.pt')
model.export(
    format='onnx',
    imgsz=640,
    dynamic=False,     # Fixed batch=1 for C++ deployment (more deterministic)
    simplify=True,     # Run onnx-simplifier to prune dead nodes
    opset=13,          # Well-supported across ONNX Runtime, TensorRT, DirectML
    half=False,        # Export FP32, quantize separately
)
```

**For dynamic batch (if needed):**
```python
model.export(format='onnx', dynamic=True)
# Sets: dynamic_axes={'images': {0: 'batch'}, 'output0': {0: 'batch'}}
```

**YOLO26 advantage:** No `nms=True` flag. The NMS-free design means the batch>1 bug (v8/v11 GitHub #23647) does not exist.

### 6.2 NMS: Baked-In vs External

| Approach | Latency | Determinism | ONNX Compatibility | ROC Recommendation |
|----------|---------|-------------|---------------------|---------------------|
| Baked-in NMS (ONNX NonMaxSuppression op) | Variable (0.5–3 ms) | Non-deterministic | Limited TRT support | Avoid |
| External NMS (CPU post-processing) | Adds CPU→GPU transfer | Predictable | Universal | Acceptable for v8/v11 |
| NMS-free (YOLO26, RT-DETR) | Zero post-processing | Fully deterministic | Perfect | **Strongly preferred** |

### 6.3 Quantization Strategies

**FP16 (Half Precision):**
- Typically <0.3 mAP drop on COCO
- 2× memory bandwidth reduction
- Native on RTX 4070/4090 Tensor Cores
- YOLO26's DFL-free graph quantizes cleanly

**INT8 (8-bit Integer):**
- 0.5–1.5 mAP drop depending on calibration quality
- 4× memory bandwidth reduction, 2× compute throughput
- Requires calibration dataset (100–500 representative images)
- NVIDIA Model Optimizer (formerly TensorRT Model Optimizer) supports FP8, INT8, INT4_AWQ, NVFP4
- Use Opset 20 for latest quantization features

**Quantization-Aware Training (QAT):**
- Train with simulated quantization noise
- Recovers 0.3–0.8 mAP vs post-training INT8
- Recommended only if INT8 PTQ mAP drop exceeds 1.0

**YOLO26 quantization advantage:** Removal of DFL reduces the number of operators that are brittle across compilers/runtimes. The simplified graph maps more predictably to INT8/FP16.

### 6.4 TensorRT Compatibility

- ONNX opsets 9–22 supported
- Engine caching is critical on Windows — first inference builds the engine (can take 30–120 seconds). Cache to SSD.
- `trt_fp16_enable=1` for Tensor Core acceleration
- `trt_max_workspace_size=4GB` recommended
- YOLO26 NMS-free graph: no custom plugins needed — pure standard ops

### 6.5 DirectML EP (Windows-native fallback)

- Supports ONNX opsets up to 18
- No INT8 acceleration (FP16 only)
- Broader GPU compatibility (AMD, Intel, NVIDIA)
- 20–40% slower than CUDA EP on NVIDIA hardware
- Useful as fallback when CUDA SDK is unavailable

---

## 7. Benchmark Comparison Table

### 7.1 Architecture Comparison (COCO val2017, 640px input)

| Model | Params | GFLOPs | mAP@50-95 | T4 TRT FP16 (ms) | NMS-Free | Multi-Task | ONNX C++ Maturity |
|-------|--------|--------|-----------|-------------------|----------|------------|-------------------|
| **YOLOv8m** | 25.9M | 78.9 | 50.2 | ~5.0 | No | Yes | Excellent |
| **YOLOv8l** | 43.7M | 165.2 | 52.9 | ~8.0 | No | Yes | Excellent |
| **YOLOv9c** | 25.5M | 102.8 | 53.0 | ~6.5 | No | Partial | Good |
| **YOLOv9e** | 58.1M | 192.5 | 55.6 | ~12.0 | No | Partial | Good |
| **YOLOv10m** | 15.4M | 59.1 | 51.1 | 4.7 | Yes | No | Fair |
| **YOLOv10l** | 24.4M | 120.3 | 53.2 | 7.3 | Yes | No | Fair |
| **YOLO11m** | 20.1M | 68.0 | 51.5 | 4.7 | No | Yes | Excellent |
| **YOLO11l** | 25.3M | 86.9 | 53.4 | 6.2 | No | Yes | Excellent |
| **YOLO26m** | 20.4M | 68.2 | **53.1** | **4.7** | **Yes** | **Yes** | Excellent |
| **YOLO26l** | 24.8M | 86.4 | **55.0** | **6.2** | **Yes** | **Yes** | Excellent |
| **YOLO26x** | 55.7M | 193.9 | **57.5** | 11.8 | **Yes** | **Yes** | Excellent |
| RT-DETR-R50 | 42M | — | 53.1 | 9.3 | Yes | No | Fair |
| RT-DETR-L | 32M | — | 53.0 | 8.8 | Yes | No | Fair |
| RT-DETRv2-L | ~32M | — | 53.4 | 8.8 | Yes | No | Good |
| DEIM-D-FINE-L | — | — | 54.7 | 8.1 | Yes | No | Fair |
| Co-DINO R50 | ~47M | — | 55.4 | ~25 | Yes | No | Poor |
| Co-DETR ViT-L | ~300M | — | 66.0 | >50 | Yes | Yes | Poor |

### 7.2 Estimated RTX 4070/4090 Latency (TensorRT FP16, batch=1, 640px)

Based on T4 benchmarks scaled by published GPU compute ratios (4090 ~3× T4, 4070 ~2× T4):

| Model | RTX 4090 Est. | RTX 4070 Est. | Fits 1 kHz EKF? |
|-------|---------------|---------------|------------------|
| YOLO26n | ~0.6 ms | ~0.9 ms | Yes (budget to spare) |
| YOLO26s | ~0.8 ms | ~1.3 ms | Yes |
| YOLO26m | ~1.6 ms | ~2.4 ms | Yes |
| YOLO26l | ~2.1 ms | ~3.1 ms | Borderline (4070) |
| YOLO26x | ~3.9 ms | ~5.9 ms | No (4070) |
| RT-DETR-R50 | ~3.1 ms | ~4.7 ms | No |
| RT-DETRv2-L | ~2.9 ms | ~4.4 ms | No |
| YOLO11m | ~1.6 ms | ~2.4 ms | Yes (but +NMS adds 0.5–3ms) |
| Co-DINO R50 | ~8 ms | ~13 ms | No |

**Note:** The 1 kHz EKF runs its predict step at 1 kHz; the detection model does not need to run at 1 kHz. The detection feeds measurements to the EKF asynchronously. The real constraint is that detection latency must be low enough to provide fresh measurements at a useful rate (30–120 Hz). At YOLO26m's ~1.6 ms on 4090, the detection loop can theoretically run at ~625 Hz — more than sufficient.

---

## 8. Recommendation for ROC AI Vision

### 8.1 Primary Architecture: YOLO26m (Custom-Trained)

**Rationale:**

1. **NMS-free deterministic latency** — eliminates the single largest source of jitter in the detection pipeline. For a system feeding a 1000 Hz EKF, latency determinism is as important as raw speed.

2. **STAL for small objects** — directly addresses head and accessory detection, which are the hardest classes due to small spatial extent relative to the full person bounding box.

3. **Best mAP/latency ratio** — 53.1 mAP at 4.7 ms (T4) matches or exceeds YOLO11m (51.5 mAP at 4.7 ms) and RT-DETR-R50 (53.1 mAP at 9.3 ms) while being NMS-free.

4. **Clean ONNX export** — DFL-free, NMS-free graph with standard operators. No batch>1 bug. Clean INT8/FP16 quantization path.

5. **Ultralytics ecosystem** — production-grade Python API, C++ examples, active maintenance, broad community.

### 8.2 Fallback: YOLO26s (If Latency-Constrained on RTX 4070)

YOLO26s at 48.6 mAP and ~1.3 ms estimated on RTX 4070 provides substantial latency headroom. Accuracy gap vs 26m can be partially recovered by:
- Fine-tuning on domain-specific surveillance data
- Higher input resolution (768 or 832 instead of 640) if budget allows
- Training with specialized augmentation (motion blur, night scenes)

### 8.3 Secondary Model (If Needed): RT-DETRv2-Wholebody25

If single-model YOLO26 custom training on the 6-class problem (person, head, upper_torso, hat, vest, backpack) underperforms on head/accessories, consider:
- PINTO's RT-DETRv2-Wholebody25 as a pre-trained starting point
- Already trained on 25 body-part classes including body, head, face
- NMS-free by design
- Trade-off: higher latency (~9 ms on T4)

### 8.4 Not Recommended

| Model | Reason |
|-------|--------|
| YOLOv8/v9/v11 | NMS-dependent — latency jitter incompatible with deterministic EKF feeding |
| YOLOv10 | NMS-free but lower recall on overlapping objects, not Ultralytics-maintained |
| Co-DETR | Far too heavy for real-time — accuracy ceiling reference only |
| YOLO-World | Zero-shot accuracy insufficient for production class-specific detection |

### 8.5 Training Plan

```
Dataset:       CrowdHuman + COCO-WholeBody + PPE + Objects365 backpack
Base model:    yolo26m.pt (COCO pretrained)
Classes:       [person, head, upper_torso, hat, vest, backpack]
Input size:    640 × 640
Training:      100 epochs, MuSGD, ProgLoss + STAL enabled
Augmentation:  Mosaic, MixUp, RandomCrop, motion blur, night-scene simulation
Export:        ONNX (opset 13, FP32) → TensorRT FP16 engine → cache on SSD
Target:        RTX 4070/4090, ONNX Runtime C++ with CUDA EP or TensorRT EP
Latency goal:  < 3 ms per frame at 640px (TensorRT FP16)
```

---

## 9. References

### Papers

1. Sapkota et al., "YOLO26: An Analysis of NMS-Free End to End Framework for Real-Time Object Detection," arXiv:2601.12882, Jan 2026
2. Sapkota et al., "YOLO26: Key Architectural Enhancements and Performance Benchmarking for Real-Time Object Detection," arXiv:2509.25164, Sep 2025
3. Sapkota & Karkee, "Ultralytics YOLO Evolution: An Overview of YOLO26, YOLO11, YOLOv8 and YOLOv5," arXiv:2510.09653, Oct 2025
4. Wang et al., "YOLOv9: Learning What You Want to Learn Using Programmable Gradient Information," arXiv:2402.13616, Feb 2024
5. Wang et al., "YOLOv10: Real-Time End-to-End Object Detection," NeurIPS 2024, arXiv:2405.14458
6. Zhao et al., "DETRs Beat YOLOs on Real-time Object Detection (RT-DETR)," CVPR 2024, arXiv:2304.08069
7. Lv et al., "RT-DETRv2: Improved Baseline with Bag-of-Freebies for Real-Time Detection Transformer," arXiv:2407.17140, Jul 2024
8. Shao et al., "CrowdHuman: A Benchmark for Detecting Human in a Crowd," arXiv:1805.00123, 2018
9. Jin et al., "Whole-Body Human Pose Estimation in the Wild," ECCV 2020, arXiv:2007.11858
10. Zong et al., "DETRs with Collaborative Hybrid Assignments Training (Co-DETR)," ICCV 2023
11. Chen et al., "DEIM: DETR with Improved Matching for Fast Convergence," CVPR 2025, arXiv:2412.04234

### Repositories

- Ultralytics YOLO: https://github.com/ultralytics/ultralytics
- YOLO-World: https://github.com/AILab-CVC/YOLO-World
- RT-DETR: https://github.com/lyuwenyu/RT-DETR
- RT-DETRv2 ONNX Runtime C++ Deploy: https://github.com/CVHub520/rtdetr-onnxruntime-deploy
- Co-DETR TensorRT: https://github.com/anenbergb/Co-DETR-TensorRT
- PINTO Model Zoo: https://github.com/PINTO0309/PINTO_model_zoo
- YOLOX-Body-Head-Hand (Model 426): https://github.com/PINTO0309/PINTO_model_zoo/tree/main/426_YOLOX-Body-Head-Hand
- YOLOX-WholeBody12 (Model 449): https://github.com/PINTO0309/PINTO_model_zoo/tree/main/449_YOLOX-WholeBody12
- YOLOv8-CrowdHuman: https://github.com/yakhyo/yolov8-crowdhuman
- ONNX-YOLO-World: https://github.com/ibaiGorordo/ONNX-YOLO-World-Open-Vocabulary-Object-Detection
- NVIDIA Model Optimizer: https://github.com/NVIDIA/TensorRT-Model-Optimizer
- DEIM: https://github.com/sebastianfis/DEIM

### Benchmark & Documentation Sources

- Ultralytics YOLO26 Docs: https://docs.ultralytics.com/models/yolo26/
- Ultralytics ONNX Export: https://www.ultralytics.com/blog/using-the-onnx-integration-to-export-ultralytics-yolo-models
- NVIDIA TensorRT ONNX Conversion: https://docs.nvidia.com/deeplearning/tensorrt-rtx/latest/installing-tensorrt-rtx/onnx-conversion.html
- NVIDIA TensorRT Best Practices: https://docs.nvidia.com/deeplearning/tensorrt/latest/performance/best-practices.html
- MOTChallenge: https://motchallenge.net/
- ONNX Runtime CUDA EP: https://onnxruntime.ai/docs/execution-providers/CUDA-ExecutionProvider.html
- ONNX Runtime TensorRT EP: https://onnxruntime.ai/docs/execution-providers/TensorRT-ExecutionProvider.html

---

*This document constitutes the detection model selection research for ROC AI Vision Phase 1. The recommendation is to proceed with YOLO26m custom-trained on a merged CrowdHuman + COCO-WholeBody + PPE dataset, exported via ONNX to TensorRT FP16, targeting <3 ms per-frame latency on RTX 4070/4090 class hardware. No production code should be written until this architecture selection is validated with a proof-of-concept training run and ONNX Runtime C++ inference benchmark on the target hardware.*
