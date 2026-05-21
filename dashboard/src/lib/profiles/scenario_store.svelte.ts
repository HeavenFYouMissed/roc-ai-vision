import { browser } from '$app/environment';
import { SCENARIOS, type ScenarioDef } from './scenarios';
import { profileStore } from '$lib/stores/profile_store.svelte';

const STORAGE_KEY = 'roc.scenario.activeId';

class ScenarioStore {
	activeScenarioId = $state<string>(SCENARIOS[0].id);
	private hydrated = false;

	hydrate(): void {
		if (this.hydrated || !browser) return;
		try {
			const saved = localStorage.getItem(STORAGE_KEY);
			if (saved && SCENARIOS.some((s) => s.id === saved)) {
				this.activeScenarioId = saved;
			}
		} catch {
			// ignore
		}
		this.hydrated = true;
	}

	get active(): ScenarioDef {
		return SCENARIOS.find((s) => s.id === this.activeScenarioId) ?? SCENARIOS[0];
	}

	setScenario(id: string): void {
		if (!SCENARIOS.some((s) => s.id === id)) return;
		this.activeScenarioId = id;
		if (browser) {
			try {
				localStorage.setItem(STORAGE_KEY, id);
			} catch {
				// ignore
			}
		}
		// Per ss5.5 Scenarios bind a default Profile — auto-switch on Scenario
		// change so the operator never accidentally runs an AC Research session
		// with the wrong EP order, or Forensic Review in LIVE mode.
		profileStore.hydrate();
		const target = this.active.defaultProfileId;
		if (target && profileStore.activeProfileId !== target) {
			profileStore.setProfile(target);
		}
	}
}

export const scenarioStore = new ScenarioStore();
