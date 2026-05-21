/**
 * roc-ai-vision · dashboard · biometrics/export.ts
 *
 * Export utilities for the Biometrics destination (UI_SPEC_SHEET §16.5):
 *  - exportSlotJSONL(slot, label, slotKind) : writes one JSONL header line
 *    + one sample line per ring sample. Loadable directly in Python via
 *    `pandas.read_json(path, lines=True)`.
 *  - exportRowAsPNG(rowEl, label, slotKind) : rasterizes a DOM row (the
 *    5-panel CURRENT or PREVIOUS row of the destination) into a single PNG.
 *
 * Filename convention (both): `mdc_{YYYYMMDD-HHMMSS}_{LABEL}_{slot}.{ext}`
 *
 * Notes on the PNG composite: SVG and Canvas elements are sampled separately
 * because Canvas can be drawImage'd directly, but SVG must be serialized via
 * XMLSerializer + loaded as an Image (the only browser-native way to
 * rasterize SVG to a canvas). Text panels are read via the DOM and re-rendered
 * with ctx.fillText so monospace alignment is preserved.
 */

import type { CaptureSlot } from './mouse_capture.svelte';
import { RING_CAPACITY } from './mouse_capture.svelte';

export type SlotKind = 'current' | 'previous';

// =============================================================================
// Filename
// =============================================================================

function timestampStr(d: Date = new Date()): string {
	const pad = (n: number, w = 2) => String(n).padStart(w, '0');
	return (
		d.getFullYear() +
		pad(d.getMonth() + 1) +
		pad(d.getDate()) +
		'-' +
		pad(d.getHours()) +
		pad(d.getMinutes()) +
		pad(d.getSeconds())
	);
}

function makeFilename(label: string, slotKind: SlotKind, ext: string): string {
	return `mdc_${timestampStr()}_${label}_${slotKind}.${ext}`;
}

// =============================================================================
// JSONL export
// =============================================================================

export function exportSlotJSONL(slot: CaptureSlot, label: string, slotKind: SlotKind): void {
	const cap = RING_CAPACITY;
	const N = slot.fill;
	const start = (slot.head - N + cap) % cap;

	const lines: string[] = [];
	const startedIso = slot.startedMs > 0 ? new Date(slot.startedMs).toISOString() : '';
	lines.push(
		JSON.stringify({
			_: 'mdc-v1',
			label,
			slot: slotKind,
			started: startedIso,
			duration_ms: slot.durationMs,
			report_count: N
		})
	);

	for (let k = 0; k < N; k++) {
		const idx = (start + k) % cap;
		// Mouse movementX/Y are integers in the spec; trunc just in case fp
		// round-trip introduces noise.
		lines.push(
			JSON.stringify({
				t: +slot.t[idx].toFixed(3),
				dx: slot.dx[idx] | 0,
				dy: slot.dy[idx] | 0
			})
		);
	}

	const blob = new Blob([lines.join('\n') + '\n'], { type: 'application/x-jsonlines' });
	downloadBlob(blob, makeFilename(label, slotKind, 'jsonl'));
}

// =============================================================================
// PNG export (DOM row composite)
// =============================================================================

/**
 * Rasterize a DOM row (the .biometrics-row element wrapping all 5 panels)
 * into a PNG. Walks child panels: <canvas> -> drawImage; <svg> -> serialize
 * + load as Image; everything else -> rendered as a snapshot of the DOM
 * background + foreground text.
 *
 * Returns a Promise that resolves once the download is triggered.
 */
export async function exportRowAsPNG(
	rowEl: HTMLElement,
	label: string,
	slotKind: SlotKind
): Promise<void> {
	const rect = rowEl.getBoundingClientRect();
	const dpr = window.devicePixelRatio || 1;
	const W = Math.max(1, Math.floor(rect.width));
	const H = Math.max(1, Math.floor(rect.height));

	const off = document.createElement('canvas');
	off.width = Math.floor(W * dpr);
	off.height = Math.floor(H * dpr);
	const ctx = off.getContext('2d');
	if (!ctx) throw new Error('exportRowAsPNG: 2D context unavailable');

	// Page-background fill so transparent regions match the dashboard chrome.
	ctx.fillStyle = '#0e0e10';
	ctx.fillRect(0, 0, off.width, off.height);

	// Walk DOM in document order, paint each panel into its rect.
	const panels = rowEl.querySelectorAll<HTMLElement>('[data-export-panel]');
	const tasks: Promise<void>[] = [];
	for (const panel of panels) {
		tasks.push(paintPanelToCtx(ctx, panel, rect, dpr));
	}
	await Promise.all(tasks);

	const blob = await canvasToBlob(off);
	downloadBlob(blob, makeFilename(label, slotKind, 'png'));
}

async function paintPanelToCtx(
	ctx: CanvasRenderingContext2D,
	panel: HTMLElement,
	rowRect: DOMRect,
	dpr: number
): Promise<void> {
	const r = panel.getBoundingClientRect();
	const x = (r.left - rowRect.left) * dpr;
	const y = (r.top - rowRect.top) * dpr;
	const w = r.width * dpr;
	const h = r.height * dpr;

	const style = getComputedStyle(panel);

	// Background + border (lightweight reconstruction).
	ctx.fillStyle = style.backgroundColor || '#161618';
	ctx.fillRect(x, y, w, h);
	const bColor = style.borderTopColor || '#27272a';
	ctx.strokeStyle = bColor;
	ctx.lineWidth = 1 * dpr;
	ctx.strokeRect(x + 0.5, y + 0.5, w - 1, h - 1);

	// Children: canvases drawn direct, SVGs serialized, anything else text-only
	// title overlay via fillText.
	const canvases = panel.querySelectorAll('canvas');
	for (const c of canvases) {
		const cr = c.getBoundingClientRect();
		const cx = (cr.left - rowRect.left) * dpr;
		const cy = (cr.top - rowRect.top) * dpr;
		const cw = cr.width * dpr;
		const ch = cr.height * dpr;
		try {
			ctx.drawImage(c, cx, cy, cw, ch);
		} catch {
			/* canvas may be tainted from a cross-origin asset — best effort */
		}
	}

	const svgs = panel.querySelectorAll('svg');
	for (const svg of svgs) {
		const sr = svg.getBoundingClientRect();
		const sx = (sr.left - rowRect.left) * dpr;
		const sy = (sr.top - rowRect.top) * dpr;
		const sw = sr.width * dpr;
		const sh = sr.height * dpr;
		try {
			const img = await svgToImage(svg);
			ctx.drawImage(img, sx, sy, sw, sh);
		} catch {
			/* unsupported svg feature — skip */
		}
	}

	// Text overlay pass: walk descendant text nodes and render with fillText.
	// We render at each text node's bounding rect so monospaced alignment
	// stays visually correct. Skip text inside SVG/Canvas (handled above).
	const walker = document.createTreeWalker(panel, NodeFilter.SHOW_TEXT, {
		acceptNode(node) {
			const parent = node.parentElement;
			if (!parent) return NodeFilter.FILTER_REJECT;
			if (parent.closest('svg, canvas')) return NodeFilter.FILTER_REJECT;
			if (!node.textContent || !node.textContent.trim()) return NodeFilter.FILTER_REJECT;
			return NodeFilter.FILTER_ACCEPT;
		}
	});
	let node = walker.nextNode();
	while (node) {
		const parent = node.parentElement;
		if (parent) {
			const s = getComputedStyle(parent);
			const pr = parent.getBoundingClientRect();
			ctx.fillStyle = s.color || '#d4d0c8';
			ctx.font = `${s.fontWeight} ${parseFloat(s.fontSize) * dpr}px ${s.fontFamily}`;
			ctx.textBaseline = 'top';
			const tx = (pr.left - rowRect.left + parseFloat(s.paddingLeft || '0')) * dpr;
			const ty = (pr.top - rowRect.top + parseFloat(s.paddingTop || '0')) * dpr;
			ctx.fillText(node.textContent!.trim(), tx, ty, pr.width * dpr);
		}
		node = walker.nextNode();
	}
}

function svgToImage(svg: SVGElement): Promise<HTMLImageElement> {
	return new Promise((resolve, reject) => {
		const xml = new XMLSerializer().serializeToString(svg);
		const blob = new Blob([xml], { type: 'image/svg+xml;charset=utf-8' });
		const url = URL.createObjectURL(blob);
		const img = new Image();
		img.onload = () => { URL.revokeObjectURL(url); resolve(img); };
		img.onerror = (e) => { URL.revokeObjectURL(url); reject(e); };
		img.src = url;
	});
}

function canvasToBlob(c: HTMLCanvasElement): Promise<Blob> {
	return new Promise((resolve, reject) => {
		c.toBlob((b) => {
			if (b) resolve(b);
			else reject(new Error('canvas.toBlob returned null'));
		}, 'image/png');
	});
}

function downloadBlob(blob: Blob, filename: string): void {
	const url = URL.createObjectURL(blob);
	const a = document.createElement('a');
	a.href = url;
	a.download = filename;
	a.style.display = 'none';
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);
	// Defer revoke so the browser actually starts the download.
	setTimeout(() => URL.revokeObjectURL(url), 1000);
}
