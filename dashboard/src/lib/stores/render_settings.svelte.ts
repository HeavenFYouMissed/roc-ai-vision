// =============================================================================
// roc-ai-vision · dashboard · render_settings.svelte.ts
//
// Reactive store for ALL user-controllable rendering settings on the Operate
// canvas. Drives the Threshold Controls panel in the right rail, and
// every layer/toggle on the canvas reads from here.
//
// All settings persisted to localStorage under key `roc.render_settings.v1`
// so the operator's preferences survive page reloads.
// =============================================================================

import { browser } from '$app/environment';

export type LabelDisplayMode = 'confidence' | 'labels' | 'both' | 'none';
export type BBoxStyle = 'solid' | 'dashed' | 'corners' | 'outline';

interface PersistShape {
	confidenceThreshold: number;
	iouThreshold: number;
	labelMode: LabelDisplayMode;
	bboxStyle: BBoxStyle;
	classVisibility: boolean[];
	showVideo: boolean;
	showTrajectories: boolean;
	showActuatorTrace: boolean;
	showDetections: boolean;
	showHierarchy: boolean;
	showCrosshair: boolean;
	showTrackId: boolean;
	showAnchor: boolean;
	showVelocityArrow: boolean;
	showUncertaintyEllipse: boolean;
	showFovRing: boolean;
}

const STORAGE_KEY = 'roc.render_settings.v1';

const DEFAULTS: PersistShape = {
	confidenceThreshold: 0.5,
	iouThreshold: 0.45,
	labelMode: 'both',
	bboxStyle: 'solid',
	classVisibility: [true, true, true, true, true, true],
	showVideo: true,
	showTrajectories: true,
	showActuatorTrace: true,
	showDetections: true,
	showHierarchy: true,
	showCrosshair: true,
	showTrackId: true,
	showAnchor: true,
	showVelocityArrow: true,
	showUncertaintyEllipse: false,
	showFovRing: true
};

function loadFromStorage(): PersistShape {
	if (!browser) return { ...DEFAULTS };
	try {
		const raw = localStorage.getItem(STORAGE_KEY);
		if (!raw) return { ...DEFAULTS };
		const parsed = JSON.parse(raw) as Partial<PersistShape>;
		return { ...DEFAULTS, ...parsed };
	} catch {
		return { ...DEFAULTS };
	}
}

class RenderSettingsStore {
	confidenceThreshold = $state(DEFAULTS.confidenceThreshold);
	iouThreshold = $state(DEFAULTS.iouThreshold);
	labelMode = $state<LabelDisplayMode>(DEFAULTS.labelMode);
	bboxStyle = $state<BBoxStyle>(DEFAULTS.bboxStyle);

	// 6-element array indexed by Label enum value (0..5)
	classVisibility = $state<boolean[]>([...DEFAULTS.classVisibility]);

	// Layer toggles (mapped to v / o hotkeys + Layer Visibility Toolbar)
	showVideo = $state(DEFAULTS.showVideo);
	showTrajectories = $state(DEFAULTS.showTrajectories);
	showActuatorTrace = $state(DEFAULTS.showActuatorTrace);
	showDetections = $state(DEFAULTS.showDetections);
	showHierarchy = $state(DEFAULTS.showHierarchy);
	showCrosshair = $state(DEFAULTS.showCrosshair);

	// Per-element display toggles (apply inside Layer 2/3/4)
	showTrackId = $state(DEFAULTS.showTrackId);
	showAnchor = $state(DEFAULTS.showAnchor);
	showVelocityArrow = $state(DEFAULTS.showVelocityArrow);
	showUncertaintyEllipse = $state(DEFAULTS.showUncertaintyEllipse);
	showFovRing = $state(DEFAULTS.showFovRing);

	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		const loaded = loadFromStorage();
		this.confidenceThreshold = loaded.confidenceThreshold;
		this.iouThreshold = loaded.iouThreshold;
		this.labelMode = loaded.labelMode;
		this.bboxStyle = loaded.bboxStyle;
		this.classVisibility = [...loaded.classVisibility];
		this.showVideo = loaded.showVideo;
		this.showTrajectories = loaded.showTrajectories;
		this.showActuatorTrace = loaded.showActuatorTrace;
		this.showDetections = loaded.showDetections;
		this.showHierarchy = loaded.showHierarchy;
		this.showCrosshair = loaded.showCrosshair;
		this.showTrackId = loaded.showTrackId;
		this.showAnchor = loaded.showAnchor;
		this.showVelocityArrow = loaded.showVelocityArrow;
		this.showUncertaintyEllipse = loaded.showUncertaintyEllipse;
		this.showFovRing = loaded.showFovRing;
		this.hydrated = true;
	}

	persist(): void {
		if (!browser) return;
		const snap: PersistShape = {
			confidenceThreshold: this.confidenceThreshold,
			iouThreshold: this.iouThreshold,
			labelMode: this.labelMode,
			bboxStyle: this.bboxStyle,
			classVisibility: [...this.classVisibility],
			showVideo: this.showVideo,
			showTrajectories: this.showTrajectories,
			showActuatorTrace: this.showActuatorTrace,
			showDetections: this.showDetections,
			showHierarchy: this.showHierarchy,
			showCrosshair: this.showCrosshair,
			showTrackId: this.showTrackId,
			showAnchor: this.showAnchor,
			showVelocityArrow: this.showVelocityArrow,
			showUncertaintyEllipse: this.showUncertaintyEllipse,
			showFovRing: this.showFovRing
		};
		try {
			localStorage.setItem(STORAGE_KEY, JSON.stringify(snap));
		} catch {
			// quota exceeded or storage unavailable - silently fail; runtime state still works
		}
	}

	resetDefaults(): void {
		this.confidenceThreshold = DEFAULTS.confidenceThreshold;
		this.iouThreshold = DEFAULTS.iouThreshold;
		this.labelMode = DEFAULTS.labelMode;
		this.bboxStyle = DEFAULTS.bboxStyle;
		this.classVisibility = [...DEFAULTS.classVisibility];
		this.showVideo = DEFAULTS.showVideo;
		this.showTrajectories = DEFAULTS.showTrajectories;
		this.showActuatorTrace = DEFAULTS.showActuatorTrace;
		this.showDetections = DEFAULTS.showDetections;
		this.showHierarchy = DEFAULTS.showHierarchy;
		this.showCrosshair = DEFAULTS.showCrosshair;
		this.showTrackId = DEFAULTS.showTrackId;
		this.showAnchor = DEFAULTS.showAnchor;
		this.showVelocityArrow = DEFAULTS.showVelocityArrow;
		this.showUncertaintyEllipse = DEFAULTS.showUncertaintyEllipse;
		this.showFovRing = DEFAULTS.showFovRing;
		this.persist();
	}

	// Whether a given track (by class label + confidence) should render
	shouldRender(label: number, confidence: number): boolean {
		if (confidence < this.confidenceThreshold) return false;
		if (!this.classVisibility[label]) return false;
		return true;
	}
}

export const renderSettings = new RenderSettingsStore();
