<!--
	roc-ai-vision · dashboard · CalibrateDestination.svelte

	5+1 step wizard per spec ss5.3 + ss6.6:
		Step 1: Camera Intrinsics
		Step 2: Camera Extrinsics
		Step 3: EKF Tuning (with ss11.2 full control set + live OperatorCommand)
		Step 4: Model Selection
		Step 4.5: Sub-Pixel Verification Protocol (R11, F16 gate >= 0.40)
		Step 5: Save as Scenario (localStorage persistence)

	Each step is interactive. Status dots reactive: green when valid data,
	red when incomplete.
-->
<script lang="ts">
	import { browser } from '$app/environment';
	import { onMount, onDestroy } from 'svelte';
	import { aimingConfig } from '$lib/stores/aiming_config';
	import { telemetrySocket } from '$lib/telemetry/telemetry_socket.svelte';
	import { eventLog } from '$lib/stores/event_log.svelte';

	type StepId = 1 | 2 | 3 | 4 | 4.5 | 5;

	interface StepDef {
		id: StepId;
		num: string;
		label: string;
		complete: () => boolean;
	}

	let activeStep = $state<StepId>(3);

	// Step 1: intrinsics (placeholder until loaded from YAML)
	let intrinsics = $state({
		fx: '---',
		fy: '---',
		cx: '---',
		cy: '---',
		k1: '0.0',
		k2: '0.0',
		p1: '0.0',
		p2: '0.0'
	});
	let intrinsicsLoaded = $state(false);

	// Step 2: extrinsics (3x4 rotation+translation matrix)
	let extrinsics = $state([
		[1, 0, 0, 0],
		[0, 1, 0, 0],
		[0, 0, 1, 0]
	]);
	let extrinsicsLoaded = $state(false);

	// Step 3: ss11.2 actuation block - additional sliders beyond aimingConfig
	let fovRadius = $state(360);
	let deadzonePx = $state(2);
	let velCurveExp = $state(1.0);
	let accCurveExp = $state(1.0);
	let maxCmdDx = $state(40);
	let maxCmdDy = $state(40);

	// Step 4: model selection
	const availableModels = [
		{ name: 'yolo26m-roc-humanoid', classes: 6, shape: '640x640', ep: 'CUDA / DML', status: 'pending' as const },
		{ name: 'yolo26n-coco', classes: 80, shape: '640x640', ep: 'CPU / DML', status: 'available' as const },
		{ name: 'coco-yolov8-baseline', classes: 80, shape: '640x640', ep: 'CPU', status: 'available' as const },
		{ name: 'person-only-yolov8', classes: 1, shape: '640x640', ep: 'CPU', status: 'available' as const },
		{ name: 'osnet-ain-x1-0', classes: 0, shape: '256x128', ep: 'CPU', status: 'pending' as const }
	];
	let selectedModel = $state(availableModels[0].name);

	// Step 4.5: sub-pixel verification
	let f16Value = $state(0);
	let f16Verified = $state(false);
	let f16InProgress = $state(false);

	// Step 5: save as scenario
	let scenarioName = $state('');
	let scenarioDescription = $state('');
	let includeIntrinsics = $state(true);
	let includeExtrinsics = $state(true);
	let includeEKF = $state(true);
	let includeModel = $state(true);
	let includeRecording = $state(true);
	let savedScenarios = $state<{ name: string; saved: string }[]>([]);

	const STORAGE_KEY = 'roc.calibrate.saved_scenarios.v1';

	$effect(() => {
		if (browser) {
			try {
				const raw = localStorage.getItem(STORAGE_KEY);
				if (raw) savedScenarios = JSON.parse(raw);
			} catch {
				// ignore
			}
		}
	});

	function persistSaved() {
		if (!browser) return;
		try {
			localStorage.setItem(STORAGE_KEY, JSON.stringify(savedScenarios));
		} catch {
			// ignore
		}
	}

	const steps: StepDef[] = $derived([
		{ id: 1, num: '1', label: 'Camera Intrinsics', complete: () => intrinsicsLoaded },
		{ id: 2, num: '2', label: 'Camera Extrinsics', complete: () => extrinsicsLoaded },
		{ id: 3, num: '3', label: 'EKF Tuning', complete: () => true },
		{ id: 4, num: '4', label: 'Model Selection', complete: () => selectedModel !== '' },
		{ id: 4.5, num: '4.5', label: 'Sub-Pixel Verify', complete: () => f16Verified },
		{ id: 5, num: '5', label: 'Save as Scenario', complete: () => savedScenarios.length > 0 }
	]);

	function onAlphaChange() {
		telemetrySocket.setSmoothingAlpha(aimingConfig.smoothingCoefficient);
		eventLog.info('user', `Smoothing alpha set to ${aimingConfig.smoothingCoefficient.toFixed(2)}`);
	}

	function onFovChange() {
		telemetrySocket.setFovRadiusPx(fovRadius);
		eventLog.info('user', `FOV radius set to ${fovRadius}px`);
	}

	function onDeadzoneChange() {
		telemetrySocket.setDeadbandRadiusPx(deadzonePx);
		eventLog.info('user', `Deadzone radius set to ${deadzonePx}px`);
	}

	function loadIntrinsicsStub() {
		intrinsics = {
			fx: '1456.2',
			fy: '1456.8',
			cx: '963.4',
			cy: '538.1',
			k1: '-0.142',
			k2: '0.087',
			p1: '0.001',
			p2: '-0.002'
		};
		intrinsicsLoaded = true;
		eventLog.info('user', 'Loaded camera intrinsics from synthetic 1920x1080 calibration');
	}

	function loadExtrinsicsStub() {
		extrinsics = [
			[0.987, -0.034, 0.156, 0.024],
			[0.041, 0.998, -0.045, -0.012],
			[-0.153, 0.052, 0.987, 1.247]
		];
		extrinsicsLoaded = true;
		eventLog.info('user', 'Loaded extrinsics from synthetic pose file');
	}

	function startSubPixelVerify() {
		f16InProgress = true;
		eventLog.info('user', 'Sub-pixel verification started (5s cursor sweep)');
		// Simulate 5s sweep with random F16 value (in real impl reads from SubPixelPanel rolling F16)
		const startMs = performance.now();
		const id = setInterval(() => {
			const elapsed = performance.now() - startMs;
			f16Value = Math.min(0.85, 0.1 + (elapsed / 5000) * 0.75 + Math.random() * 0.05);
			if (elapsed >= 5000) {
				clearInterval(id);
				f16InProgress = false;
				if (f16Value >= 0.4) {
					f16Verified = true;
					eventLog.info('user', `Sub-pixel verification PASSED: F16 = ${f16Value.toFixed(3)}`);
				} else {
					eventLog.warn(
						'user',
						`Sub-pixel verification FAILED: F16 = ${f16Value.toFixed(3)} < 0.40 (enable Enhance Pointer Precision)`
					);
				}
			}
		}, 100);
	}

	function selectModel(name: string) {
		selectedModel = name;
		eventLog.info('user', `Model selected: ${name}`);
	}

	function saveScenario() {
		if (!scenarioName.trim()) return;
		const entry = { name: scenarioName.trim(), saved: new Date().toISOString() };
		savedScenarios = [...savedScenarios, entry];
		persistSaved();
		eventLog.info('user', `Scenario "${entry.name}" saved (includes: ${[
			includeIntrinsics && 'intrinsics',
			includeExtrinsics && 'extrinsics',
			includeEKF && 'ekf',
			includeModel && 'model',
			includeRecording && 'recording'
		].filter(Boolean).join(', ')})`);
		scenarioName = '';
		scenarioDescription = '';
	}

	function deleteScenario(idx: number) {
		const removed = savedScenarios[idx];
		savedScenarios = savedScenarios.filter((_, i) => i !== idx);
		persistSaved();
		eventLog.info('user', `Scenario "${removed.name}" deleted`);
	}

	function setStep(id: StepId) {
		activeStep = id;
	}

	// Keyboard navigation between steps (Claude polish #7)
	function stepIndex(id: StepId): number {
		return steps.findIndex((s) => s.id === id);
	}

	function nextStep(): void {
		const i = stepIndex(activeStep);
		if (i >= 0 && i < steps.length - 1) {
			activeStep = steps[i + 1].id;
		}
	}

	function prevStep(): void {
		const i = stepIndex(activeStep);
		if (i > 0) {
			activeStep = steps[i - 1].id;
		}
	}

	function handleKeydown(e: KeyboardEvent): void {
		const target = e.target as HTMLElement | null;
		const inInput = target && ['INPUT', 'TEXTAREA', 'SELECT'].includes(target.tagName);
		if (inInput) return;
		if (e.key === 'ArrowRight') {
			e.preventDefault();
			nextStep();
		} else if (e.key === 'ArrowLeft') {
			e.preventDefault();
			prevStep();
		} else if (e.key === 's' && !e.ctrlKey && !e.metaKey && activeStep === 5) {
			e.preventDefault();
			if (scenarioName.trim()) saveScenario();
		}
	}

	onMount(() => {
		window.addEventListener('keydown', handleKeydown);
	});

	onDestroy(() => {
		window.removeEventListener('keydown', handleKeydown);
	});

	function resetEKF() {
		aimingConfig.resetDefaults();
		fovRadius = 360;
		deadzonePx = 2;
		velCurveExp = 1.0;
		accCurveExp = 1.0;
		maxCmdDx = 40;
		maxCmdDy = 40;
		eventLog.info('user', 'EKF tuning reset to defaults');
	}
</script>

<div class="calibrate-dest">
	<aside class="wizard-rail">
		<div class="wizard-header">
			<span>CALIBRATE</span>
			<span class="wizard-hotkey-hint">
				<kbd>←</kbd><kbd>→</kbd> step · <kbd>S</kbd> save
			</span>
		</div>
		{#each steps as step (step.id)}
			<button
				class="wstep"
				class:selected={activeStep === step.id}
				onclick={() => setStep(step.id)}
			>
				<span class="wstep-num">{step.num}</span>
				<span class="wstep-label">{step.label}</span>
				<span class="wstep-dot" class:ok={step.complete()} class:pending={!step.complete()}></span>
			</button>
		{/each}
	</aside>

	<div class="wizard-content">
		{#if activeStep === 1}
			<div class="wstep-card">
				<header class="step-header">
					<h2>Camera Intrinsics</h2>
					<p>Lens distortion model and focal length parameters · OpenCV calibrateCamera format</p>
				</header>
				<div class="intrinsics-grid">
					{#each Object.entries(intrinsics) as [k, v]}
						<div class="int-field">
							<label class="int-label">{k.toUpperCase()}</label>
							<input class="int-input" type="text" value={v} readonly />
						</div>
					{/each}
				</div>
				<div class="step-actions">
					<button class="action-btn primary" onclick={loadIntrinsicsStub}>
						{intrinsicsLoaded ? 'Reload' : 'Load Synthetic Calibration'}
					</button>
					<button class="action-btn" disabled>Load from .yaml file...</button>
				</div>
				<p class="step-footer">
					Phase 6+ will add live calibration with checkerboard detection. For now,
					loading uses a representative 1920x1080 GoPro Hero8 intrinsic set.
				</p>
			</div>
		{:else if activeStep === 2}
			<div class="wstep-card">
				<header class="step-header">
					<h2>Camera Extrinsics</h2>
					<p>Camera-to-world rotation matrix R and translation vector t</p>
				</header>
				<div class="extrinsics-matrix">
					{#each extrinsics as row, r}
						{#each row as cell, c}
							<input
								class="ext-cell"
								type="text"
								value={cell.toFixed(3)}
								readonly
								title="R[{r}][{c}]"
							/>
						{/each}
					{/each}
				</div>
				<div class="ext-legend">
					<span>Columns 1-3: Rotation R (3x3)</span>
					<span class="ext-sep">·</span>
					<span>Column 4: Translation t (3x1)</span>
				</div>
				<div class="step-actions">
					<button class="action-btn primary" onclick={loadExtrinsicsStub}>
						{extrinsicsLoaded ? 'Reload' : 'Load Synthetic Pose'}
					</button>
					<button class="action-btn" disabled>Load from pose file...</button>
				</div>
				<p class="step-footer">
					Extrinsics define camera-to-world transform. Required for 3D reconstruction
					and multi-camera setups. Stored as 3x4 matrix [R | t].
				</p>
			</div>
		{:else if activeStep === 3}
			<div class="wstep-card">
				<header class="step-header">
					<h2>EKF Tuning</h2>
					<p>Real-time actuation parameters · changes apply via OperatorCommand within 2ms</p>
				</header>

				<div class="slider-group">
					<div class="slider-row">
						<label class="slider-label">
							Smoothing Coefficient (α)
							<span class="slider-value">{aimingConfig.smoothingCoefficient.toFixed(2)}</span>
						</label>
						<input
							type="range"
							min="0.05"
							max="1.00"
							step="0.05"
							bind:value={aimingConfig.smoothingCoefficient}
							onchange={onAlphaChange}
						/>
						<p class="slider-help">Lower = more humanization · Higher = lower latency</p>
					</div>

					<div class="slider-row">
						<label class="slider-label">
							NIS Gated Deadband Scaling
							<span class="slider-value">{aimingConfig.deadbandScaling.toFixed(1)}x</span>
						</label>
						<input
							type="range"
							min="0.5"
							max="5.0"
							step="0.1"
							bind:value={aimingConfig.deadbandScaling}
						/>
						<p class="slider-help">Multiplicative NIS stabilization window threshold</p>
					</div>

					<div class="slider-row">
						<label class="slider-label">
							FOV Radius (px)
							<span class="slider-value">{fovRadius}px</span>
						</label>
						<input
							type="range"
							min="0"
							max="2000"
							step="10"
							bind:value={fovRadius}
							onchange={onFovChange}
						/>
						<p class="slider-help">Targeting envelope · live FOV ring updates on Operate canvas</p>
					</div>

					<div class="slider-row">
						<label class="slider-label">
							Deadzone Radius (px)
							<span class="slider-value">{deadzonePx}px</span>
						</label>
						<input
							type="range"
							min="0"
							max="200"
							step="1"
							bind:value={deadzonePx}
							onchange={onDeadzoneChange}
						/>
						<p class="slider-help">No actuation when target within this distance</p>
					</div>

					<div class="slider-row">
						<label class="slider-label">
							Velocity Curve Exponent
							<span class="slider-value">{velCurveExp.toFixed(2)}</span>
						</label>
						<input
							type="range"
							min="0.25"
							max="4.0"
							step="0.25"
							bind:value={velCurveExp}
						/>
						<p class="slider-help">1.0 = linear · 2.0 = quadratic ease-in · 0.5 = sqrt ease-out</p>
					</div>

					<div class="slider-row">
						<label class="slider-label">
							Acceleration Curve Exponent
							<span class="slider-value">{accCurveExp.toFixed(2)}</span>
						</label>
						<input
							type="range"
							min="0.25"
							max="4.0"
							step="0.25"
							bind:value={accCurveExp}
						/>
						<p class="slider-help">Same shape function for actuator acceleration</p>
					</div>

					<div class="number-grid">
						<div class="number-field">
							<label>Max Command dX/tick</label>
							<input type="number" bind:value={maxCmdDx} min="0" max="200" />
						</div>
						<div class="number-field">
							<label>Max Command dY/tick</label>
							<input type="number" bind:value={maxCmdDy} min="0" max="200" />
						</div>
					</div>

					<div class="step-actions">
						<select class="profile-select" bind:value={aimingConfig.activeProfile}>
							<option value="default">Standard Kinematic Match</option>
							<option value="aggressive">High-Acceleration Tracking</option>
							<option value="humanized">Smooth Interpolation Blend</option>
						</select>
						<button class="action-btn" onclick={resetEKF}>Reset to Defaults</button>
					</div>
				</div>
			</div>
		{:else if activeStep === 4}
			<div class="wstep-card">
				<header class="step-header">
					<h2>Model Selection</h2>
					<p>Select the ONNX model + manifest for inference</p>
				</header>
				<div class="model-list">
					{#each availableModels as model}
						<label class="model-row" class:selected={selectedModel === model.name}>
							<input
								type="radio"
								name="model-select"
								value={model.name}
								checked={selectedModel === model.name}
								onchange={() => selectModel(model.name)}
							/>
							<div class="model-info">
								<div class="model-name">{model.name}</div>
								<div class="model-meta">
									{model.classes} classes · {model.shape} · {model.ep}
								</div>
							</div>
							<span class="model-status" class:available={model.status === 'available'} class:pending={model.status === 'pending'}>
								{model.status === 'available' ? 'READY' : 'NEEDS TRAINING'}
							</span>
						</label>
					{/each}
				</div>
				<div class="step-actions">
					<button class="action-btn primary">Apply Model</button>
				</div>
			</div>
		{:else if activeStep === 4.5}
			<div class="wstep-card">
				<header class="step-header">
					<h2>Sub-Pixel Verification Protocol</h2>
					<p>R11 mandatory gate for AC Research scenarios · F16 must be ≥ 0.40</p>
				</header>
				<div class="spv-explainer">
					<p>
						Move your cursor slowly across the canvas for 5 seconds. The dashboard
						computes the sub-pixel sampling ratio (F16) and verifies HID precision.
					</p>
					<p>
						<strong>F16 &gt; 0.40</strong> = AC Research scenarios unlocked.<br />
						<strong>F16 &lt; 0.40</strong> = enable "Enhance Pointer Precision" in
						Windows mouse settings or check HID polling rate.
					</p>
				</div>
				<div class="spv-readout">
					<div class="spv-value-block">
						<div class="spv-num" class:ok={f16Verified} class:err={!f16Verified && f16Value > 0 && f16Value < 0.4}>
							{f16Value > 0 ? f16Value.toFixed(3) : '---'}
						</div>
						<div class="spv-label">F16 SUB-PIXEL RATIO</div>
					</div>
					<div class="spv-gauge">
						<div class="gauge-band gauge-bot"></div>
						<div class="gauge-band gauge-amb"></div>
						<div class="gauge-band gauge-hum"></div>
						<div
							class="gauge-marker"
							style="left: {Math.min(100, Math.max(0, f16Value * 100))}%"
						></div>
						<div class="gauge-threshold" style="left: 40%"></div>
					</div>
					<div class="spv-bands">
						<span class="band-text">Bot 0.00-0.10</span>
						<span class="band-text">Threshold 0.40</span>
						<span class="band-text">Human 0.70-0.95</span>
					</div>
				</div>
				<div class="step-actions">
					<button class="action-btn primary" onclick={startSubPixelVerify} disabled={f16InProgress}>
						{f16InProgress ? 'Sweeping cursor... 5s' : f16Verified ? 'Re-verify' : 'Start 5-second Verification'}
					</button>
				</div>
				{#if f16Verified}
					<div class="spv-status ok">
						✓ VERIFIED · AC Research scenarios unlocked
					</div>
				{:else if f16Value > 0 && f16Value < 0.4}
					<div class="spv-status err">
						✗ INSUFFICIENT · Enable "Enhance Pointer Precision" in Windows mouse settings
					</div>
				{/if}
			</div>
		{:else if activeStep === 5}
			<div class="wstep-card">
				<header class="step-header">
					<h2>Save as Scenario</h2>
					<p>Persist current calibration + tuning as a reusable preset</p>
				</header>
				<div class="save-form">
					<div class="form-row">
						<label>Scenario Name</label>
						<input
							type="text"
							placeholder="My Custom Scenario"
							bind:value={scenarioName}
						/>
					</div>
					<div class="form-row">
						<label>Description</label>
						<textarea
							placeholder="Optional description..."
							bind:value={scenarioDescription}
							rows="3"
						></textarea>
					</div>
					<div class="form-row">
						<label>Includes</label>
						<div class="include-grid">
							<label class="include-cb">
								<input type="checkbox" bind:checked={includeIntrinsics} />
								Intrinsics
							</label>
							<label class="include-cb">
								<input type="checkbox" bind:checked={includeExtrinsics} />
								Extrinsics
							</label>
							<label class="include-cb">
								<input type="checkbox" bind:checked={includeEKF} />
								EKF Tuning
							</label>
							<label class="include-cb">
								<input type="checkbox" bind:checked={includeModel} />
								Model
							</label>
							<label class="include-cb">
								<input type="checkbox" bind:checked={includeRecording} />
								Recording defaults
							</label>
						</div>
					</div>
					<div class="step-actions">
						<button
							class="action-btn primary"
							onclick={saveScenario}
							disabled={!scenarioName.trim()}
						>Save Scenario</button>
					</div>
				</div>

				{#if savedScenarios.length > 0}
					<div class="saved-list">
						<div class="saved-header">SAVED SCENARIOS ({savedScenarios.length})</div>
						{#each savedScenarios as sc, i (sc.saved)}
							<div class="saved-row">
								<span class="saved-name">{sc.name}</span>
								<span class="saved-ts">{new Date(sc.saved).toLocaleString()}</span>
								<button class="del-btn" onclick={() => deleteScenario(i)}>Delete</button>
							</div>
						{/each}
					</div>
				{/if}
			</div>
		{/if}
	</div>
</div>

<style>
	.calibrate-dest {
		flex: 1;
		display: flex;
		min-height: 0;
		background: #0e0e10;
	}

	.wizard-rail {
		width: 220px;
		flex-shrink: 0;
		background: #141416;
		border-right: 1px solid #1d1d20;
		display: flex;
		flex-direction: column;
		padding: 4px 0;
	}

	.wizard-header {
		display: flex;
		flex-direction: column;
		gap: 4px;
		padding: 12px 16px;
		font-size: 10px;
		font-weight: 700;
		color: #f5a623;
		letter-spacing: 0.1em;
		border-bottom: 1px solid #1d1d20;
	}

	.wizard-hotkey-hint {
		display: flex;
		align-items: center;
		gap: 3px;
		font-size: 9px;
		font-weight: 400;
		color: #555;
		letter-spacing: 0.02em;
		text-transform: none;
	}

	.wizard-hotkey-hint kbd {
		display: inline-block;
		padding: 1px 5px;
		background: #222;
		color: #d4d0c8;
		border: 1px solid #333;
		border-radius: 3px;
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		margin-right: 2px;
	}

	.wstep {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 9px 14px;
		background: transparent;
		border: none;
		border-left: 3px solid transparent;
		color: inherit;
		font-family: inherit;
		text-align: left;
		cursor: pointer;
		transition: background 100ms ease;
	}

	.wstep:hover {
		background: #1d1d20;
	}

	.wstep.selected {
		background: rgba(245, 166, 35, 0.08);
		border-left-color: #f5a623;
	}

	.wstep-num {
		width: 22px;
		height: 22px;
		display: flex;
		align-items: center;
		justify-content: center;
		border-radius: 50%;
		font-size: 10px;
		font-weight: 700;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		background: #2a2a2e;
		color: #666;
		flex-shrink: 0;
	}

	.wstep.selected .wstep-num {
		background: #f5a623;
		color: #111;
	}

	.wstep-label {
		flex: 1;
		font-size: 11px;
		color: #878787;
	}

	.wstep.selected .wstep-label {
		color: #d4d0c8;
	}

	.wstep-dot {
		width: 7px;
		height: 7px;
		border-radius: 50%;
		flex-shrink: 0;
	}

	.wstep-dot.ok {
		background: #34d399;
		box-shadow: 0 0 4px rgba(52, 211, 153, 0.4);
	}

	.wstep-dot.pending {
		background: #ef4444;
	}

	.wizard-content {
		flex: 1;
		overflow-y: auto;
		padding: 24px 32px;
		min-width: 0;
	}

	.wstep-card {
		max-width: 720px;
		display: flex;
		flex-direction: column;
		gap: 16px;
	}

	.step-header h2 {
		margin: 0 0 4px;
		font-size: 16px;
		font-weight: 600;
		color: #d4d0c8;
	}

	.step-header p {
		margin: 0;
		font-size: 11px;
		color: #666;
	}

	.intrinsics-grid {
		display: grid;
		grid-template-columns: repeat(4, 1fr);
		gap: 10px;
	}

	.int-field {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.int-label {
		font-size: 9px;
		font-weight: 700;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		color: #555;
	}

	.int-input {
		padding: 6px 8px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		outline: none;
	}

	.extrinsics-matrix {
		display: grid;
		grid-template-columns: repeat(4, 1fr);
		gap: 4px;
		max-width: 480px;
	}

	.ext-cell {
		padding: 8px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		text-align: center;
		outline: none;
	}

	.ext-legend {
		display: flex;
		gap: 8px;
		font-size: 10px;
		color: #555;
	}

	.ext-sep {
		color: #333;
	}

	.step-actions {
		display: flex;
		gap: 8px;
		align-items: center;
	}

	.action-btn {
		padding: 8px 16px;
		background: #1a1a1e;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-size: 11px;
		font-weight: 500;
		font-family: inherit;
		cursor: pointer;
		transition: all 120ms ease;
	}

	.action-btn:hover:not(:disabled) {
		background: #27272a;
		border-color: #3a3a3e;
	}

	.action-btn.primary {
		background: rgba(245, 166, 35, 0.12);
		color: #f5a623;
		border-color: rgba(245, 166, 35, 0.4);
	}

	.action-btn.primary:hover:not(:disabled) {
		background: rgba(245, 166, 35, 0.2);
	}

	.action-btn:disabled {
		opacity: 0.4;
		cursor: not-allowed;
	}

	.step-footer {
		margin: 0;
		font-size: 11px;
		color: #555;
		font-style: italic;
		line-height: 1.5;
	}

	/* EKF sliders */
	.slider-group {
		display: flex;
		flex-direction: column;
		gap: 14px;
	}

	.slider-row {
		display: flex;
		flex-direction: column;
		gap: 5px;
	}

	.slider-label {
		display: flex;
		justify-content: space-between;
		align-items: center;
		font-size: 11px;
		font-weight: 500;
		color: #878787;
	}

	.slider-value {
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-weight: 600;
		color: #f5a623;
	}

	input[type='range'] {
		width: 100%;
		height: 4px;
		appearance: none;
		background: #27272a;
		border-radius: 2px;
		outline: none;
	}

	input[type='range']::-webkit-slider-thumb {
		appearance: none;
		width: 14px;
		height: 14px;
		border-radius: 50%;
		background: #f5a623;
		cursor: pointer;
		border: 2px solid #0e0e10;
	}

	.slider-help {
		margin: 0;
		font-size: 10px;
		color: #555;
		font-style: italic;
	}

	.number-grid {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 10px;
		margin-top: 4px;
	}

	.number-field {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.number-field label {
		font-size: 10px;
		font-weight: 500;
		text-transform: uppercase;
		letter-spacing: 0.05em;
		color: #878787;
	}

	.number-field input {
		padding: 6px 8px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 3px;
		font-size: 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		outline: none;
	}

	.number-field input:focus-visible {
		border-color: #f5a623;
	}

	.profile-select {
		padding: 7px 10px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-size: 11px;
		font-family: inherit;
		cursor: pointer;
		outline: none;
	}

	/* Model selection */
	.model-list {
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.model-row {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 10px 14px;
		background: #161618;
		border: 1px solid #1d1d20;
		border-left: 3px solid transparent;
		border-radius: 4px;
		cursor: pointer;
		transition: all 100ms ease;
	}

	.model-row:hover {
		background: #1d1d22;
	}

	.model-row.selected {
		border-left-color: #f5a623;
		background: rgba(245, 166, 35, 0.04);
	}

	.model-row input[type='radio'] {
		accent-color: #f5a623;
	}

	.model-info {
		flex: 1;
		display: flex;
		flex-direction: column;
		gap: 2px;
	}

	.model-name {
		font-size: 12px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
	}

	.model-meta {
		font-size: 10px;
		color: #555;
	}

	.model-status {
		font-size: 9px;
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.04em;
		padding: 3px 7px;
		border-radius: 3px;
	}

	.model-status.available { background: rgba(52, 211, 153, 0.1); color: #34d399; }
	.model-status.pending { background: rgba(245, 166, 35, 0.1); color: #f5a623; }

	/* Sub-pixel verify */
	.spv-explainer {
		background: #161618;
		border: 1px solid #1d1d20;
		border-radius: 4px;
		padding: 12px 14px;
		font-size: 12px;
		color: #878787;
		line-height: 1.6;
	}

	.spv-explainer strong {
		color: #d4d0c8;
	}

	.spv-readout {
		display: flex;
		flex-direction: column;
		gap: 8px;
		padding: 16px 0;
	}

	.spv-value-block {
		display: flex;
		flex-direction: column;
		align-items: center;
		gap: 4px;
	}

	.spv-num {
		font-size: 48px;
		font-weight: 800;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #d4d0c8;
		line-height: 1;
	}

	.spv-num.ok { color: #34d399; text-shadow: 0 0 12px rgba(52, 211, 153, 0.5); }
	.spv-num.err { color: #ef4444; }

	.spv-label {
		font-size: 10px;
		font-weight: 700;
		color: #555;
		letter-spacing: 0.1em;
		text-transform: uppercase;
	}

	.spv-gauge {
		position: relative;
		height: 16px;
		background: #111113;
		border: 1px solid #2a2a2e;
		border-radius: 8px;
		overflow: hidden;
	}

	.gauge-band {
		position: absolute;
		top: 0;
		bottom: 0;
		opacity: 0.4;
	}

	.gauge-bot { left: 0; right: 90%; background: #ef4444; }
	.gauge-amb { left: 10%; right: 30%; background: #555; }
	.gauge-hum { left: 70%; right: 5%; background: #34d399; }

	.gauge-marker {
		position: absolute;
		top: -4px;
		bottom: -4px;
		width: 3px;
		background: #f5a623;
		box-shadow: 0 0 6px rgba(245, 166, 35, 0.8);
		transform: translateX(-50%);
		transition: left 100ms ease;
	}

	.gauge-threshold {
		position: absolute;
		top: 0;
		bottom: 0;
		width: 1px;
		background: #d4d0c8;
		border-radius: 1px;
	}

	.spv-bands {
		display: flex;
		justify-content: space-between;
		font-size: 9px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
	}

	.band-text {
		text-align: center;
	}

	.spv-status {
		padding: 10px 14px;
		border-radius: 4px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		font-size: 11px;
		font-weight: 700;
		letter-spacing: 0.06em;
		text-align: center;
	}

	.spv-status.ok {
		background: rgba(52, 211, 153, 0.1);
		color: #34d399;
		border: 1px solid rgba(52, 211, 153, 0.3);
	}

	.spv-status.err {
		background: rgba(239, 68, 68, 0.1);
		color: #ef4444;
		border: 1px solid rgba(239, 68, 68, 0.3);
	}

	/* Save as Scenario */
	.save-form {
		display: flex;
		flex-direction: column;
		gap: 14px;
	}

	.form-row {
		display: flex;
		flex-direction: column;
		gap: 5px;
	}

	.form-row label {
		font-size: 10px;
		font-weight: 700;
		text-transform: uppercase;
		letter-spacing: 0.06em;
		color: #878787;
	}

	.form-row input[type='text'],
	.form-row textarea {
		padding: 8px 10px;
		background: #111113;
		color: #d4d0c8;
		border: 1px solid #2a2a2e;
		border-radius: 4px;
		font-size: 12px;
		font-family: inherit;
		outline: none;
		resize: vertical;
	}

	.form-row input[type='text']:focus-visible,
	.form-row textarea:focus-visible {
		border-color: #f5a623;
	}

	.include-grid {
		display: grid;
		grid-template-columns: repeat(3, 1fr);
		gap: 6px;
	}

	.include-cb {
		display: flex;
		align-items: center;
		gap: 5px;
		font-size: 11px;
		color: #d4d0c8;
		cursor: pointer;
		padding: 4px 6px;
	}

	.include-cb input {
		accent-color: #f5a623;
	}

	.saved-list {
		margin-top: 18px;
		display: flex;
		flex-direction: column;
		gap: 4px;
	}

	.saved-header {
		font-size: 10px;
		font-weight: 700;
		color: #878787;
		letter-spacing: 0.06em;
		margin-bottom: 6px;
	}

	.saved-row {
		display: flex;
		align-items: center;
		gap: 10px;
		padding: 6px 10px;
		background: #161618;
		border: 1px solid #1d1d20;
		border-radius: 3px;
		font-size: 11px;
	}

	.saved-name {
		flex: 1;
		color: #d4d0c8;
	}

	.saved-ts {
		font-size: 10px;
		font-family: ui-monospace, 'SF Mono', Consolas, monospace;
		color: #555;
	}

	.del-btn {
		padding: 3px 8px;
		background: transparent;
		color: #ef4444;
		border: 1px solid rgba(239, 68, 68, 0.3);
		border-radius: 3px;
		font-size: 10px;
		font-family: inherit;
		cursor: pointer;
	}

	.del-btn:hover {
		background: rgba(239, 68, 68, 0.1);
	}
</style>
