import { DEFAULT_PROFILE, type RocProfile } from './schema';

export type SessionLabel = 'clean' | 'dirty' | 'ambiguous' | 'unsupervised';
export type ScenarioSinkKind = 'noop' | 'hid' | 'dynamixel' | 'pwm_pantilt' | 'modbus' | 'logfile_csv';
export type AppMode = 'live' | 'replay';

export interface ScenarioDef {
	id: string;
	label: string;
	description: string;
	defaultProfileId: string;
	profile: RocProfile;
	inspectPanelIds: string[];
	sinkKind: ScenarioSinkKind;
	defaultModel: string | null;
	defaultMode: AppMode;
	recordingDefaults: { enabled: boolean; defaultLabel: SessionLabel };
}

export const SCENARIOS: ScenarioDef[] = [
	// 1. Default first-run scenario
	{
		id: 'surveillance-shift',
		label: 'Surveillance Shift',
		description: 'Active humanoid tracking · default first-run scenario',
		defaultProfileId: 'default',
		profile: {
			...DEFAULT_PROFILE,
			id: 'surveillance',
			label: 'Surveillance Shift Profile'
		},
		inspectPanelIds: [
			'spectator-telemetry-matrix',
			'velocity-profile',
			'subpixel'
		],
		sinkKind: 'noop',
		defaultModel: 'yolo26m-roc-humanoid',
		defaultMode: 'live',
		recordingDefaults: { enabled: false, defaultLabel: 'unsupervised' }
	},

	// 2. Calibration session - wizard active
	{
		id: 'calibration-session',
		label: 'Calibration Session',
		description: 'Setting up new rig · Calibrate destination wizard active',
		defaultProfileId: 'default',
		profile: {
			...DEFAULT_PROFILE,
			id: 'calibration',
			label: 'Calibration Profile'
		},
		inspectPanelIds: ['subpixel'],
		sinkKind: 'noop',
		defaultModel: null,
		defaultMode: 'live',
		recordingDefaults: { enabled: false, defaultLabel: 'unsupervised' }
	},

	// 3. Forensic Review - REPLAY mode
	{
		id: 'forensic-review',
		label: 'Forensic Review',
		description: 'Post-incident scrub · REPLAY mode default',
		defaultProfileId: 'replay',
		profile: {
			...DEFAULT_PROFILE,
			id: 'forensic',
			label: 'Forensic Replay Profile'
		},
		inspectPanelIds: [
			'mouse-dynamics-inspector',
			'spectator-telemetry-matrix',
			'fitts-residual',
			'subpixel'
		],
		sinkKind: 'noop',
		defaultModel: null,
		defaultMode: 'replay',
		recordingDefaults: { enabled: false, defaultLabel: 'unsupervised' }
	},

	// 4. Defensive Engagement
	{
		id: 'defensive-engagement',
		label: 'Defensive Engagement',
		description: 'Trained-operator actuator session · Engage armed-but-disarmed',
		defaultProfileId: 'prod-nvidia',
		profile: {
			...DEFAULT_PROFILE,
			id: 'defensive',
			label: 'Defensive Engagement Profile'
		},
		inspectPanelIds: ['spectator-telemetry-matrix', 'velocity-profile'],
		sinkKind: 'hid',
		defaultModel: 'yolo26m-roc-humanoid',
		defaultMode: 'live',
		recordingDefaults: { enabled: true, defaultLabel: 'unsupervised' }
	},

	// 5. AC Research Clean
	{
		id: 'ac-research-clean',
		label: 'AC Research — Behavioural Profiling (Clean)',
		description: 'Capture humanised cursor traces with EKF+Ruckig smoothing',
		defaultProfileId: 'prod-amd-intel',
		profile: {
			...DEFAULT_PROFILE,
			id: 'ac-clean',
			label: 'AC Clean Trace Profile',
			actuation: { ...DEFAULT_PROFILE.actuation, linear_smoothing_alpha: 0.35 }
		},
		inspectPanelIds: [
			'mouse-dynamics-inspector',
			'spectator-telemetry-matrix',
			'velocity-profile',
			'fitts-residual',
			'subpixel'
		],
		sinkKind: 'noop',
		defaultModel: 'yolo26m-roc-humanoid',
		defaultMode: 'live',
		recordingDefaults: { enabled: true, defaultLabel: 'clean' }
	},

	// 6. AC Research Dirty
	{
		id: 'ac-research-dirty',
		label: 'AC Research — Behavioural Profiling (Dirty)',
		description: 'Capture raw step-delta cursor traces · EKF+Ruckig OFF',
		defaultProfileId: 'prod-amd-intel',
		profile: {
			...DEFAULT_PROFILE,
			id: 'ac-dirty',
			label: 'AC Dirty Trace Profile',
			actuation: { ...DEFAULT_PROFILE.actuation, linear_smoothing_alpha: 0.0 }
		},
		inspectPanelIds: [
			'mouse-dynamics-inspector',
			'spectator-telemetry-matrix',
			'velocity-profile',
			'fitts-residual',
			'subpixel'
		],
		sinkKind: 'noop',
		defaultModel: 'yolo26m-roc-humanoid',
		defaultMode: 'live',
		recordingDefaults: { enabled: true, defaultLabel: 'dirty' }
	}
];

export function getScenarioById(id: string): ScenarioDef | undefined {
	return SCENARIOS.find((s) => s.id === id);
}
