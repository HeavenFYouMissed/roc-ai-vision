# Re-Identification Embedding Backbone Research

> **Project:** ROC AI Vision — Humanoid Entity Tracking  
> **Date:** 2026-05-17  
> **Scope:** Appearance embedding CNN selection for DeepSORT-style association layer  
> **Runtime Target:** ONNX Runtime C++ (Windows x64, CUDA EP / DirectML)  
> **Embedding Contract:** 128-D L2-normalized vectors, cosine distance metric  
> **Entity Hierarchy:** person → head → upper_torso → accessories

---

## Table of Contents

1. [FastReID — Backbone Survey](#1-fastreid--backbone-survey)
2. [OSNet Family — Lightweight Re-ID Networks](#2-osnet-family--lightweight-re-id-networks)
3. [Bag of Tricks (BoT) — Training Improvements](#3-bag-of-tricks-bot--training-improvements)
4. [CLIP-ReID & Foundation Model Approaches](#4-clip-reid--foundation-model-approaches)
5. [Embedding Dimension Analysis](#5-embedding-dimension-analysis)
6. [Body Part Re-ID — Hierarchical Embeddings](#6-body-part-re-id--hierarchical-embeddings)
7. [Training Data & Fine-Tuning Strategies](#7-training-data--fine-tuning-strategies)
8. [ONNX Export & Quantization](#8-onnx-export--quantization)
9. [Pretrained Models & Repository Links](#9-pretrained-models--repository-links)
10. [Recommendation for ROC AI Vision](#10-recommendation-for-roc-ai-vision)

---

## 1. FastReID — Backbone Survey

**Repository:** [JDAI-CV/fast-reid](https://github.com/JDAI-CV/fast-reid) (MIT License)

FastReID is a SOTA re-identification research platform developed by JD AI Research. It provides a unified codebase for training, evaluating, and deploying re-ID models across multiple backbones and methods.

### 1.1 Supported Backbones

| Backbone | Type | Params (M) | Notes |
|---|---|---|---|
| ResNet-50 | CNN | 23.5 | Vanilla baseline |
| ResNet-50-IBN-a | CNN | ~23.5 | IN+BN fusion for domain invariance |
| ResNet-101-IBN-a | CNN | ~42.5 | Deeper IBN variant |
| ResNeXt-101 | CNN | ~44.2 | Grouped convolution variant |
| RegNet | CNN | varies | NAS-derived efficient backbone |
| OSNet-x1.0 | CNN | 2.2 | Lightweight omni-scale |
| SE-ResNet-50 | CNN | ~26.0 | Squeeze-and-Excitation blocks |
| Vision Transformer (ViT) | Transformer | ~86 (ViT-B) | Patch-based attention backbone |

### 1.2 FastReID Methods & Market-1501 Results

| Method | Backbone | Rank-1 (%) | mAP (%) |
|---|---|---|---|
| BoT (Bag of Tricks) | R50 | 94.4 | 86.1 |
| BoT | R50-IBN-a | 94.9 | 87.6 |
| BoT | S50 (SE-ResNet) | 95.1 | 88.5 |
| BoT | R101-IBN-a | 95.4 | 88.9 |
| AGW | R50 | 95.3 | 88.2 |
| AGW | R50-IBN-a | 95.1 | 88.7 |
| AGW | R101-IBN-a | 95.5 | 89.5 |
| SBS (Stronger Baseline) | R50-IBN-a | 95.7 | 89.3 |
| SBS | R101-IBN-a | **96.3** | **90.3** |
| MGN | R50-IBN-a | 95.7 | 86.9 |

### 1.3 ONNX Export Support

FastReID includes native ONNX export via `tools/deploy/onnx_export.py`. The pipeline supports:
- Direct PyTorch → ONNX conversion (opset 11+)
- TensorRT engine building from the exported ONNX
- Caffe2 backend deployment

**Critical note for ROC:** FastReID's export path is battle-tested in production MOT systems. BoT-SORT and StrongSORT both use FastReID as their re-ID backbone provider. The exported ONNX models are directly loadable by ONNX Runtime C++ with CUDA EP.

### 1.4 MGN (Multiple Granularity Network)

**Paper:** Wang et al., "Learning Discriminative Features with Multiple Granularities for Person Re-Identification" (ACM MM 2018)

Architecture:
- Three branches splitting from ResNet-50 after `res_conv4_1`
- **Global Branch:** Full image → 2048-D global feature
- **Part-2 Branch:** Image split into 2 horizontal stripes → 2 × 256-D local features
- **Part-3 Branch:** Image split into 3 horizontal stripes → 3 × 256-D local features
- All features concatenated at test time → 2048 + 512 + 768 = 3328-D (before projection)

Performance: 95.7% Rank-1, 86.9% mAP on Market-1501 (without re-ranking).

**Verdict for ROC:** MGN's multi-branch design yields excellent accuracy but the concatenated feature dimension (3328-D pre-projection) and 3× backbone computation make it too expensive for per-detection-per-frame inference. The multi-branch forward pass approximately triples ResNet-50's latency. **Not recommended** for real-time tracking unless the final projection layer compresses to 128-D and you accept the 3× cost.

---

## 2. OSNet Family — Lightweight Re-ID Networks

**Paper (OSNet):** Zhou et al., "Omni-Scale Feature Learning for Person Re-Identification" (ICCV 2019)  
**Paper (OSNet-AIN):** Zhou et al., "Learning Generalisable Omni-Scale Representations for Person Re-Identification" (TPAMI 2021)  
**Repository:** [KaiyangZhou/deep-person-reid](https://github.com/KaiyangZhou/deep-person-reid)

### 2.1 Architecture

OSNet learns omni-scale features through multiple parallel convolutional streams at different depths/receptive fields. Key components:
- **LightConv3x3:** Factorized 3×3 convolution = 1×1 linear conv + depthwise 3×3 conv
- **Multi-stream residual blocks:** Parallel streams at scales T=1..T, each a different number of stacked LightConv3x3
- **Unified Aggregation Gate (AG):** Input-dependent channel-wise attention that dynamically fuses multi-scale features
- **Output:** 512-D global average pooled feature (projected to desired dimension)

OSNet-AIN adds **Adaptive Instance Normalization** layers whose placement is determined by differentiable architecture search, yielding superior cross-domain generalization.

### 2.2 Model Variants — Full Specification Table

| Model | Params (M) | GFLOPs | Input | Market1501 R1/mAP | DukeMTMC R1/mAP | MSMT17 R1/mAP |
|---|---|---|---|---|---|---|
| osnet_x1_0 | **2.2** | **0.98** | 256×128 | 94.2 / 82.6 | 87.0 / 70.2 | 74.9 / 43.8 |
| osnet_x0_75 | 1.3 | 0.57 | 256×128 | 93.7 / 81.2 | 85.8 / 69.8 | 72.8 / 41.4 |
| osnet_x0_5 | 0.6 | 0.27 | 256×128 | 92.5 / 79.8 | 85.1 / 67.4 | 69.7 / 37.5 |
| osnet_x0_25 | **0.2** | **0.08** | 256×128 | 91.2 / 75.0 | 82.0 / 61.4 | 61.4 / 29.5 |
| osnet_ibn_x1_0 | 2.2 | 0.98 | 256×128 | — | — | — |
| osnet_ain_x1_0 | 2.2 | 0.98 | 256×128 | — | — | — |

### 2.3 Cross-Domain Generalization (OSNet-AIN)

When trained on MSMT17 (combineall=True) and tested on unseen domains:

| Model | MSMT17→Market1501 (R1/mAP) | MSMT17→DukeMTMC (R1/mAP) |
|---|---|---|
| resnet50 | 46.3 / 22.8 | 52.3 / 32.1 |
| osnet_x1_0 | 66.6 / 37.5 | 66.0 / 45.3 |
| osnet_ain_x1_0 | **70.1 / 43.3** | **71.1 / 52.7** |

OSNet-AIN's cross-domain performance is **dramatically** superior to ResNet-50 — a critical advantage for deployment in novel surveillance environments without domain-specific fine-tuning.

### 2.4 Inference Latency

**Important caveat:** OSNet's multi-stream architecture creates a latency bottleneck where shallow streams must wait for deep streams before fusion. Reported benchmarks:

| Config | Hardware | Latency |
|---|---|---|
| OSNet-x1.0 (PyTorch, bs=1) | NVIDIA M60 | ~30 ms/image |
| ResNet-18 (PyTorch, bs=1) | NVIDIA M60 | ~4 ms/image |
| OSNet-AIN (TensorRT FP16, bs=32) | Titan XP | ~28 ms/batch (~0.875 ms/image) |
| ResNet-18 (TensorRT FP16, bs=32) | Titan XP | ~6 ms/batch (~0.19 ms/image) |

**Analysis for ROC:** The ~30 ms PyTorch single-image latency is misleading — that includes Python overhead and no CUDA graph optimization. With ONNX Runtime + CUDA EP and proper batching, expect **2–5 ms per image** on an RTX 3060+. With TensorRT EP, the batched amortized cost drops below 1 ms/image.

### 2.5 ONNX Export

Torchreid added ONNX export support (August 2022). Export path:
```python
import torchreid
model = torchreid.models.build_model(name='osnet_x1_0', num_classes=1000)
torchreid.utils.load_pretrained_weights(model, 'osnet_x1_0_msmt17.pth')
dummy = torch.randn(1, 3, 256, 128)
torch.onnx.export(model, dummy, "osnet_x1_0.onnx", opset_version=12,
                  input_names=["input"], output_names=["embedding"])
```

The exported graph is clean — no custom ops, no dynamic shapes needed. Compatible with ONNX Runtime 1.15+ C++ API.

---

## 3. Bag of Tricks (BoT) — Training Improvements

**Paper:** Luo et al., "Bag of Tricks and A Strong Baseline for Deep Person Re-identification" (CVPR-W 2019)

### 3.1 The Six Canonical Tricks

| Trick | Impact | Mechanism |
|---|---|---|
| **Warm-up LR** | +1.5% mAP | Linear ramp from 3.5×10⁻⁵ to 3.5×10⁻⁴ over 10 epochs |
| **Random Erasing Augmentation (REA)** | +2.1% mAP | Randomly erase rectangular patches (p=0.5) during training; simulates occlusion |
| **Label Smoothing** | +0.8% mAP | ε=0.1 smoothing on classification targets; prevents over-confident embeddings |
| **Last Stride = 1** | +2.4% mAP | Change ResNet layer4 stride from 2→1; doubles spatial resolution of final feature map |
| **BNNeck** | +3.2% mAP | Batch-norm layer between backbone and loss heads; decouples ID loss and triplet loss feature spaces |
| **Center Loss** | +1.0% mAP | Penalizes intra-class distance; tightens embedding clusters |

**Combined effect:** Applying all tricks to ResNet-50 baseline achieves **94.5% Rank-1, 85.9% mAP** on Market-1501 — competitive with far more complex architectures.

### 3.2 BNNeck Architecture Detail

```
backbone → GAP → 2048-D → [BN Layer] → FC(2048→N_classes)  ← ID loss
                              ↓
                         2048-D normalized  ← triplet loss
```

The BN layer normalizes features onto a hypersphere before the classification head. During inference, the **pre-BN feature** (or a linear projection of it) serves as the embedding. This separation is why BoT-trained models can optimize both classification and metric learning simultaneously without interference.

### 3.3 Modern Extensions (2024–2026)

**DynaMix (2025):** Introduces dynamic relabeling and mixed data sampling for training on million-scale datasets with hundreds of thousands of identities. Three components:
1. **Relabeling Module** — dynamically refines pseudo-labels for single-camera identities
2. **Efficient Centroids Module** — maintains robust identity representations at massive scale
3. **Data Sampling Module** — balanced mini-batch composition controlling learning complexity

**FastReID SBS (Stronger Baseline):** Extends BoT with:
- Auto-augment policies
- Cosine annealing LR schedule
- GeM (Generalized Mean) pooling instead of GAP
- Non-local attention blocks in backbone
- Achieves **95.7% Rank-1, 89.3% mAP** on Market-1501 with R50-IBN-a

### 3.4 Relevance to ROC

All BoT tricks are **architecture-agnostic** — they apply equally to ResNet-50-IBN and OSNet backbones. For ROC's training pipeline, the minimum viable trick set should include: warm-up LR, REA, BNNeck, and center loss. Label smoothing and last-stride modification are backbone-specific (last-stride applies only to ResNet-style architectures).

---

## 4. CLIP-ReID & Foundation Model Approaches

### 4.1 CLIP-ReID

**Paper:** Li et al., "CLIP-ReID: Exploiting Vision-Language Model for Image Re-identification without Concrete Text Labels" (AAAI 2023)  
**Repository:** [Syliz517/CLIP-ReID](https://github.com/Syliz517/CLIP-ReID)

Two-stage training strategy:
1. **Stage 1:** Optimize learnable text tokens while freezing CLIP's image and text encoders
2. **Stage 2:** Fine-tune the image encoder with frozen text tokens as soft constraints

CLIP-ReID achieves SOTA on standard benchmarks by leveraging CLIP's pre-trained visual-semantic alignment, but its advantage lies primarily in **cross-domain generalization** and **zero-shot** scenarios.

### 4.2 Recent CLIP-Based Methods (2025–2026)

| Method | Year | Key Innovation | Occluded-Duke R1 |
|---|---|---|---|
| CLIP-ReID | 2023 | Learnable text tokens | ~72% |
| SAGA-ReID | 2026 | Patch token alignment | +10.6% over baseline |
| CLIMB-ReID | 2025 (AAAI) | CLIP + Mamba hybrid | improved video ReID |
| CG-CLIP | 2026 | MLLM-generated captions + cross-attention | high-difficulty video |

### 4.3 Latency Analysis — Disqualifying for Real-Time

| Model | Params (M) | Latency (bs=1, T4 GPU) |
|---|---|---|
| ResNet-50 (re-ID) | 23.5 | ~4–5 ms |
| OSNet-x1.0 (re-ID) | 2.2 | ~3–5 ms (ONNX RT) |
| CLIP ViT-B/32 | ~86 | ~29 ms |
| CLIP ViT-B/16 | ~86 | ~40–50 ms (estimated) |
| CLIP ViT-L/14 | ~304 | ~80–120 ms (estimated) |

**Verdict for ROC:** CLIP-based re-ID is **categorically disqualified** for per-detection-per-frame inference. At 10 detections per frame, CLIP ViT-B/32 alone would consume 290 ms — exceeding a 30 fps frame budget of 33 ms by an order of magnitude. Even with TensorRT FP16 optimization and batching, ViT-B/16 cannot compete with CNN backbones on a per-crop basis.

CLIP-ReID is viable only as an **offline re-ranking** or **gallery enrichment** tool — not in the hot tracking loop.

### 4.4 History-Aware Feature Transformation (HAT)

**Paper:** Ciampi et al., "History-Aware Transformation of ReID Features for Multiple Object Tracking" (2025)

A training-free, zero-shot approach that transforms raw re-ID features using Fisher Linear Discriminant (FLD) conditioned on historical trajectory features. Key properties:
- No additional neural network inference
- Applied as a post-processing transform on existing embeddings
- Shows strong zero-shot transfer across datasets
- Maximizes inter-trajectory discrimination within a video sequence

**Relevance to ROC:** HAT is a lightweight post-processing layer that could enhance any chosen backbone's discriminative power without adding inference cost. Worth implementing as an optional enhancement in the association layer.

---

## 5. Embedding Dimension Analysis

### 5.1 Dimension Trade-offs

| Dimension | Storage/Track (100 gallery) | Cosine Dist Compute | Re-ID Accuracy | Recommendation |
|---|---|---|---|---|
| **64-D** | 6.4 KB | Fastest | Degraded | Too low for person re-ID |
| **128-D** | 12.8 KB | Fast | Optimal (validated) | **DeepSORT standard; FairMOT validated** |
| **256-D** | 25.6 KB | Moderate | Marginal gain | Diminishing returns vs 128-D |
| **512-D** | 51.2 KB | Slower | Marginal gain | Common in research; overkill for tracking |
| **2048-D** | 204.8 KB | Expensive | Maximal | Raw backbone output; never used in tracking |

### 5.2 Empirical Evidence for 128-D

**FairMOT finding:** The FairMOT developers explicitly tested 128-D vs 512-D on their validation set and found **128-D outperformed 512-D**. Their hypothesis: lower-dimensional spaces regularize the embedding, preventing overfitting to fine-grained identity details that don't generalize across viewpoint/pose changes in tracking scenarios.

**DeepSORT standard:** The original Deep SORT paper (Wojke et al., 2017) trained and validated on 128-D embeddings using cosine metric learning. The 128-D choice has been validated across thousands of deployments.

**Mathematical justification:** For cosine similarity in ℝ^d with L2-normalized vectors:
- The concentration of measure phenomenon means that in high dimensions, random unit vectors become nearly orthogonal (expected cosine ≈ 0)
- 128-D provides sufficient angular separation for ~1000 concurrent identities while remaining computationally tractable
- The cosine GEMM `C = A × Bᵀ` (where A is [N_dets × 128], B is [N_tracks × 128]) stays in L1/L2 cache for reasonable track counts

### 5.3 Gallery Management Strategies

**DeepSORT nn_budget:** The original DeepSORT maintains a bounded list of appearance descriptors per track, with `nn_budget` controlling the maximum gallery size. After each update, confirmed tracks add their current embedding to the gallery and excess features are dropped (FIFO).

**Ring buffer approach (recommended for ROC):** Our `track.hpp` already implements `EmbeddingGallery` as a fixed-capacity ring. The optimal parameters:

| Parameter | Recommended Value | Rationale |
|---|---|---|
| Ring capacity | 30–50 | Covers ~1–2 seconds at 30 fps; enough to average over pose variation |
| Distance metric | Min-cosine over gallery | One strong match suffices for association |
| EMA smoothing | Optional, α=0.9 | Running average embedding for rapid comparison |
| Stale threshold | 90 frames (3 sec) | Gallery entries older than this get discarded on association |

**Hybrid strategy:** Maintain both a **ring buffer of recent raw embeddings** (for min-distance matching) and a **running EMA embedding** (for fast approximate comparison during cascade first-pass). The cascade checks EMA first; if ambiguous, falls back to full gallery min-distance.

### 5.4 Wire Format Impact

With 128-D × FP32: 512 bytes per embedding per track per frame. For the wire format (86 bytes/track currently), embedding data is NOT transmitted per-frame — it stays local in the tracking engine. Only the track ID and state vector cross the wire. This is correct and should not change.

---

## 6. Body Part Re-ID — Hierarchical Embeddings

### 6.1 ROC Entity Hierarchy Mapping

ROC's entity hierarchy (person → head → upper_torso → accessories) maps naturally to part-based re-ID research. The question: can separate part embeddings improve robustness?

### 6.2 Key Papers on Part-Based Re-ID

**PAFormer (August 2024):** Uses pose estimation with Vision Transformer to perform precise part-to-part comparisons. Introduces learnable "pose tokens" correlating each body part with image regions. Includes a visibility predictor for handling occlusions. No additional localization module needed during inference.

**Multi-Stream Head+Body Re-ID (March 2024):** Three-stream architecture:
1. **Global stream** — full-body identity encoding
2. **Head stream** — cropped head region identity features
3. **Local body part stream** — per-part identity features with part-to-part matching

This maps directly to ROC's hierarchy.

**DROP (January 2024):** Decouples re-ID features from human parsing features, arguing they need different granularity. Re-ID focuses on instance-level part differences; parsing needs semantic spatial context. Achieves 76.8% Rank-1 on Occluded-Duke.

**PAB-ReID (April 2024):** Part-attention mechanism using human parsing labels to guide attention map generation. Part triplet loss supervises learning of separate local features.

### 6.3 Part-Based Embedding Architecture for ROC

**Proposed approach for ROC's hierarchy:**

```
Detection crop (256×128)
      │
      ├─ Global backbone ──→ 128-D person embedding
      │
      ├─ Head crop (64×64) ──→ lightweight head net ──→ 64-D head embedding
      │
      └─ Torso crop (128×96) ──→ lightweight torso net ──→ 64-D torso embedding
```

**Composite distance metric:**
```
d_composite = w_global * d_cosine(e_person, g_person)
            + w_head   * d_cosine(e_head, g_head)     [if head visible]
            + w_torso  * d_cosine(e_torso, g_torso)    [if torso visible]
```

Where weights are visibility-gated:
- `w_global = 0.6`, `w_head = 0.2`, `w_torso = 0.2` (when all visible)
- If head occluded: `w_global = 0.75`, `w_torso = 0.25`
- If torso occluded: `w_global = 0.75`, `w_head = 0.25`

### 6.4 Practical Considerations

| Concern | Analysis |
|---|---|
| Inference cost | 3 forward passes vs 1; mitigated by smaller crops for parts |
| Crop extraction | Requires head/torso localization; use the detection hierarchy that already exists |
| Gallery complexity | 3 ring buffers per track instead of 1 |
| Training data | No standard datasets with part-level ID annotations; requires pseudo-labeling |
| Recommendation | **Phase 2 enhancement.** Start with global-only 128-D person embedding. Add part embeddings after baseline tracking is validated. |

---

## 7. Training Data & Fine-Tuning Strategies

### 7.1 Standard Benchmarks

| Dataset | Identities | Images | Cameras | Resolution | Status |
|---|---|---|---|---|---|
| **Market-1501** | 1,501 | 32,668 | 6 | 128×64 avg | Available; widely used |
| **DukeMTMC-reID** | 1,812 | 36,411 | 8 | ~192×80 | ⚠️ Taken down (privacy); mirrors exist |
| **MSMT17** | 4,101 | 126,441 | 15 | varies | Largest standard benchmark; indoor+outdoor |
| **CUHK03** | 1,467 | 14,097 | 2 | varies | Small; often used with detected boxes |
| **LUPerson** | 200,000+ | 4M+ | wild | varies | Largest unlabeled person dataset |

**MSMT17 is the recommended primary training set** for ROC due to:
- Largest annotated identity count (4,101)
- Most cameras (15), including indoor and outdoor
- Complex lighting variations across long time spans
- Most representative of real surveillance conditions

### 7.2 Transfer Learning Strategy for ROC

**Recommended pipeline:**

1. **Stage 1 — Pre-train on MSMT17:** Train chosen backbone with BoT tricks (warm-up LR, REA, BNNeck, center loss, label smoothing). Use combineall=True for maximum data.

2. **Stage 2 — Domain adaptation to ROC's target environment:** If labeled data from the deployment environment is available, fine-tune with:
   - Reduced LR (1/10 of original)
   - Freeze early layers (first 2 residual stages)
   - Heavy augmentation (color jitter, REA p=0.7, horizontal flip)

3. **Stage 3 — Unsupervised domain adaptation (if no labels):** Apply Mutual Mean-Teaching (MMT) or GLT:
   - Generate pseudo-labels via clustering (DBSCAN)
   - Iterative refinement with dual-model mutual training
   - MMT achieves 82.0% Rank-1 on Market→Duke transfer

**Multi-source domain generalization (recommended):**  
Train on MSMT17 + Market-1501 + DukeMTMC + CUHK03 simultaneously using OSNet-AIN's multi-source protocol. This yields the best cross-domain generalization:
- MS+D+C → M: 73.3% Rank-1
- MS+M+C → D: 65.6% Rank-1
- Best generalization to completely unseen domains

### 7.3 Data Augmentation Summary

| Augmentation | Impact | When to Use |
|---|---|---|
| Random Horizontal Flip | +0.5% | Always |
| Random Erasing (p=0.5) | +2.1% | Always (simulates occlusion) |
| Color Jitter | +1.0% | Cross-domain training |
| Random Crop + Pad | +0.8% | Same-domain training |
| Auto-Augment | +1.5% | SBS stronger baseline |
| CutMix / MixUp | +0.5% | Large-scale training |

---

## 8. ONNX Export & Quantization

### 8.1 ONNX Export Pipeline

**FastReID export:**
```bash
python tools/deploy/onnx_export.py \
  --config-file configs/Market1501/sbs_R50-ibn.yml \
  --name "sbs_R50_ibn_market" \
  --output outputs/onnx/ \
  --opts MODEL.WEIGHTS path/to/model.pth
```

**Torchreid (OSNet) export:**
```python
import torch
import torchreid

model = torchreid.models.build_model(
    name='osnet_ain_x1_0', num_classes=751,
    pretrained=True
)
model.eval()

dummy = torch.randn(1, 3, 256, 128)
torch.onnx.export(
    model, dummy, "osnet_ain_x1_0.onnx",
    opset_version=12,
    input_names=["input"],
    output_names=["embedding"],
    dynamic_axes={"input": {0: "batch"}, "embedding": {0: "batch"}}
)
```

### 8.2 Precision Levels — Accuracy vs Latency

| Precision | Storage | Latency Multiplier | Accuracy Impact | Recommendation |
|---|---|---|---|---|
| **FP32** | 100% | 1.0× | Baseline | Development and validation |
| **FP16** | 50% | 0.5–0.7× | Negligible for CNNs (<0.5% mAP) | **Production default on CUDA EP** |
| **INT8 (PTQ)** | 25% | 0.3–0.5× | **Catastrophic for OSNet** (>95% drop reported) | NOT RECOMMENDED without QAT |
| **INT8 (QAT)** | 25% | 0.3–0.5× | 1–3% mAP loss (architecture dependent) | Only with quantization-aware training |

### 8.3 INT8 Quantization — Critical Warnings

**OSNet-specific failure:** INT8 post-training quantization (PTQ) of OSNet has been documented to cause **catastrophic accuracy degradation (>95% accuracy loss)**. The multi-stream architecture with lightweight depthwise convolutions is extremely sensitive to quantization noise. The aggregation gates (which compute input-dependent channel weights) amplify small numerical errors.

**ResNet-50-IBN is more robust:** Traditional ResNet architectures with batch normalization are more tolerant of INT8 quantization. Expected accuracy retention with proper calibration: 1–3% mAP loss.

**FP16 is the safe choice:** For CUDA EP on Windows:
- ResNet-50 FP16: negligible accuracy loss, ~2× speedup over FP32
- OSNet FP16: <0.5% accuracy loss, meaningful speedup
- ViT FP16: **dangerous** — can cause overflow in LayerNorm/Softmax; needs mixed-precision with FP32 for sensitive ops

### 8.4 ONNX Runtime C++ Configuration for ROC

```cpp
Ort::SessionOptions session_options;
session_options.SetIntraOpNumThreads(1);  // GPU inference; minimize CPU threads
session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

// CUDA EP with FP16
OrtCUDAProviderOptionsV2* cuda_options = nullptr;
Ort::GetApi().CreateCUDAProviderOptions(&cuda_options);
std::vector<const char*> keys   = {"device_id", "gpu_mem_limit", "cudnn_conv_algo_search"};
std::vector<const char*> values = {"0",         "2147483648",    "EXHAUSTIVE"};
Ort::GetApi().UpdateCUDAProviderOptions(cuda_options, keys.data(), values.data(), 3);
session_options.AppendExecutionProvider_CUDA_V2(*cuda_options);

Ort::Session session(env, L"osnet_ain_x1_0.onnx", session_options);
```

### 8.5 Quantization Decision Matrix for ROC

| Scenario | Precision | Backbone | Rationale |
|---|---|---|---|
| Development / accuracy validation | FP32 | Any | Ground truth comparison |
| Production (NVIDIA GPU) | **FP16** | ResNet-50-IBN or OSNet-AIN | Best latency/accuracy tradeoff |
| Production (DirectML / AMD) | FP32 | OSNet-AIN | DirectML FP16 support varies; test first |
| Edge / latency-critical | INT8 QAT | ResNet-50-IBN only | OSNet INT8 is broken; ResNet-50 tolerates INT8 |
| CPU fallback | INT8 QAT | ResNet-50 | Maximize throughput on CPU EP |

---

## 9. Pretrained Models & Repository Links

### 9.1 Direct Download Links — OSNet (Torchreid)

**Same-domain models (Market-1501 trained):**

| Model | Link |
|---|---|
| osnet_x1_0 | [Google Drive](https://drive.google.com/file/d/1vduhq5DpN2q1g4fYEZfPI17MJeh9qyrA/view) |
| osnet_x0_75 | [Google Drive](https://drive.google.com/file/d/1ozRaDSQw_EQ8_93OUmjDbvLXw9TnfPer/view) |
| osnet_x0_5 | [Google Drive](https://drive.google.com/file/d/1PLB9rgqrUM7blWrg4QlprCuPT7ILYGKT/view) |
| osnet_x0_25 | [Google Drive](https://drive.google.com/file/d/1z1UghYvOTtjx7kEoRfmqSMu-z62J6MAj/view) |

**Cross-domain models (MSMT17 trained, combineall=True):**

| Model | Link |
|---|---|
| osnet_x1_0 | [Google Drive](https://drive.google.com/file/d/1IosIFlLiulGIjwW3H8uMRmx3MzPwf86x/view) |
| osnet_ain_x1_0 | [Google Drive](https://drive.google.com/file/d/1SigwBE6mPdqiJMqhuIY4aqC7--5CsMal/view) |
| osnet_ibn_x1_0 | [Google Drive](https://drive.google.com/file/d/1q3Sj2ii34NlfxA4LvmHdWO_75NDRmECJ/view) |

**Multi-source domain generalization models (best cross-domain performance):**

| Model | Training Sets | Link |
|---|---|---|
| osnet_ain_x1_0 | MS+D+C→M | [Google Drive](https://drive.google.com/file/d/1nIrszJVYSHf3Ej8-j6DTFdWz8EnO42PB/view) |
| osnet_ain_x1_0 | MS+M+C→D | [Google Drive](https://drive.google.com/file/d/1YjJ1ZprCmaKG6MH2P9nScB9FL_Utf9t1/view) |
| osnet_ain_x1_0 | D+M+C→MS | [Google Drive](https://drive.google.com/file/d/1KcoUKzLmsUoGHI7B6as_Z2fXL50gzexS/view) |

**HuggingFace repository:**
```bash
pip install huggingface_hub
python -c "from huggingface_hub import snapshot_download; snapshot_download('kaiyangzhou/osnet', cache_dir='./')"
```

### 9.2 FastReID Pretrained Models

**Repository:** [JDAI-CV/fast-reid](https://github.com/JDAI-CV/fast-reid)  
**Model Zoo:** Available in the repo's `MODEL_ZOO.md`

Key models with download links are provided through the FastReID model zoo. Notable configurations:
- `sbs_R50-ibn` — Stronger Baseline with R50-IBN (Market-1501 and MSMT17)
- `sbs_R101-ibn` — Stronger Baseline with R101-IBN (highest accuracy)
- `mgn_R50-ibn` — MGN with R50-IBN backbone

### 9.3 IBN-Net Pretrained Models

**Repository:** [XingangPan/IBN-Net](https://github.com/XingangPan/IBN-Net)

| Model | Download |
|---|---|
| ResNet-50-IBN-a | [resnet50_ibn_a-d9d0bb7b.pth](https://github.com/XingangPan/IBN-Net/releases/download/v1.0/resnet50_ibn_a-d9d0bb7b.pth) |
| ResNet-101-IBN-a | Available in releases |

### 9.4 Other Relevant Repositories

| Repository | Purpose | URL |
|---|---|---|
| BoT-SORT | MOT tracker using FastReID | [NirAharon/BoT-SORT](https://github.com/NirAharon/BoT-SORT) |
| StrongSORT | Enhanced DeepSORT | [dyhBUPT/StrongSORT](https://github.com/dyhBUPT/StrongSORT) |
| Torchreid (deep-person-reid) | Training framework + OSNet | [KaiyangZhou/deep-person-reid](https://github.com/KaiyangZhou/deep-person-reid) |
| Person_reID_baseline_pytorch | Clean PyTorch baseline + TRT | [layumi/Person_reID_baseline_pytorch](https://github.com/layumi/Person_reID_baseline_pytorch) |
| CLIP-ReID | CLIP-based re-ID (research only) | [Syliz517/CLIP-ReID](https://github.com/Syliz517/CLIP-ReID) |
| deep-efficient-person-reid | EfficientNet re-ID | [lannguyen0910/deep-efficient-person-reid](https://github.com/lannguyen0910/deep-efficient-person-reid) |
| NVIDIA TAO ReIdentificationNet | Enterprise re-ID toolkit | [NVIDIA TAO Docs](https://docs.nvidia.com/tao/tao-toolkit/latest/text/cv_finetuning/pytorch/re_identification/re_identification.html) |

---

## 10. Recommendation for ROC AI Vision

### 10.1 Primary Backbone Selection

**Winner: OSNet-AIN x1.0 with BoT training tricks**

| Criterion | OSNet-AIN x1.0 | ResNet-50-IBN-a (SBS) | CLIP ViT-B/16 |
|---|---|---|---|
| Parameters | **2.2M** | 23.5M | 86M |
| GFLOPs | **0.98** | 2.7 | ~17.5 |
| Market-1501 R1 | 94.2% | 95.7% | ~96% |
| Cross-domain generalization | **Best in class** | Good | Excellent |
| ONNX export | Clean | Clean | Complex (custom ops) |
| FP16 safe | Yes | Yes | No (overflow risk) |
| INT8 safe | **No** | Yes | No |
| Estimated ONNX RT latency (bs=1, RTX 3060) | **2–4 ms** | 3–5 ms | 30–50 ms |
| Fits ROC latency budget | **Yes** | Yes | **No** |

### 10.2 Phased Implementation Plan

**Phase 1 (Immediate):**
- Export OSNet-AIN x1.0 (MSMT17 multi-source pretrained) to ONNX
- Integrate into vision pipeline via ONNX Runtime C++ CUDA EP
- FP16 inference, 128-D output with L2 normalization
- Single global person embedding per detection
- Ring buffer gallery (capacity=30) per track

**Phase 2 (After baseline validation):**
- Fine-tune on domain-specific data if available
- Implement HAT (History-Aware Transformation) as post-processing
- Benchmark OSNet-AIN vs ResNet-50-IBN-a SBS in production

**Phase 3 (Enhancement):**
- Add head and torso part embeddings (64-D each)
- Visibility-gated composite distance metric
- Evaluate INT8 QAT for ResNet-50-IBN if latency budget is tight

### 10.3 Fallback Strategy

If OSNet-AIN's multi-stream architecture causes unexpected latency issues on DirectML (non-CUDA path), fall back to **ResNet-50-IBN-a (SBS)** from FastReID. It is 10× larger but has more predictable inference characteristics and tolerates INT8 quantization. The 3–5 ms per crop is still within budget for ≤15 detections per frame at 30 fps.

### 10.4 Architecture Decision Record

| Decision | Choice | Rationale |
|---|---|---|
| Backbone | OSNet-AIN x1.0 | 10× fewer params than ResNet-50, best cross-domain generalization, 2.2M params fit in cache |
| Embedding dim | 128-D | DeepSORT standard, validated by FairMOT, optimal for cosine GEMM in L1 cache |
| Training data | MSMT17 multi-source | Largest diversity; 4,101 IDs, 15 cameras |
| Training tricks | BoT (warm-up, REA, BNNeck, center loss) | +8% mAP over vanilla; architecture-agnostic |
| Precision | FP16 on CUDA EP | ~2× speedup, negligible accuracy loss |
| Gallery | Ring buffer (cap=30) + EMA embedding | Bounded memory, fast approximate + exact matching |
| Part embeddings | Deferred to Phase 3 | Global-only sufficient for baseline; parts add complexity |
| CLIP-ReID | Rejected for hot loop | 10× latency overhead; viable only for offline re-ranking |

---

## References

1. Luo et al., "Bag of Tricks and A Strong Baseline for Deep Person Re-identification," CVPR-W 2019
2. Zhou et al., "Omni-Scale Feature Learning for Person Re-Identification," ICCV 2019
3. Zhou et al., "Learning Generalisable Omni-Scale Representations for Person Re-Identification," TPAMI 2021
4. Wang et al., "Learning Discriminative Features with Multiple Granularities for Person Re-Identification," ACM MM 2018
5. Li et al., "CLIP-ReID: Exploiting Vision-Language Model for Image Re-identification without Concrete Text Labels," AAAI 2023
6. Pan et al., "Two at Once: Enhancing Learning and Generalization Capacities via IBN-Net," ECCV 2018
7. Wojke & Bewley, "Deep Cosine Metric Learning for Person Re-Identification," 2018
8. Aharon et al., "BoT-SORT: Robust Associations Multi-Pedestrian Tracking," 2022
9. Wei et al., "Person Transfer GAN to Bridge Domain Gap for Person Re-Identification," CVPR 2018
10. Ciampi et al., "History-Aware Transformation of ReID Features for Multiple Object Tracking," 2025
11. Zhang et al., "FairMOT: On the Fairness of Detection and Re-Identification in Multiple Object Tracking," IJCV 2021
