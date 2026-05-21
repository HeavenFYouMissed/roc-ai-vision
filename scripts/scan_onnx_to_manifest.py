#!/usr/bin/env python3
"""
scan_onnx_to_manifest.py
========================

Bulk ONNX -> YAML manifest auto-generator for the ROC AI Vision pipeline.

This script walks a directory (default: `<repo>/models/`), reads every `.onnx`
file with the `onnx` package, introspects its input / output tensor shapes and
data types, classifies it into one of the supported postprocessor heads
(`yolo26_detect`, `yolov8_detect`, `osnet_reid`), and emits a co-located
`.yaml` manifest that conforms exactly to the §3.3 schema enforced by
`core/vision_pipeline/src/manifest_loader.cpp`.

Inference rules (deterministic, no heuristics that fail silently)
-----------------------------------------------------------------
1. **Input shape** is read from `model.graph.input[0]` and forced to
   `[1, C, H, W]` after substituting any dynamic dim with the sensible
   default (`B=1`, `C=3`, `H=W=640` for detection / `H=256 W=128` for re-id).

2. **Head classification** is decided from the OUTPUT tensor rank+shape:
     * rank-3 `[B, K, A]` with K > 5  AND  A > 100   -> `yolo26_detect`
       (Ultralytics' YOLO26/YOLOv8 end-to-end shape, K = 4 + num_classes,
       A = total anchors after STAL).
     * rank-3 `[B, A, K]` (transposed)               -> `yolov8_detect`
       (some exporters use the transposed layout).
     * rank-2 `[B, D]` with D in {128, 256, 512, 1024}, OR rank-4
       `[B, D, 1, 1]` collapsed     -> `osnet_reid`.
     * any other rank/shape -> `unknown` (manifest emitted with a clear
       TODO header instead of guessing; the operator must hand-edit).

3. **num_classes** is computed from `K - 4` for detect heads. If the
   inferred count > 1000 (likely a Concat artefact) we emit a stub
   single-class fallback and tag the manifest with a `# REVIEW:` line.

4. **Class table** ships pre-populated for the well-known counts:
     *  80 -> COCO labels (mapped to ROC labels where applicable)
     *   1 -> single `person` class (-> roc_label: Person)
     *   6 -> ROC humanoid hierarchy
       (person, head, upper_torso, hat, vest, backpack)
     * other -> generic `class_<id>` rows mapped to `Unknown`
       (operator must hand-edit to assign roc_labels).

The script never overwrites an existing `.yaml` unless `--force` is passed.
This is intentional: re-running with new ONNX drops is non-destructive.

Usage
-----
    python scripts/scan_onnx_to_manifest.py                # scan ./models/
    python scripts/scan_onnx_to_manifest.py path/to/dir
    python scripts/scan_onnx_to_manifest.py --force        # overwrite stubs
    python scripts/scan_onnx_to_manifest.py --dry-run      # no writes
    python scripts/scan_onnx_to_manifest.py --verbose

Dependencies
------------
    pip install onnx pyyaml

Exit codes
----------
    0  success (>= 0 manifests written, 0 errors)
    1  fatal (missing onnx package, unreadable directory, etc)
    2  partial success (some files failed introspection — see stderr)
"""

from __future__ import annotations

import argparse
import os
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

# -------------------------------------------------------------------------
# Dependency guards
# -------------------------------------------------------------------------
try:
    import onnx
    from onnx import TensorProto
except ImportError:  # pragma: no cover
    print(
        "ERROR: 'onnx' is required. Install with:\n"
        "    pip install onnx pyyaml",
        file=sys.stderr,
    )
    sys.exit(1)

try:
    import yaml  # noqa: F401 (used implicitly via the hand-rolled writer below)
except ImportError:  # pragma: no cover
    print(
        "ERROR: 'pyyaml' is required. Install with:\n"
        "    pip install onnx pyyaml",
        file=sys.stderr,
    )
    sys.exit(1)


# =============================================================================
# Constants — single source of truth for the canonical YAML schema strings.
# =============================================================================
SCHEMA_VERSION = 1
SUPPORTED_HEADS = ("yolo26_detect", "yolov8_detect", "osnet_reid")

ONNX_DTYPE_TO_MANIFEST = {
    TensorProto.FLOAT: "float32",
    TensorProto.FLOAT16: "float16",
    TensorProto.UINT8: "uint8",
}

# ROC kinematics Label enum (mirror of
# core/kinematics_engine/include/roc/kinematics/types.hpp).
ROC_LABELS = {
    "Unknown",
    "Person",
    "Head",
    "UpperTorso",
    "AccessoryHat",
    "AccessoryBackpack",
}

# COCO 80-class mapping (id -> name) with ROC label projection.
COCO_CLASSES: list[tuple[int, str, str]] = [
    (0,  "person",        "Person"),
    (1,  "bicycle",       "Unknown"),
    (2,  "car",           "Unknown"),
    (3,  "motorcycle",    "Unknown"),
    (4,  "airplane",      "Unknown"),
    (5,  "bus",           "Unknown"),
    (6,  "train",         "Unknown"),
    (7,  "truck",         "Unknown"),
    (8,  "boat",          "Unknown"),
    (9,  "traffic_light", "Unknown"),
    (10, "fire_hydrant",  "Unknown"),
    (11, "stop_sign",     "Unknown"),
    (12, "parking_meter", "Unknown"),
    (13, "bench",         "Unknown"),
    (14, "bird",          "Unknown"),
    (15, "cat",           "Unknown"),
    (16, "dog",           "Unknown"),
    (17, "horse",         "Unknown"),
    (18, "sheep",         "Unknown"),
    (19, "cow",           "Unknown"),
    (20, "elephant",      "Unknown"),
    (21, "bear",          "Unknown"),
    (22, "zebra",         "Unknown"),
    (23, "giraffe",       "Unknown"),
    (24, "backpack",      "AccessoryBackpack"),
    (25, "umbrella",      "Unknown"),
    (26, "handbag",       "AccessoryBackpack"),
    (27, "tie",           "Unknown"),
    (28, "suitcase",      "Unknown"),
    (29, "frisbee",       "Unknown"),
    (30, "skis",          "Unknown"),
    (31, "snowboard",     "Unknown"),
    (32, "sports_ball",   "Unknown"),
    (33, "kite",          "Unknown"),
    (34, "baseball_bat",  "Unknown"),
    (35, "baseball_glove","Unknown"),
    (36, "skateboard",    "Unknown"),
    (37, "surfboard",     "Unknown"),
    (38, "tennis_racket", "Unknown"),
    (39, "bottle",        "Unknown"),
    (40, "wine_glass",    "Unknown"),
    (41, "cup",           "Unknown"),
    (42, "fork",          "Unknown"),
    (43, "knife",         "Unknown"),
    (44, "spoon",         "Unknown"),
    (45, "bowl",          "Unknown"),
    (46, "banana",        "Unknown"),
    (47, "apple",         "Unknown"),
    (48, "sandwich",      "Unknown"),
    (49, "orange",        "Unknown"),
    (50, "broccoli",      "Unknown"),
    (51, "carrot",        "Unknown"),
    (52, "hot_dog",       "Unknown"),
    (53, "pizza",         "Unknown"),
    (54, "donut",         "Unknown"),
    (55, "cake",          "Unknown"),
    (56, "chair",         "Unknown"),
    (57, "couch",         "Unknown"),
    (58, "potted_plant",  "Unknown"),
    (59, "bed",           "Unknown"),
    (60, "dining_table",  "Unknown"),
    (61, "toilet",        "Unknown"),
    (62, "tv",            "Unknown"),
    (63, "laptop",        "Unknown"),
    (64, "mouse",         "Unknown"),
    (65, "remote",        "Unknown"),
    (66, "keyboard",      "Unknown"),
    (67, "cell_phone",    "Unknown"),
    (68, "microwave",     "Unknown"),
    (69, "oven",          "Unknown"),
    (70, "toaster",       "Unknown"),
    (71, "sink",          "Unknown"),
    (72, "refrigerator",  "Unknown"),
    (73, "book",          "Unknown"),
    (74, "clock",         "Unknown"),
    (75, "vase",          "Unknown"),
    (76, "scissors",      "Unknown"),
    (77, "teddy_bear",    "Unknown"),
    (78, "hair_drier",    "Unknown"),
    (79, "toothbrush",    "Unknown"),
]

ROC_HUMANOID_CLASSES: list[tuple[int, str, str]] = [
    (0, "person",        "Person"),
    (1, "head",          "Head"),
    (2, "upper_torso",   "UpperTorso"),
    (3, "hat",           "AccessoryHat"),
    (4, "vest",          "AccessoryBackpack"),
    (5, "backpack",      "AccessoryBackpack"),
]


# =============================================================================
# Data types
# =============================================================================
@dataclass
class TensorInfo:
    name: str
    shape: list[int]        # dynamic dims replaced with sentinel (-1)
    dtype: int              # ONNX TensorProto.DataType

    @property
    def manifest_dtype(self) -> str:
        return ONNX_DTYPE_TO_MANIFEST.get(self.dtype, "float32")


@dataclass
class IntrospectionResult:
    onnx_path: Path
    yaml_path: Path
    input: Optional[TensorInfo]
    outputs: list[TensorInfo]
    head: str               # one of SUPPORTED_HEADS or "unknown"
    num_classes: int        # 0 for re-id heads
    notes: list[str] = field(default_factory=list)


# =============================================================================
# ONNX introspection
# =============================================================================
def _shape_from_value_info(vi) -> list[int]:
    """Return a Python list of dim sizes; dynamic dims become -1."""
    out: list[int] = []
    for d in vi.type.tensor_type.shape.dim:
        if d.HasField("dim_value") and d.dim_value > 0:
            out.append(int(d.dim_value))
        else:
            # Dynamic axis (string param OR zero) — sentinel.
            out.append(-1)
    return out


def _read_tensor_info(vi) -> TensorInfo:
    return TensorInfo(
        name=vi.name,
        shape=_shape_from_value_info(vi),
        dtype=vi.type.tensor_type.elem_type or TensorProto.FLOAT,
    )


def _resolve_input_shape(input_shape: list[int], head: str) -> list[int]:
    """Replace -1 dims with sensible defaults based on the head guess."""
    resolved = list(input_shape)
    # Always force batch to 1; ROC pipeline is single-image-per-frame.
    if len(resolved) >= 1:
        resolved[0] = 1
    # Channels default to 3 (RGB).
    if len(resolved) >= 2 and resolved[1] in (-1, 0):
        resolved[1] = 3
    if head == "osnet_reid":
        # Re-id input default: 256x128 (height x width) per OSNet paper.
        if len(resolved) >= 3 and resolved[2] in (-1, 0):
            resolved[2] = 256
        if len(resolved) >= 4 and resolved[3] in (-1, 0):
            resolved[3] = 128
    else:
        # Detection default: 640x640.
        if len(resolved) >= 3 and resolved[2] in (-1, 0):
            resolved[2] = 640
        if len(resolved) >= 4 and resolved[3] in (-1, 0):
            resolved[3] = 640
    return resolved


def _classify_head(outputs: list[TensorInfo]) -> tuple[str, int, list[str]]:
    """Return (head_id, num_classes, notes_emitted_during_classification)."""
    notes: list[str] = []
    if not outputs:
        notes.append("REVIEW: ONNX has zero outputs; cannot classify head")
        return "unknown", 0, notes

    primary = outputs[0]
    shape = primary.shape

    # Helper: collapse rank-4 [B,D,1,1] to rank-2 [B,D] for re-id detection.
    flat = [d for d in shape if d != 1] if len(shape) >= 2 else shape

    # --- Detection heads ---------------------------------------------------
    # YOLO26 / YOLOv8 standard layout: [1, 4+C, A]
    if len(shape) == 3:
        b, k, a = shape
        # NMS-in-graph end-to-end format: [1, MaxDet, 6] or [1, MaxDet, 7].
        # Columns are typically [(batch_id), class_id, score, x1, y1, x2, y2].
        # We do NOT have a postprocessor for this layout yet — flag it so the
        # operator hand-edits or sources the raw-output ONNX variant.
        if a in (6, 7) and k >= 10:
            notes.append(
                f"REVIEW: output shape [{b}, {k}, {a}] looks like NMS-in-graph "
                "(end-to-end) — current pipeline expects raw [B, 4+C, A]. "
                "Either re-export the model without NMS or wait for the "
                "post-nms decoder (deferred to Phase 1.5)."
            )
            return "unknown", 0, notes
        # Try canonical (K, A) interpretation: K = 4+C in [5, 1000), A = anchors.
        # Anchor count is typically >= 100 (8400 for 640x640 YOLO).
        if 5 <= k < 1000 and a >= 100:
            return "yolo26_detect", k - 4, notes
        # Transposed (A, K) interpretation: K = 4+C, A = anchors.
        if 5 <= a < 1000 and k >= 100:
            return "yolov8_detect", a - 4, notes

    # --- Re-id heads -------------------------------------------------------
    if len(flat) == 2 and flat[1] in (128, 256, 512, 1024, 2048):
        return "osnet_reid", 0, notes

    # --- Could not infer ---------------------------------------------------
    notes.append(
        "REVIEW: output[0] shape "
        f"{shape} did not match any known detect/re-id signature"
    )
    return "unknown", 0, notes


def introspect(onnx_path: Path) -> IntrospectionResult:
    """Load `onnx_path` and produce a populated IntrospectionResult."""
    yaml_path = onnx_path.with_suffix(".yaml")
    try:
        # `load` validates the proto; for very large models pass
        # load_external_data=False to skip the weight blob read.
        model = onnx.load(str(onnx_path), load_external_data=False)
    except Exception as exc:
        return IntrospectionResult(
            onnx_path=onnx_path,
            yaml_path=yaml_path,
            input=None,
            outputs=[],
            head="unknown",
            num_classes=0,
            notes=[f"REVIEW: onnx.load failed: {exc}"],
        )

    graph = model.graph
    if not graph.input:
        return IntrospectionResult(
            onnx_path=onnx_path,
            yaml_path=yaml_path,
            input=None,
            outputs=[],
            head="unknown",
            num_classes=0,
            notes=["REVIEW: ONNX graph has no inputs"],
        )

    # In some ONNX exports the inputs list contains initializers (weights).
    # Filter to only graph-level placeholder inputs.
    initializer_names = {init.name for init in graph.initializer}
    real_inputs = [i for i in graph.input if i.name not in initializer_names]
    if not real_inputs:
        # Fall back to first listed input.
        real_inputs = list(graph.input)

    input_info = _read_tensor_info(real_inputs[0])
    output_infos = [_read_tensor_info(o) for o in graph.output]

    head, num_classes, notes = _classify_head(output_infos)

    # Resolve dynamic input dims given the head guess.
    input_info.shape = _resolve_input_shape(input_info.shape, head)

    # Sanity-tag absurd class counts.
    if head in ("yolo26_detect", "yolov8_detect") and num_classes > 1000:
        notes.append(
            f"REVIEW: inferred num_classes={num_classes} > 1000; "
            "falling back to single-class stub"
        )
        num_classes = 1

    return IntrospectionResult(
        onnx_path=onnx_path,
        yaml_path=yaml_path,
        input=input_info,
        outputs=output_infos,
        head=head,
        num_classes=num_classes,
        notes=notes,
    )


# =============================================================================
# YAML emission — hand-rolled writer to match the exact in-repo style
# (block-flow [a, b, c] arrays, aligned key columns, comment headers).
# We avoid PyYAML's default_flow_style chaos because the existing yaml-cpp
# parser is happy with either, but readability matters to humans.
# =============================================================================
def _format_shape(shape: list[int]) -> str:
    return "[" + ", ".join(str(d) for d in shape) + "]"


def _emit_classes(num_classes: int) -> list[str]:
    """Return the YAML body for the `classes:` section as a list of lines."""
    if num_classes == 0:
        return []  # re-id models have no classes

    if num_classes == 80:
        rows = COCO_CLASSES
    elif num_classes == 6:
        rows = ROC_HUMANOID_CLASSES
    elif num_classes == 1:
        rows = [(0, "person", "Person")]
    else:
        # Generic stub: class_<id> -> Unknown. Operator must hand-edit.
        rows = [(i, f"class_{i}", "Unknown") for i in range(num_classes)]

    lines = ["classes:"]
    for cid, cname, roc in rows:
        # NOTE: the comma between `name` and `roc_label` is REQUIRED — without
        # it yaml-cpp treats the rest of the line as a continuation of the
        # name value and the manifest fails §3.4 validation. (Verified the
        # hard way during the v7 Apex-Heavy regenerate.)
        lines.append(
            f"  - {{id: {cid:>3}, name: {cname:<16}, roc_label: {roc}}}"
        )
    return lines


def render_manifest(result: IntrospectionResult, model_name: str) -> str:
    """Produce a complete YAML manifest string for `result`."""
    inp = result.input
    head = result.head
    purpose = "reid" if head == "osnet_reid" else "detection"
    out_shape = (
        result.outputs[0].shape if result.outputs else [-1, -1, -1]
    )

    lines: list[str] = []
    lines.append(f"# Auto-generated by scripts/scan_onnx_to_manifest.py")
    lines.append(
        f"# Source ONNX: {result.onnx_path.name}"
    )
    if result.notes:
        for n in result.notes:
            lines.append(f"# {n}")
    lines.append("")
    lines.append(f"schema_version: {SCHEMA_VERSION}")
    lines.append("")
    lines.append(f"name:        {model_name}")
    lines.append(f"purpose:     {purpose}")
    lines.append(f"trained_on:  unknown  # REVIEW: edit me")
    lines.append(
        f"notes:       Auto-generated stub. Verify shapes & classes before use."
    )
    lines.append("")

    # ----- input ----------------------------------------------------------
    # Manifest expects [B, C, H, W] always; if the ONNX was rank-3 we still
    # emit rank-4 (B=1 is forced above).
    in_shape = list(inp.shape) if inp else [1, 3, 640, 640]
    while len(in_shape) < 4:
        in_shape.append(640 if head != "osnet_reid" else 128)
    in_shape = in_shape[:4]

    if head == "osnet_reid":
        mean = "[0.485, 0.456, 0.406]"
        std = "[0.229, 0.224, 0.225]"
        scale = "0.00392156862745"
        letterbox = "false"
        pad_value = "[]"
        pad_to = "none"
    else:
        # YOLO family expects 1/255 normalisation, no per-channel mean.
        mean = "[0.0, 0.0, 0.0]"
        std = "[1.0, 1.0, 1.0]"
        scale = "0.00392156862745"
        letterbox = "true"
        pad_value = "[114, 114, 114]"
        pad_to = "model_input"

    lines.append("input:")
    lines.append(f"  layout:           NCHW")
    lines.append(f"  channel_order:    RGB")
    lines.append(f"  expected_shape:   {_format_shape(in_shape)}")
    lines.append(f"  dtype:            {inp.manifest_dtype if inp else 'float32'}")
    lines.append(f"  normalization:")
    lines.append(f"    scale:          {scale}")
    lines.append(f"    mean:           {mean}")
    lines.append(f"    std:            {std}")
    lines.append(f"  preprocessing:")
    lines.append(f"    letterbox:      {letterbox}")
    lines.append(f"    pad_value:      {pad_value}")
    lines.append(f"    pad_to:         {pad_to}")
    lines.append("")

    # ----- output ---------------------------------------------------------
    if head == "yolo26_detect":
        head_str, fmt = "yolo26_detect", "cxcywh_logits"
    elif head == "yolov8_detect":
        head_str, fmt = "yolov8_detect", "cxcywh_logits"
    elif head == "osnet_reid":
        head_str, fmt = "osnet_reid", "reid_embedding"
    else:
        # Unknown head — manifest will FAIL the loader's strict parse_head,
        # which is exactly what we want. The operator MUST hand-edit.
        head_str, fmt = "yolo26_detect", "cxcywh_logits"
        lines.append(
            "# REVIEW: head could not be inferred; defaulted to "
            "yolo26_detect — verify against the ONNX output schema."
        )

    # Manifest output shape is rank-3 [B, K, A] — coerce.
    out_render = list(out_shape)
    while len(out_render) < 3:
        out_render.append(-1)
    out_render = out_render[:3]

    lines.append("output:")
    lines.append(f"  head:             {head_str}")
    lines.append(f"  format:           {fmt}")
    lines.append(f"  expected_shape:   {_format_shape(out_render)}")
    lines.append(f"  num_classes:      {result.num_classes}")
    lines.append(f"  nms_in_graph:     false")
    if head != "osnet_reid":
        lines.append(f"  thresholds:")
        lines.append(f"    score:          0.25")
    lines.append("")

    # ----- runtime_hints --------------------------------------------------
    lines.append("runtime_hints:")
    lines.append("  preferred_ep:     DirectML")
    lines.append("  precision:        fp16")
    lines.append("  cache_compiled:   true")
    if head == "osnet_reid":
        lines.append("  batchable:        true")
    lines.append("")

    # ----- classes --------------------------------------------------------
    class_lines = _emit_classes(result.num_classes)
    if class_lines:
        lines.extend(class_lines)
        lines.append("")

    # Ensure terminal newline (POSIX text-file convention).
    return "\n".join(lines).rstrip() + "\n"


# =============================================================================
# Main driver
# =============================================================================
def main() -> int:
    parser = argparse.ArgumentParser(
        description="ONNX -> ROC Vision manifest auto-generator"
    )
    parser.add_argument(
        "directory",
        nargs="?",
        default=str(Path(__file__).resolve().parent.parent / "models"),
        help="Directory to scan (default: <repo>/models/)",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="Overwrite existing .yaml files (default: skip when present)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print what would be written; do not touch disk",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Per-file diagnostics",
    )
    args = parser.parse_args()

    root = Path(args.directory).resolve()
    if not root.is_dir():
        print(f"ERROR: not a directory: {root}", file=sys.stderr)
        return 1

    onnx_files = sorted(root.rglob("*.onnx"))
    if not onnx_files:
        print(f"No .onnx files found under {root}")
        return 0

    print(f"Scanning {len(onnx_files)} ONNX file(s) under {root}")

    written = 0
    skipped = 0
    failed = 0
    reviewed = 0

    for onnx_path in onnx_files:
        try:
            result = introspect(onnx_path)
        except Exception as exc:  # noqa: BLE001 — never let a single bad file stop the batch
            print(
                f"  [FAIL] {onnx_path.name}: {exc}",
                file=sys.stderr,
            )
            failed += 1
            continue

        # Model name == file stem.
        model_name = onnx_path.stem

        if result.yaml_path.exists() and not args.force:
            if args.verbose:
                print(f"  [SKIP] {onnx_path.name} (manifest exists)")
            skipped += 1
            continue

        yaml_body = render_manifest(result, model_name)

        if any(n.startswith("REVIEW") for n in result.notes):
            reviewed += 1

        if args.dry_run:
            print(f"--- WOULD WRITE {result.yaml_path} ---")
            print(yaml_body)
            print(f"--- END {result.yaml_path.name} ---\n")
        else:
            result.yaml_path.write_text(yaml_body, encoding="utf-8")
            if args.verbose:
                head_tag = result.head if result.head != "unknown" else "REVIEW"
                print(
                    f"  [OK]   {onnx_path.name} -> "
                    f"{result.yaml_path.name} "
                    f"(head={head_tag}, classes={result.num_classes})"
                )
            written += 1

    print()
    print(f"Done: wrote {written}, skipped {skipped}, failed {failed}, flagged-for-review {reviewed}")
    if failed:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
