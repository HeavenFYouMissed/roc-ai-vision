# AGENTS

## Project

ROC AI Vision — humanoid entity tracking for intelligent surveillance. Binding spec: `workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md`.

## Discipline (non-negotiable)

- After every code change, update `workspace_blueprint/project_tree.md` (Last-updated header + tree if files moved/created/renamed).
- For UI/dashboard changes, also update `workspace_blueprint/UI_SPEC_SHEET.md`.
- Never commit unless explicitly asked.
- No placeholder code (no `// TODO: implement later`, no shallow stubs). Production-grade only.
- All `.cursor/rules/*.mdc` are binding — read and follow them; they take precedence over general practices.
- C++ work: C++17, RAII, no raw owning pointers, `-ffast-math` stays OFF. Dashboard work: Svelte 5 runes (`$state` / `$derived` / `$effect`), TypeScript strict, zero per-frame heap allocation in RAF / wire-decode paths.

## Quick map

- `workspace_blueprint/roadmap.md` — phase timeline + execution log
- `workspace_blueprint/architecture.md` — high-level system architecture
- `workspace_blueprint/research_notes/dashboard_menu_architecture_2026.md` — binding spec (v2+v3+v4+v5)
- `workspace_blueprint/UI_SPEC_SHEET.md` — living UI spec (design tokens, component tree, wire format, §14 stage-div pattern)
- `workspace_blueprint/PHASE_5C_HANDOFF.md` — current phase execution handoff (T01-T24)
- `.cursor/rules/000-roc-global.mdc` — global persona + Living Blueprint mandate
- `.cursor/rules/200-dashboard-telemetry.mdc` — dashboard binding rules (cardinality-of-4, zero-GC, defensive framing)
- `.cursor/rules/220-operate-canvas-stage.mdc` — overlay-alignment pattern (binding for all `dashboard/**/*.svelte`)

## Current state pointer

See `workspace_blueprint/project_tree.md` "Last updated" header for the live state of the codebase.
