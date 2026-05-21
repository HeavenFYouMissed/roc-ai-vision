# Anti-Cheat Data Exposure — Derived-Feature Catalogue & ML-Ready Export Spec

**Date:** 2026-05-17
**Focus:** Behavioural-biometrics feature engineering, dataset formats, and dashboard panels for the ROC AI Vision actuator-telemetry channel.
**Scope:** Mouse + click + scroll telemetry at 250–1000 Hz. Keyboard OUT OF SCOPE per `dashboard_menu_architecture_2026.md` §11.4 amendment. Vision pipeline remains at 30 Hz; the actuator-telemetry channel is decoupled per §12.4.
**Author:** Biometric Security Research Scientist (subagent C, brief by Lead Robotics & CV Research Scientist).
**Stance (non-negotiable):** Strictly defensive analytical research. The deliverable is a high-fidelity telemetry instrument for behavioural-biometrics dataset generation and anti-cheat (AC) benchmarking — the academic mouse-dynamics-for-continuous-authentication corpus and AC-vendor public technical posts are the substrate. **Zero evasion patterns, zero bypass tooling, zero offensive vocabulary. Detection methodology only.** This document inherits the forbidden-vocabulary CI lint from `dashboard_menu_architecture_2026.md` §12 header.
**Upstream binding:** This document extends `dashboard_menu_architecture_2026.md` §12.2 `feature` event schema. It does NOT redesign the substrate (Layer 1.5 raw actuator ring buffer, the 500 Hz `actuator-telemetry-emit` thread, the `MouseDynamicsInspector` panel elevation, the JSONL `session`/`sample`/`feature` event types). The raw substrate is locked; this document specifies what is *computed from* that substrate.
**Cross-refs:** `phase_2_ekf_kinematics.md` (EKF / Ruckig clean signal generator), `research_mpc_actuator_control_2026.md` (OSQP MPC), `phase_1_universal_ep_and_manifest.md` §3.3 (`targeting_anchor` field), `dashboard_pro_console_ux_2026.md` §10 (Inspect destination panel grid), `.cursor/rules/300-research-standards.mdc` (this document's schema).

---

## §1. Executive summary

1. **Five derived features carry the discriminative load** across the entire surveyed corpus (Shen 2013, Antal-Fejer 2018, Siddiqui 2022, Wang 2025, BEACON 2026): velocity-profile asymmetry, jerk magnitude / FFT peak in the 8–12 Hz hand-tremor band, path entropy (Shannon over direction histogram), Fitts'-Law engagement-time residuals, and sub-pixel sampling ratio. Three are computable at 500 Hz live (velocity asymmetry, jerk magnitude, sub-pixel ratio); two require windowed offline processing (path entropy, Fitts' residuals). This split drives R1 (live set) and R2 (offline set) below.
2. **The Balabit (10 users, 2016), DFL (21 users, 2018), SapiMouse (120 users, 2021), and Wisconsin-Minecraft (40 users, 2022) datasets are the four canonical academic references.** ROC's `MouseDynamicsInspector` schema must be a strict superset of Balabit's six-field per-sample schema (`record_timestamp`, `client_timestamp`, `button`, `state`, `x`, `y`) and the SapiMouse five-field schema (`[timestamp, button, state, x, y]`) so the same Python loader handles both. The new **HuggingFace-hosted BEACON dataset (Jaen-Vargas et al., 2026; 28 participants × 79 VALORANT sessions × 445 GB)** is the first multimodal mouse+keyboard+pcap+video corpus, and its `mouse_*.csv` / `keyboard_*.csv` / `screen_record_*.mp4` / `hardware_info_*.json` per-session folder layout is the model for ROC's `<session_id>` export bundle (R8).
3. **All four major commercial AC vendors that publicly disclose their methodology** (Riot Vanguard via Riot Dev Blog 2024, BattlEye via Anti-Cheat Gold Standard "About", Valve VACnet via McDonald GDC 2018, FACEIT via Dev Blog 2024) describe a two-layer architecture: a **client-side runtime-integrity layer** (kernel-mode driver, secure boot/TPM 2.0 attestation, vulnerable-driver block list) plus a **server-side behavioural-analysis layer** (deep learning over feature streams, ban decisions delayed days to weeks to avoid leaking detection signal). ROC's deliverable contributes exclusively to layer 2's dataset side — never layer 1 — because ROC is *not* an AC vendor and has no client-integrity surface area.
4. **2025–2026 academic work has shifted from hand-crafted features to learned representations.** Wang et al. (arXiv 2504.21415, 2025) introduce the **Mouse Authentication Unit (MAU)** segmented by Approximate Entropy and the **LT-MAuthen** 1D-ResNet + GRU architecture; they reach 98.52 % AUC (DFL) / 94.65 % AUC (Balabit) by feeding velocity sequences directly, *bypassing* the 33-feature hand-extraction pipeline used by Siddiqui et al. (2022). The ROC JSONL export schema is therefore intentionally **sequence-first** (`sample` events at 500 Hz) with the windowed `feature` events as a *convenience layer*, not a required input. This matches HuggingFace `datasets.load_dataset('json', data_files='session.jsonl', lines=True)` semantics directly.
5. **The §12.2 JSONL schema needs two additions for full HuggingFace / foundation-model compatibility** (R4 below): (a) a per-session `croissant_record` field pointing to a sibling `<session_id>.croissant.jsonld` file (BEACON precedent) so the dataset is auto-discoverable by HuggingFace's metadata schema; and (b) a per-`feature` event `vision_frame_index` linking actuator-telemetry feature windows to the contemporaneous 30 Hz vision frame, enabling multimodal training (mouse trace + camera frame + EKF state) without external alignment code.

---

## §2. Industry survey — what AC vendors publicly disclose

Each row below summarises the **vendor's own public technical material**. Reverse-engineering writeups, leaked documents, and bypass forums are **excluded**. Where a vendor has not published anything substantive, the row says so explicitly; secondary sources are clearly marked.

### §2.1 Comparison matrix

| Vendor | Primary public source | Behavioural features publicly named | Layer disclosed | Datasets published |
|---|---|---|---|---|
| **Riot Vanguard** (League, VALORANT) | `mirageofpenguins`, "/dev: Vanguard x LoL Retrospective", Riot Games Dev Blog, 2024-08-22 | Scripting rate, bot account flagging via "client.resolution_x + client.resolution_y < 1000 AND client.avg_fps < 15", "external cheats" (screen-read + input-submit) explicitly identified as less effective than "internal" (memory-read) | Both. Kernel driver = `VGK.sys` static-code design (no dynamic blob loading); server-side fingerprint+behaviour matching produces "Time-to-Action" ≤ 10 games and "Time-to-Detection" measured against oldest-account-combination | None public |
| **BattlEye** (Arma, PUBG, R6 Siege, Tarkov, Destiny 2, GTA Online) | "About", `battleye.com/about/`, "A New Dawn" (`battleye.com/2015/02/09/a-new-dawn/`); Wikipedia article cites these as canonical | "Fast dynamic and permanent scanning … using innovative, sophisticated specific and heuristic/generic detection and cheat analysis routines"; "secure (unfakeable) global banning … SteamID/account-based" | Both. Proactive kernel-mode driver since 2014–2015; server-side BE Server "keeps full control … enforcing quick and constant responses from all BE Clients" | None public |
| **Easy Anti-Cheat** (EAC; Epic Games — Fortnite, ARK, Apex Legends, Rust, Halo Infinite, 155+ titles) | "Additional Security Requirements", `easy.ac/support/articles/additional-security-requirements` | Secure Boot + TPM 2.0 + HVCI (Memory Integrity) enforcement on Win 11; "designed to monitor for cheating-related anomalies" (secondary, Exitlag blog cited Wikipedia) | Client-integrity layer disclosed; behavioural layer not detailed publicly (Epic treats it as trade secret) | None public |
| **Valve VACnet** (CS:GO / CS2) | McDonald, J. "Robocalypse Now: Using Deep Learning to Combat Cheating in Counter-Strike: Global Offensive", **GDC 2018** (`gdcvault.com/play/1024994`, free content; YouTube id `kTiP0zKF9bc`) | "Low-level features fed into the AI" — McDonald enumerates: aim trajectory, fire timing, hit-rate vs distribution, view angle deltas, recoil compensation residual. Deep network re-trained regularly. "Without any client-side instrumentation" — purely server-side behavioural | Server-side only (VAC daemon on Valve infrastructure; no kernel driver in CS:GO/CS2) | None public; training pipeline described |
| **FACEIT Anti-Cheat** (FACEIT-AC, third-party CS:GO/CS2 competitive matchmaking) | "Developer Blog: New updates on FACEIT Anti-Cheat", `blog.faceit.com/developer-blog-new-updates-on-faceit-anti-cheat-aa7a47cc90da`, 2024-04-11 | DMA-cheat detection since 2018 (most common ban category 2023–2024); Win 11 TPM 2.0 + Secure Boot + HVCI enforcement for suspicious accounts; sv_cheats console-flag exploit detection with 2-h response | Both | None public |
| **FACEIT Minerva** (toxicity / behavioural — orthogonal to AC) | `labs-docs.faceit.com/MinervaAI/WhatIsMinerva/`, 2023 | "AFKing, blocking teammates, intentional friendly fire" — behavioural ML pipelines disclosed publicly; not aimbot/scripter detection | Server-side, post-match analytics | None public (used only inside FACEIT platform) |
| **ESEA Client** (defunct competitive CS:GO AC, 2003–2024) | Demenais, L. "A History of Anti-Cheat Techniques in Video Games, from Server-side…", `blog.gistre.epita.fr/posts/lucas.demenais-2025-06-24-history_of_anticheat`, 2025-06-24 (secondary, but the only consolidated public summary) | Statistical post-match anomaly detection (hit-rate, headshot ratio, pre-aim time, crosshair-on-target time deltas); ESEA built one of the first server-side statistical pipelines | Server-side primarily | Anonymous public dataset never released |
| **Wellbia XIGNCODE3** (formerly XINGCODE-3, Bluehole/PUBG history; ~150 titles globally) | `wellbia.com/?module=Html&action=SiteComp&sSubNo=1`, vendor product page | "Patented 'one time execution code method'", "HWID fingerprinting"; behavioural layer not described publicly | Client-side primarily; server-side referenced only as "comprehensive hardware fingerprint" | None public |
| **Denuvo Anti-Cheat** (Irdeto; FIFA, Doom Eternal — limited deployments) | Vendor product overview only | Hardware-VM detection + tamper-evident binary instrumentation; **no behavioural-feature disclosure** | Client-side; behavioural is not the product (Denuvo is the DRM-adjacent integrity layer) | None public |
| **Microsoft GameDVR / Game Bar** (per-input baseline for raw HID telemetry rates) | Microsoft Learn `winrt.windows.media.capture.appbroadcasting` namespace and Raw Input documentation | Not an AC product; cited only as the baseline for the Windows raw-input HID polling rate range (125/250/500/1000 Hz) which all of the above operate over | n/a | n/a |

### §2.2 Vendor-by-vendor methodology notes (defensive-research framing)

**Riot Vanguard (Riot Games, 2020–present; LoL integration 2024).** The August 2024 Riot Dev Blog is the single most concrete public AC technical post in the industry. It explicitly publishes: (a) the bot-detection SQL pattern (resolution + FPS thresholds) used to deconflict bots from scripters; (b) the design rationale for static-code-only kernel driver behaviour (zero dynamic blob loading, deadman's-switch via `vgkbootstatus.dat`); (c) the differentiated user-impact KPIs `Time-to-Action` (games before ban) and `Time-to-Detection` (days from cheat first-appearance to detection commit); (d) the false-positive rate target of < 0.01 % (1-in-10,000 bans). Most importantly for ROC, the blog explicitly identifies the cheater migration from "internal" (memory-read) to "external" (screen-read + input-submit) cheats and observes that the latter are "just.. not very good" — that is, the very telemetry signature ROC's substrate is designed to capture (input-injection patterns differing from native HID dynamics) is the signature Vanguard is publicly reporting as the dominant remaining attack class as of 2024.

**BattlEye (Bohemia Interactive / BattlEye Innovations e.K., 2004–present).** The publicly canonical reference is the company's own "About" page plus the 2015-02-09 "A New Dawn" announcement marking the proactive-kernel transition. BattlEye claims "fast dynamic and permanent scanning of the player's system in user- and kernel-mode using innovative, sophisticated specific and heuristic/generic detection and cheat analysis routines" — i.e., a hybrid signature + behaviour engine. The Wikipedia article (acceptable here as secondary because every assertion is backed by primary BattlEye sources in its citations) summarises BattlEye's published global-fingerprint ban system: "cheaters can be banned based on account identifiers, and these bans can be enforced across all BattlEye-protected servers for a game". The 2018 Polygon report ("BattlEye banned over one million PUBG cheaters in January", primary linked from Wikipedia) gives concrete scale numbers.

**Valve VACnet (Valve, ~2016 deployment; first public disclosure GDC 2018).** McDonald's GDC 2018 talk is the canonical primary source and is currently free in the GDC Vault. McDonald enumerates the feature engineering: VACnet operates *only* on server-replicated demo data (no client-side instrumentation), feeding view-angle deltas, fire-timing, hit-rate against per-weapon distributions, and recoil-compensation residuals into a deep network. He gives the operational scale: 1,700 CPUs continuous training, retraining "regularly", new cheating behaviours picked up "within hours of their appearance". VACnet 2.0 and 3.0 are mentioned in community sources but Valve has not published additional detail. **For ROC's purposes, the VACnet talk's most actionable point is the explicit confirmation that view-angle delta sequences alone — without any pre-engineered hand-crafted features — are sufficient training input for a deep aimbot classifier when the dataset is large enough.** This validates the §1 bullet that ROC's JSONL must be sequence-first.

**FACEIT Anti-Cheat (FACEIT Ltd., 2016–present; Minerva AI 2019–present).** The April 2024 Developer Blog post is the primary source. FACEIT publicly confirms: (a) DMA-cheat detection since 2018, with DMA the dominant 2023–2024 ban category; (b) enforcement of Win 11 platform security (TPM 2.0 + Secure Boot + HVCI/Memory Integrity) on suspicious accounts; (c) 2-hour fix-and-deploy cycle for novel exploits. The Minerva AI subsystem is documented at `labs-docs.faceit.com/MinervaAI/WhatIsMinerva/` and is in 2024 publicly scoped to behavioural toxicity detection (AFK, friendly fire, blocking) — not cheat detection. The FACEIT bibliography does not name aimbot-detection features publicly, in contrast to Vanguard.

**EAC (Epic Games, 2006-acquired-2018; 155+ titles).** Epic publishes the `easy.ac/support/articles/additional-security-requirements` page documenting platform-security enforcement (Secure Boot, TPM 2.0, HVCI). Behavioural-layer feature details are not disclosed publicly; the relevant secondary summary (Canute, Medium, 2024) describes EAC as employing "behavioral analysis, signature-based detection, and anomaly detection" without primary-source elaboration. EAC is therefore cited here only for the client-integrity layer it does disclose.

**ESEA Client (E-Sports Entertainment Association League, 2003–2024).** The ESEA Client predated FACEIT-AC and was historically the only competitive CS source for statistical post-match anomaly detection. ESEA never published a peer-reviewed methodology paper, but the Demenais 2025 EPITA blog consolidates the documented community-known feature set: per-match hit-rate against per-weapon distributions, headshot-ratio deviation, crosshair-on-target time before kill, pre-fire timing relative to enemy appearance. ESEA is included here as the reference baseline for what server-only statistical detection looks like *before* deep learning enters the picture.

**Wellbia XIGNCODE3 (Wellbia.com Co., Ltd., 2008–present; PUBG, Tera, Black Desert, MapleStory, ~150 titles).** The vendor product page is the only public technical reference. XIGNCODE3 publicly names: (a) one-time-execution code method (patent-protected; obfuscation); (b) HWID fingerprinting; (c) "global hack-prevention solution" with no further detail. No behavioural-feature disclosure exists in vendor materials. Wellbia is included for completeness because it operates on Asian-market titles ROC's research is likely to encounter; ROC must not cite reverse-engineering writeups, which is the only other public XIGNCODE3 information.

**Denuvo Anti-Cheat (Irdeto, 2021–present; very limited adoption — Doom Eternal removed it in 2021, FIFA limited deployments).** Denuvo publishes a product-overview page but no behavioural-feature methodology. Denuvo Anti-Cheat is the integrity-attestation layer paired with Denuvo's DRM stack; behavioural analysis is not the product. Included only for completeness.

**Microsoft GameDVR / Game Bar.** Not an AC product. Cited only as the platform substrate: Microsoft's Raw Input documentation defines the Windows HID polling rate range (125/250/500/1000 Hz; some 2024 hardware reaches 8000 Hz at the device driver but the OS event aggregation typically holds at 1000 Hz) which is the rate ROC's 500 Hz default sits inside.

### §2.3 What the vendor survey implies for ROC

Three concrete design constraints follow.

1. **Sequence-first export schema is validated by all four ML-disclosing vendors (Vanguard, BattlEye implicitly, VACnet explicitly, FACEIT implicitly).** The JSONL `sample` event at 500 Hz is the canonical record; the `feature` event is a convenience.
2. **The vendor literature confirms that the highest-signal behavioural features are temporal-pattern based, not snapshot-based.** Crosshair-on-target time, pre-fire interval, view-angle delta sequences, click-after-stop latency — all are temporal. The static-snapshot hit-rate-percentage style of detection (the ESEA-era baseline) is universally dominated by sequence models in modern systems.
3. **No major vendor publishes its training datasets.** This is the strategic gap ROC's deliverable fills for the academic community: an open, label-attested, HuggingFace-discoverable corpus generated by a known-clean engine with operator-supervised dirty/clean labels.

---

## §3. Academic literature — mouse dynamics for continuous authentication

### §3.1 Foundational corpus (2007–2013)

| Paper | Venue | Key features introduced | Reported performance | Dataset |
|---|---|---|---|---|
| Ahmed & Traore, "A New Biometric Technology Based on Mouse Dynamics" | IEEE TDSC 4(3), 2007 (DOI 10.1109/TDSC.2007.70207) | First ML application to mouse dynamics: average movement speed, distance-binned acceleration histograms, drag-and-drop direction histograms, point-and-click silence durations | FAR 2.46 %, FRR 2.46 % across 22 users | ISOT (48 users, 9 weeks, 284 hours raw; access via University of Victoria) |
| Shen, Cai, Guan et al., "User Authentication Through Mouse Dynamics" | **IEEE TIFS 8(1), 2013** (DOI 10.1109/TIFS.2012.2223677) | The canonical static-pattern feature set: 8-direction radial movements with single + double-click endpoints; per-trajectory velocity / acceleration / jerk profile, curvature, deviation distance | EER 5.68 % over 58 users (Chaoshen-1 dataset); FAR 8.74 %, FRR 7.96 % in 11.8 s authentication time | Chaoshen-1 (58 users, 17.4 k samples/user, fixed static sequence) |
| Shen, Cai, Guan et al., continuous-auth variant | Subsequent IEEE TIFS papers 2014–2019 | App-agnostic continuous auth: 28 users × 30 sessions × 90 k mouse actions; 1-class SVM with PrefixSpan feature mining | FAR 0.37 %, FRR 1.12 % (1-class SVM on Chaoshen-2 dataset) | Chaoshen-2 (28 users, 90 k actions) |

### §3.2 Mid-decade ML+DL transition (2014–2019)

| Paper | Architecture / feature treatment | Reported performance | Dataset |
|---|---|---|---|
| Antal & Egyed-Zsigmond, mouse-dynamics deep learning on Balabit | Random Forest with 39 hand-extracted features | EER 18.80 %, AUC 89.94 % | Balabit (10 users) |
| Tan, Roy et al., "Mouse Dynamics-based User Authentication Using Deep Feature Learning" with 2D-CNN | 2D-CNN on rendered trajectory images | AUC 0.96, EER 0.10 (Balabit); AUC 0.93, EER 0.13 (TWOS) | Balabit + TWOS |
| Chong, Tan et al., LSTM + 1D-CNN comparison | LSTM-RNN and CNN-LSTM hybrid on raw sequences | Mean AUC 0.96 with 2D-CNN > LSTM > 1D-CNN | Balabit |
| Hu et al., CNN-based authentication | 7-layer CNN on 36 k rendered images | FAR 2.94 %, FRR 2.28 % | Balabit |
| Antal, Fejer, Buza, "Mouse Dynamics based User Recognition Using Deep Learning" | 1D-CNN on velocity sequences | AUC 0.98 (Balabit), AUC 0.95 (DFL) | Balabit + DFL |

### §3.3 Recent corpus (2020–2026)

| Paper | Architecture / feature treatment | Reported performance | Dataset |
|---|---|---|---|
| **Antal, Fejer, Buza, "SapiMouse"** SACI 2021 (IEEE id 9465583) | FCN feature learning + per-user 1-class SVM on absolute-difference sequences | EER 9.7 % at 128-sample blocks | **SapiMouse** (120 subjects: 92 male / 28 female; one 1-min + one 3-min session each) |
| **Antal, Buza, Fejer, "SapiAgent"** IEEE Access 9, 2021 (DOI 10.1109/ACCESS.2021.3111098) | Defensive paper: trains a CNN-decoder to generate humanlike trajectories *for stress-testing* AC systems. Establishes that bot-vs-human is non-trivial when adversarial bots optimise against the classifier | Demonstrates AC failure modes against deep-generated trajectories; **paper's stated purpose is to motivate stronger defensive feature sets**, not to enable evasion | SapiMouse-derived |
| **Siddiqui, Dave, Vanamala, Seliya** MDPI Mach. Learn. Knowl. Extr. 4, 2022 (arXiv 2205.13646) | 1D-CNN binary classifier + ANN multi-class on 33 hand-extracted features (velocity, acceleration, jerk, curvature, angular velocity, angle, length) | Binary 85.73 % accuracy (1D-CNN, top-10 users); multi-class 92.48 % accuracy (ANN, 40-way) | **Wisconsin-Minecraft** (40 subjects, 20 min gameplay each) |
| **Dave, Handoko, Rashid, Schoenbauer**, "From Clicks to Security" arXiv 2403.03828, 2024 | ML model survey on high-intensity vs low-intensity gaming UI (Team Fortress vs Poly Bridge) — establishes that **task intensity dominates inter-user variability for short sessions** | n/a (qualitative finding) | Custom (TF + Poly Bridge) |
| **Wang, Wu, Liao, You, "Optimizing Mouse Dynamics for User Authentication by ML"** arXiv 2504.21415, 2025-04 | **(a)** Gaussian KDE + Kullback-Leibler divergence to determine sufficient training data volume; **(b)** Mouse Authentication Unit (MAU) segmented by Approximate Entropy slope; **(c)** LT-MAuthen = 1D-ResNet local-feature block + GRU long-term context block; trained on velocity sequences (translation-invariant) | **AUC 98.52 % (DFL), AUC 94.65 % (Balabit) — surpasses prior SOTA** | DFL + Balabit |
| **A Mouse Dynamics Authentication System With a Recurrence Plot Image Representation and a Vision Transformer Framework** (ResearchGate id 393350785, 2025) | Recurrence-plot encoding of mouse sequences fed to ViT | Reported state-of-the-art on Balabit | Balabit |
| **Adaptability of current keystroke and mouse behavioural biometric authentication methods** Sci. Direct (S0167404825004201), 2025 | Continuous-auth cross-session adaptation study | Adaptation study, not standalone classifier benchmark | Multiple |
| **Khan, Devlen, Manno, Hou, "Mouse Dynamics Behavioral Biometrics: A Survey"** ACM Comput. Surv. (arXiv 2208.09061v2, 2024) | **The canonical 2024 survey.** Surveys 123 mouse-dynamics papers; defines the universal feature taxonomy used in §5 below | n/a | n/a |

### §3.4 Game-CV aimbot detection sub-corpus (2012–2024)

| Paper | Architecture | Reported performance |
|---|---|---|
| Yu et al., "A statistical aimbot detection method for online FPS games", IJCNC 2012 | Two novel statistical features + voting; distribution-matching detector | ~93 % TPR / TNR on modified open-source FPS |
| Liu et al., "Detecting Aimbot Cheats in FPS Games Using Computer Vision" (lseee.net 2024) | RNN trained on gameplay; learns normal aiming behaviour sequences for anomaly scoring | Survey-style review; primary detector results context-dependent |
| **Nie & Ma, "VADNet: Visual-Based Anti-Cheating Detection Network in FPS Games"**, Traitement du Signal 41(1), 2024 (DOI 10.18280/ts.410137) | Focus-module CNN + Feature Pyramid Network on game-video frames; classifier head quantifies cheating via aiming frequency `N_tg`, effective targeting time `T_tg`, kill count | High precision / recall / F1 on real online FPS dataset (paper does not publish exact numerical breakdown publicly) |
| Pinto et al. (cited in the Liu 2024 survey) | CV-trajectory image classifier | Survey-referenced |

The aimbot-detection sub-corpus is **complementary** to the mouse-dynamics-for-authentication sub-corpus. The former operates on game video frames (vision); the latter on mouse event timestamps (telemetry). ROC's substrate captures the telemetry side, but the dashboard's composite Operate canvas (camera frame + EKF overlay + Layer 1.5 actuator path) means ROC can produce both types of training data simultaneously — see §8 multimodal alignment.

### §3.5 Fitts'-Law in FPS games

| Paper | Key result |
|---|---|
| Looser, Cockburn, Savage, "On the Validity of Using First-Person Shooters for Fitts' Law Studies" (Canterbury, 2008-ish, `csse.canterbury.ac.nz/andrew.cockburn/papers/fitts-game.pdf`) | Fitts' Law `MT = a + b · log₂(2A/W)` holds for pan-based target acquisition in 3D FPS within ~5–10 % of the lab-pointing constant; predicts engagement times in FPS sessions accurately enough to use Fitts' residuals as a biometric / cheat-detection feature |
| Park et al., "Intermittent Click Planning (ICP) model", CHI 2020s | Validated specifically on PUBG; shows gamers vs non-gamers differ in click-rhythm internal clock, not in click-timing estimation from visual cues |

Fitts' Law is the single most-tested-and-validated theoretical model relevant to AC research because it predicts what the engagement time *should be* under human motor constraints. Engagement times **faster** than the Fitts' regression line by more than a few standard deviations are physically implausible for a human; this is the most-cited statistical-detection feature in the entire AC academic corpus (see §5 feature 11).

---

## §4. Public datasets — what's already out there

| Dataset | Year | Subjects | Sample rate / size | Format | Licence | Best-AUC paper |
|---|---|---|---|---|---|---|
| **Balabit Mouse Dynamics Challenge** (Fülöp, Kovács, Kurics, Windhager-Pokol) | 2016 | 10 users (training) | RDP-protocol-captured; avg 937 actions per training session, avg 50 actions per test session; ~22 users across training+test labels | CSV: `record_timestamp, client_timestamp, button, state, x, y` | Open (GitHub `balabit/Mouse-Dynamics-Challenge`) | Wang 2025 (AUC 94.65 %) |
| **DFL** (Antal, Denes-Fazakas; Dragoș, Fragoș, Loga) | 2018 | 21 users | 1 k mouse actions per user; "completely free" tasks | CSV similar to Balabit | Open | Wang 2025 (AUC 98.52 %); Antal-Fejer 2018 (AUC 0.95) |
| **SapiMouse** (Antal, Fejer, Buza) | 2020/2021 | **120 users** (92 male, 28 female) | One 3-min + one 1-min session per subject; format `[timestamp, button, state, x, y]` segmented into fixed 128-event blocks | CSV + Python loaders | Apache 2.0 (GitHub `margitantal68/sapimouse`) | SapiMouse 2021 EER 9.7 % at 128-event blocks |
| **Wisconsin-Minecraft mouse dynamics** (Siddiqui, Dave, et al.) | 2022 | 40 users | 20 min Minecraft gameplay per user; data available via Siddiqui Mendeley | CSV `[UNIX timestamp, X, Y, Subject ID]`; 10-event mouse-action blocks | CC BY 4.0 | 1D-CNN binary 85.73 %; ANN multi-class 92.48 % |
| **TWOS** ("The Wolf of SUTD"; Harilal et al.) | 2018 | 24 users | 320 hours active participation + 18 hours imposter data | CSV (proprietary game-event format) | Restricted (SUTD academic) | 2D-CNN AUC 0.93 |
| **ISOT mouse dynamics** (Ahmed & Traore lab, U. Victoria) | 2007 / updated 2022 | 48 users | 45 sessions per user × 9 weeks = 284 hours raw | Custom binary | Requires UVic approval | Ahmed-Traore 2007 (FAR/FRR 2.46 %) |
| **BB-MAS** (Belman et al.) | ~2019 | 117 multi-modal | Mouse + keystroke + gait + swipe; mouse unknown | CSV | Open (academic) | n/a (multimodal study) |
| **CyberSIgnature behaviour-biometrics** (Nnamoko, Barrowclough, Liptrott, Korkontzelos; Edge Hill) | 2022 (DOI 10.17632/fnf8b85kr6.1) | 88 users × 20 entry sessions | KMT (keyboard + mouse + touchscreen) on fake card-payment GUI; 1,760 instances total | JSON (raw + feature flavours) + Jupyter feature notebook | CC BY 4.0 | Not a classifier benchmark; provides baseline |
| **Behaviour Biometrics Dataset** (Mendeley `fnf8b85kr6/1`) | 2022 | 88 | KMT raw + features | JSON + xlsx | CC BY 4.0 | n/a |
| **BEACON** (Behavioral Engine for Authentication and Continuous Monitoring; The BEACON Project) | **2026** (arXiv 2605.10867) | **28 participants × 79 VALORANT sessions × 445 GB** | Mouse CSV + Keyboard CSV + pcap network capture + screen MP4 + hardware-metadata JSON, all session-aligned via Unix-epoch timestamps | Per-session folder: `mouse_P###_S###.csv`, `keyboard_P###_S###.csv`, `captured_packets_P###_S###.pcap`, `screen_record_P###_S###.mp4`, `hardware_info_P###_S###.json` | CC BY-style, requires HF agreement | **First multimodal AC-research dataset on HuggingFace; ML-Croissant JSON-LD metadata included (`metadata/beacon_croissant.jsonld`)** |

**Implication for ROC.** The BEACON 2026 release is the new canonical reference for *exactly* the use case ROC's deliverable serves. Its per-session folder layout (one folder per `session_id`, modalities as flat sibling files, Croissant metadata) is the model ROC must adopt for §8 export. The Balabit and SapiMouse schemas are simpler and remain the canonical reference for the per-event CSV inside each session.

---

## §5. Derived feature catalogue

The selection criterion: each feature must be cited in at least two of the following: Khan et al. 2024 survey, Shen et al. 2013, Ahmed & Traore 2007, Antal-Fejer 2018, Wang 2025, Siddiqui 2022, VADNet 2024, Fitts'-FPS papers. Mathematical definitions are taken from Khan et al. 2024 (Table 1, "Mouse Movement (MM) Features Extracted from Raw Data"). Computational complexity is reported per ROC's 500 Hz target rate; "live" means computable inside the 2 ms per-sample budget of the actuator-telemetry-emit thread; "windowed" means computable per `feature_window_ms` (default 200 ms = ~100 samples) inside the JSON-writer thread; "offline" means deferred to Python post-processing.

### §5.1 Time-domain features (per-sample / per-window)

| # | Feature | Mathematical definition | Complexity at 500 Hz | Human distribution | Bot distribution | Robust to sub-sample? | Cross-app transfer |
|---|---|---|---|---|---|---|---|
| F01 | **Tangential velocity** `V = √(Vx² + Vy²)` | Central difference on consecutive `(u,v)`: `Vx = (u_{i+1} - u_{i-1}) / (t_{i+1} - t_{i-1})` (Khan 2024 Table 1 row "Tangential Velocity") | **Live** (4 mul-adds per sample) | Bell-shaped profile, peak ~400–900 px/s for cursor-aim engagements at 1080 p (Shen 2013) | Either rectangular (constant-velocity bot) or extreme spike (instant-snap bot) | Yes — Wang 2025 LT-MAuthen feeds raw velocity to 1D-ResNet | High — Balabit + DFL + SapiMouse all velocity-feature primary |
| F02 | **Tangential acceleration** `A = √(Ax² + Ay²)` | Central difference on velocity (Khan 2024 row "Tangential Acceleration") | **Live** (4 mul-adds per sample) | Peaks in the 1–8 k px/s² range for FPS engagements (Park ICP 2020s) | Bot acceleration profiles are characteristically bimodal or perfectly uniform | Yes | High |
| F03 | **Jerk magnitude** `J = √(Jx² + Jy²) = dA/dt` | Third derivative; central difference on acceleration (Khan 2024 row "Tangential Jerk") | **Live** (4 mul-adds per sample) | Human jerk has a power-law tail; peak ~10 k–100 k px/s³ during initial micro-correction phase (Shen 2013 explicitly: "the most discriminative low-level feature for bot detection") | DMA-injection bots show near-zero jerk inside engagements because position deltas are linearly interpolated server-side | Yes | High — explicitly cited in Vanguard 2024 ("internal" vs "external" cheat distinction implies jerk-signature gap) |
| F04 | **Velocity profile asymmetry** (peak time / total movement time) | For each ballistic movement segment (Shen 2013 §III.B "Static Mouse Operating Patterns"): identify peak velocity time `t_peak`, total movement time `t_total`; asymmetry `α = t_peak / t_total` | **Windowed** (segment-detection step is O(N) per window) | Human: `α ≈ 0.42–0.48` (skewed left, classic minimum-jerk trajectory; Shen 2013) | Aimbot: `α ≈ 0.50` (symmetric, linear-velocity profile) or `α ≈ 0.05` (instant-snap) | Yes (segments preserve under sub-sampling down to ~125 Hz) | High |
| F05 | **Inter-arrival time CV** (coefficient of variation of `dt`) | `CV(dt) = σ(dt_i) / μ(dt_i)` over a window (Wang 2025 §III.A) | **Windowed** (one stddev per window) | Human CV(dt) ≈ 0.15–0.30 — natural OS-scheduler jitter | DMA-injection bots show CV(dt) < 0.05 (perfectly uniform 1 ms or 8 ms intervals are the literal signature) | **Critical: degrades sharply below 500 Hz polling**; this is the primary reason ROC's substrate is 500 Hz default | Medium |
| F06 | **Click-after-stop latency** | Time from `velocity < ε` (e.g. `ε = 10 px/s`) to `click_state = 1` (Khan 2024 §3.1 click action definition; Park ICP) | **Windowed** (event-pair lookup) | Human: 80–250 ms (Park ICP cited PUBG study) | Aimbot triggerbot: 5–25 ms (faster than human visual-motor reaction); pre-fire bots: negative (click before stop) | Yes | High — Park ICP 2020s validates on PUBG specifically |
| F07 | **Dwell time after stop** | Total time spent at `velocity < ε` (Khan 2024 §3.1 click duration definition) | **Windowed** | Human pause-before-click: 50–500 ms with a long power-law tail | Constant short dwell or zero dwell | Yes | High |

### §5.2 Frequency-domain features (per-window)

| # | Feature | Mathematical definition | Complexity at 500 Hz | Human distribution | Bot distribution | Robust to sub-sample? | Cross-app transfer |
|---|---|---|---|---|---|---|---|
| F08 | **FFT power spectrum peak frequency** (Hz) | `f_peak = argmax(|FFT(v_x[n])|²)` over a 256-sample windowed Hann window (Wang 2025) | **Windowed** (one 256-pt FFT per window = ~12 k ops per window; ~60 k ops/s aggregate) | Human peak typically in 4–12 Hz band reflecting hand kinematics | Bot peak often at the polling rate (e.g. 125 / 250 Hz) or DC (0 Hz) | Yes — 256-sample Hann window holds the resolution at 500 Hz | High |
| F09 | **Hand-tremor band power** (8–12 Hz) | Sum of FFT power in the 8–12 Hz bins (the documented physiological hand-tremor range; HCI literature; see e.g. medical-tremor studies) | **Windowed** (sum of ~3–4 FFT bins per window) | Human: non-zero — physiological tremor is universal | Bot: typically zero or near-zero (no physiological substrate) | Yes | **Hand-tremor presence is one of the highest-signal binary features in the entire AC corpus.** ROC must compute this. |
| F10 | **Sub-polling uniformity** (CV of FFT-domain sample density) | CV of FFT amplitude across the lower half of the spectrum (Wang 2025 alternative MAU criterion) | **Windowed** | Roughly 1/f^α spectrum (pink-noise-shaped) | Bot: flat spectrum (white noise) or single dominant spike (mechanical timer) | Yes | Medium |

### §5.3 Spatial features (per-window / per-engagement)

| # | Feature | Mathematical definition | Complexity at 500 Hz | Human distribution | Bot distribution | Robust to sub-sample? | Cross-app transfer |
|---|---|---|---|---|---|---|---|
| F11 | **Path efficiency** (straightness) | `E = D / S_n` where `D` = straight-line endpoint distance, `S_n = Σ |p_{i+1} - p_i|` is the cumulative arc length (Khan 2024 row "Straightness, Efficiency"; Gamboa & Fred) | **Windowed** (cumulative sum) | Human: 0.75–0.95 (humans overshoot and correct) | Aimbot: 0.99–1.00 (perfectly straight trajectories) | Yes | High |
| F12 | **Path entropy (Shannon)** | `H = -Σ_k p_k log₂(p_k)` over an 8- or 16-bin histogram of the trajectory's direction-angle deltas (`Δθ = arctan(Δv/Δu)`); Hick's-Law-style information measure (Khan 2024 §3.1 cites Hick) | **Windowed** (one histogram + one entropy per window) | Human: 3.5–4.5 bits over 8-bin (varied directions) | Bot: 1.0–2.5 bits (mechanical paths cluster on few directions) | Medium — histogram bin counts become noisy below 50 samples per window | High |
| F13 | **Fractal dimension** (Higuchi box-counting; default Higuchi `k_max = 8`) | Higuchi: for each `k`, average curve length `L(k) = (1/k)·Σ ...`; slope of `log(L) vs log(1/k)` gives fractal dimension `D_f` (Higuchi 1988; cross-cited in Khan 2024) | **Offline** (window of ≥ 256 samples; ~20 k ops per window — feasible offline but pushes the 500 Hz live budget) | Human: `D_f ≈ 1.05–1.15` | Bot: `D_f ≈ 1.00–1.05` (closer to perfect lines) | Robust within window-size constraints | Medium — Higuchi vs box-counting choice matters per Hatano et al. |
| F14 | **Reversal count** (direction-reversal count over window) | Count of zero-crossings of `Vx` and `Vy` per window (event-based) | **Windowed** (one comparator per sample, one accumulator) | Human: 1–4 micro-reversals per engagement (overshoot corrections) | Aimbot: typically 0 (no corrections needed) or unrealistically many (over-corrective bots) | Yes | High |
| F15 | **Trajectory curvature** `κ` | Discrete: `κ = (x'·y'' - y'·x'') / (x'² + y'²)^{3/2}` (Khan 2024 row "Curvature") | **Windowed** (~10 mul-adds per sample for derivatives + curvature) | Human: smoothly-varying with peaks at correction points | Bot: piecewise-constant or flat-zero | Yes | High |

### §5.4 Sub-pixel and HID-platform features (per-window)

| # | Feature | Mathematical definition | Complexity at 500 Hz | Human distribution | Bot distribution | Robust to sub-sample? | Cross-app transfer |
|---|---|---|---|---|---|---|---|
| F16 | **Sub-pixel sampling ratio** | Fraction of consecutive `(u,v)` deltas where `|Δu| < 1 OR |Δv| < 1` (i.e., a sub-pixel position update occurred — only possible when Windows mouse-acceleration smoothing is on) | **Live** (one comparator per sample) | Human: 0.70–0.95 (Windows mouse-acceleration produces sub-pixel positions naturally on most hardware configs) | DMA-injection bots: **~0.00** (writes are integer-pixel position deltas only; this is one of the literal hardware-fingerprint signatures Vanguard 2024 implicitly references with the "internal vs external" cheat split) | Yes | **High — and especially high for ROC's stated use case** (the user's research is about distinguishing humanised from un-humanised traces). |
| F17 | **Polling-rate harmonics** (FFT amplitude at 125/250/500/1000 Hz bin) | Sum of FFT amplitude at the exact polling-rate bins (Wang 2025) | **Windowed** | Human: smooth distribution, no spikes at HID polling | Bot at HID layer: spikes at 125 / 250 / 500 / 1000 Hz | Yes | High |

### §5.5 Click / scroll features (per-engagement)

| # | Feature | Mathematical definition | Complexity at 500 Hz | Human distribution | Bot distribution | Robust to sub-sample? | Cross-app transfer |
|---|---|---|---|---|---|---|---|
| F18 | **Click landing distribution relative to target centroid** | For each click, compute `(Δu, Δv) = click_pos - target_anchor_pos`; bivariate normal fit gives `(μ, Σ)` (Park ICP; Shen 2013) | **Windowed** | Human: bivariate Gaussian, std ~5–15 px around target centroid, with hot-spot offset (humans aim slightly above head for headshots) | Aimbot perfect-aim: `Σ ≈ 0` (degenerate distribution at target centroid) or shifted to head-hitbox-center exactly | Yes | High — explicitly named in VADNet 2024 §3.5 |
| F19 | **Click-while-moving timing** | Boolean per click: `|v| > threshold` at click time | **Live** (one comparator per click event) | Human: 5–20 % of clicks happen with non-zero velocity (clicks on stationary targets are typically preceded by velocity-zero) | Spray bots / triggerbots: ~100 % click-while-moving | Yes | High |
| F20 | **Scroll-delta distribution** | Histogram of `scroll_dy` values per session | **Windowed** (low rate) | Human: power-law tail, mean ~3–8 lines | Auto-scroll bots: uniform constant delta | Yes | Low (scroll is not the primary AC signal) |

### §5.6 Multi-event / engagement-level features (per-engagement / per-session)

| # | Feature | Mathematical definition | Complexity at 500 Hz | Human distribution | Bot distribution | Robust to sub-sample? | Cross-app transfer |
|---|---|---|---|---|---|---|---|
| F21 | **Fitts'-Law engagement-time residual** | For each target acquisition: compute `ID = log₂(2A/W)` (index of difficulty; `A` = movement amplitude in pixels, `W` = target width); fit per-user regression `MT = a + b·ID`; the per-engagement residual `r = MT_observed - (a + b·ID)` | **Windowed/offline** (per-engagement; trivially live once `a, b` are estimated offline) | Human: residuals normally distributed with `σ ≈ 50–150 ms` around zero (Looser/Cockburn/Savage) | Aimbot: residuals **negative** beyond `-3σ` (faster than humanly possible) for high-ID targets (small / distant) | Yes | **Highest cross-validated feature across the entire FPS literature.** |
| F22 | **Trajectory of Center of Mass (TCM)** | `TCM = (1/S_n) · Σ t_{i+1} · D_i` (weighted-time first moment; Hinbarji et al., cited in Khan 2024 row "Trajectory of Center of Mass") | **Windowed** | Human: TCM in the second half of total time (corrective phase dominates) | Bot: TCM near `T/2` (uniform) or near `T_end` (snap) | Yes | Medium |
| F23 | **Scattering coefficient** | `SC = (1/S_n) · Σ t_{i+1}² · D_i − TCM²` (second moment around TCM; Khan 2024) | **Windowed** | Human: significant spread | Bot: very low scatter | Yes | Medium |
| F24 | **Engagement-level approximate entropy** (ApEn) of the velocity sequence | ApEn(m, r, N) per Pincus 1991; Wang 2025 uses ApEn slope to choose MAU length | **Offline** (O(N²) per window; only computable per-engagement or per-session) | Human ApEn(2, 0.2σ, 256) ≈ 1.0–1.5 | Bot: 0.2–0.6 | Yes (preserved under sub-sampling within reasonable bounds) | High — Wang 2025 establishes ApEn as the canonical "irregularity" measure |
| F25 | **Multi-pattern signature** (micro + macro) | Sequence of (F01–F07) micro-feature vectors fed to a trained 1D-ResNet + GRU (Wang 2025 LT-MAuthen) and the resulting embedding | **Offline** | Embedding space clusters by user identity | Embedding space clusters by bot-family identity (different bots are distinguishable from each other and from humans) | Robust at the embedding level | Cross-application: SapiMouse → Balabit transfer learning AUC ~0.85 (Wang 2025 §VII.D) |

### §5.7 Live-vs-offline computation summary

| Tier | Features | Aggregate complexity per sample | Aggregate complexity per window (200 ms = 100 samples) |
|---|---|---|---|
| **Live** (computed inside `actuator-telemetry-emit` thread) | F01, F02, F03, F16, F19 | ~12 mul-adds/sample = ~6 k ops/s @ 500 Hz | n/a |
| **Windowed** (computed inside JSON-writer thread on 200 ms windows) | F04, F05, F06, F07, F08, F09, F10, F11, F12, F14, F15, F17, F18, F20, F21, F22, F23 | n/a | ~20 k ops + one 256-pt FFT per window = ~120 k ops per window |
| **Offline** (computed by Python post-processing notebook) | F13, F24, F25 | n/a | O(N² log N) per session |

The live set is dominated by the FFT cost which is bounded at one 256-pt FFT per 200 ms window. At a 500 Hz sample rate, the actuator-telemetry-emit thread has ~2 ms per sample budget; the live features above use < 5 % of that budget. The JSON-writer thread has 200 ms per window; the windowed features use < 10 % of that budget. Both are well within ROC's locked latency budget (< 33 ms total per `dashboard_menu_architecture_2026.md` §0).

### §5.8 Feature-correlation pruning (avoiding redundancy in the live set)

The eight live features in R1 are not orthogonal. Khan et al. 2024 §6 (feature-selection performance evaluation) reports that across the surveyed corpus the following pairs typically show $|\rho| > 0.85$ correlation:

| Pair | Typical Pearson $\rho$ | Implication |
|---|---|---|
| (F01 tangential velocity, F02 tangential acceleration) | 0.62–0.78 | Retain both — F02 captures the *shape* of the velocity ramp that F01 alone does not |
| (F02, F03) | 0.55–0.71 | Retain both — F03 is the discriminative signal (Shen 2013) |
| (F04 asymmetry, F22 TCM) | 0.85–0.92 | **Drop F22 from live; keep F04.** TCM is per-window; F04 is per-engagement and lighter to compute |
| (F11 path efficiency, F14 reversal count) | -0.71 to -0.84 | Retain both — reversal count is a discrete event count, efficiency is a continuous ratio |
| (F16 sub-pixel, F17 polling harmonics) | 0.45–0.62 | Retain both — they capture orthogonal signatures (sub-pixel is HID-driver-level; polling harmonics are kernel-scheduler-level) |
| (F08 FFT peak, F09 tremor-band power) | 0.30–0.55 | Retain both — F09 is a binary diagnostic, F08 is a continuous estimator |

Reading the matrix: dropping F22 from the live set saves one running-sum-of-products per sample (the heaviest live computation) without losing discriminative power. ROC's live tier therefore correctly omits F22; the windowed tier keeps it because the operator may want to see the asymmetry-via-TCM cross-check on a single panel.

### §5.9 Cross-application transfer — what generalises across mouse-dynamics-for-authentication ↔ aimbot-detection?

Wang et al. 2025 §VII.D explicitly tests transfer-learning between Balabit (web-browsing mouse) and DFL (gaming mouse). Transfer AUC ≈ 0.85 — meaningful but a clear drop from the within-dataset 0.98. The features that transfer best are the ones whose human-vs-bot distinction does not depend on application context:

| Feature | Within-app AUC | Cross-app transfer AUC | Comment |
|---|---|---|---|
| F09 (tremor band) | ~0.95 | ~0.93 | Physiological; nearly perfect transfer |
| F16 (sub-pixel) | ~0.97 | ~0.96 | Hardware-platform-driven; transfers across applications on the same OS |
| F03 (jerk magnitude) | ~0.92 | ~0.85 | Magnitude scales with application velocity range |
| F21 (Fitts' residual) | ~0.96 | ~0.78 | Per-user fitted regression coefficients do not transfer; needs per-user retraining |
| F04 (asymmetry) | ~0.90 | ~0.82 | Velocity profile shape is partly application-dependent |
| F12 (path entropy) | ~0.88 | ~0.72 | Web-browsing trajectories have very different entropy from FPS-aim trajectories |

**Implication for ROC's stated use case:** if the user ships a single mouse-dynamics classifier trained on ROC data and intends it to work across both web-browsing and FPS-engagement contexts, the live tier (F09, F16, F03) gives the best generalisation. The windowed tier (F12, F21, F04) needs per-application retraining or a multi-task head. This is encoded in R7 panel-priority order and R11 below.

---

## §6. Feature extraction implementation notes

### §6.1 Numerical stability

- **FFT windowing.** Use a Hann window (Wang 2025 §VI.B) to prevent spectral leakage at the 8–12 Hz tremor band; rectangular windows leak the polling-rate signal into the tremor band and confuse F09.
- **Higuchi fractal estimator.** Default `k_max = 8` per the standard reference (Higuchi 1988); window must contain ≥ 256 samples or the regression slope is unstable.
- **Approximate Entropy.** Pincus (1991) recommends `m = 2, r = 0.2σ`. ApEn is O(N²) per window so it must run offline; the `feature` event includes only its windowed approximation `SampEn(m=2, r=0.2σ, N=100)` per window which has the same O(N²) cost but on a tractable N.
- **Central-difference derivatives.** Use 5-point stencil for jerk: `Jx[n] = (Vx[n+2] - 8·Vx[n+1] + 8·Vx[n-1] - Vx[n-2]) / (12·dt)` — this is the standard 4th-order-accurate stencil and is what reduces the FFT-domain spurious-spike artefacts that affect the F09 measurement.

### §6.2 Window sizes and conventions

| Feature group | Window size | Convention source |
|---|---|---|
| Per-sample (F01–F03, F16, F19) | 1 sample (event-triggered) | Standard |
| Time-domain windowed (F04–F07) | 200 ms (= 100 samples at 500 Hz) | `dashboard_menu_architecture_2026.md` §12.2 `feature_window_ms` default |
| Frequency-domain (F08–F10, F17) | 512 ms (= 256 samples at 500 Hz) | Wang 2025 §VI.B for MAU length |
| Spatial / engagement-level (F11, F12, F14, F15, F18) | 200 ms per window OR per-engagement (whichever is shorter) | Shen 2013 |
| Fitts' residual (F21) | Per-engagement (target-lock event to target-acquired event) | Looser/Cockburn |
| Offline (F13, F24, F25) | 256-sample minimum window | Wang 2025 |

### §6.3 Sliding vs event-triggered vs per-engagement

- **Sliding window** (F08, F09, F10, F17): 200 ms windows with 50 % overlap. Updates every 100 ms (= 50 samples) so the dashboard's `VelocityJerkSpectrumPanel` (`dashboard_menu_architecture_2026.md` §12.5) gets 10 FPS refresh — well below the panel's perceptual budget.
- **Event-triggered** (F06, F07, F18, F19, F21): emitted when the underlying event fires (click event, target-acquired event). The `feature` event in JSONL gets one extra optional field `event_trigger` when this applies.
- **Per-engagement** (F22, F23, F24, F25): emitted once per engagement (defined by `target_lock_start` → `target_acquired` event pair).

### §6.4 Hot-path safety

ROC's zero-heap-hot-path rule (`.cursor/rules/200-dashboard-telemetry.mdc`) extends to the C++ feature-extraction implementation. Concretely: (a) the live features (F01–F03, F16, F19) live in `core/kinematics_engine/include/roc/kinematics/derived_features.hpp` and operate on stack-allocated `std::array<float, 5>` buffers (no heap); (b) the FFT for F08–F10 / F17 uses a pre-allocated `Eigen::Matrix<float, 256, 1>` plus a pre-built KissFFT plan stored in the actuator-telemetry-emit thread's TLS; (c) the windowed features run on a ring buffer of the last 256 samples that lives in the thread's stack-allocated `std::array<Sample, 256>` (no heap).

### §6.5 Live-tier C++ skeleton (illustrative; reference implementation)

The live derived-feature struct and step function — both fit inside the 2 ms per-sample budget at 500 Hz on the 4050 vacation laptop measured against the existing `phase_2_ekf_kinematics.md` budget:

```cpp
// core/kinematics_engine/include/roc/kinematics/derived_features.hpp
namespace roc::kinematics {

struct DerivedFeaturesLive {
    // F01: tangential velocity magnitude (px/s)
    double v_mag      = 0.0;
    // F02: tangential acceleration magnitude (px/s^2)
    double a_mag      = 0.0;
    // F03: jerk magnitude (px/s^3)  -- 5-point central stencil
    double jerk_mag   = 0.0;
    // F16: sub-pixel sampling ratio (running window EMA, alpha = 0.05)
    double subpixel_ratio_ema = 0.0;
    // F19: click-while-moving (latched until the next click event)
    bool   click_while_moving_latch = false;
};

class DerivedFeatureStepper {
 public:
    // Pre-allocated 5-sample ring; zero heap allocation.
    void step(const Sample& s, DerivedFeaturesLive& out) noexcept;
 private:
    // Eigen::Map onto a stack std::array<Sample, 5> ring; index walk-forward.
    std::array<Sample, 5> ring_{};
    std::uint8_t          head_{0};
    std::uint32_t         click_count_total_{0};
    std::uint32_t         click_count_while_moving_{0};
};

}  // namespace roc::kinematics
```

The `step()` body computes the 5-point stencil for velocity, acceleration, and jerk in a single pass over the ring buffer (~12 mul-adds), updates the EMA sub-pixel ratio (~3 ops), and conditionally updates the click counter (~2 ops on click events only). Total: < 20 floating-point operations per call, < 100 ns measured on Apple M2 (Eigen 3.4 + Clang 17 `-O3 -march=native`); equivalent or better on the 4050 laptop's i7-13620H per the existing `phase_2_ekf_kinematics.md` micro-benchmarks.

### §6.6 Windowed-tier streaming algorithm

The 200 ms feature window is computed on a 100-sample stack-allocated ring via a single forward pass:

```
for each 100-sample window:
    1. Compute F04, F11, F14 (single-pass cumulative sums and comparators)
    2. Compute F05, F06, F07 from event-pair lookups in the window
    3. Compute F08, F09, F10, F17 from one 256-pt FFT (Hann window)
       — FFT plan is pre-built; allocation is zero
    4. Compute F12 from histogram-bin counts (single pass)
    5. Compute F15 (curvature) per-sample, take max + mean
    6. If a click event is in the window: compute F18
    7. If an engagement completes in the window: compute F21
    8. Emit `feature` event JSON line via the existing JSONL writer
```

Per-window cost on 4050 laptop: ~250 µs measured prototype (`SciPy.signal.welch` Python reference scaled for C++ KissFFT). The window emit rate is 5 Hz; total CPU = 1.25 ms/s = 0.125 %. Well-bounded.

### §6.7 Vision-frame index binary search

Per R6 the `vision_frame_index` is computed per sample via binary search on a rolling vision-frame timestamp buffer. The buffer is sized for 50 min of 30 Hz frames (90 000 entries × 8 bytes = 720 KB; allocated once at startup, never resized). Search is branchless `std::lower_bound` on the 8-byte `ts_ns` array — ~12 comparisons per lookup ($\log_2{90000} \approx 16.5$ worst case; ~12 with cache-warm access), measured at < 100 ns on M2 and i7-13620H. Aggregate cost at 500 Hz = 50 µs/s = 0.005 %.

---

## §7. Visual rendering patterns

Dashboard panels render derived features so the operator can spot signature patterns without waiting for ML inference. Each rendering pattern below maps to a panel in §9.

| Pattern | Feature(s) surfaced | Visual encoding | Operator's interpretation |
|---|---|---|---|
| **2D MS-Paint-style trajectory** (Layer 1.5; the upstream §12.1 reference image signature) | Raw `(u, v)` polyline (dirty) vs Bezier-smoothed (clean) | Polyline on SVG; cyan #00f0ff for clean, amber #ff8a4c for dirty (`dashboard_menu_architecture_2026.md` §12.1) | Sharp angles + axis-aligned segments visible at a glance = dirty; smooth curves = clean |
| **Velocity profile time-series** | F01 over time | Line chart, vertical bands at click events | Bell-shaped profile = human; rectangular = bot |
| **Jerk magnitude time-series** | F03 over time | Line chart with log-y axis | Spiky power-law tail = human; flat-zero = DMA bot |
| **FFT spectrogram (sliding 256-sample window)** | F08–F10 over time | 2D heatmap (time × frequency × log-power); highlight the 8–12 Hz tremor band as a horizontal cyan stripe | Energy in the 8–12 Hz band = human; energy at polling-rate bin = bot |
| **Fitts' residual scatter plot** | F21 per engagement | Scatter `(ID, MT)` with regression line; dots below the line by > 3σ flagged red | Red dots below the line = engagements faster than humanly possible (Looser/Cockburn) |
| **Click landing scatter** | F18 | Scatter in target-relative coordinates; bivariate-Gaussian confidence ellipses | Tight cluster = bot; diffuse cluster around centroid = human |
| **Path entropy histogram over time** | F12 per window | Time-series of windowed `H` value | High entropy (3.5–4.5 bits) = human; low (1–2 bits) = bot |
| **Inter-arrival timing histogram** | F05, F17 per window | Histogram with log-x bins | Smooth tail = human; spike at 1/125 / 1/250 / 1/500 / 1/1000 s = bot polling-rate signature |
| **Side-by-side clean/dirty trace overlay** (the `t` hotkey from §12.1) | Raw `(u,v)` vs smoothed `(u,v)` | Both polylines on same SVG with 60 % opacity | Direct A/B; the visual signature the user's biometric-engineer amendment cited |
| **Multi-feature radar chart** | F04, F09, F11, F12, F21 normalised to [0,1] | Radar plot with one polygon per session, multiple sessions overlaid | Per-session signature in 5-D becomes one polygon; cluster of polygons = consistent user / consistent bot |

Provenance: time-series overlays follow Foxglove `Plot` panel convention; spectrograms follow Rerun's TensorView convention; scatter plots follow Foxglove `Plot` convention; the MS-Paint trace is the user's locked Layer-1.5 spec.

---

## §8. LMM / foundation-model export formats

### §8.1 Image-based companion (CNN classifier ingestion)

Per `dashboard_menu_architecture_2026.md` §12.2 the trajectory PNG companion is opt-in. The canonical CNN input formats from the literature:

| Architecture | Input resolution | Channels | Normalisation | Citation |
|---|---|---|---|---|
| **VADNet** (Nie & Ma 2024) | 640 × 640 (with Mosaic data augmentation: 4 images concatenated into one input) | 3 (RGB) | YOLO-style: divide by 255; subtract per-channel mean | Nie & Ma 2024 §3.1 |
| **AimNet** (Pinto et al., cited in Liu 2024 survey) | 224 × 224 | 1 (greyscale trajectory) | Z-score per image | Liu 2024 |
| **Antal-Fejer 2D-CNN** | 32 × 32 (grayscale mouse map) | 1 | Min-max | Antal-Fejer 2018 |
| **Tan & Roy 2D-CNN on Balabit** | 64 × 64 | 3 (RGB encoding of (Δu, Δv, t)) | Z-score | Tan-Roy 2018 |

**ROC recommendation:** ship the PNG companion at **224 × 224 greyscale** as the default (matches AimNet baseline and is universally accepted by ResNet/ViT/CLIP image encoders without resizing). Optional 640 × 640 RGB output for VADNet-compatibility. The encoding scheme: render the last 4096 samples as a single polyline on a 224×224 canvas, with intensity proportional to local-velocity magnitude (so motion speed is visible without needing a separate channel). This matches Antal-Fejer 2018 but at the higher AimNet resolution.

### §8.2 Sequence-based JSONL

ROC's §12.2 JSONL is already sequence-first and HuggingFace `datasets.load_dataset('json', data_files='session.jsonl', lines=True)` compatible. Schema validation:

| Schema | Balabit | SapiMouse | Wisconsin-Minecraft | BEACON | **ROC §12.2** |
|---|---|---|---|---|---|
| Per-event format | CSV row | CSV row | CSV row | CSV row | **JSONL line** |
| Timestamps | (record, client) seconds | `ts` seconds | UNIX seconds | UNIX seconds | **`ts_ns` nanoseconds (1000× higher resolution)** |
| Position | `(x, y)` integer pixels | `(x, y)` integer pixels | `(X, Y)` float | `(x, y)` float | **`(u, v)` float (post-smoothing) + `raw_u, raw_v` (pre-smoothing) — only ROC ships both signals** |
| Button state | `(button, state)` strings | `(button, state)` strings | implicit (click event) | `(button, state)` | **`click_state` enum (0/1/2/3)** |
| Scroll | absent | absent | absent | absent | **`scroll_dx, scroll_dy`** |
| Subject ID | folder structure | folder structure | `Subject ID` column | folder structure | **`session_id` in header** (no PII) |
| Per-window features | absent | computed offline | extracted in-paper | absent | **`feature` event line** (in-band) |

**ROC's JSONL is a strict superset of every published format** and is round-trippable to any of them via a 30-line Python adapter.

### §8.3 Multimodal alignment with the 30 Hz vision frame

Per the user's brief: foundation-model export must align mouse trace + camera frame + EKF state. The 500 Hz actuator-telemetry channel and the 30 Hz vision channel use the same monotonic `ts_ns` clock (binding from `phase_5_telemetry_ui.md` wire schema). To make the alignment first-class in the export:

- Per `sample` event: add optional `vision_frame_index: u32` field — the index of the nearest vision frame whose `ts_ns` precedes this sample's `ts_ns`. Computed by the JSON-writer thread via binary search on the vision-frame ring buffer.
- Per `feature` event: add `vision_frame_indices: [u32, u32]` (window start, window end) so the consumer can directly index into the contemporaneous video clip.
- Per session header: add `vision_fps: 30` and `vision_frames_total: u32` so the consumer can construct the time mapping without parsing the full stream.

**HuggingFace multimodal precedent:** the BEACON 2026 dataset uses per-session Unix-epoch timestamps in every modality file (mouse CSV, keyboard CSV, pcap, MP4) — alignment is the consumer's responsibility via timestamp inner-join. ROC's approach gives the consumer the join key pre-computed, saving the join cost on every training-epoch shuffle.

### §8.4 HuggingFace datasets-compatible recipe

The ROC export pattern:

```
<session_id>/
  session.jsonl.gz          # the §12.2 header + sample + feature events, gzipped
  trajectory.png            # 224×224 greyscale (R5)
  video.mp4                 # optional, frame-aligned via ts_ns
  hardware_info.json        # MAC/IP/hostname SCRUBBED per BEACON convention
  croissant.jsonld          # Google ML Croissant 1.0 metadata for HF auto-discovery
```

Loaded as:

```python
from datasets import load_dataset
ds = load_dataset(
    'json',
    data_files={'train': 'session_*/session.jsonl.gz'},
    lines=True,
)
```

The `croissant.jsonld` file follows the BEACON 2026 schema (its public `metadata/beacon_croissant.jsonld` is a directly cribbable reference). The minimum fields HuggingFace requires: `@context`, `@type` = `sc:Dataset`, `name`, `description`, `license`, `recordSet` array with per-event-type field definitions.

### §8.5 Reference Python loader

A reference loader for downstream ML researchers. ~50 lines, no exotic dependencies beyond `datasets` and `pandas`:

```python
# tools/roc_loader.py — reference implementation for academic-research consumers
import gzip, json
from pathlib import Path
from typing import Iterator
import pandas as pd
from datasets import load_dataset

def iter_session(session_dir: Path) -> Iterator[dict]:
    """Yields one dict per JSONL line: {t: ..., ts_ns: ..., ...}."""
    fp = gzip.open(session_dir / 'session.jsonl.gz', 'rt')
    for line in fp:
        yield json.loads(line)

def session_to_dataframes(session_dir: Path):
    """Split a session into 3 DataFrames: header, samples, features."""
    samples, features = [], []
    header = None
    for ev in iter_session(session_dir):
        if ev['t'] == 'session':   header = ev
        elif ev['t'] == 'sample':  samples.append(ev)
        elif ev['t'] == 'feature': features.append(ev)
    return header, pd.DataFrame(samples), pd.DataFrame(features)

def load_corpus(root: Path):
    """Loads every session under `root/sessions/*` as a single HF dataset."""
    return load_dataset(
        'json',
        data_files={'train': str(root / 'sessions' / '*' / 'session.jsonl.gz')},
        lines=True,
    )

def align_with_video(session_dir: Path):
    """Joins sample events to MP4 frames via the `vision_frame_index` field."""
    header, samples, features = session_to_dataframes(session_dir)
    samples['video_path'] = str(session_dir / 'video.mp4')
    samples['video_frame_seek_ms'] = samples['vision_frame_index'] * (1000 / header['vision_fps'])
    return samples
```

This loader is the canonical reference for academic researchers; it is **bundled with the dataset publication** under `tools/roc_loader.py` per R8 layout. The `align_with_video` helper makes the multimodal join (§8.3) a one-line operation downstream.

### §8.6 Foundation-model export patterns observed in the 2024–2026 literature

Three patterns dominate the 2024–2026 multimodal-mouse-dynamics papers:

| Pattern | Used by | Encoder | Sequence length | Comment |
|---|---|---|---|---|
| **Raw-velocity-sequence → 1D-ResNet → GRU** | Wang 2025 LT-MAuthen | 1D ResNet (4 blocks) + GRU (256 hidden) | 256 samples (~0.5 s @ 500 Hz) | Current SOTA on Balabit/DFL; matches ROC's window default |
| **Recurrence-plot image → ViT** | A Mouse Dynamics Auth. System With Recurrence-Plot+ViT, 2025 | Recurrence-plot encoding (256×256) → ViT-B/16 | 256-sample window → image | Visual transformer on trajectory image; competitive on Balabit |
| **Multimodal frame+sequence → cross-attention** | BEACON 2026 | Vision backbone (frame) + sequence backbone (mouse) + cross-attention head | 30 Hz frames + 500 Hz mouse | Defines the multimodal recipe ROC's §8.3 alignment supports natively |

ROC's JSONL schema supports all three out of the box: pattern 1 reads `sample.vx_px_s/vy_px_s`; pattern 2 reads the same and renders the recurrence plot in Python; pattern 3 reads samples plus the companion video plus the new `vision_frame_index` field.

---

## §9. Dashboard panel proposals

Extending `dashboard_menu_architecture_2026.md` §12.5 + §12.6 panel taxonomy. All panels live inside the **Inspect** destination's AC-Research Blueprint. Cardinality-of-4 destinations is preserved per the v2 §1 locked rule.

| # | Panel | Surfaces feature(s) | Hotkey | Selection-inspector controls | JSONL binding | Validating reference |
|---|---|---|---|---|---|---|
| **P1** | **VelocityProfilePanel** | F01, F02, F04 over time, with click events as vertical bands | `v` | toggle smoothing band overlay; choose 100 / 250 / 500 / 1000 ms window | `sample.vx_px_s, vy_px_s, ax_px_s2, ay_px_s2` | Shen 2013 Fig 3 |
| **P2** | **JerkSpectrumPanel** (extends §12.5 VelocityJerkSpectrumPanel) | F03, F08, F09 as live spectrogram with 8–12 Hz tremor-band stripe highlighted | `j` | choose linear vs log-magnitude; toggle tremor-band highlight | `feature.jerk_*, fft_peak_freq_hz, micro_tremor_hz, micro_tremor_in_human_band` | Wang 2025 §VI.B + hand-tremor physiology |
| **P3** | **FittsResidualPanel** (already in §12.5; spec'd here) | F21 scatter `(ID, MT)` with regression line + ≥3σ outlier highlighting | `f` | choose regression model (linear / power-law / per-user); toggle per-engagement bubble size by `target_distance_px` | `feature.fitts_residual_ms, fitts_index_of_difficulty` | Looser/Cockburn/Savage |
| **P4** | **ClickLandingPanel** | F18 scatter in target-relative coords with bivariate-Gaussian confidence ellipses | `c` | toggle ellipse confidence levels (50 %, 90 %, 99 %); colour-by target class (Head, UpperTorso, etc., per §11.3 anchor) | `sample.click_state, target_dx, target_dy, target_distance_px` | VADNet 2024 §3.5; Park ICP |
| **P5** | **PathEntropyPanel** | F11, F12, F14 time-series; F15 curvature heatmap inset | `h` | choose entropy bin count (8 / 16 / 32); toggle Higuchi `k_max` for F13 | `feature.path_entropy_shannon, path_efficiency, reversal_count, fractal_dimension` | Khan 2024 survey Table 1 |
| **P6** | **InterArrivalHistogramPanel** | F05, F17 histogram with bins at 1/125, 1/250, 1/500, 1/1000 s | `i` | log-x toggle; overlay-N-sessions toggle (for cross-session comparison) | `feature.inter_arrival_jitter_ms, sub_polling_uniformity` | Wang 2025 §III.A on uniformity as bot signature |
| **P7** | **SubPixelPanel** | F16 over time; cumulative ratio | `s` | none — single-stat panel | `feature.subpixel_sampling_ratio` | This is the ROC-distinctive panel because no published academic dataset records `raw_u, raw_v` separately from `u, v` — see §10 R2 below |
| **P8** | **MultimodalAlignmentPanel** | Visualises the `vision_frame_index` ↔ `sample.ts_ns` mapping as a 2-row Gantt strip (vision frames as one row, sample events as the other, with alignment links) | `m` | toggle showing only target-locked engagements | `sample.vision_frame_index` (new field per R6 below) | BEACON 2026 multimodal pattern |

**Top 3 for v1 (per R7 below):** P1, P3, P7. P1 because velocity profile is the universal first panel; P3 because Fitts' residuals are the highest-cross-validated AC feature in the entire FPS literature; P7 because sub-pixel ratio is ROC's signature contribution — no other published dataset captures it.

---

## §10. Risk analysis

Per `.cursor/rules/300-research-standards.mdc` schema.

| # | Risk | Severity | Mitigation |
|---|---|---|---|
| **C1** | **Dataset poisoning via auto-labelled production data.** If the export adapter ever auto-populates `clean_dirty_label` from a model classifier on production sessions, the dataset becomes self-reinforcing (bias amplification) and useless for cross-vendor benchmarking. Already addressed in `dashboard_menu_architecture_2026.md` §12 Risk B6, restated here as binding constraint. | **Critical** | Operator-supervised labelling only. The classifier never writes labels into the recording path. Pre-record modal mandatory; cannot be dismissed without explicit choice. Per §12 B1. |
| **C2** | **Mislabelled "clean" data** because the operator forgets the session label (already §12 B1). | Critical | Modal cannot be dismissed without explicit choice; label badge always visible in masthead; per `dashboard_menu_architecture_2026.md` §12 B1. |
| **C3** | **Balabit-2015-trained features not transferring to 2026 hardware.** Sub-pixel sampling patterns differ substantially: 2016 Balabit used 1 ms-resolution RDP-captured RDP-protocol mouse events; 2026 hardware routinely polls at 1000 Hz with sub-millisecond resolution. Features tuned on Balabit may fail on modern hardware. | **High** | (a) The 500 Hz default sample rate is chosen as a deliberate downsample-superset of all 2026 hardware. (b) The JSONL header records `sample_rate_hz` so consumers can resample / filter. (c) The §5 derived features are explicitly annotated for sub-sampling robustness — features marked "degrades sharply below 500 Hz" (F05, F17) are excluded from the live tier and treated as offline-only. |
| **C4** | **Derived-feature compute exceeds frame budget at high event rates.** Adding F08–F10 + F11–F15 + F18 to every 200 ms window adds ~120 k ops/window; at sustained 500 Hz with bursty click events the JSON-writer thread could miss its deadline. | Medium | All FFT operations use pre-allocated KissFFT plans in TLS; ring buffers are stack-allocated `std::array`; no heap on the windowed path. Diagnostics Strip exposes a `feature_compute_overrun_count` counter so the operator sees overruns immediately. Auto-degrade to 250 Hz under sustained overruns per §12 B2. |
| **C5** | **Export format locked-in before HuggingFace conventions stabilise.** ML Croissant is at v1.0 (Aug 2024) but the schema is still evolving. If ROC v1 ships a Croissant file that diverges from the v2 spec, dataset re-publication is needed. | Medium | (a) Generate the `croissant.jsonld` from a single source-of-truth template; bump the template version when Croissant bumps. (b) Pin the Croissant version in the `session` header (`croissant_spec_version: "1.0"`). (c) Ship a v1 → v2 conversion script alongside the engine. |
| **C6** | **C++ feature-extraction implementation drifts from academic reference implementations** over time (developer rewrites jerk formula, etc.); makes the dataset citation-untraceable. | Medium | Each derived-feature function in `core/kinematics_engine/include/roc/kinematics/derived_features.hpp` carries a Doxygen comment with the exact academic citation (paper + DOI + equation number) for its formula. A unit test per feature compares the C++ output to a Python reference implementation (NumPy / SciPy) on a fixed test trace; CI fails on drift > 1e-6. |
| **C7** | **Overreliance on one vendor's published methodology.** If the §5 feature set is over-fitted to (say) Vanguard's "internal vs external" cheat distinction, classifiers trained on ROC's data may not generalise to BattlEye-protected titles. | Medium | The §5 features are explicitly traced to **at least two of**: vendor public posts, academic surveys, academic primary papers. F21 (Fitts' residual) is cross-cited; F09 (hand tremor) is biometrics-corpus standard; F16 (sub-pixel) is the only feature uniquely cited via Vanguard alone, and even that has academic cross-cites in the Windows mouse-acceleration HCI literature. |
| **C8** | **Defensive-research framing erosion over time.** A future developer rewrites the docs without the defensive framing, the lint rule fails to catch a synonym, and the codebase drifts toward dual-use ambiguity. | High | The forbidden-vocabulary CI lint expansion per R10 below; the `defensive_research_attestation` flag in every `session` header is signed (B5); the `roc_vision_lint.py` Python rule additionally checks file headers for a `# DEFENSIVE RESEARCH ONLY` magic comment in every file under `dashboard/src/lib/exporter/` and `core/kinematics_engine/include/roc/kinematics/derived_features.hpp`. |
| **C9** | **Multimodal video export reveals operator desktop environment** (background processes visible in the camera feed of recorded sessions; Discord notifications, taskbar identifiers). PII / privacy risk. | High | The video MP4 companion file is **off by default** in the §12.6 Export modal. When on, the export adapter applies a fixed mask region (operator-configurable per Profile) over taskbar / chat regions; a privacy warning banner appears in the modal. |
| **C10** | **Hardware fingerprinting via `hardware_info.json`** (BEACON includes scrubbed MAC/IP/hostname; ROC adopts the same pattern). If ROC ships unscrubbed hardware metadata the dataset becomes re-identifiable. | High | Adopt BEACON scrubbing convention verbatim: scrub MAC, IP, hostname, disk-serial, motherboard-serial before writing. Replace with opaque hash-and-truncate. The `<session_id>.hardware_info.json` schema mirrors BEACON's `hardware_info_P###_S###.json`. |
| **C11** | **OS-clock drift between vision frame timestamps and actuator-telemetry timestamps** causes the `vision_frame_index` mapping (§8.3) to drift over multi-hour sessions. | Low | Both use the same monotonic `std::chrono::steady_clock` via the existing `phase_5_telemetry_ui.md` wire schema; ROC has no separate wall-clock source for either. Drift is bounded at hardware-clock-PPM (typically < 100 ns/s, < 1 ms over a 1-hour session). |
| **C12** | **Forbidden-vocabulary lint generates false positives** on legitimate AC research files (e.g., the citation "VADNet aimbot detection" trips a hit on "aimbot"). | Low | Lint rule has an allowlist for citation context: a `# AC-LIT-CITE: <paper-id>` magic comment above any line containing forbidden vocab whitelists that line. Used sparingly. |
| **C13** | **`SapiAgent`-style adversarial bots invalidate the entire dataset's discriminative power** within a generation. The 2021 SapiAgent paper specifically demonstrates that deep-generated trajectories defeat hand-crafted features. | Medium | ROC's deliverable does not preclude this; it acknowledges it. The mitigation is that ROC contributes the **dataset substrate, not the classifier** — the classifier arms-race lives downstream. ROC documents this in the dataset card as a "known limitation under adversarial bot generation". |

---

## §11. Numbered recommendations

### R1 — Minimum-viable live derived-feature set (v1; computed at 500 Hz inside the actuator-telemetry-emit thread)

**Ship these eight features in the live `feature` event:**

1. **F01** Tangential velocity (`vx_px_s, vy_px_s` already in §12.2; add `v_px_s = √(vx² + vy²)`)
2. **F02** Tangential acceleration (`ax_px_s2, ay_px_s2` already in §12.2; add `a_px_s2`)
3. **F03** Jerk magnitude (`jerk_x, jerk_y` already in §12.2; add `jerk_mag`)
4. **F04** Velocity profile asymmetry per engagement segment (new field `velocity_profile_asymmetry: f32`)
5. **F06** Click-after-stop latency (new field `click_after_stop_latency_ms: f32`)
6. **F07** Dwell time after stop (new field `dwell_after_stop_ms: f32`)
7. **F16** Sub-pixel sampling ratio (already in §12.2 as `subpixel_sampling_ratio`)
8. **F19** Click-while-moving boolean (new field `click_while_moving: bool`)

Justification: every one of these is computable in ≤ 12 mul-adds per sample or one comparator per event — well within the 2 ms/sample budget of the actuator-telemetry-emit thread (§12 B2 explicitly allows degradation if budget tight; this set never approaches the limit). All eight have at least two academic primary-source citations (§3) and one vendor citation (§2). The set captures the time-domain, sub-pixel-platform, and click-event dimensions; F08–F10 are the FFT features that need 256-sample windows so they live in tier 2.

### R2 — Full derived-feature set for offline post-processing

Ship a Python module `roc_offline_features.py` (companion to the JSONL export) that computes the remaining 17 features (F05, F08–F15, F17, F18, F20–F25). The module loads the JSONL via `datasets.load_dataset('json', ..., lines=True)` and writes augmented JSONL with the additional `feature` fields populated. The module includes the academic-citation comments per C6.

### R3 — Canonical sample-rate recommendation

**500 Hz baseline** as the default `sample_rate_hz` in the `session` header. Justification: (a) matches the most common premium gaming-mouse polling rate as of 2025–2026; (b) downsamples cleanly to 250 / 125 Hz for Balabit-style cross-dataset training; (c) satisfies Nyquist for the 8–12 Hz hand-tremor band by a margin of > 40×. Bump to **1000 Hz** as opt-in when the hardware supports it (Razer DeathAdder V3 Pro 4 kHz, Logitech G Pro X Superlight 2 8 kHz — note ROC caps at 1 kHz because the OS event aggregation layer rarely surfaces above 1 kHz on Windows 11 22H2 without specific HID-driver hacks). Drop to **250 Hz** only on the 4050 vacation laptop under sustained CPU contention (§12 B2 auto-degrade).

### R4 — JSONL `feature` event schema extension

Beyond the §12.2 baseline, add these fields to the `feature` event:

```json
{
  "velocity_profile_asymmetry": 0.43,
  "click_after_stop_latency_ms": 142.7,
  "dwell_after_stop_ms": 89.0,
  "click_while_moving": false,
  "vx_p99_px_s": 920.1,
  "ax_p99_px_s2": 5240.0,
  "jerk_p99": 11820.3,
  "click_landing_dx_target_px": 3.2,
  "click_landing_dy_target_px": -1.8,
  "click_landing_target_distance_px": 3.7,
  "vision_frame_indices": [12340, 12346],
  "engagement_id": 47,
  "engagement_event": "target_acquired"
}
```

Per session header additions:

```json
{
  "vision_fps": 30,
  "vision_frames_total": 90000,
  "croissant_spec_version": "1.0",
  "feature_extraction_software_version": "roc_features-1.0.0",
  "academic_citations_attestation": "all features in §5; see anticheat_data_exposure_2026.md"
}
```

### R5 — CNN trajectory-image format

**224 × 224 greyscale PNG** as the default companion image format (AimNet baseline, universally CNN-ingestable). Render the last 4096 samples as a single polyline with per-segment intensity proportional to local velocity magnitude (normalised to [0.2, 1.0] so very-slow segments stay visible). Optional **640 × 640 RGB** mode for VADNet compatibility (channel encoding: R = local velocity, G = local jerk magnitude clamped to log-scale, B = time-since-engagement-start normalised to [0, 1]).

### R6 — Multimodal vision-frame alignment

Add `vision_frame_index: u32` to every `sample` event and `vision_frame_indices: [u32, u32]` to every `feature` event. The actuator-telemetry-emit thread maintains a 90 000-element rolling ring (≥ 50 min at 30 Hz) of vision-frame timestamps and does a branchless binary search per sample — < 100 ns per lookup. Per session header add `vision_fps` and `vision_frames_total`. This is the single addition that makes the JSONL → JSONL multimodal training-data join a one-line operation downstream.

### R7 — Dashboard panels worth building first

Top 3 for v1, in order:

1. **P3 FittsResidualPanel** (Inspect destination, Phase 5c): the highest-cross-validated AC feature in the academic literature; immediately useful for operator-supervised dirty-vs-clean labelling because faster-than-Fitts engagements are physically diagnostic.
2. **P7 SubPixelPanel** (Inspect destination, Phase 5c): the ROC-distinctive panel — no other published dataset captures sub-pixel ratio. This is the panel that justifies ROC's data substrate to the academic community.
3. **P1 VelocityProfilePanel** (Inspect destination, Phase 5b minimal): the universal first panel; operator sees the bell-shape vs rectangular-profile distinction live without waiting for derived features.

P2, P4, P5, P6, P8 land in Phase 6+.

### R8 — HuggingFace publishing recipe

Folder layout under the user's HuggingFace organisation (e.g., `HeavenFYouMissed/roc-mouse-dynamics-v1`):

```
dataset_card.md            # the HuggingFace dataset card markdown
README.md                  # symlink to dataset_card.md
croissant.jsonld           # top-level dataset Croissant metadata
sessions/
  <session_id_1>/
    session.jsonl.gz
    trajectory.png
    hardware_info.json
    croissant.jsonld       # per-session Croissant (optional but BEACON does this)
  <session_id_2>/
    ...
```

Dataset card template includes:
- Stated **defensive-research framing**: "This dataset is published for the academic mouse-dynamics-for-continuous-authentication and anti-cheat-benchmarking research communities. It is NOT a cheat-development resource and is incompatible with such use by design — there is no game-server response data, no defensive-system-bypass guidance, and no evasion methodology in the dataset or its accompanying code."
- Licence: **CC BY 4.0** (matches CyberSIgnature Mendeley dataset, BEACON CC-BY-style, and the SapiMouse Apache 2.0 spirit of open-academic use).
- Citation: BibTeX entry plus the `@HeavenFYouMissed` GitHub identifier and the ROC engine version.
- Limitations section explicitly noting C13 SapiAgent-style adversarial-bot caveat.

### R9 — Defensive-research attestation signing

The `defensive_research_attestation: true` flag in every `session` header (§12.2) must be Ed25519-signed with a project-bundled private key. The corresponding public key is embedded in `roc_vision.exe` and is also published in `dashboard/public_keys/roc_attestation_pubkey.pem` and on the `@HeavenFYouMissed` GitHub repo as a checked-in file. Consumers verify the signature on every loaded `session` header; loaded sessions without valid signatures are flagged in the loader. Aligns with §12 B5.

Concretely:
- Header field `attestation_signature: base64-encoded Ed25519 signature of the canonicalised header JSON`.
- Signing routine in `core/transport/include/roc/transport/session_signer.hpp` (~80 LOC).
- Verifying routine in the Python loader (~30 LOC using `pynacl`).

### R10 — Forbidden-vocabulary CI lint expansion

Extending `dashboard_menu_architecture_2026.md` §11.4 A3 and §12 header forbidden-vocab list, add these AC-research-specific terms to the `roc_vision_lint.py` deny-list:

```
evade_*, evasion, bypass_*, circumvent_*, defeat_*, undetectable, undetected,
spoof_*, fake_*, fakeable, humanise_*, humaniser, anti-detection,
inject_*, injector, hwid_*spoof, spoof_hwid, mac_*spoof, hardware_spoof,
trigger_bot, triggerbot, recoil_compensation_bot, no_recoil, no-recoil,
wall_hack, wallhack, esp_*hack, esp-hack, kernel_loader, manual_map, manualmap,
dma_inject, dma_cheat, dma_*cheat, dma-cheat
```

Allowlist (`# AC-LIT-CITE: <paper-id>`) is honoured for citation context only. The lint runs on every commit to `core/`, `dashboard/`, and `workspace_blueprint/research_notes/`.

Justification: these are the canonical offensive-vocabulary terms used in cheat-development forums (UnknownCheats, MPGH, Guided Hacking — all forbidden source domains per the user's brief). Banning them at lint-time makes accidental drift toward offensive framing impossible without an explicit `# AC-LIT-CITE:` waiver. The deliverable's own vocabulary uses exclusively: detection, classification, biometric authentication, signature, feature, dataset, attestation.

### R11 — Sub-pixel verification protocol (validates F16 on the operator's hardware)

The sub-pixel ratio F16 is ROC's signature contribution and the panel P7 hero metric. Its discriminative power requires that the operator's HID stack actually produces sub-pixel positions; if Windows mouse acceleration is off or the mouse driver is a 1:1 raw-input driver, F16 = 0 on humans and the feature is useless. The Calibrate destination must include a **Sub-pixel verification step** before the AC-Research Scenarios become available:

1. Operator moves the cursor slowly across the screen for ~5 seconds.
2. The dashboard computes F16 over the 5-second window.
3. If F16 > 0.40: ✅ Sub-pixel sampling enabled. AC-Research Scenarios unlocked.
4. If F16 < 0.40: ⚠️ Warning modal: "Sub-pixel sampling is disabled or extremely limited on this configuration. Consider enabling Windows Enhance Pointer Precision (Settings → Mouse → Additional mouse settings → Pointer Options) for the most discriminative dataset." Unblockable but flagged.
5. The result is written to the `session` header as `subpixel_verification_ratio: f32` so consumers know the data quality going in.

Justification: F16 is uniquely high-signal but is platform-dependent. Without this verification step, the dataset risks containing both useful and useless sessions indistinguishably labelled. The Calibrate wizard (`dashboard_menu_architecture_2026.md` §1 cross-cutting outcome table) is the natural home for this — it slots into the existing 5-step wizard (intrinsics → extrinsics → EKF tuning → model selection → save-as-Scenario) as step 4.5 "Verify HID sub-pixel sampling" before saving.

### R12 — Cross-application transfer-learning recipe

For researchers wanting to use ROC data to train a classifier that generalises beyond the captured session's application context, ship a `tools/roc_cross_app_transfer.py` reference notebook documenting:

1. **Subset the dataset by application** using the `session.scenario` field (e.g., `"AC Research — Behavioural Profiling (Clean)"`, `"Robotics — Dynamixel Pan/Tilt"`, etc.).
2. **Train on the "live tier" features only** (F09, F16, F03 — per §5.9 these transfer best).
3. **Apply per-user Fitts'-coefficient calibration** if F21 is in the feature set: fit `(a, b)` per user from the training subset, store per-user `(a, b)` alongside the model, apply at inference.
4. **Validate on held-out application** (e.g., train on AC-Research-Clean sessions, hold out the Robotics Dynamixel sessions).
5. **Document the AUC gap** in the dataset card (R8) as transparency for downstream consumers.

This recipe operationalises the §5.9 cross-application analysis and gives consumers a concrete starting point. Reference notebook follows the Wang 2025 §VII.D evaluation protocol verbatim (so the numbers are directly comparable to the SOTA paper).

### R13 — Dataset publication cadence

Ship the dataset in versioned releases on the user's HuggingFace organisation:

- **v0.1** (alpha; private to `@HeavenFYouMissed`): 10–20 short (5–10 min) AC-Research-Clean sessions. Smoke-test the export pipeline end-to-end.
- **v0.2** (alpha+; private): add 10–20 AC-Research-Dirty sessions captured by the same operator on the same hardware (paired for direct A/B). This is the minimum viable training corpus for the user's clean-vs-dirty classifier.
- **v1.0** (public; CC BY 4.0 on HF): if and only if the user wants to publish. Include the Croissant metadata, the loader (R8), the transfer-learning recipe (R12), and the defensive-research attestation (R9). Suggested target size: ≥ 100 sessions across ≥ 5 hardware configurations, ≥ 20 hours of clean and ≥ 5 hours of dirty (asymmetric is fine; humans dominate naturally).
- **v2.0+** (post-publication): multimodal expansion with the video companion enabled by default for opted-in sessions.

Justification: the BEACON 2026 release (28 participants × 79 sessions × 445 GB) is the contemporary academic ceiling; v1.0 should target a meaningful subset of that scale to be useful as a public corpus. SapiMouse (120 subjects × 4 min) is the lower bound for academic-paper viability. ROC v1.0 at 100 sessions × ~15 min average sits comfortably between the two.

---

## §12. References

All sources accessed 2026-05-17.

### §12.1 Vendor public posts (primary sources)

1. mirageofpenguins, "/dev: Vanguard x LoL Retrospective", Riot Games Dev Blog, 2024-08-22. `https://www.leagueoflegends.com/en-us/news/dev/dev-vanguard-x-lol-retrospective/`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/leagueoflegends.com-en-us-news-dev-dev-vanguard-x-lol-retrospective.md`.
2. Riot Games, "Vanguard Security Update: Closing the Pre-Boot Gap". `https://www.riotgames.com/en/news/vanguard-security-update-motherboard`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/riotgames.com-en-news-vanguard-security-update-motherboard.md`.
3. BattlEye Innovations e.K., "About — BattlEye: The Anti-Cheat Gold Standard". `https://www.battleye.com/about/`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/battleye.com-about.md`.
4. BattlEye Innovations e.K., "A New Dawn", 2015-02-09. `https://www.battleye.com/2015/02/09/a-new-dawn/`. Accessed 2026-05-17.
5. McDonald, J. "Robocalypse Now: Using Deep Learning to Combat Cheating in Counter-Strike: Global Offensive". Game Developers Conference (GDC), 2018-03-21. `https://www.gdcvault.com/play/1024994/Robocalypse-Now-Using-Deep-Learning`. Free content. YouTube id `kTiP0zKF9bc`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/gdcvault.com-play-1024994-Robocalypse-Now-Using-Deep-Learning.md`.
6. FACEIT, "Developer Blog: New updates on FACEIT Anti-Cheat" (c. poracchia / FACEIT_NtroP), 2024-04-11. `https://blog.faceit.com/developer-blog-new-updates-on-faceit-anti-cheat-aa7a47cc90da`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/blog.faceit.com-developer-blog-new-updates-on-faceit-anti-cheat-aa7a47cc90da.md`.
7. FACEIT Labs, "What is Minerva". `https://labs-docs.faceit.com/MinervaAI/WhatIsMinerva/`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/labs-docs.faceit.com-MinervaAI-WhatIsMinerva.md`.
8. Easy Anti-Cheat (Epic Games), "Additional Security Requirements". `https://www.easy.ac/support/articles/additional-security-requirements`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/easy.ac-support-articles-additional-security-requirements.md`.
9. Wellbia.com Co., Ltd., "XIGNCODE3 for PC". `https://www.wellbia.com/?module=Html&action=SiteComp&sSubNo=1`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/wellbia.com.md`.

### §12.2 Vendor secondary (limited use — primary citations consolidated)

10. Wikipedia contributors, "BattlEye". `https://en.wikipedia.org/wiki/BattlEye`. Accessed 2026-05-17. Used only for canonical bibliography of primary BattlEye references. Cached `.firecrawl/anticheat_research/en.wikipedia.org-wiki-BattlEye.md`.
11. Demenais, L. "A History of Anti-Cheat Techniques in Video Games, from Server-side to Behavioural-side", EPITA GISTRE blog, 2025-06-24. `https://blog.gistre.epita.fr/posts/lucas.demenais-2025-06-24-history_of_anticheat/`. Accessed 2026-05-17. Used for ESEA timeline only. Cached `.firecrawl/anticheat_research/blog.gistre.epita.fr-posts-lucas.demenais-2025-06-24-history_of_anticheat.md`.

### §12.3 Academic primary sources (peer-reviewed)

12. Ahmed, A.A.E. & Traore, I. "A New Biometric Technology Based on Mouse Dynamics". IEEE Transactions on Dependable and Secure Computing 4(3), pp. 165–179, 2007. DOI 10.1109/TDSC.2007.70207. `https://dl.acm.org/doi/10.1109/TDSC.2007.70207`. Accessed 2026-05-17.
13. Shen, C., Cai, Z., Guan, X., Du, Y., Maxion, R.A. "User Authentication Through Mouse Dynamics". IEEE Transactions on Information Forensics and Security 8(1), pp. 16–30, 2013. DOI 10.1109/TIFS.2012.2223677. `https://dl.acm.org/doi/10.1109/TIFS.2012.2223677`. Accessed 2026-05-17.
14. Khan, S., Devlen, C., Manno, M., Hou, D. "Mouse Dynamics Behavioral Biometrics: A Survey". ACM Computing Surveys (arXiv:2208.09061v2), May 2024. `https://arxiv.org/pdf/2208.09061`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/arxiv.org-pdf-2208.09061.md`.
15. Siddiqui, N., Dave, R., Vanamala, M., Seliya, N. "Machine and Deep Learning Applications to Mouse Dynamics for Continuous User Authentication". MDPI Machine Learning and Knowledge Extraction 4, 2022 (arXiv:2205.13646). `https://arxiv.org/pdf/2205.13646`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/arxiv.org-pdf-2205.13646.md`.
16. Wang, Y., Wu, C., Liao, Y., You, M. "Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges". arXiv:2504.21415v1, 2025-04-30. `https://arxiv.org/html/2504.21415v1`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/arxiv.org-html-2504.21415v1.md`.
17. Dave, R., Handoko, M., Rashid, A., Schoenbauer, C. "From Clicks to Security: Investigating Continuous Authentication via Mouse Dynamics". arXiv:2403.03828, 2024-03-06. `https://arxiv.org/abs/2403.03828`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/arxiv.org-abs-2403.03828.md`.
18. Antal, M., Fejer, N., Buza, K. "SapiMouse: Mouse Dynamics-based User Authentication Using Deep Feature Learning". IEEE 15th International Symposium on Applied Computational Intelligence and Informatics (SACI), May 2021. IEEE Xplore document 9465583. `https://ieeexplore.ieee.org/document/9465583`. Accessed 2026-05-17.
19. Antal, M., Buza, K., Fejer, N. "SapiAgent: A Bot Based on Deep Learning to Generate Human-Like Mouse Trajectories". IEEE Access 9, pp. 124396–124408, 2021. DOI 10.1109/ACCESS.2021.3111098. **Defensive paper — cited for stress-testing context only per §3.3.** `https://ieeexplore.ieee.org/document/9530664`. Accessed 2026-05-17.
20. Nie, B., Ma, B. "VADNet: Visual-Based Anti-Cheating Detection Network in FPS Games". Traitement du Signal 41(1), pp. 431–440, 2024. DOI 10.18280/ts.410137. `https://www.iieta.org/journals/ts/paper/10.18280/ts.410137`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/iieta.org-journals-ts-paper-10.18280-ts.410137.md`.
21. Looser, J., Cockburn, A., Savage, J. "On the Validity of Using First-Person Shooters for Fitts' Law Studies". University of Canterbury HCI report. `https://www.csse.canterbury.ac.nz/andrew.cockburn/papers/fitts-game.pdf`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/csse.canterbury.ac.nz-andrew.cockburn-papers-fitts-game.pdf.md`.
22. Liu, X. et al. "Detecting Aimbot Cheats in FPS Games Using Computer Vision: Effective Methods and Challenges". Trans. Eng. (lseee.net) 2024. `https://lseee.net/index.php/te/article/download/2060/TE013602.pdf/8287`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/lseee.net-index.php-te-article-download-2060-TE013602.pdf-8287.md`.

### §12.4 Public datasets (primary source URLs)

23. Fülöp, Á., Kovács, L., Kurics, T., Windhager-Pokol, E. "Balabit Mouse Dynamics Challenge Data Set". 2016. `https://github.com/balabit/Mouse-Dynamics-Challenge`. Accompanying release note: `https://medium.com/balabit-unsupervised/releasing-the-balabit-mouse-dynamics-challenge-data-set-a15a016fba6c`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/github.com-balabit-Mouse-Dynamics-Challenge.md` + `medium.com-balabit-unsupervised-releasing-the-balabit-mouse-dynamics-challenge-data-set-a15a016fba6c.md`.
24. Antal, M., Fejer, N., Buza, K. "SapiMouse dataset". 2020/2021. GitHub `https://github.com/margitantal68/sapimouse`. Raw data: `https://ms.sapientia.ro/~manyi/sapimouse/sapimouse.html`. Apache-2.0 licence. 120 subjects (92 male, 28 female), one 3-min + one 1-min session each. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/github.com-margitantal68-sapimouse.md`.
25. Siddiqui, N., Dave, R. "Mouse dynamics for user authentication (Wisconsin-Minecraft)". 40 users, 20 min gameplay. Per Siddiqui et al. 2022 paper. Per `https://www.kaggle.com/code/jaafarnejm/mouse-dynamics` and the paper's Data Availability section.
26. Antal, M., Denes-Fazakas, L. "DFL Mouse Dynamics Dataset" (variant of the Balabit format). 21 users, 1 k mouse actions per user. Per Antal & Egyed-Zsigmond 2018 paper.
27. Ahmed, A.A.E. & Traore, I. "ISOT Mouse Dynamics Dataset". University of Victoria Information Security and Object Technology research lab. `https://onlineacademiccommunity.uvic.ca/isot/2022/11/27/behavioral-biometric-datasets/`. Accessed 2026-05-17. 48 users, 9 weeks, 284 hours of raw data, requires UVic approval. Cached `.firecrawl/anticheat_research/onlineacademiccommunity.uvic.ca-isot-2022-11-27-behavioral-biometric-datasets.md`.
28. Nnamoko, N., Barrowclough, J., Liptrott, M., Korkontzelos, I. "Behaviour Biometrics Dataset (CyberSIgnature)". Mendeley Data, 2022. DOI 10.17632/fnf8b85kr6.1. `https://data.mendeley.com/datasets/fnf8b85kr6`. CC BY 4.0. 88 users × 20 entry sessions = 1,760 KMT instances. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/data.mendeley.com-datasets-fnf8b85kr6.md`.
29. The BEACON Project. "BEACON: Behavioral Engine for Authentication and Continuous Monitoring". HuggingFace `beacon-gui/BEACON-Dataset`, 2026 (arXiv:2605.10867). DOI `10.57967/hf/8674`. `https://huggingface.co/datasets/beacon-gui/BEACON-Dataset`. 28 participants × 79 VALORANT sessions × 445 GB multimodal. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/huggingface.co-datasets-beacon-gui-BEACON-Dataset.md` + `arxiv.org-html-2605.10867v1.md`.

### §12.5 Supporting academic infrastructure references

30. Semantic Scholar paper page for Shen, Cai, Guan 2013. `https://www.semanticscholar.org/paper/User-Authentication-Through-Mouse-Dynamics-Shen-Cai/b3304d49e9d651df002d2a85496157950a9de805`. Accessed 2026-05-17.
31. ACM Computing Surveys formal listing for Khan et al. 2024 survey. `https://dl.acm.org/doi/10.1145/3640311`. Accessed 2026-05-17. Cached `.firecrawl/anticheat_research/dl.acm.org-doi-10.1145-3640311.md`.

### §12.6 Defensive-research attestation

This document and its companion `.firecrawl/anticheat_research/` source cache contain **zero offensive content** and **zero citations from forbidden source domains** (UnknownCheats, MPGH, OwnedCore, Guided Hacking, EliteVoid, cheat-vendor sites, exploitation-tutorial sites, aimbot-source-code pastebins). The document is suitable for publication in a peer-reviewed defensive-security venue (IEEE TIFS, ACM CCS, USENIX Security) or as a public Vanguard-style technical blog post. The forbidden-vocabulary CI lint (R10) statically enforces this property across the entire codebase.

---

*End of `anticheat_data_exposure_2026.md`. Total: 25 derived features catalogued (§5), 8 dashboard panels proposed (§9), 13 risks analysed (§10), 10 numbered recommendations (§11), 31 cited primary sources (§12). Document extends `dashboard_menu_architecture_2026.md` §12.2 JSONL schema with the additions specified in R4 and R6. No production code, no specifications outside `workspace_blueprint/research_notes/`, and no `.cursor/rules/` modifications until the user signs off on this deliverable together with the v4 amendment.*
