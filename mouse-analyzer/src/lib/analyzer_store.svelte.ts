// Mouse sample ring buffer + metrics. Zero per-frame allocation.
// All computation is O(N) over the buffer window, which is bounded.

export interface MouseSample {
	x: number;       // absolute screen X
	y: number;       // absolute screen Y
	dx: number;      // delta X since last sample
	dy: number;      // delta Y since last sample
	ts: number;      // performance.now() ms
}

export interface SessionMetrics {
	duration_s: number;
	path_points: number;
	reports: number;
	dir_changes: number;
	sign_flips_x: number;
	sign_flips_y: number;
	total_flips_per_sec: number;
	avg_report_rate: number;
	min_report_rate: number;
	max_report_rate: number;
	avg_velocity: number;
	max_velocity: number;
}

const MAX_SAMPLES = 65536; // ~10 min at 100 Hz

export class MouseAnalyzerStore {
	samples = $state<MouseSample[]>([]);
	// ring buffer head index
	private head = 0;

	private prevSample: MouseSample | null = null;
	private recording = $state(false);
	private sessionStart = 0;

	// smoothed velocity for sign flip detection
	private smoothedVx = 0;
	private smoothedVy = 0;
	private readonly ewmaAlpha: number;

	// ring buffers for rolling metrics
	private velBuf = new Float64Array(256);
	private velBufIdx = 0;
	private rateBuf = new Float64Array(256);
	private rateBufIdx = 0;
	private flipBuf = new Float64Array(256);
	private flipBufIdx = 0;

	// Previous session frozen snapshot
	prevSamples: MouseSample[] = $state([]);
	prevMetrics: SessionMetrics | null = $state(null);

	currentMetrics: SessionMetrics | null = $state(null);
	plotWindowSec = $state(3.0);
	smoothingAlpha = $state(0.20);
	dirChangeAngleDeg = $state(45);
	directionChanges = $state(0);
	signFlipsX = $state(0);
	signFlipsY = $state(0);

	constructor(alpha = 0.20) {
		this.ewmaAlpha = alpha;
	}

	startRecording() {
		this.samples = [];
		this.head = 0;
		this.prevSample = null;
		this.smoothedVx = 0;
		this.smoothedVy = 0;
		this.velBuf.fill(0);
		this.velBufIdx = 0;
		this.rateBuf.fill(0);
		this.rateBufIdx = 0;
		this.flipBuf.fill(0);
		this.flipBufIdx = 0;
		this.directionChanges = 0;
		this.signFlipsX = 0;
		this.signFlipsY = 0;
		this.recording = true;
		this.sessionStart = performance.now();
		this.currentMetrics = null;
	}

	stopRecording() {
		this.recording = false;
		// Freeze current session as previous
		this.prevSamples = [...this.samples];
		this.prevMetrics = this.computeMetrics(this.prevSamples, this.sessionStart);
		this.currentMetrics = this.computeMetrics(this.samples, this.sessionStart);
	}

	get isRecording(): boolean { return this.recording; }

	addSample(absX: number, absY: number, now: number) {
		if (!this.recording) return;

		let dx = 0, dy = 0;
		if (this.prevSample) {
			dx = absX - this.prevSample.x;
			dy = absY - this.prevSample.y;
		}

		const sample: MouseSample = { x: absX, y: absY, dx, dy, ts: now };

		if (this.samples.length < MAX_SAMPLES) {
			this.samples.push(sample);
		} else {
			this.samples[this.head] = sample;
			this.head = (this.head + 1) % MAX_SAMPLES;
		}

		// Update rolling metrics
		if (this.prevSample && dx !== 0 && dy !== 0) {
			const dt = (now - this.prevSample.ts) / 1000;
			if (dt > 0) {
				const vel = Math.sqrt(dx * dx + dy * dy) / dt;
				this.velBuf[this.velBufIdx] = vel;
				this.velBufIdx = (this.velBufIdx + 1) % this.velBuf.length;

				this.rateBuf[this.rateBufIdx] = 1.0 / dt;
				this.rateBufIdx = (this.rateBufIdx + 1) % this.rateBuf.length;

				// EWMA smoothed velocity
				const a = this.ewmaAlpha;
				this.smoothedVx = a * (dx / dt) + (1 - a) * this.smoothedVx;
				this.smoothedVy = a * (dy / dt) + (1 - a) * this.smoothedVy;

				// Sign flip detection on smoothed velocity
				const prevSignX = Math.sign(this.smoothedVx - a * (dx / dt) - (1 - a) * this.smoothedVx);
				// Actually detect properly:
				if (this.prevSample) {
					const prevDx = this.prevSample.dx;
					const prevDy = this.prevSample.dy;
					if (prevDx !== 0 && dx !== 0 && Math.sign(prevDx) !== Math.sign(dx)) {
						this.signFlipsX++;
					}
					if (prevDy !== 0 && dy !== 0 && Math.sign(prevDy) !== Math.sign(dy)) {
						this.signFlipsY++;
					}
				}
			}
		}

		// Direction change detection
		if (this.prevSample && this.prevSample.dx !== 0 && this.prevSample.dy !== 0
			&& dx !== 0 && dy !== 0) {
			const prevAngle = Math.atan2(this.prevSample.dy, this.prevSample.dx);
			const curAngle = Math.atan2(dy, dx);
			let angleDiff = Math.abs(curAngle - prevAngle) * (180 / Math.PI);
			if (angleDiff > 180) angleDiff = 360 - angleDiff;
			if (angleDiff >= this.dirChangeAngleDeg) {
				this.directionChanges++;
			}
		}

		this.prevSample = sample;
	}

	/** Recompute metrics over the full sample set (or a provided set) */
	computeMetrics(samps?: MouseSample[], sessionStartTs?: number): SessionMetrics {
		const s = samps ?? this.samples;
		const start = sessionStartTs ?? this.sessionStart;
		if (s.length < 2) {
			return {
				duration_s: 0, path_points: s.length, reports: Math.max(0, s.length - 1),
				dir_changes: 0, sign_flips_x: 0, sign_flips_y: 0, total_flips_per_sec: 0,
				avg_report_rate: 0, min_report_rate: 0, max_report_rate: 0,
				avg_velocity: 0, max_velocity: 0
			};
		}

		const duration = (s[s.length - 1].ts - start) / 1000;
		let dirChanges = 0, flipsX = 0, flipsY = 0;
		let totalVel = 0, maxVel = 0;
		const rates: number[] = [];

		for (let i = 1; i < s.length; i++) {
			const prev = s[i - 1];
			const cur = s[i];
			const dt = (cur.ts - prev.ts) / 1000;
			if (dt > 0) {
				const vel = Math.sqrt(cur.dx * cur.dx + cur.dy * cur.dy) / dt;
				totalVel += vel;
				if (vel > maxVel) maxVel = vel;
				rates.push(1.0 / dt);
			}
			if (prev.dx !== 0 && cur.dx !== 0 && Math.sign(prev.dx) !== Math.sign(cur.dx)) flipsX++;
			if (prev.dy !== 0 && cur.dy !== 0 && Math.sign(prev.dy) !== Math.sign(cur.dy)) flipsY++;
			if (prev.dx !== 0 && prev.dy !== 0 && cur.dx !== 0 && cur.dy !== 0) {
				const prevA = Math.atan2(prev.dy, prev.dx);
				const curA = Math.atan2(cur.dy, cur.dx);
				let diff = Math.abs(curA - prevA) * (180 / Math.PI);
				if (diff > 180) diff = 360 - diff;
				if (diff >= this.dirChangeAngleDeg) dirChanges++;
			}
		}

		const reportCount = Math.max(0, s.length - 1);
		return {
			duration_s: duration,
			path_points: s.length,
			reports: reportCount,
			dir_changes: dirChanges,
			sign_flips_x: flipsX,
			sign_flips_y: flipsY,
			total_flips_per_sec: duration > 0 ? (flipsX + flipsY) / duration : 0,
			avg_report_rate: duration > 0 ? reportCount / duration : 0,
			min_report_rate: rates.length > 0 ? Math.min(...rates) : 0,
			max_report_rate: rates.length > 0 ? Math.max(...rates) : 0,
			avg_velocity: reportCount > 0 ? totalVel / reportCount : 0,
			max_velocity: maxVel
		};
	}

	/** Get samples within the plot window (last N seconds) */
	getWindowSamples(): MouseSample[] {
		if (this.samples.length === 0) return [];
		const now = this.samples[this.samples.length - 1].ts;
		const cutoff = now - this.plotWindowSec * 1000;
		const result: MouseSample[] = [];
		for (let i = this.samples.length - 1; i >= 0; i--) {
			if (this.samples[i].ts >= cutoff) {
				result.unshift(this.samples[i]);
			} else {
				break;
			}
		}
		return result;
	}

	clearHistory() {
		this.samples = [];
		this.head = 0;
		this.prevSample = null;
		this.smoothedVx = 0;
		this.smoothedVy = 0;
		this.velBuf.fill(0);
		this.rateBuf.fill(0);
		this.flipBuf.fill(0);
		this.directionChanges = 0;
		this.signFlipsX = 0;
		this.signFlipsY = 0;
		this.prevSamples = [];
		this.prevMetrics = null;
		this.currentMetrics = null;
	}

	/** Rolling window stats for charts */
	getVelocityWindow(): Float64Array {
		const out = new Float64Array(128);
		let count = 0;
		for (let i = Math.max(0, this.velBufIdx - 128); i < this.velBufIdx; i++) {
			const idx = ((i % this.velBuf.length) + this.velBuf.length) % this.velBuf.length;
			if (this.velBuf[idx] > 0) out[count++] = this.velBuf[idx];
		}
		return out.slice(0, count);
	}

	getRateWindow(): Float64Array {
		const out = new Float64Array(128);
		let count = 0;
		for (let i = Math.max(0, this.rateBufIdx - 128); i < this.rateBufIdx; i++) {
			const idx = ((i % this.rateBuf.length) + this.rateBuf.length) % this.rateBuf.length;
			if (this.rateBuf[idx] > 0) out[count++] = this.rateBuf[idx];
		}
		return out.slice(0, count);
	}
}

export const analyzer = new MouseAnalyzerStore();
