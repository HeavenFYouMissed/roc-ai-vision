class AimingConfigStore {
	smoothingCoefficient = $state(0.35);
	deadbandScaling = $state(1.2);
	hidConnected = $state(false);
	activeProfile = $state('default');

	resetDefaults() {
		this.smoothingCoefficient = 0.35;
		this.deadbandScaling = 1.2;
		this.hidConnected = false;
		this.activeProfile = 'default';
	}
}

export const aimingConfig = new AimingConfigStore();
