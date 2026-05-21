# ROC AI Vision — Next-Phase Work Handoff

Self-contained handoff for the next contributor (human or AI assistant)
picking up the active work. Read these in order.

| # | File | What it covers |
|---|---|---|
| 1 | [`handoff/01-orientation.md`](handoff/01-orientation.md) | Repo layout, build, what's done, what's planned |
| 2 | [`handoff/02-track-a-dxgi.md`](handoff/02-track-a-dxgi.md) | DXGI Desktop Duplication capture (perf upgrade over BitBlt) |
| 3 | [`handoff/03-track-b-ndi.md`](handoff/03-track-b-ndi.md) | NDI network video receiver (OBS / capture cards / IP cameras) |
| 4 | [`handoff/04-track-c-conveyor.md`](handoff/04-track-c-conveyor.md) | Ender-3 conveyor sorter (overhead camera + class routing + G-code) |
| 5 | [`handoff/05-track-d-motion.md`](handoff/05-track-d-motion.md) | Jerk-limited motion profiles (Ruckig integration) |
| 6 | [`handoff/06-conventions.md`](handoff/06-conventions.md) | Coding style, build quirks, mingw-w64 / Svelte 5 gotchas |
| 7 | [`handoff/07-out-of-scope.md`](handoff/07-out-of-scope.md) | What to refuse if asked — keeps the project defensible |

**Repo:** https://github.com/HeavenFYouMissed/roc-ai-vision
**Framing:** read `README.md` + `workspace_blueprint/research_notes/anticheat_data_exposure_2026.md` before touching code.

**The four work tracks A / B / C / D are independent.** Each can be done in
isolation; pick whichever has the clearest first concrete artifact for you
and start there. The recommended sequence is **A → C → D → B** because:
- A is the easiest win and unblocks better demos
- C is the most defensible non-gaming application (industrial sorting)
- D is required for C to not destroy hardware
- B is the largest external-SDK lift (NewTek NDI) and can wait
