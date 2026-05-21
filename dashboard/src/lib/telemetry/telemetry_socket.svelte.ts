// =============================================================================
// roc-ai-vision · dashboard · telemetry_socket.ts
//
// Live binary WebSocket client that consumes the C++ orchestrator's egress
// channel (`core/transport/ws_egress.cpp` — IXWebSocket binary server bound
// to `127.0.0.1:8765` per `research_ipc_middleware_2026.md` §8.2).
//
// Inbound:  raw ArrayBuffer wire frames (12 B header + N × 86 B/track) →
//           handed straight to `wire_decoder.decode_frame()` with the
//           shared TrackPool — same path the mock_frame_source uses, so the
//           canvas RAF doesn't care which source produced the bytes.
//
// Outbound: 22-byte little-endian OperatorCommand frames matching
//           `core/transport/include/roc/transport/operator_command.hpp`:
//           offset 0  u8   kind        (1=LockTarget, 2=ReleaseLock, 3=UpdateSelectorWeights)
//           offset 1  u8   reserved    (always 0)
//           offset 2  u64  target_track_id
//           offset 10 f32  weight_proximity
//           offset 14 f32  weight_confidence
//           offset 18 f32  weight_stability
//           offset 22 ─── end
//
// Steady-state heap allocation: zero (one ArrayBuffer + one DataView reused
// for every outbound command; ArrayBuffer inbound is the browser's per-frame
// allocation which we cannot avoid).
// =============================================================================

import { decode_frame } from './wire_decoder';

export type ConnectionState = 'disconnected' | 'connecting' | 'connected';

// Mirror of `roc::transport::OperatorCommandKind` — DO NOT reorder; the
// numeric values are wire-format identifiers.
export const OperatorCommandKind = {
	None: 0,
	LockTarget: 1,
	ReleaseLock: 2,
	UpdateSelectorWeights: 3,
	SetSmoothingAlpha: 4,
	SetDeadbandRadiusPx: 5,
	SetFovRadiusPx: 6,
	SetTargetingAnchor: 7
} as const;

const COMMAND_WIRE_SIZE = 22;
const RECONNECT_BASE_MS = 1000;
const RECONNECT_MAX_MS = 8000;

// Reusable encoder for outbound commands. Allocated once at module load.
const _cmdBuf = new ArrayBuffer(COMMAND_WIRE_SIZE);
const _cmdView = new DataView(_cmdBuf);

class TelemetrySocketStore {
	// Reactive surface — the Observer Matrix panel and any operator UI read
	// these. The canvas RAF deliberately does NOT — it polls the TrackPool
	// typed-array columns directly.
	state = $state<ConnectionState>('disconnected');
	bytesReceived = $state(0);
	framesReceived = $state(0);
	reconnectAttempt = $state(0);
	lastError = $state<string | null>(null);
	url = $state<string>('ws://127.0.0.1:8765');

	#ws: WebSocket | null = null;
	#reconnectTimer: ReturnType<typeof setTimeout> | null = null;
	#autoReconnect = false;

	constructor(url: string) {
		this.url = url;
	}

	// -----------------------------------------------------------------------
	// Lifecycle
	// -----------------------------------------------------------------------
	connect(): void {
		this.#autoReconnect = true;
		this.#open();
	}

	disconnect(): void {
		this.#autoReconnect = false;
		if (this.#reconnectTimer !== null) {
			clearTimeout(this.#reconnectTimer);
			this.#reconnectTimer = null;
		}
		if (this.#ws !== null) {
			try {
				this.#ws.close();
			} catch {
				/* socket may already be dead — ignore */
			}
			this.#ws = null;
		}
		this.state = 'disconnected';
	}

	setUrl(url: string): void {
		if (url === this.url) return;
		this.url = url;
		if (this.#autoReconnect) {
			this.disconnect();
			this.#autoReconnect = true;
			this.#open();
		}
	}

	// -----------------------------------------------------------------------
	// Operator commands (reverse channel)
	// -----------------------------------------------------------------------
	// Track ids are u64 on the wire. JavaScript Number can represent integers
	// up to 2^53; the kinematics engine never issues ids that large in
	// practice, but we expose hi/lo to be honest about the protocol.
	lockTarget(trackIdLo: number, trackIdHi = 0): boolean {
		this.#writeCommand(OperatorCommandKind.LockTarget, trackIdLo, trackIdHi);
		return this.#sendCommand();
	}

	releaseLock(): boolean {
		this.#writeCommand(OperatorCommandKind.ReleaseLock, 0, 0);
		return this.#sendCommand();
	}

	updateSelectorWeights(
		weight_proximity: number,
		weight_confidence: number,
		weight_stability: number
	): boolean {
		this.#writeCommand(
			OperatorCommandKind.UpdateSelectorWeights,
			0,
			0,
			weight_proximity,
			weight_confidence,
			weight_stability
		);
		return this.#sendCommand();
	}

	setSmoothingAlpha(alpha: number): boolean {
		this.#writeValueCommand(OperatorCommandKind.SetSmoothingAlpha, alpha);
		return this.#sendCommand();
	}

	setDeadbandRadiusPx(px: number): boolean {
		this.#writeValueCommand(OperatorCommandKind.SetDeadbandRadiusPx, px);
		return this.#sendCommand();
	}

	setFovRadiusPx(px: number): boolean {
		this.#writeValueCommand(OperatorCommandKind.SetFovRadiusPx, px);
		return this.#sendCommand();
	}

	setTargetingAnchor(anchorOrdinal: number): boolean {
		this.#writeValueCommand(OperatorCommandKind.SetTargetingAnchor, anchorOrdinal);
		return this.#sendCommand();
	}

	// -----------------------------------------------------------------------
	// Internals
	// -----------------------------------------------------------------------
	#open(): void {
		this.state = 'connecting';
		this.lastError = null;
		let socket: WebSocket;
		try {
			socket = new WebSocket(this.url);
		} catch (e) {
			this.lastError = e instanceof Error ? e.message : String(e);
			this.state = 'disconnected';
			this.#scheduleReconnect();
			return;
		}
		socket.binaryType = 'arraybuffer';
		this.#ws = socket;

		socket.onopen = () => {
			this.state = 'connected';
			this.reconnectAttempt = 0;
		};

		socket.onmessage = (ev: MessageEvent) => {
			if (!(ev.data instanceof ArrayBuffer)) return;
			this.bytesReceived += ev.data.byteLength;
			this.framesReceived += 1;
			// The decoder dispatches internally on byte 0 (version) and routes to
			// the right pool — track / actuator / bg-odometry (per
			// `200-dashboard-telemetry.mdc` wire-format version dispatch table).
			decode_frame(ev.data);
		};

		socket.onerror = () => {
			// `MessageEvent` for `error` carries no useful detail in the spec —
			// the actual reason (DNS, refused, etc.) typically shows up in the
			// devtools console; we mark the symptom and wait for `onclose`.
			this.lastError = 'WebSocket error (see browser console for detail)';
		};

		socket.onclose = () => {
			this.#ws = null;
			this.state = 'disconnected';
			// Clear all telemetry pools so stale tracks/actuator/bg data
			// don't persist on the canvas after disconnect.
			trackPool.reset();
			actuatorPool.reset();
			bgOdometryPool.reset();
			if (this.#autoReconnect) this.#scheduleReconnect();
		};
	}

	#scheduleReconnect(): void {
		if (this.#reconnectTimer !== null) return;
		// Exponential backoff: 1 s → 2 s → 4 s → 8 s, then hold at 8 s.
		const delay = Math.min(
			RECONNECT_BASE_MS * Math.pow(2, this.reconnectAttempt),
			RECONNECT_MAX_MS
		);
		this.reconnectAttempt += 1;
		this.#reconnectTimer = setTimeout(() => {
			this.#reconnectTimer = null;
			this.#open();
		}, delay);
	}

	#writeCommand(
		kind: number,
		targetLo: number,
		targetHi: number,
		weightProx = 0,
		weightConf = 0,
		weightStab = 0
	): void {
		_cmdView.setUint8(0, kind);
		_cmdView.setUint8(1, 0);
		_cmdView.setUint32(2, targetLo >>> 0, true);
		_cmdView.setUint32(6, targetHi >>> 0, true);
		_cmdView.setFloat32(10, weightProx, true);
		_cmdView.setFloat32(14, weightConf, true);
		_cmdView.setFloat32(18, weightStab, true);
	}

	#writeValueCommand(kind: number, value: number): void {
		_cmdView.setUint8(0, kind);
		_cmdView.setUint8(1, 0);
		_cmdView.setUint32(2, 0, true);
		_cmdView.setUint32(6, 0, true);
		_cmdView.setFloat32(10, value, true);
		_cmdView.setFloat32(14, 0, true);
		_cmdView.setFloat32(18, 0, true);
	}

	#sendCommand(): boolean {
		if (this.state !== 'connected' || this.#ws === null) return false;
		try {
			this.#ws.send(_cmdBuf);
			return true;
		} catch (e) {
			this.lastError = e instanceof Error ? e.message : String(e);
			return false;
		}
	}
}

// Singleton instance — matches the `trackPool` / `aimingConfig` /
// `hardwareConfig` pattern used elsewhere in the dashboard.
export const telemetrySocket = new TelemetrySocketStore('ws://127.0.0.1:8765');
export type TelemetrySocket = TelemetrySocketStore;
