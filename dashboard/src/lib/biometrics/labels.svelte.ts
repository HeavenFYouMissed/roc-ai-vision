/**
 * roc-ai-vision · dashboard · biometrics/labels.ts
 *
 * Dataset label vocabulary for Biometrics destination JSONL export
 * (UI_SPEC_SHEET §16.5). The selected label is persisted to localStorage
 * so it survives page reloads — important during long data-collection
 * sessions where the operator labels many takes in a row.
 *
 * Adding new labels: append to LABEL_VOCABULARY. Filename safety is enforced
 * by the assertion in `setLabel()` — labels must be `[A-Z0-9_-]+`.
 */

const STORAGE_KEY = 'mdc.label';

export const LABEL_VOCABULARY = [
	'UNTAGGED',
	'CLEAN',
	'CHEAT',
	'TUNE-RUN'
] as const;

export type RecordingLabel = (typeof LABEL_VOCABULARY)[number];

const LABEL_RE = /^[A-Z0-9_-]+$/;

function loadInitial(): RecordingLabel {
	if (typeof localStorage === 'undefined') return 'UNTAGGED';
	const v = localStorage.getItem(STORAGE_KEY);
	if (v && (LABEL_VOCABULARY as readonly string[]).includes(v)) {
		return v as RecordingLabel;
	}
	return 'UNTAGGED';
}

class RecordingLabelStore {
	value = $state<RecordingLabel>('UNTAGGED');
	private hydrated = false;

	hydrate(): void {
		if (this.hydrated) return;
		this.value = loadInitial();
		this.hydrated = true;
	}

	setLabel(next: string): void {
		if (!LABEL_RE.test(next)) {
			console.warn(`[mdc] label '${next}' rejected — must match ${LABEL_RE}`);
			return;
		}
		this.value = next as RecordingLabel;
		if (typeof localStorage !== 'undefined') {
			localStorage.setItem(STORAGE_KEY, next);
		}
	}
}

export const recordingLabel = new RecordingLabelStore();
