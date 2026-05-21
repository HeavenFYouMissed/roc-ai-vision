# Game-CV & Streaming-Overlay UX — Menu, Hotkey & Layout Patterns for an Operator-Grade Tracking Console

> **Research Sprint — May 2026**
> **Focus.** Survey of menu / overlay / hotkey UX patterns used by in-game post-process injectors, streaming consoles, GPU vendor overlays, game-CV reverse-engineering tools, and the closest professional cross-over (Foxglove). Cross-walked into concrete recommendations for the ROC AI Vision SvelteKit dashboard.
> **Scope.** `dashboard/` menu architecture, hotkey scheme, profile/preset model, live-feed compositing, target-lock interaction grammar, performance HUD design.
> **Out of scope.** Offensive cheat mechanics, kernel-mode injection, anti-cheat evasion, anything that doesn't translate to a defensive operator console.
> **Pairs with.** `research_dashboard_tech_2026.md` (transport & render layer), `phase_5_telemetry_ui.md` (binary wire schema). **Does not modify** `dashboard/architecture_plan.md` — that is reserved for the parent synthesis pass against the pro-console research stream.
> **Author.** UX Research Scientist (subagent), under `300-research-standards.mdc`.

---

## Table of Contents

1. [Executive summary](#1-executive-summary)
2. [System-by-system survey](#2-system-by-system-survey)
   1. [ReShade](#21-reshade)
   2. [OBS Studio](#22-obs-studio)
   3. [Streamlabs Desktop](#23-streamlabs-desktop)
   4. [NVIDIA App / GeForce Experience / ShadowPlay](#24-nvidia-app--geforce-experience--shadowplay)
   5. [AMD Radeon Overlay + ReLive](#25-amd-radeon-overlay--relive)
   6. [Razer Synapse 3 / 4](#26-razer-synapse-3--4)
   7. [Logitech G HUB](#27-logitech-g-hub)
   8. [Discord In-Game Overlay](#28-discord-in-game-overlay)
   9. [Cheat Engine (defensive-research framing)](#29-cheat-engine-defensive-research-framing)
   10. [MSI Afterburner + RivaTuner Statistics Server](#210-msi-afterburner--rivatuner-statistics-server)
   11. [In-game minimap / ESP-style detection overlays (HUD theory)](#211-in-game-minimap--esp-style-detection-overlays-hud-theory)
   12. [Foxglove Studio (cross-check)](#212-foxglove-studio-cross-check)
3. [Comparison matrix](#3-comparison-matrix)
4. [Pattern extraction — what mass adoption has validated](#4-pattern-extraction--what-mass-adoption-has-validated)
5. [Concrete recommendations for ROC AI Vision](#5-concrete-recommendations-for-roc-ai-vision)
6. [Mapping ROC's 11 future features → overlay / panel / modal](#6-mapping-rocs-11-future-features--overlay--panel--modal)
7. [Risk analysis](#7-risk-analysis)
8. [References](#8-references)
9. [Addendum — answers to the two operator-specific questions](#9-addendum--answers-to-the-two-operator-specific-questions)

---

## 1. Executive summary

1. **The overlay-plus-panel hybrid wins by adoption.** Every consumer-grade in-game tool examined — ReShade, NVIDIA App overlay, AMD Radeon Overlay, Discord, MSI Afterburner+RTSS, Streamlabs — exposes the *same* feature set through (a) a thin always-on HUD and (b) a richer toggleable menu summoned by a single global hotkey. The "click into a tab to see the camera" pattern, in contrast, is the *configuration-only* pattern. For a live tracking session, ROC needs both modes, not one.
2. **Hotkeys are non-negotiable for the live path.** Every studied system requires `Alt+`-modified or function-key bindings for record / replay / toolbar-toggle / screenshot. The recurring concrete bindings are `Alt+Z` (NVIDIA toolbar), `Alt+R` (AMD Radeon Overlay / NVIDIA stats), `Alt+F9` (NVIDIA record), `Alt+F10` (instant replay), `Home`/`Shift+F2` (ReShade menu), `Shift+\`` (Discord), `Scroll Lock` (MSI Afterburner OSD), `Space` (Foxglove playback). AMD's docs *explicitly require* `Ctrl` and/or `Alt` modifiers and reserve plain function keys — a rule ROC should adopt.
3. **Profile/preset systems split into two layers.** OBS separates *Profile* (output/encoder settings) from *Scene Collection* (visual layout), and lets the user mix-and-match. Razer Synapse 3/4 and Logitech G HUB carry that further with **per-application profile auto-switching** (the profile loads when an `.exe` takes focus). For ROC this maps directly to camera/model/actuator presets — and the layering matters because operators will want to change *what model is loaded* independently of *what panels are visible*.
4. **The performance HUD is its own discipline.** RTSS+MSI Afterburner, NVIDIA App's Statistics overlay, and AMD's "Performance Monitoring" all converge on the same essential counters (frametime, FPS avg, 1% low, GPU util/temp/clock/power, VRAM) rendered in a draggable, opacity-tunable, colour-grouped corner OSD. ROC's CV-operator equivalent ("inference ms, EKF age, track count, drop rate, link RTT, actuator ack lag") is *the same product genre* and should follow the same affordances: corner placement, opacity slider, toggle hotkey, optional graph mode.
5. **The detection overlay is now a first-class panel feature even in professional tooling.** Foxglove Studio shipped *Image Overlays* on 2026-05-13 — explicitly to "composite masks, heatmaps, and model outputs directly on top of your camera feed, with per-layer opacity and blend mode controls" — which is exactly what ROC needs for the live-capture + detection composite. The 2026 industry consensus is that you do **not** put the raw feed on one tab and the projection on another; you put the projection on top of the feed inside one panel and let opacity / blend mode / layer order be operator-tunable.

---

## 2. System-by-system survey

### 2.1 ReShade

ReShade is the canonical "configurable overlay that lives inside another DirectX/OpenGL/Vulkan process". Its UX defines the genre.

| Property | Value | Source |
|---|---|---|
| Overlay toggle hotkey | `Home` (default) for ReShade 3.x+; `Shift+F2` was the documented binding in the 3.0 newcomers guide before being changed to `Home` (the same guide was later edited by `crosire` to "Fixed UI key") | [1], [2] |
| Hotkey rebind UX | "Click in the box and press your desired combo" (modal capture in the in-game menu **Settings → Overlay key**) | [2] |
| Input gating | **Settings → Input Processing → "Block all input when overlay is visible"** — explicitly recommended to prevent the user's clicks/keys from leaking into the underlying game while operating the menu | [2] |
| Menu structure | Tabbed in-game ImGui window: `Home` (effect toggles, search, per-effect parameters), `Settings`, `Statistics`, `Add-ons`, `About`. Home tab itself splits into a *config dropdown* + *shaders list* + *parameter editor* | [2] |
| Profile/preset model | "Config" files (`.ini` presets). Dropdown at the top of Home. New presets start as copies of the current one — encourages safe iteration | [2] |
| Performance feedback | The **Statistics** tab shows per-effect frametime contribution (ms) and computed FPS delta, with the explicit pedagogy that effect cost depends on baseline FPS | [2] |
| Add-on event model | Add-ons register C-callable callbacks on render events (`addon_event::reshade_present`, etc.); UI elements are registered through ReShade's overlay API | [3] |

**What works.**
- Single global hotkey toggles the entire UI. The user never has to remember more than one thing to "get back into the menu".
- The input-gating toggle is critical for any overlay sharing the same input device as the underlying interactive content — it eliminates a whole class of ghost-click bugs.
- Presets are *first-class*, not buried — visible on the Home tab.
- Cost transparency: per-effect ms readout educates the user about what they just enabled.

**What doesn't.**
- The default `Home` key collides on tenkeyless and many laptop keyboards, which is why the forum is filled with rebinding threads [1]. **Lesson**: never bind the default to a key that doesn't physically exist on every layout.
- The menu is modal-by-default; live preview while tweaking sliders has to be deliberately enabled. ROC must do the opposite — live preview is the default, the modal is the exception.

### 2.2 OBS Studio

OBS Studio is the reference for a professional streaming console run by humans on a single PC under time pressure.

| Property | Value | Source |
|---|---|---|
| Hotkey scheme | Hotkeys configured in **Settings → Hotkeys**; OBS supports *global* hotkeys (work even when OBS is unfocused). General editor shortcuts include `Ctrl+Q`, `Ctrl+Z`, `Ctrl+Shift+Z`, `Ctrl+C`/`Ctrl+V`/`Del` on the sources list, `Ctrl+E` edit transform, `Ctrl+R` reset transform, `Ctrl+F` fit-to-screen | [4] |
| Preview-canvas manipulation | Mouse-driven: drag bounding box, `Alt+drag` to crop, `Shift+drag` to stretch, `Ctrl+drag` to disable snap, `Hold Space + drag` to pan the canvas, `Hold Space + scroll` to zoom | [4] |
| Profile vs Scene Collection split | **Profile** = output/encoder settings (stream destination, bitrate, video resolution, audio device). **Scene Collection** = the visual layout (scenes + sources). Mixable: one profile, many collections; or one collection, many profiles | [5], [6] |
| Import/Export | Profiles export as JSON; Scene Collections can be imported across operating systems and migrated from OBS Classic / XSplit / Streamlabs Desktop | [5], [6] |
| Source paradigm | Each scene contains a stack of *sources* (Display Capture, Window Capture, Game Capture, Video Capture, Image, etc.). Visibility, ordering, transform, and crop are per-source | [7] |
| Docking | OBS uses Qt dockable panels: Scenes / Sources / Mixer / Transitions / Controls / Stats are all docks that can be undocked, re-anchored, hidden, and saved | [4], [7] |

**What works.**
- The *Profile vs Scene Collection* separation is a clean conceptual boundary that has survived a decade in production. The mental model: "what I'm streaming TO" vs "what I'm streaming". ROC's analogue is *"what runtime config I'm running"* (model, EP, actuator port) vs *"what panels I'm looking at"* (layout).
- Sources are stacked with z-order, visibility, and per-source transform. This translates 1-to-1 to compositing detection overlays on top of a live feed.
- Hotkeys are introspectable and rebindable in one screen — operators don't have to read docs.

**What doesn't.**
- The `Ctrl+R` "reset transform" / `Ctrl+S` "stretch to screen" overlap with browser-level browser-tab muscle memory; if our dashboard runs in a browser context we cannot count on `Ctrl+S` not triggering "save page".
- Profile/Collection switching is a *menu* operation, not a hotkey. OBS users repeatedly request hotkey-driven profile switching (forum thread referenced by the OBS Wiki) [6]. **Lesson**: if presets matter for a fast-context-switch use case, *bind them*.

### 2.3 Streamlabs Desktop

Streamlabs Desktop forked OBS and added mass-market polish. Its UX confirms what scales to non-expert users.

| Property | Value | Source |
|---|---|---|
| Hotkey config | **Settings → Hotkeys** — exposes hotkeys for streaming, recording, mic mute, replay save, alert skip, **plus per-scene and per-source visibility**. Vendor explicitly recommends "make your hotkeys a combination of at least two or more keys" to avoid accidental fires from other apps | [8] |
| Per-source hotkeys | Each Source in the Sources panel can get its own show/hide hotkey | [8] |
| Bookmarking | A dedicated *Bookmark* hotkey writes a timestamped marker into the recording for post-prod review | [8] |
| Hardware integration | First-class support for Loupedeck and Stream Deck physical controllers — the hotkey layer is intentionally *device-agnostic*: software keybind first, hardware mapping second | [8] |

**What works.**
- "Two-key minimum" is excellent defensive UX advice the user explicitly received from the vendor — adopt verbatim.
- The bookmark-into-recording pattern is exactly what an operator needs to annotate interesting CV moments in a long recording session.

**What doesn't.**
- The settings UI is several clicks deep (cog → Hotkeys → scroll). For a live tracking session this is too slow; needs a command-palette equivalent.

### 2.4 NVIDIA App / GeForce Experience / ShadowPlay

NVIDIA replaced GeForce Experience with the **NVIDIA App** in 2024, keeping the historical Alt+Z overlay binding [9], [10].

| Function | Default hotkey | Source |
|---|---|---|
| Open In-Game Overlay (toolbar) | `Alt+Z` | [9], [10] |
| Start/stop recording | `Alt+F9` | [9] |
| Toggle Instant Replay | `Alt+Shift+F10` | [9] |
| Screenshot | `Alt+F1` | [9] |
| Photo mode (Ansel) | `Alt+F2` | [9] |
| Game filter editor | `Alt+F3` | [9] |
| Microphone toggle / PTT | `Ctrl+Alt+M` | [9] |
| **Statistics overlay** toggle | `Alt+R` | [9] |

**What works.**
- All bindings are `Alt+`-prefixed — making them collision-resistant with games that consume bare function keys.
- The toolbar is summoned by `Alt+Z` but each *sub-action* has its own independent binding. The user never has to open the toolbar to record a clip — `Alt+F9` works directly. This is a critical "menu = optional" pattern.
- The Statistics overlay (corner FPS/util/temp HUD) lives separately from the toolbar — operators who only want the HUD never need to summon the menu at all. *Independent toggles per visual element.*
- Records save to a predictable, fixed-path Gallery (`C:\Users\<u>\Videos\Captures`) [9]. Predictability matters when the operator is going to grep these later.

**What doesn't.**
- Many overlay failures are caused by Windows N missing Media Feature Pack and need driver ≥ 565.90 [10] — the overlay has an opaque dependency the user can't see. ROC must surface its analogous deps (e.g. WebGPU support, EP availability) up front, not hide them.

### 2.5 AMD Radeon Overlay + ReLive

AMD's response to NVIDIA's stack, similar shape, with one important UX rule.

| Property | Value | Source |
|---|---|---|
| Toggle Radeon Overlay | `Alt+R` (default, rebindable in **Radeon Settings → Preferences**) | [11] |
| Hotkeys rule (mandatory) | "**Hotkeys must include the CTRL and/or ALT keys**. Do not choose hotkeys reserved by the operating system or another application (for example, `Alt+F4` is used by Windows to close the current application)." This is stated verbatim in both [11] and [12] | [11], [12] |
| Configurable bindings | Toggle Toolbar, Toggle Recording, Save Instant Replay, Toggle Streaming, Screenshot, Toggle Microphone, Toggle Camera, Toggle Region Recording, Toggle Region Streaming, Save Instant Gif, Save In-Game Replay | [12] |
| Overlay surface | OSD menu that exposes a reduced, *task-oriented* subset of full Radeon Settings: ReLive (record/stream), WattMan (clocks/voltage), FRTC, Game Dynamics, Game Advisor, Performance Monitoring, Display Settings. Changes made via the overlay also persist into the main app's saved state | [11] |
| WattMan profile presets | The overlay offers four named profile presets — *Power Save, Balanced, Turbo, Custom* — with one-click apply | [11] |
| Region capture | A *Borderless Region Capture* mode lets the operator define a sub-rectangle of an app to record. This is the gaming-overlay analogue of crop-to-ROI for our camera | [12] |

**What works.**
- The explicit `Ctrl`/`Alt`-required rule is an excellent norm: it's an audit-able policy ROC can enforce in the hotkey rebinder UI.
- The "overlay surface is a reduced task-oriented view of the full settings app" is a clean architectural rule — and a bidirectional sync ("changes persist back") means there is one source of truth.
- Named numeric presets (Power Save / Balanced / Turbo / Custom) are easier to reason about than raw slider positions. ROC should ship named EKF-noise / actuator-aggressiveness presets.

**What doesn't.**
- The overlay is *only* available when AMD's full driver suite is installed and enabled. The toolbar can be globally disabled from preferences but cannot be re-enabled from inside a game [11]. Symmetric on/off paths matter.

### 2.6 Razer Synapse 3 / 4

Synapse is the canonical "per-application profile + macro editor" UX.

| Property | Value | Source |
|---|---|---|
| Macro recorder | **MACRO → New Macro → Record** captures keystrokes and mouse functions in real time; supports record-with-delay (3-second countdown) and toggleable delay capture. Save, rename, reassign. Stop button always visible during capture | [13] |
| Macro authoring (Synapse 3 Insert mode) | Manually insert *Keystroke / Mouse Button / Text / Run Command* actions with per-action delays — a step-sequencer UI rather than a record-only paradigm | [13] |
| Per-app profile switching | Synapse 3/4 supports automatic profile switching by foreground application — the bound profile loads when an `.exe` takes focus. This is the gaming-industry "profile-per-game" pattern | [13] (Synapse 3 macro workflow assumes assignment to "any applicable Razer Synapse-enabled product"); see also Logitech G HUB §2.7 below for the same pattern |
| Naming defaults | Macros auto-named `Macro 1`, `Macro 2`, … then user renames via ellipsis menu | [13] |

**What works.**
- The macro recorder is a state machine the user can see — explicit Record / Stop / Save states. Live operators benefit from never being confused about "am I recording?".
- Default-named-then-rename keeps the friction low for power users producing many macros.

**What doesn't.**
- The ellipsis-menu rename interaction is a couple of clicks deep; for a hot loop you want inline edit. ROC presets should be inline-editable.

### 2.7 Logitech G HUB

G HUB is the closest commercial analogue to what an operator's "profiles + assignments" tab should look like.

| Property | Value | Source |
|---|---|---|
| Hub layout | Top-left global nav icon → global settings / apps & games profiles / community / web links. Connected devices in centre of Home page, click to open per-device config | [14] |
| Per-device config tabs | Sensitivity, LIGHTSYNC (lighting), Assignments — tab-per-domain *inside one device's panel* | [14] |
| Profile system | Per-application profiles auto-bind by detected executable; presets can be shared and downloaded from the Logitech community; preset assignment is per-game | [14] |
| Device view | Toggleable *Tile* vs *List* view of connected devices. Useful when many devices are present | [14] |
| KEYCONTROL | Newer subsystem for advanced keyboard configuration with its own dedicated UI | [14] |

**What works.**
- Auto-switching profile-per-app is the right behaviour for a tool that follows a workflow (e.g., "tracking session" preset auto-loads when the dashboard wakes the C++ core).
- Tile vs List view scales from "few items" to "many items" without redesign — relevant for ROC's 113-file ONNX collection.

**What doesn't.**
- G HUB is famously slow to load and its tab-driven device pages take more clicks than they need to. *Lesson*: for the live path, prefer hotkey + overlay; only spend tab-time on configuration.

### 2.8 Discord In-Game Overlay

Discord proves a thin always-on overlay can coexist with a fullscreen 3D application.

| Property | Value | Source |
|---|---|---|
| Toggle binding | `Shift+\`` (default; user-rebindable in keybind settings) | [15] |
| Enable | **User Settings → Overlay → Enable Overlay** toggle (cog wheel next to username) | [15] |
| Per-element pin-in-game | After summoning the overlay with the hotkey, click the **pin icon** to keep a sub-window visible while playing. Pinnable: Notifications, Voice, Video, Activity, Streams | [15] |
| Configurability | *Display Users* (Always / Only While Speaking), *Display Names* (Always / Only While Speaking / Never), *Avatar Size* (Large / Small) | [15] |
| OS support | Windows only (Win 10/11). Explicit recommendation to switch to *windowed* / *borderless windowed* if the overlay misbehaves in fullscreen | [15] |

**What works.**
- The pin-while-active pattern: the operator opens the menu, pins the elements they want to keep, dismisses the menu, and what they pinned stays visible. This is exactly the affordance ROC needs for per-track-stat HUDs.
- Three-state visibility (Always / While Active / Never) for each element gives operators a way to dial cognitive load.

**What doesn't.**
- The overlay only works in windowed / borderless windowed modes [15] — an instructive failure mode that points to choosing display modes deliberately for compositing.

### 2.9 Cheat Engine (defensive-research framing)

Including here strictly for the *scanner-plus-action loop* UX pattern, which has obvious analogues in detection-iteration workflows. We are *not* including injection or anti-anti-cheat mechanics.

| Property | Value | Source |
|---|---|---|
| Scan loop | A two-stage *first scan → next scan* refinement loop. *First Scan* options: Exact / Bigger than / Smaller than / Between / Unknown initial. *Next Scan* options: Exact / Bigger / Smaller / Between / Increased / Increased-by / Decreased / Decreased-by / Changed / Unchanged / Same-as-first | [16] |
| Result list affordances | Result rows colour-coded — *green* = static (module-relative offset), *black* = dynamic. Right-click a row for actions; double-click to add to the tracked list | [16] |
| Memory View hotkeys | `Ctrl+G` jump-to-address; `Ctrl+F` find-in-hex; `Space` follow-jmp/call; `Backspace` go-back; `Ctrl+Space` mirror disassembler/hex-view location | [17] |
| Action menus | Right-click on disassembly: Go to address / Replace with NOP / Add to code list / Toggle breakpoint / Break-and-trace / Find what accesses / Find what writes — every operation is right-click discoverable | [17] |

**What works.**
- The colour-coded result list (green static / black dynamic) is the same idea ROC needs to convey for *confirmed* vs *speculative* tracks.
- Right-click-discoverable action menus mean the user never has to memorise commands; the menu is the manual.
- The progressive-refinement scan loop maps directly to confidence-thresholded detection filtering during operator iteration.

**What doesn't.**
- The default UI is a Windows MDI-style explosion of floating windows. Operators get lost. ROC must explicitly *not* go MDI.

### 2.10 MSI Afterburner + RivaTuner Statistics Server

MSI Afterburner + RTSS is the de-facto reference for the PC performance OSD genre.

| Property | Value | Source |
|---|---|---|
| OSD toggle hotkey | User-assigned (vendor walkthrough uses **Scroll Lock**) — explicitly noted that *global hotkeys only function when the Settings menu is closed* | [18] |
| Benchmark hotkeys | `F9` begin / `Shift+F9` end (vendor walkthrough) — required to populate Framerate Avg + 1% Low fields | [18] |
| Per-metric selection | Each available metric (GPU usage, GPU clock, GPU temp, GPU power, VRAM use, VRAM clock, framerate, frametime, frametime graph, framerate avg, 1% low, CPU usage, CPU clock, CPU temp, RAM usage, fan RPM, etc.) has an independent *Show in On-Screen Display* checkbox plus draggable display order | [18] |
| Display mode | Per-metric *text* vs *text + graph* drop-down. Graphs scroll at FPS-relative speed | [18] |
| Visual config | RTSS *OverlayEditor* plugin lets the operator place / preview the OSD without a running game; OSD has corner selector, drag-anywhere positioning, zoom slider, fill-color + opacity behind text for readability over busy backgrounds, colour-grouped text (GPU colour / CPU colour / framerate colour / system colour) | [18] |
| Hardware Monitor window | `Ctrl+M` opens a separate dockable monitor window with graphs for every monitored metric, configurable column count, "Always on top" toggle, attachable graphs | [18] |
| Capture | Per-encoder selection (NVENC / Quick Sync / VCN), Matroska container, configurable bitrate / frame size / rate, separate Screen Capture tab with own hotkeys | [18] |

**What works.**
- Per-metric independent toggles plus drag-to-reorder give operators a *composable* HUD. ROC's "perf HUD" should follow this exactly.
- Group-colour scheme is a cheap, high-throughput visual hierarchy mechanism — every operator can scan colour faster than label text.
- The note that "global hotkeys only function when the Settings menu is closed" [18] is an important *modal/non-modal* boundary that ROC must respect: while the config modal is open, gameplay-grade hotkeys should be intercepted by the modal, not the live system.

**What doesn't.**
- Initial setup is many steps deep ("set the metric → check 'Show in OSD' → drag-order → assign a global toggle → close settings → press toggle in game"). Sane defaults matter — ROC should ship with the HUD on and the common four counters visible.

### 2.11 In-game minimap / ESP-style detection overlays (HUD theory)

This subsection studies overlay theory at the level of *placement and cognitive load*, drawing on game-design literature, **not** offensive tools.

| Principle | Citation |
|---|---|
| **Effortless readability.** HUD elements are constantly present, so text/icons must be readable at a glance; use high-contrast colours, familiar iconography, declutter | [19] |
| **Consistency.** Maintain consistent colour, font, style; do not move element positions between modes — "players quickly learn where to look for crucial information" | [19] |
| **Visual hierarchy.** Larger, brighter for critical (e.g., health); smaller, less demanding for ancillary (currency, time) | [19] |
| **Accessibility.** Allow the user to resize and recolour elements; ship multiple colour profiles for colour-blindness | [19] |
| **Corner placement breaks immersion.** Ubisoft designer Andrii Honcharuk argues critical info in screen corners forces saccadic eye movement to the periphery and "subconscious reconnects with reality" — peripheral vision loses contact with the scene. The genres that depend most on immersion (survival horror like *Outlast*, *Dead Space*) bias HUD elements toward the foveal/parafoveal region, sometimes diegetically embedded in the world | [20] |
| **Eye-tracker validation.** Honcharuk explicitly recommends playtesting with an eye tracker to verify where the operator actually looks — useful for ROC if we run the operator console through long surveillance shifts | [20] |

**What works for an operator console.**
- The "do not move element positions between modes" rule maps directly onto ROC: when the operator hotkey-toggles the menu, the underlying detection markers and feed should not jump.
- "Visual hierarchy via size and brightness" is the right way to encode track confidence and recency.
- Diegetic / scene-anchored markers (bounding boxes drawn *in image space*, IDs labelled at the head of the box) are the inverse of corner placement and dramatically reduce saccade load for a vigilance task.

**What doesn't.**
- Survival-horror "no HUD" aesthetics are not appropriate for ROC. Operators need persistent counters; we are not optimising for atmosphere.

### 2.12 Foxglove Studio (cross-check)

The professional-console reference, included here only at the points where its UX overlaps with the gaming overlays above.

| Property | Value | Source |
|---|---|---|
| Sidebar toggles | `[` toggles left sidebar; `]` toggles right sidebar — pure muscle-memory bracket pair | [21] |
| Panel settings | `,` toggles the panel settings sidebar; numeric inputs accept ↑/↓ to increment, or click-drag horizontally | [21] |
| Command palette | `Ctrl+K` (Cmd+K on macOS) opens the command palette — *this is the missing primitive most game tools lack* | [21] |
| Playback hotkeys | `Space` toggle, `←/→` 100 ms seek, `Shift+←/→` 10 ms seek, `Alt+←/→` 500 ms seek, `Home`/`End` to data start/end, `Shift+↑/↓` speed | [21] |
| 3D-panel camera hotkeys | `W/A/S/D` to translate, `Shift+W/A/S/D` to rotate, scroll to zoom, `1` to re-center on target frame, `3` to toggle 2D bird's-eye vs 3D perspective, `i` to toggle inspector | [21] |
| Image overlays (2026) | Released **2026-05-13**: in the Image panel, add one or more *overlay layers* (mask topic / segmentation topic / model output topic) composited live with per-layer **visibility, ordering, opacity, blend mode** (Alpha vs Add) and **Pixel alpha** controls for single-channel masks. Officially the recommended pattern for "comparing a mask to the camera image" | [22] |
| Desktop tabs (2024+) | Browser-style tabs at window level — `Ctrl+T` new tab, `Ctrl+W` close, `Ctrl+Tab`/`Ctrl+Shift+Tab` cycle, `Ctrl+/` last-active, `Ctrl+[`/`Ctrl+]` back/forward. Each tab is a fully independent workspace with its own data source and layout | [23] |
| Layouts ("Blueprints" in Rerun, "Layouts" in Foxglove) | Layouts are saved configurations of which panels exist, where they're docked, and what each is bound to. Switchable on the fly | [24] (and pre-existing in our `.firecrawl/foxglove-panels.md`) |
| Panel paradigm | Modular panels added via "Add panel" menu; each panel has a top bar with split/change-type menu + settings cog (`,`); clicking different panels rebinds the settings sidebar to the selected panel; topics can be drag-dropped from the Topics sidebar straight into compatible panels | [24] |

**What works.**
- The `[` / `]` sidebar bracket pair is the simplest possible left/right toggle scheme — minimal cognitive load, ambidextrous.
- The command palette (`Ctrl+K`) is the single best UX upgrade for a panel-rich app. ROC should ship one.
- Image overlays validate that "raw feed + detection mask composited in-panel with opacity + blend mode" is the *industry-correct* answer to ROC's live-capture-vs-canvas question, and it took the pro-console world until May 2026 to ship it — we get to skip that lag.

**What doesn't.**
- Foxglove's playback-first workflow (`Space` toggles playback) does not map cleanly to live-only operator work; *live* sessions don't have a play/pause semantic until we add recording.

---

## 3. Comparison matrix

| System | Always-on overlay? | Toggle-able menu? | Default toggle | Hotkeys reqd modifier | Per-element pin/toggle | Profile/preset model | Recording | Performance HUD | Composable layout | Command palette |
|---|---|---|---|---|---|---|---|---|---|---|
| ReShade | Effect layer always; menu off | Yes | `Home` (originally `Shift+F2`) | None enforced | Per-effect toggle in Home tab | Named `.ini` presets, dropdown | No (relies on external) | Per-effect ms in Statistics tab | No | No |
| OBS Studio | N/A (separate window) | Always on | n/a | None enforced | Per-source visibility hotkey | **Profile + Scene Collection** (independent) | Yes, hotkey-bound | Stats dock | Qt dockable panels | No |
| Streamlabs Desktop | N/A | Always on | n/a | "≥2 keys" recommended | Per-source hotkey | Forked OBS model | Yes + Bookmark hotkey | Stats panel | Dock layout | No |
| NVIDIA App / GeForce | Statistics OSD | Yes (toolbar) | `Alt+Z` toolbar / `Alt+R` stats | Alt-prefixed | Yes (Statistics independent of toolbar) | "Programs" library, per-game | Yes (`Alt+F9`, replay `Alt+Shift+F10`) | Yes (basic/advanced/custom) | No | No |
| AMD Radeon Overlay | OSD optional | Yes | `Alt+R` | **Ctrl and/or Alt required (enforced)** | Per-feature tab | WattMan presets (Power Save / Balanced / Turbo / Custom) | Yes (ReLive) | Yes (Performance Monitoring) | No | No |
| Razer Synapse 3/4 | N/A | Always on | n/a | n/a | Macro per-key | Per-app auto-switch | n/a | Hardware status | Tabbed | No |
| Logitech G HUB | N/A | Always on | n/a | n/a | Per-device tabs | Per-app auto-switch + community share | n/a | Per-device | Tile/List view | No |
| Discord overlay | Yes (pinned elements) | Yes | `Shift+\`` | None enforced | Pin icon per element | Per-app (overlay enable/disable) | No (use OBS) | No | Per-element 3-state visibility | No |
| Cheat Engine | N/A | Always on | n/a | `Ctrl+G`, `Ctrl+F`, `Space`, `Backspace`, `Ctrl+Space` | Per-row right-click | Save table (.CT) | n/a | n/a | MDI floating windows | No |
| MSI Afterburner + RTSS | OSD optional | Settings window | `Scroll Lock` (user-set) | None enforced | Per-metric toggle + drag-order | Per-card profile | Yes | **Genre reference**: per-metric, colour-grouped, drag-position | Hardware Monitor dock | No |
| Foxglove Studio | N/A | Always on | n/a | `Ctrl`-prefixed | Per-panel + sidebar toggles `[`/`]` | Layouts ("Blueprints" in Rerun) | Playback + event hotkey | Per-panel custom | Dockable panels + Tab panels + desktop tabs | **Yes — `Ctrl+K`** |

---

## 4. Pattern extraction — what mass adoption has validated

These are the patterns that recur across *at least three* of the eleven systems above and have demonstrably scaled to millions of operators. Each is a candidate ROC should adopt unless we have a specific reason not to.

**P1 — Single global hotkey opens a thin, dismissable menu, while every sub-action also has its own independent hotkey.**
Validated by: ReShade (`Home` → Settings tab also has per-effect toggles), OBS (Settings → Hotkeys lets each scene/source bind individually), NVIDIA App (`Alt+Z` toolbar but `Alt+F9` records directly, `Alt+R` toggles stats directly), AMD Radeon Overlay (`Alt+R` toolbar plus separate per-feature toggles), Discord (`Shift+\`` plus per-element pin), MSI Afterburner+RTSS (settings hotkey + per-action hotkeys). **The menu is optional, never a single chokepoint.**

**P2 — Hotkey bindings must use modifier-prefixed combinations.**
Validated by: AMD (explicitly enforced [11], [12]), Streamlabs (vendor-recommended [8]), NVIDIA (every documented binding is Alt-prefixed [9]), OBS (every editor binding is Ctrl-prefixed [4]), Foxglove (every desktop-tab binding is Ctrl-prefixed [21]). **Plain function keys collide with games, browsers, and OS-level accessibility tooling.**

**P3 — Profile/preset model is two-layered: "what config I'm running" is separate from "what panels I'm looking at".**
Validated by: OBS (Profile = encoder/output, Scene Collection = layout, mixable [5], [6]), Razer Synapse and G HUB (per-app profile auto-switch separate from per-device layout), AMD (WattMan profiles separate from overlay layout). **The two axes are orthogonal and operators want them separately switchable.**

**P4 — Per-element visibility toggles + drag-to-reorder are the right primitives for an operator HUD.**
Validated by: MSI Afterburner+RTSS (per-metric checkbox + drag-order [18]), Discord (per-element pin + 3-state visibility [15]), Streamlabs (per-source visibility hotkeys [8]), OBS (per-source visibility + order [4], [7]). **One toggle per logical element, ordering is user-controlled.**

**P5 — The detection overlay composites *on top of* the live feed in the same panel, with operator-controllable opacity, blend mode, and layer order.**
Validated by: Foxglove Image Overlays (2026-05-13, explicit support for `Alpha` and `Add` blend modes + `Pixel alpha` for single-channel masks [22]), OBS sources stack (sources are z-ordered with per-source opacity), HUD-design literature (markers in foveal vision out-perform corner HUDs [20]). **Splitting raw feed and detection canvas into separate tabs forces unnecessary saccade and breaks the "anchor markers to the world" rule.**

**P6 — Performance HUDs converge on the same surface: corner-placed, opacity-tunable, drag-to-reposition, per-metric toggle, optional graph, colour-grouped text.**
Validated by: MSI Afterburner+RTSS (vendor reference [18]), NVIDIA App Statistics overlay [9], AMD Performance Monitoring [11]. **The genre is solved; copy it.**

**P7 — Input-gating is mandatory when the overlay shares an input device with the underlying live process.**
Validated by: ReShade ("Block all input when overlay is visible" [2]), Discord (overlay needs windowed / borderless windowed mode and only Windows, otherwise click-leak issues [15]), MSI Afterburner ("global hotkeys only function when Settings menu is closed" [18]). **Modal/non-modal boundaries are part of the contract.**

**P8 — Asymmetric on/off paths break operators.**
Validated negatively by: AMD (overlay disable from settings can't be undone from a game [11]), NVIDIA (overlay failures hidden behind Windows N Media Pack / driver-version deps [10]). **Every "off" must have a discoverable "on" reachable from where you turned it off.**

**P9 — Command palette closes the gap between "I know what I want" and "I remember where it lives".**
Validated by: Foxglove Studio (`Ctrl+K` [21]). This pattern is *underused* in the gaming-overlay world and is one of the clearest ROC could-do-better levers. **Especially valuable with 113 ONNX files, a model selector, calibration profiles, hotkey rebinder, etc., all needing fuzzy-searchable access.**

**P10 — Saved layouts ("Blueprints" / "Layouts") are first-class and switchable on the fly.**
Validated by: Foxglove (layouts), Rerun (Blueprints [24]), OBS (Scene Collections [6]). **Operators want one layout for "calibration", another for "active surveillance", another for "post-incident review".**

---

## 5. Concrete recommendations for ROC AI Vision

> All recommendations are scoped to the **menu / hotkey / layout architecture only**. Wire-protocol, rendering, and transport choices remain governed by `research_dashboard_tech_2026.md` and `phase_5_telemetry_ui.md`.

**R1 — Menu paradigm: replace the fixed 4-tab "Domain Controller Matrix" with a dockable-panel system that has an overlay+panel hybrid mode.**
- *Primary mode:* Foxglove-style dockable panels (per `dashboard/architecture_plan.md`'s existing `TelemetryCanvas` / config-panel split), with **saved layouts** ("Calibration", "Active Tracking", "Replay/Review", "Configuration"). Per P10.
- *Secondary mode:* A summonable overlay sheet for during a live tracking session, where the operator wants the camera + detection + actuator-state to fill the screen. Per P1, P5.
- *Rejection of current 4-tab paradigm:* the four hard-coded tabs (`Observer Matrix`, `Spatial Projection Canvas`, `Kinematic Trajectory Controller`, `Hardware Actuation Interface`) violate P10 (no operator-defined layouts) and P5 (forces the live feed and EKF projection into separate tabs). Recommend the parent synthesis pass repurpose them as *layout presets* rather than hard-coded screens.
- *Migration is cheap:* the `TelemetryCanvas.svelte` / `NavigationShell.svelte` split already enforces that domain panels never read the hot-path `trackPool`. Replacing `NavigationShell` with a layout-aware dock manager keeps the rule.

**R2 — Hotkey scheme: ship five non-negotiable bindings, all `Alt`-prefixed, all rebindable, all enforcing the `Alt`/`Ctrl` modifier requirement.**

| Action | Binding | Justification |
|---|---|---|
| Toggle operator overlay (the "menu") | `Alt+Z` | Universal gaming muscle memory (NVIDIA App [9]). Single chokepoint into the menu, *not* the only path to action (P1). |
| Toggle target lock on/off the currently highlighted track | `Alt+L` | Mirrors the "single canonical action" pattern of NVIDIA's `Alt+F9` record. Confirm-and-cancel via the same key (toggle). Click is the fallback for ambiguous multi-target frames (see §9 addendum 2). |
| Toggle performance HUD | `Alt+R` | Industry convergence: AMD Radeon Overlay [11], NVIDIA Statistics overlay [9]. Decouples the perf HUD from the menu (P1). |
| Start/stop recording | `Alt+F9` | NVIDIA Share parity [9]. Operators recording surveillance sessions inherit muscle memory from gaming. |
| Save "instant replay" (last N seconds of detection+telemetry to disk) | `Alt+Shift+F10` | NVIDIA parity [9]. Critical for "I just saw something weird" post-hoc analysis. |

Plus three "highly recommended":

| Action | Binding | Justification |
|---|---|---|
| Command palette | `Ctrl+K` | Foxglove convention [21]. Pattern P9. Fuzzy-search models, presets, panels, settings. |
| Toggle left / right sidebar | `[` / `]` | Foxglove convention [21]. Ambidextrous, single-key, no-modifier exception justified by panel-app context (no underlying game). |
| Cycle through tracked entities (focus selection) | `Tab` / `Shift+Tab` | Industry-standard focus traversal; needed before `Alt+L` lock toggle. |

Enforce in the rebinder: **the modifier-prefix rule from P2** — reject any binding that is a bare letter, digit, or function key (matches AMD's [11] explicit policy and Streamlabs' [8] explicit guidance).

**R3 — Ship a profile/preset system from day one. Minimum viable preset model is two-layered.**
- *Layer 1 — Runtime Profile*: model (ONNX file), EP order, EKF noise params, actuator port + baud, calibration matrix. Analogous to OBS Profile [5].
- *Layer 2 — Workspace Layout*: which panels exist, where docked, what each binds to, what's pinned in the operator overlay. Analogous to OBS Scene Collection [6].
- Mixable: one profile can be used with any layout and vice versa.
- Ship 4 starter profiles: `Surveillance`, `Calibration`, `Bench-Test`, `Replay`. Following AMD's WattMan precedent (named, not numeric) [11].
- **Auto-load** the profile that matches the operator's previous session (Synapse-style [13] / G HUB-style [14]). Surface a "switch profile" hotkey in `Ctrl+K`.

**R4 — Live camera feed and detection overlay belong in one composite panel, not separate tabs.**
- Adopt the Foxglove Image Overlays model [22]: a base image topic (camera) + N overlay layers (detection boxes, EKF projection, hierarchy tree connections, target-lock crosshair) each with independent:
  - **Visibility** (per-layer on/off, hotkey-able).
  - **Opacity** (0-100%, slider).
  - **Layer order** (drag-to-reorder, matching MSI Afterburner per-metric order [18]).
  - **Blend mode** (`Alpha` for boxes/text overlays, `Add` for heatmap/saliency layers per Foxglove [22]).
- The single composite panel is "Spatial Projection Canvas" — but the *raw camera feed is one of its layers*, not a sibling panel. See §9 Q1 for the long-form answer.
- The current 4-tab split forces operators to leave the spatial canvas to verify "is the camera actually feeding?". This is a vigilance failure mode and breaks P5.

**R5 — Target-lock confirm/cancel must support both click and hotkey, with a hotkey-then-confirm pattern for safety.**
- *Click semantics* (familiar from OBS source selection [4], Cheat Engine row selection [16]): single-click on a bounding box selects the track; double-click locks; click on empty space cancels.
- *Hotkey semantics*: `Tab` cycles selection across visible tracks; `Alt+L` toggles lock on selected; `Esc` cancels selection.
- *Two-stage confirmation for actuator-bound lock*: when the lock will drive the pan/tilt actuator, require a deliberate confirmation. Match Cheat Engine's "Replace with NOPs adds to code list automatically" pattern [17] — every actuator-bound lock writes to a visible "active locks" panel with an immediate undo affordance. Operators must never be surprised by an actuator command.
- Following the input-gating rule from P7: while a confirmation modal is open, `Esc` always cancels and **never** sends actuator commands.

**R6 — Performance HUD: ship six non-negotiable counters in the corner OSD, drag-position-able, opacity-tunable, per-counter toggleable.**

The MSI Afterburner+RTSS [18] + NVIDIA App Statistics [9] + AMD Performance Monitoring [11] convergence yields a clear genre baseline. Translated to CV-operator units:

| Counter | Why non-negotiable | Game-overlay analogue |
|---|---|---|
| Inference latency p50 / p99 (ms) | Tracks whether the EKF measurement update is meeting its 33 ms budget per `300-research-standards.mdc` constraint gate | Frametime / 1% low |
| Frames/s of the consumer (dashboard) loop | Detects render-thread starvation independently of producer rate | FPS |
| Track count (alive / confirmed / tentative) | Quick scan of "how busy is the scene" | Player count / enemy count |
| Wire egress rate (frames/s, MB/s) | Detects WS / SPSC backpressure (critical per `research_ipc_middleware_2026.md`) | Network bandwidth |
| Actuator ack lag (ms) | Detects serial line stalls before they affect pan/tilt | Input latency / PC latency [9] |
| EKF measurement age (ms since last update) | Detects stale tracks vs live tracks | "Time since last hit" debug overlay |

Per-counter toggle + group-colour scheme + corner placement + text-or-text-and-graph drop-down per RTSS [18]. Default-on so operators see them immediately without setup.

**R7 — Adopt the input-gating rule explicitly: a modal config window must intercept hotkeys; live-path hotkeys only fire when no modal is open.**
Per P7. State this in the architecture plan so the dashboard's keybinding system enforces it. Failing to do so leaks operator keystrokes into the C++ core and risks unintended actuator commands during configuration.

**R8 — Ship a `Ctrl+K` command palette in v1.**
Per P9. Fuzzy-searchable over: every panel ("show 3D inspector"), every preset ("load Calibration profile"), every model file ("load yolo26m-roc-humanoid"), every actuator command ("center pan/tilt"), every layout. Foxglove's command-palette implementation [21] is the canonical reference; the additional ROC-specific commands keep the muscle memory but remove the need to memorise menu paths.

---

## 6. Mapping ROC's 11 future features → overlay / panel / modal

| # | Feature | Placement | Rationale (pattern + source) |
|---|---|---|---|
| 1 | Live camera feed | **Composite panel layer 0** (the base) | Foxglove Image Overlays [22]; pattern P5. Operators must see the raw feed at all times during a live session. |
| 2 | EKF state visualisation (bounding boxes, predicted-vs-measured deltas, confidence ellipses) | **Composite panel layer 1+** on top of the camera feed; **dock panel** ("EKF Inspector") for numeric tabular drill-down | P5. The visual overlay is for vigilance; the dock panel is for analysis. Mirrors MSI's per-graph dock window pattern [18]. |
| 3 | Model selector (113 ONNX files) | **Modal** triggered from `Ctrl+K` command palette + dock panel under "Configuration" layout | P9. With 113 files, a modal-with-search beats any always-visible list. G HUB's *Tile/List* view toggle [14] is the right precedent for scaling beyond ~20 items. |
| 4 | Recording / replay | **Hotkey-driven** (`Alt+F9` start/stop, `Alt+Shift+F10` save instant replay), with a small **persistent corner badge** showing "REC ● 00:01:23" while active | NVIDIA Share [9]. The badge is the always-on overlay analogue; the hotkeys are P1. |
| 5 | Calibration (intrinsic + extrinsic) | **Dedicated layout preset** ("Calibration"); modal wizard for first-run | P10. Calibration is rare and dense; a layout switch is the right gesture, not a tab. Matches OBS Scene Collection [6]. |
| 6 | Actuator control (pan/tilt jog, home, kill switch) | **Pinned overlay element** in the live overlay (Discord pin pattern [15]) + dock panel for advanced tuning | P5, P4. The kill switch must be one click / one keystroke from anywhere — never buried in a tab. |
| 7 | Hierarchy explorer (person → head/torso/accessory tree) | **Composite panel layer** drawn as anchored polylines from parent box to child boxes + **dock panel** for tree view | P5. Matches the "diegetic-in-world" principle [20]; lines anchored to image-space coordinates outperform a separate tree widget for vigilance. The dock panel covers drill-down. |
| 8 | Per-track metrics (id, age, hits/misses, last update, embedding distance) | **Dock panel** ("Track Inspector"), updated when selection changes; **inline mini-stats** next to box label in the composite panel | OBS source-properties pattern [7]: select an entity → settings sidebar shows its config. Mirrors Foxglove's "click panel → settings sidebar follows" [21]. |
| 9 | Profiles / presets (Runtime Profile + Workspace Layout per R3) | **Top-bar menu** (OBS Profile menu placement [5]) + `Ctrl+K` command palette | P3, P9. Always reachable; never buried. |
| 10 | Hotkey UI (rebinder) | **Modal**, summoned from `Ctrl+K` or "Settings → Hotkeys" | Streamlabs / OBS Settings → Hotkeys precedent [4], [8]. Modal because it must intercept keys to capture rebinds (ReShade rebind pattern [2]). |
| 11 | Performance HUD | **Always-on corner OSD**, six counters default, drag-position-able, opacity-tunable, per-metric toggle, `Alt+R` global toggle | R6. Genre is solved by MSI Afterburner+RTSS [18] / NVIDIA Statistics [9] / AMD Performance Monitoring [11]. |

---

## 7. Risk analysis

| Risk | Severity | Mitigation |
|---|---|---|
| Adopting a "gamey" aesthetic that makes the console look unprofessional and unsuitable for surveillance / defensive deployment. | High | The patterns extracted are *interaction patterns*, not visual aesthetics. Apply OBS / Foxglove visual language (neutral dark theme, accent colour for critical state) — *not* RGB / neon / animated. Reject gamer skeuomorphism explicitly in the style guide. |
| Mode confusion between "overlay" and "panel" views — operator thinks they're toggling visibility but they're toggling *which-mode-am-I-in*. | High | Enforce P8 (symmetric on/off paths). One global hotkey (`Alt+Z` per R2) toggles the overlay; the panel mode is the default; opening overlay never destroys panel state. Visible state indicator in the top bar at all times. |
| Hotkey-vs-mouse mismatch — operator clicks expecting a confirm, but the action also has a hotkey that fired earlier without their intent. | High | Per R7, modals must intercept hotkeys. Per R5, actuator-bound actions require two-stage confirmation. Per R2, all live-path bindings carry modifier prefixes that are unlikely to be hit accidentally. |
| Overcomplicated MDI-style explosion (Cheat Engine failure mode [16], [17]) where new operators get lost. | Medium | Default to a *single layout* on first run ("Surveillance"). Show layout-switcher as a chip strip in the top bar, not a hidden menu. Cap the number of always-visible docks at 4 in the default layout. |
| Importing aimbot-grade patterns (target-lock without confirmation, scripted firing) that conflate ROC's defensive scope with offensive cheats. | Critical | Per R5, every actuator-driving action requires explicit confirmation; per the project framing, the project is surveillance/defensive (humanoid tracking, not engagement). Document explicitly that the click-to-lock and hotkey-to-lock affordances are *operator confirmation gates*, not automation gates. **There is no "auto-lock-and-fire" code path; the actuator only pans/tilts to track, never to discharge.** |
| Lifting the Discord per-element-pin model and ending up with a pin-spam UX where every panel becomes a floating sticky. | Medium | Cap the overlay to 6 pinned elements (matches the perf HUD counter cap from R6); use a per-pin priority/colour scheme from the HUD-theory section [19] to keep visual hierarchy. |
| Adopting plain function-key hotkeys that collide with browser shortcuts (the dashboard runs in a Chromium-class browser environment per `research_dashboard_tech_2026.md`). | High | Enforce R2's modifier-prefix rule in the rebinder. `Ctrl+S`, `Ctrl+R`, `Ctrl+W` etc. are owned by the browser at the chrome level and should be *avoided* even when a modifier is present unless we ship the dashboard as a desktop wrapper (Electron/Tauri) where we can capture them. |
| Operator opens the menu mid-track-event and misses a critical state change because the menu opaquely covered the live feed. | Medium | The overlay sheet should default to 70-85% opacity (Discord pattern [15]) and animate-in over ~120 ms so peripheral motion in the feed remains visible. Live-feed compositing in the same panel (R4) makes the menu less likely to be needed mid-event in the first place. |
| Saved layouts grow unbounded as operators iterate; finding the right one becomes its own task. | Low | Following OBS Scene Collection precedent [6], expose import/export to JSON so layouts can be backed up, shared, and pruned. Tag layouts (e.g. "Calibration", "Operator-Alice", "Bench"). Surface in `Ctrl+K`. |

---

## 8. References

1. Crosire and ReShade community, "ReShade — Forum: 'i dont have a home button on my keyboard'" + related rebinding threads, https://reshade.me/forum/troubleshooting/9235-i-dont-have-a-home-button-on-my-keyboard-how-do-i-open-reshade-now (accessed 2026-05-17).
2. Marty McFly (community guide author) with edits by crosire, "ReShade 3.0 guide for newcomers", ReShade Forum, https://reshade.me/forum/general-discussion/2501-reshade-3-0-guide-for-newcomers (accessed 2026-05-17).
3. crosire, "ReShade API documentation — Main page", https://crosire.github.io/reshade-docs/ via https://reshade.me/docs (accessed 2026-05-17).
4. OBS Project, "Keyboard Shortcuts" knowledge base article, https://obsproject.com/kb/keyboard-shortcuts (retrieved via Wayback Machine capture 2026-01-09; accessed 2026-05-17 because the live page returned 502 Bad Gateway at scrape time).
5. OBS Project, "Profiles" knowledge base article, https://obsproject.com/kb/profiles (accessed 2026-05-17).
6. OBS Project, "Scene Collections" knowledge base article, https://obsproject.com/kb/scene-collections (accessed 2026-05-17).
7. OBS Project, "Quick Start Guide" / "OBS Studio Overview Guide", https://obsproject.com/kb/quick-start-guide and https://obsproject.com/kb/obs-studio-overview (accessed 2026-05-17).
8. Ethan May (Streamlabs), "How to Set up Hotkeys in Streamlabs Desktop" (last updated 2025-10-08), https://streamlabs.com/content-hub/post/how-to-set-up-hotkeys-in-streamlabs-desktop (accessed 2026-05-17).
9. Max Miller, "How To Open NVIDIA's Game Overlay On Your PC (And What You Can Do With It)", SlashGear, 2025-01-23, https://www.slashgear.com/1764589/how-to-open-nvidia-overlay-pc-windows-also-remove/ (accessed 2026-05-17). Source is corroborated against the official NVIDIA App "Unable to bring up the NVIDIA app In-Game Overlay when pressing Alt+Z" knowledge base article [10].
10. NVIDIA Customer Help, "Unable to bring up the NVIDIA app In-Game Overlay when pressing Alt+Z" (article id 3386, updated 2024-12-14), https://nvidia.custhelp.com/app/answers/detail/a_id/3386 (accessed 2026-05-17).
11. AMD, "Using Radeon™ Overlay to Adjust Gaming and Visual Settings" (article DH-026), https://www.amd.com/en/resources/support-articles/faqs/DH-026.html (accessed 2026-05-17).
12. AMD, "Capture and Stream Gameplay Using AMD Radeon™ ReLive" (article DH-023), https://www.amd.com/en/resources/support-articles/faqs/DH-023.html (accessed 2026-05-17).
13. Razer Support, "How to create or delete macros on Razer Synapse" (answer id 1483, updated 2026-05-13), https://mysupport.razer.com/app/answers/detail/a_id/1483/ (accessed 2026-05-17).
14. Logitech G, "G HUB Basics — Getting Started", https://www.logitechg.com/en-eu/software/guides/g-hub-basics (accessed 2026-05-17).
15. Discord Support, "Game Overlay 101" (article id 217659737), https://support.discord.com/hc/en-us/articles/217659737-Game-Overlay-101 (accessed 2026-05-17).
16. Cheat Engine Project, "Cheat Engine:Memory Scanning" (wiki, oldid=7205), https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning (accessed 2026-05-17). Included strictly for scanner-loop / colour-coded-result-list UX patterns; offensive mechanics omitted.
17. Cheat Engine Project, "Memory view (Disassembly View)" help page, https://cheatengine.org/help/Memoryview.htm (accessed 2026-05-17). Included strictly for right-click action menu and `Ctrl+G` / `Ctrl+F` / `Space` / `Backspace` / `Ctrl+Space` hotkey patterns.
18. Rick Novlesky (MSI), "MSI Afterburner Walkthrough Part 2: On Screen Display, Monitoring and Features", MSI blog, 2023-09-27, https://www.msi.com/blog/msi-afterburner-on-screen-display (accessed 2026-05-17).
19. Page Flows Team, "Game HUD Essentials: Designs for 2024", https://pageflows.com/resources/game-hud/ (published 2024-09-05; accessed 2026-05-17).
20. Andrii Honcharuk (Ubisoft, Game Designer), "My personal crusade against mini-maps and other corner based HUD elements in immersive games", Game Developer (UBM), 2017-07-14, https://www.gamedeveloper.com/design/my-personal-crusade-against-mini-maps-and-other-corner-based-hud-elements-in-immersive-games- (accessed 2026-05-17).
21. Foxglove Technologies, "Controls and shortcuts", Foxglove Docs, https://docs.foxglove.dev/docs/visualization/shortcuts (accessed 2026-05-17).
22. Foxglove Technologies, "Announcing: Image Overlays" (product release blog post), 2026-05-13, https://foxglove.dev/blog/announcing-image-overlays (accessed 2026-05-17).
23. Foxglove Technologies, "Announcing: Tabs for the Foxglove Desktop App", https://foxglove.dev/blog/announcing-tabs-for-the-foxglove-desktop-app (accessed 2026-05-17).
24. Foxglove Technologies, "Panels" docs and Rerun, "Blueprints" docs, https://docs.foxglove.dev/docs/visualization/panels and https://rerun.io/docs/concepts/visualization/blueprints (accessed 2026-05-17, also cached at `.firecrawl/foxglove-panels.md` and `.firecrawl/rerun-blueprint.md`).

---

## 9. Addendum — answers to the two operator-specific questions

### Q1. "Should live capture have its own tab, or be composited into the spatial projection canvas?"

**Recommendation: Composited into the spatial projection canvas, as the base layer of a layered Image-panel-style composite. The live feed gets its own panel only as a layout option, not as the primary access path.**

Justification, with three cited examples:

1. **Foxglove Image Overlays (May 2026)** [22] is the most direct precedent. Foxglove's own engineering blog explains exactly this problem: *"When you are debugging perception, a raw camera stream is rarely enough on its own. You also need to see what your stack thinks is in the scene: lane masks, object tracks, traversability grids, or the output of a segmentation model."* Their answer was to add `Image overlays` to the Image panel — pick a base image topic, then add overlay layers with per-layer visibility, ordering, opacity, and blend mode (`Alpha` for boxes/text, `Add` for intensity/heatmap). Before the feature shipped, the recommended pattern was *"opening two panels side by side"*, which they explicitly identified as the inferior workflow. ROC starts in 2026 with the wisdom they had to acquire: do not split.

2. **OBS Studio's source stack** [4], [7] is the proven mass-market model: one canvas, N sources stacked z-order, each with visibility / order / opacity / transform. A second display source (camera) is just another layer on top of, or under, the others. Operators don't think in "tabs" when they think about what they're streaming; they think in "what's on the canvas right now". This is *exactly* the mental model ROC operators want for "what does the model see vs what does the camera see".

3. **The HUD-design literature** [19], [20] — particularly Honcharuk's foveal/peripheral argument [20] — establishes that putting critical info in a corner forces saccades and breaks vigilance. A separate "live camera" tab is the worst case: the operator has to switch *out of* the analysis view to verify "is the feed actually live?". Anchoring detection markers directly to image-space coordinates inside the same panel is the foveal-friendly answer.

**Concrete spec for ROC.**
- Single `SpatialProjectionPanel.svelte` (renames the current `TelemetryCanvas` domain) hosts:
  - Layer 0: `camera` (raw feed via `<video>` or `<canvas>`)
  - Layer 1: `detection-boxes` (SVG, the current `trackPool` consumer)
  - Layer 2: `ekf-overlay` (predicted-vs-measured deltas, confidence ellipses)
  - Layer 3: `hierarchy-links` (parent-child polylines)
  - Layer 4: `target-lock-crosshair` (only when a lock is active)
- Each layer is independently toggleable via the panel-settings sidebar (Foxglove pattern, `,` to open per [21]), with opacity 0-100% and reorderable order.
- A separate "Raw Feed Only" layout preset (R3) exists for the case where the operator wants the camera alone (e.g., during calibration, during a model swap), but it is *not* the default and *not* a tab — it's a layout toggle accessible via `Ctrl+K` per R8.
- The Hardware Actuation panel, Kinematic Trajectory panel, and EKF Inspector dock alongside the SpatialProjectionPanel rather than replacing it; the current 4-tab paradigm becomes a 4-layout paradigm.

### Q2. "How do game CV engines handle mouse + movement tracking display in their menus?"

**Recommendation: Render the mouse / actuator-driven movement vector as a thin, world-anchored vector inside the camera panel (not in a corner widget), with the cursor itself acting as the operator's confirmation cursor only when the overlay is open. Show velocity / pan-tilt heading as a directional arrow attached to the locked target's centroid, with a short trailing path showing recent motion history.**

Justification, with three cited examples:

1. **Foxglove's 3D-panel camera controls** [21] are the closest professional analogue. They use `W/A/S/D` for translation, `Shift+W/A/S/D` for rotation, scroll for zoom, drag for pan, `Alt+drag` to engage the z-axis, `Shift+drag` / right-click-drag to rotate the camera around the target. The cursor is *the* navigation primitive; the camera is what moves, and the cursor is what the operator points with. Hotkey `1` re-centers on the target frame and `3` toggles 2D top-down vs 3D perspective — both relevant to ROC because when the pan/tilt actuator is wired in, the operator will need to switch between "world-anchored" and "actuator-anchored" reference frames mid-session. The lesson: never bury camera/cursor controls in a separate panel; bind them to the canvas they manipulate.

2. **Cheat Engine's right-click discoverability + `Space` follow-jump pattern** [17] applies directly to the cursor-on-target interaction. The disassembly-view rule "if you have a `jmp` or `call` selected, press `Space` to follow that address; `Backspace` to go back" maps onto our use case as: *if a track is selected, press `Space` to centre the actuator on it; `Backspace` to release.* This gives the operator a clean **navigation stack** for jumping between tracked targets without committing to a lock. The vendor calls out that "you can access the various menus along the top of the window, you can see a registers and flags pane to the right of the screen when a breakpoint has been hit" — the lesson is *contextual side panels open in response to the cursor*, not because the operator manually navigated there.

3. **OBS Studio's preview-window cursor semantics** [4] establish the precedent for "the cursor in the live view is functional, not decorative": drag the bounding box to move a source, `Alt+drag` to crop, `Shift+drag` to stretch, `Ctrl+drag` to disable snap, `Hold Space + drag` to pan the entire preview, `Hold Space + scroll` to zoom. Every cursor gesture is overloaded with `Alt` / `Shift` / `Ctrl` / `Space` to access additional commands without changing tools. ROC should do the same: when the spatial canvas is focused, **the cursor is the lock/zoom/jog primitive**, and modifier keys (`Alt`, `Shift`, `Space`) extend its semantics — rather than forcing the operator to switch into a "lock mode" or a "jog mode" from a menu.

**Concrete spec for ROC.**
- *Mouse cursor representation*: standard OS cursor, no custom skin. Crosshair only appears when target-lock mode is engaged (`Alt+L` per R2), at which point the cursor changes to a crosshair within the spatial canvas only. Matches Discord's overlay-mode cursor behaviour [15] where the cursor reverts to OS default outside the overlay.
- *Target movement vector*: per detected/tracked entity, render an arrow from the centroid to the EKF-predicted next position 100 ms ahead, with length = velocity magnitude scaled to a configurable display unit (per `phase_5_telemetry_ui.md`). Optional trailing line showing last 1 s of position history, opacity-fading older points (matches MSI Afterburner's text+graph mode pattern [18] where graphs scroll at FPS-relative speed).
- *Actuator-driven movement*: render the *intended* pan/tilt vector (commanded heading from the actuator controller) as a second arrow distinguishable by colour (per HUD-design visual-hierarchy principle [19]) — so the operator can see at a glance the divergence between EKF prediction and actuator setpoint. This is the same "predicted vs measured" pattern that perf-overlay tooling uses to show GPU clock target vs achieved [11], [18].
- *Cursor + actuator coupling rule (safety)*: per R5 and Risk §7, the cursor *never* directly drives the actuator. The cursor selects a track; the lock toggle commits the track; the actuator follows the EKF state of the committed track. The cursor is for *operator confirmation*; the actuator is driven by the kinematics engine. This mirrors Cheat Engine's separation between selecting an address (cursor) and committing an action (right-click menu) [17] — never one without the other.

---

> **End of subagent research deliverable.** Parent agent: please synthesise with the parallel pro-console research stream (`.firecrawl/console-research/`) before binding any of the above into `dashboard/architecture_plan.md` or `.cursor/rules/200-dashboard-telemetry.mdc`.
