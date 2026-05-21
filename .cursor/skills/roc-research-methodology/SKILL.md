---
name: roc-research-methodology
description: >-
  Encodes how to conduct and document research for the ROC AI Vision project.
  Covers file placement, formatting standards, benchmark requirements,
  constraint analysis, and cross-referencing. Use when creating research
  notes, conducting literature review, evaluating libraries, or writing
  technical analysis documents in workspace_blueprint/research_notes/.
disable-model-invocation: true
---

# ROC Research Methodology

Follow this process whenever producing research or technical analysis for the project.

## 1. Pre-Research: Check Existing Work

Before starting, read or scan every file in `workspace_blueprint/research_notes/` to avoid duplicating prior research.

Existing research inventory:
- `phase_1_inference_and_targeting.md` — ONNX model metadata querying, letterbox math, inverse transforms
- `phase_1_onnx_gpu_core.md` — CUDA EP / TensorRT EP C++ API patterns, RAII, zero-copy tensors
- `phase_2_ekf_kinematics.md` — Full CA-EKF mathematical blueprint (1200+ lines), DeepSORT association
- `phase_5_telemetry_ui.md` — Binary WS schema, zero-GC Svelte store, SVG render architecture
- `target_prediction_ai.md` — Target prediction research

If your topic overlaps with an existing file, **extend that file** rather than creating a new one (unless the scope is clearly distinct).

## 2. File Placement & Naming

- **Location:** `workspace_blueprint/research_notes/`
- **Naming pattern:** `phase_N_descriptive_topic.md` or `topic_name.md`
- **After creating/renaming a file:** immediately update `workspace_blueprint/project_tree.md`

## 3. Required Document Structure

Every research document must include:

```markdown
# [Title]

**Date:** YYYY-MM-DD
**Focus:** [one-line scope statement]
**Scope:** [what this covers and what it doesn't]

---

## 1. [First major section]
...

## N. Benchmark Analysis

| Metric | Value | Conditions | Source |
|--------|-------|------------|--------|
| ...    | ...   | ...        | ...    |

## Risk Analysis

| Risk | Severity | Mitigation |
|------|----------|------------|
| ...  | ...      | ...        |

## Recommendations

1. [Specific, actionable recommendation with justification]
2. ...

## References

- [Author/Org, Title, URL, Date accessed]
```

## 4. Mandatory Content Requirements

### Benchmark Numbers

Every technical claim must be backed by concrete numbers:
- Latency measurements in milliseconds with hardware context
- Memory usage in KB/MB with allocation pattern noted
- Throughput in fps/ops-per-second
- Accuracy metrics with dataset and methodology

### Paper Citations

When referencing academic work:
- Full author list, title, venue, year
- ArXiv / DOI link where available
- Specific section or equation number when citing a result

### Repository URLs

When evaluating open-source tools:
- GitHub/GitLab URL
- Last commit date or version number
- License type
- Stars / maintenance status assessment

### Constraint Analysis

Every research finding must be evaluated against the project's specific constraints:

| Constraint | Requirement |
|------------|-------------|
| Latency    | < 33 ms/frame (30 fps budget) |
| Platform   | Windows x64, single machine |
| GPU        | NVIDIA RTX 4090 class, CUDA 12.x |
| C++ standard | C++17 (C++20 only for std::span where needed) |
| Memory     | Zero heap allocation in hot path |
| Precision  | Double for filter math, float for wire/embeddings |
| Determinism | Reproducible results across runs |

If a solution violates any constraint, document the violation and propose a mitigation or alternative.

## 5. Post-Research Actions

1. **Update `workspace_blueprint/roadmap.md`** — add an execution log entry under the relevant phase with date and one-line summary of what was researched.
2. **Update `workspace_blueprint/project_tree.md`** — if any files were created or renamed.
3. **Cross-reference** — if the research affects existing blueprints (e.g., a new finding changes the EKF design), add a note in both the new research and the affected existing document.

## 6. Quality Checklist

Before finalising any research document:

- [ ] Scanned existing research notes for overlap
- [ ] Includes benchmark table with real numbers
- [ ] All claims cite sources (papers, docs, repos)
- [ ] Constraint analysis section present
- [ ] Risk analysis with severity and mitigation
- [ ] Actionable recommendations (not vague suggestions)
- [ ] Roadmap execution log updated
- [ ] Project tree updated if files changed
- [ ] No placeholder text ("TBD", "TODO", "fill in later")
