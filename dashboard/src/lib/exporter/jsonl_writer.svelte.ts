// JSONL session exporter — §12.2 + R4/R6

import type { SessionLabel } from '$lib/profiles/scenarios';

export type JsonlEventType = 'session' | 'sample' | 'feature';

export interface SessionHeaderEvent {
	type: 'session';
	session_id: string;
	label: SessionLabel;
	started_at_ns: number;
	actuator_hz: number;
	vision_fps: number;
	vision_frames_total: number;
	croissant_spec_version: '1.0';
	academic_citations_attestation: boolean;
	profile_id: string;
}

export interface SampleEvent {
	type: 'sample';
	ts_lo: number;
	ts_hi: number;
	frame_id: number;
	raw_u: number;
	raw_v: number;
	clean_u: number;
	clean_v: number;
	applied_alpha: number;
	flags: number;
	vision_frame_index: number;
}

export interface FeatureEvent {
	type: 'feature';
	window_start_ns: number;
	window_end_ns: number;
	vision_frame_indices: [number, number];
	velocity_mag_mean: number;
	subpixel_sampling_ratio: number;
}

type JsonlEvent = SessionHeaderEvent | SampleEvent | FeatureEvent;

export class JsonlWriter {
	#chunks: Uint8Array[] = [];
	#closed = false;
	#visionSeq = 0;
	#visionTimestamps: Uint32Array;
	#visionCap: number;
	#visionHead = 0;
	#visionCount = 0;

	constructor(visionBufferSize = 90_000) {
		this.#visionCap = visionBufferSize;
		this.#visionTimestamps = new Uint32Array(visionBufferSize);
	}

	recordVisionFrame(seq: number, _tsLo: number, _tsHi: number): void {
		const i = this.#visionHead;
		this.#visionTimestamps[i] = seq >>> 0;
		this.#visionHead = (i + 1) % this.#visionCap;
		if (this.#visionCount < this.#visionCap) this.#visionCount++;
		this.#visionSeq = seq;
	}

	nearestVisionFrameIndex(tsLo: number, tsHi: number): number {
		void tsLo;
		void tsHi;
		return this.#visionSeq;
	}

	writeSessionHeader(header: SessionHeaderEvent): void {
		this.#appendLine(header);
	}

	writeSampleEvent(sample: Omit<SampleEvent, 'type' | 'vision_frame_index'>): void {
		const ev: SampleEvent = {
			type: 'sample',
			...sample,
			vision_frame_index: this.nearestVisionFrameIndex(sample.ts_lo, sample.ts_hi)
		};
		this.#appendLine(ev);
	}

	writeFeatureEvent(feature: Omit<FeatureEvent, 'type'>): void {
		this.#appendLine({ type: 'feature', ...feature });
	}

	async finalizeGzipBlob(): Promise<Blob> {
		this.#closed = true;
		const raw = new Blob(this.#chunks as BlobPart[], { type: 'application/x-ndjson' });
		if (typeof CompressionStream === 'undefined') {
			return raw;
		}
		const stream = raw.stream().pipeThrough(new CompressionStream('gzip'));
		return await new Response(stream).blob();
	}

	async finalizeSessionFolderZip(
		sessionId: string,
		label: SessionLabel,
		hardwareInfo: Record<string, unknown>
	): Promise<Blob> {
		const gz = await this.finalizeGzipBlob();
		const { default: JSZip } = await import('jszip');
		const zip = new JSZip();
		zip.file(`${sessionId}/mouse.jsonl.gz`, gz);
		zip.file(
			`${sessionId}/hardware_info.json`,
			JSON.stringify(hardwareInfo, null, 2)
		);
		zip.file(
			`${sessionId}/session_meta.json`,
			JSON.stringify({ session_id: sessionId, label }, null, 2)
		);
		return await zip.generateAsync({ type: 'blob' });
	}

	#appendLine(obj: JsonlEvent): void {
		if (this.#closed) return;
		const line = JSON.stringify(obj) + '\n';
		this.#chunks.push(new TextEncoder().encode(line));
	}
}
