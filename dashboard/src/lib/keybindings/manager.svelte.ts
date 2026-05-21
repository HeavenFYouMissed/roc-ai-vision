import { DEFAULT_BINDINGS, comboMatchesEvent, type KeyBinding } from './defaults';

type ActionHandler = (e: KeyboardEvent) => void;

const INPUT_TAGS = new Set(['INPUT', 'TEXTAREA', 'SELECT']);

class KeybindingManager {
	isModalOpen = $state(false);
	showCheatsheet = $state(false);

	private handlers = new Map<string, ActionHandler>();
	private bindings: KeyBinding[] = DEFAULT_BINDINGS;

	register(action: string, handler: ActionHandler): void {
		this.handlers.set(action, handler);
	}

	unregister(action: string): void {
		this.handlers.delete(action);
	}

	dispatch(e: KeyboardEvent): boolean {
		const target = e.target as HTMLElement | null;
		const inInput = target && INPUT_TAGS.has(target.tagName);

		for (const binding of this.bindings) {
			if (!comboMatchesEvent(binding.combo, e)) continue;

			if (this.isModalOpen && binding.action !== 'global:dismiss') continue;
			if (inInput && !binding.requiresModifier) continue;

			const handler = this.handlers.get(binding.action);
			if (handler) {
				e.preventDefault();
				handler(e);
				return true;
			}
		}
		return false;
	}

	getBindings(): KeyBinding[] {
		return this.bindings;
	}
}

export const keybindingManager = new KeybindingManager();
