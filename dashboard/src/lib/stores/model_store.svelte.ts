// =============================================================================
// roc-ai-vision · dashboard · model_store.svelte.ts
//
// Reactive client for the orchestrator's Phase 4 REST API:
//   GET  /api/models        -> { models: [ModelInfo, ...] }
//   GET  /api/model/active  -> ActiveModelSummary
//   POST /api/model/active  -> SwitchResult
//
// Population pattern mirrors `source_store.svelte.ts`:
//   - In-memory built-in defaults so the dashboard renders something even
//     when the orchestrator is offline.
//   - `refresh()` calls the REST endpoints, merges in real data, and
//     records the last error for the drawer's "offline" banner.
//   - `switchTo(name)` issues a POST and on success calls refresh() to
//     re-sync the "active" state from the server.
//
// The C++ side enforces a single Ort::Session at a time — this store does
// not maintain client-side multi-selection. There is exactly one active
// model, and the UI's job is to show which one it is and let the operator
// switch.
// =============================================================================

import { browser } from '$app/environment';

export interface ModelInfo {
	name: string;
	manifestPath: string;
	onnxPath: string;
	purpose: 'detection' | 'reid' | '';
	head: string;                  // e.g. "yolo26_detect" / "osnet_reid"
	inputW: number;
	inputH: number;
	numClasses: number;
	active: boolean;
	loadable: boolean;
	statusNote: string;            // "ready" / "missing .onnx alongside .yaml" / ...
}

export interface ActiveModelSummary {
	name: string;
	purpose: string;
	head: string;
	inputW: number;
	inputH: number;
	numClasses: number;
	ep: string;                    // "DirectML" | "OpenVino" | "Cpu"
	epDescription: string;
}

export interface SwitchResult {
	ok: boolean;
	active?: string;
	ep?: string;
	error?: string;
}

const MODELS_ENDPOINT  = 'http://127.0.0.1:8766/api/models';
const ACTIVE_ENDPOINT  = 'http://127.0.0.1:8766/api/model/active';
const FETCH_TIMEOUT_MS = 2500;
const SWAP_TIMEOUT_MS  = 30_000;   // ORT JIT can take ~1 s on first DML compile

async function fetchJson<T>(url: string, init: RequestInit = {}, timeout = FETCH_TIMEOUT_MS): Promise<T> {
	const ctrl = new AbortController();
	const tid  = setTimeout(() => ctrl.abort(), timeout);
	try {
		const res = await fetch(url, { ...init, signal: ctrl.signal });
		if (!res.ok) throw new Error(`HTTP ${res.status} ${res.statusText}`);
		return (await res.json()) as T;
	} finally {
		clearTimeout(tid);
	}
}

class ModelStore {
	models = $state<ModelInfo[]>([]);
	active = $state<ActiveModelSummary | null>(null);

	loading      = $state(false);
	switching    = $state<string | null>(null);   // name being switched to, or null
	lastError    = $state<string | null>(null);
	lastSwitch   = $state<SwitchResult | null>(null);

	get hasModels(): boolean {
		return this.models.length > 0;
	}

	get activeName(): string {
		return this.active?.name ?? this.models.find((m) => m.active)?.name ?? '';
	}

	get readyCount(): number {
		return this.models.filter((m) => m.loadable).length;
	}

	get brokenCount(): number {
		return this.models.filter((m) => !m.loadable).length;
	}

	async refresh(): Promise<void> {
		if (!browser) return;
		this.loading = true;
		try {
			const [listResp, activeResp] = await Promise.all([
				fetchJson<{ models: ModelInfo[] }>(MODELS_ENDPOINT),
				fetchJson<ActiveModelSummary>(ACTIVE_ENDPOINT).catch(() => null)
			]);
			this.models = listResp.models ?? [];
			this.active = activeResp;
			// Server truth always wins for the `active` flag; reconcile.
			if (this.active) {
				const wanted = this.active.name;
				for (const m of this.models) m.active = m.name === wanted;
			}
			this.lastError = null;
		} catch (e) {
			this.lastError = e instanceof Error ? e.message : String(e);
		} finally {
			this.loading = false;
		}
	}

	async switchTo(name: string): Promise<SwitchResult> {
		if (!browser) return { ok: false, error: 'not in browser' };
		if (this.switching) return { ok: false, error: 'another swap is in progress' };

		this.switching = name;
		try {
			const body = JSON.stringify({ name });
			const result = await fetchJson<SwitchResult>(
				ACTIVE_ENDPOINT,
				{
					method: 'POST',
					headers: { 'Content-Type': 'application/json' },
					body
				},
				SWAP_TIMEOUT_MS
			);
			this.lastSwitch = result;
			if (result.ok) {
				// Re-pull authoritative state from the server.
				await this.refresh();
			} else {
				this.lastError = result.error ?? 'swap failed';
			}
			return result;
		} catch (e) {
			const result: SwitchResult = {
				ok: false,
				error: e instanceof Error ? e.message : String(e)
			};
			this.lastSwitch = result;
			this.lastError = result.error ?? null;
			return result;
		} finally {
			this.switching = null;
		}
	}
}

export const modelStore = new ModelStore();
