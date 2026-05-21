/**
 * roc-ai-vision · dashboard · biometrics/mouse_capture.svelte.ts
 *
 * Pointer-Lock-driven mouse-input capture engine for the Biometrics
 * destination (Amendment v6 §14). Captures raw mouse deltas at the device's
 * native report rate via `pointermove` `movementX/movementY`, packs them into
 * a zero-allocation ring buffer, and exposes two slots (current + previous)
 * for the dashboard panels.
 *
 * Hard constraints (UI_SPEC_SHEET §16.6):
 *  - MUST NOT import from $lib/telemetry/* or any *_pool.svelte.ts. This
 *    destination has a self-contained data plane separate from the C++
 *    vision pipeline.
 *  - Pointer Lock MUST be requested on a user gesture (click), never on
 *    mount. Browser will reject otherwise.
 *  - Esc MUST cleanly release pointer lock + drop SM to IDLE. In-flight
 *    RECORDING is dropped (NOT rotated to PREVIOUS) since a forced cancel
 *    is not a clean session.
 */

import { recordingLabel } from './labels.svelte';

// =============================================================================
// Constants
// =============================================================================

/** Capacity of each ring buffer. 16384 samples = ~16s @ 1 kHz, ~80s @ 200 Hz. */
export const RING_CAPACITY = 16384;

/** Mouse button index for "Mouse4" (a.k.a. browser back button, X1). */
const MOUSE4_BUTTON = 3;

/** Spacebar fallback for keyboards / mice without side buttons. */
const SPACE_KEY = ' ';

// =============================================================================
// Types
// =============================================================================

export type CaptureState =
	| 'IDLE'                   // pointer lock NOT held
	| 'POINTERLOCK_REQUESTED'  // requestPointerLock() in flight
	| 'ARMED'                  // pointer lock held, waiting for Mouse4/Space
	| 'RECORDING'              // Mouse4/Space held, samples landing in currentSlot
	| 'COMPLETED';             // brief transient between RECORDING and ARMED

/**
 * A capture slot — the typed-array backing store for one session worth of
 * mouse deltas. Two slots exist in steady state: `current` (live, being
 * recorded into) and `previous` (frozen, last completed session).
 */
export interface CaptureSlot {
	dx: Float32Array;
	dy: Float32Array;
	t: Float32Array;       // ms since session start
	fill: number;          // number of valid samples [0, RING_CAPACITY]
	head: number;          // next write index [0, RING_CAPACITY)
	startedMs: number;     // wall-clock ms when session began (Date.now())
	durationMs: number;    // wall-clock duration; updated each tick while RECORDING
	label: string;         // dataset label captured at session start
}

function makeEmptySlot(): CaptureSlot {
	return {
		dx: new Float32Array(RING_CAPACITY),
		dy: new Float32Array(RING_CAPACITY),
		t: new Float32Array(RING_CAPACITY),
		fill: 0,
		head: 0,
		startedMs: 0,
		durationMs: 0,
		label: 'UNTAGGED'
	};
}

// =============================================================================
// Reactive store
// =============================================================================

class MouseCaptureStore {
	state = $state<CaptureState>('IDLE');
	/** Bumped every time `current` gets new samples — drives RAF redraw. */
	currentVersion = $state(0);
	/** Bumped when `previous` gets a fresh snapshot — drives PREVIOUS panel redraw. */
	previousVersion = $state(0);
	/** Last error message from pointer lock subsystem (e.g. "user denied"). */
	lastError = $state<string | null>(null);
	/** Whether a recording has ever completed in this session (drives UI affordances). */
	hasPrevious = $state(false);

	current: CaptureSlot = makeEmptySlot();
	previous: CaptureSlot = makeEmptySlot();

	private captureEl: HTMLElement | null = null;
	private recordingT0: number = 0;
	/** `t` reference: performance.now() at session start; sample t = now - t0. */
	private samplePerfT0: number = 0;
	private bound = false;
	/**
	 * True iff `current` holds the result of a cleanly-committed (Mouse4/Space
	 * released, not blurred/Esc'd) recording. Drives the rotate-on-begin
	 * behavior: only committed sessions get promoted into `previous` when the
	 * NEXT recording starts. Dropped sessions are discarded silently and do
	 * NOT displace the prior `previous` slot, so the user always has the most
	 * recent CLEAN session available for live comparison.
	 */
	private currentCommitted = false;

	// ---------------------------------------------------------------------------
	// Lifecycle
	// ---------------------------------------------------------------------------

	/** Wire up the capture area (the destination's main click target). */
	bind(el: HTMLElement): void {
		if (this.bound) this.unbind();
		this.captureEl = el;
		el.addEventListener('click', this.onCaptureClick);
		document.addEventListener('pointerlockchange', this.onPointerLockChange);
		document.addEventListener('pointerlockerror', this.onPointerLockError);
		document.addEventListener('pointermove', this.onPointerMove);
		document.addEventListener('pointerdown', this.onPointerDown);
		document.addEventListener('pointerup', this.onPointerUp);
		document.addEventListener('keydown', this.onKeyDown);
		document.addEventListener('keyup', this.onKeyUp);
		window.addEventListener('blur', this.onWindowBlur);
		this.bound = true;
	}

	/** Tear down listeners and release pointer lock if held. */
	unbind(): void {
		if (!this.bound) return;
		const el = this.captureEl;
		if (el) el.removeEventListener('click', this.onCaptureClick);
		document.removeEventListener('pointerlockchange', this.onPointerLockChange);
		document.removeEventListener('pointerlockerror', this.onPointerLockError);
		document.removeEventListener('pointermove', this.onPointerMove);
		document.removeEventListener('pointerdown', this.onPointerDown);
		document.removeEventListener('pointerup', this.onPointerUp);
		document.removeEventListener('keydown', this.onKeyDown);
		document.removeEventListener('keyup', this.onKeyUp);
		window.removeEventListener('blur', this.onWindowBlur);
		if (document.pointerLockElement) document.exitPointerLock();
		this.captureEl = null;
		this.bound = false;
	}

	// ---------------------------------------------------------------------------
	// Public actions (called from UI)
	// ---------------------------------------------------------------------------

	/** Clear both slots and reset state to IDLE (with pointer lock released). */
	clearHistory(): void {
		this.resetSlot(this.current);
		this.resetSlot(this.previous);
		this.hasPrevious = false;
		this.currentCommitted = false;
		this.currentVersion++;
		this.previousVersion++;
		if (document.pointerLockElement) document.exitPointerLock();
		this.state = 'IDLE';
		this.lastError = null;
	}

	// ---------------------------------------------------------------------------
	// Internal — state transitions
	// ---------------------------------------------------------------------------

	private resetSlot(slot: CaptureSlot): void {
		slot.fill = 0;
		slot.head = 0;
		slot.startedMs = 0;
		slot.durationMs = 0;
	}

	private snapshotCurrentToPrevious(): void {
		const c = this.current;
		const p = this.previous;
		p.fill = c.fill;
		p.head = c.head;
		p.startedMs = c.startedMs;
		p.durationMs = c.durationMs;
		p.label = c.label;
		// Bulk copy the active range. Float32Array.set is a memcpy under the hood.
		p.dx.set(c.dx);
		p.dy.set(c.dy);
		p.t.set(c.t);
		this.hasPrevious = true;
		this.previousVersion++;
	}

	private beginRecording(): void {
		if (this.state !== 'ARMED') return;
		// Rotate the just-completed (clean) session into PREVIOUS *before*
		// wiping current. This is the inversion of the naive "snapshot at end"
		// model: rotating at begin keeps PREVIOUS = the prior committed
		// session for the entire duration of the new recording, so the user
		// can compare live against their last clean trace.
		if (this.currentCommitted && this.current.fill > 0) {
			this.snapshotCurrentToPrevious();
		}
		this.resetSlot(this.current);
		this.currentCommitted = false;
		this.recordingT0 = Date.now();
		this.samplePerfT0 = performance.now();
		this.current.startedMs = this.recordingT0;
		this.current.label = recordingLabel.value;
		this.state = 'RECORDING';
		this.currentVersion++;
	}

	private endRecording(commit: boolean): void {
		if (this.state !== 'RECORDING') return;
		this.current.durationMs = Date.now() - this.recordingT0;
		// Promotion to PREVIOUS deliberately does NOT happen here — it is
		// deferred to the start of the next `beginRecording()` so the user
		// keeps the prior session visible for comparison.
		this.currentCommitted = commit && this.current.fill > 0;
		this.state = 'COMPLETED';
		this.currentVersion++;
		// Schedule transition back to ARMED on next microtask so any final
		// pointermove for the same event-loop tick has time to land.
		queueMicrotask(() => {
			if (this.state === 'COMPLETED' && document.pointerLockElement) {
				this.state = 'ARMED';
			}
		});
	}

	// ---------------------------------------------------------------------------
	// Internal — DOM event handlers (arrow methods so `this` is bound for
	// add/removeEventListener parity)
	// ---------------------------------------------------------------------------

	private onCaptureClick = (): void => {
		if (this.state !== 'IDLE') return;
		const el = this.captureEl;
		if (!el) return;
		this.state = 'POINTERLOCK_REQUESTED';
		this.lastError = null;
		// requestPointerLock returns a promise in modern browsers; in older
		// ones it returns undefined and signals success via pointerlockchange.
		// We tolerate both.
		try {
			const result = el.requestPointerLock();
			if (result && typeof (result as Promise<void>).catch === 'function') {
				(result as Promise<void>).catch((err: Error) => {
					this.lastError = err.message || 'requestPointerLock rejected';
					this.state = 'IDLE';
				});
			}
		} catch (err) {
			this.lastError = (err as Error).message ?? 'requestPointerLock threw';
			this.state = 'IDLE';
		}
	};

	private onPointerLockChange = (): void => {
		if (document.pointerLockElement === this.captureEl) {
			this.state = 'ARMED';
		} else {
			if (this.state === 'RECORDING') {
				// Lock lost mid-session: drop, do NOT commit.
				this.endRecording(false);
			}
			this.state = 'IDLE';
		}
	};

	private onPointerLockError = (): void => {
		this.lastError = 'pointer lock error (browser refused)';
		this.state = 'IDLE';
	};

	private onPointerMove = (e: PointerEvent): void => {
		if (this.state !== 'RECORDING') return;
		const slot = this.current;
		if (slot.fill >= RING_CAPACITY) return; // ring full — drop further samples
		const i = slot.head;
		slot.dx[i] = e.movementX;
		slot.dy[i] = e.movementY;
		slot.t[i] = performance.now() - this.samplePerfT0;
		slot.head = (i + 1) % RING_CAPACITY;
		slot.fill++;
		slot.durationMs = Date.now() - this.recordingT0;
		this.currentVersion++;
	};

	private onPointerDown = (e: PointerEvent): void => {
		if (this.state !== 'ARMED') return;
		if (e.button !== MOUSE4_BUTTON) return;
		this.beginRecording();
	};

	private onPointerUp = (e: PointerEvent): void => {
		if (this.state !== 'RECORDING') return;
		if (e.button !== MOUSE4_BUTTON) return;
		this.endRecording(true);
	};

	private onKeyDown = (e: KeyboardEvent): void => {
		if (this.state !== 'ARMED') return;
		if (e.key !== SPACE_KEY) return;
		if (e.repeat) return; // ignore auto-repeat
		e.preventDefault();
		this.beginRecording();
	};

	private onKeyUp = (e: KeyboardEvent): void => {
		if (this.state !== 'RECORDING') return;
		if (e.key !== SPACE_KEY) return;
		e.preventDefault();
		this.endRecording(true);
	};

	private onWindowBlur = (): void => {
		if (this.state === 'RECORDING') {
			// Window lost focus mid-session: drop, do NOT commit.
			this.endRecording(false);
		}
		if (document.pointerLockElement) document.exitPointerLock();
	};
}

export const mouseCapture = new MouseCaptureStore();
