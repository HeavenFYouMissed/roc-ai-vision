// =============================================================================
// roc-ai-vision · dashboard · event_log.svelte.ts
//
// Append-only ring of timestamped events emitted by the dashboard runtime
// and (eventually) by the C++ orchestrator over a metadata WS channel.
// Used by the EventLogDock at the bottom of Inspect.
// =============================================================================

import { browser } from '$app/environment';

export type EventLevel = 'debug' | 'info' | 'warn' | 'err' | 'critical';
export type EventSource =
	| 'orchestrator'
	| 'ws'
	| 'decoder'
	| 'tracker'
	| 'user'
	| 'shell'
	| 'engage'
	| 'inspect';

export interface LogEvent {
	id: number;
	tsMs: number;
	level: EventLevel;
	source: EventSource;
	message: string;
	pinned?: boolean;
}

const MAX_EVENTS = 512;

class EventLogStore {
	events = $state<LogEvent[]>([]);
	autoScroll = $state(true);
	levelFilter = $state<EventLevel | 'all'>('all');
	searchFilter = $state('');
	activeTab = $state<'console' | 'network' | 'performance'>('console');
	collapsed = $state(true);

	private nextId = 1;

	emit(level: EventLevel, source: EventSource, message: string): void {
		const ev: LogEvent = {
			id: this.nextId++,
			tsMs: Date.now(),
			level,
			source,
			message
		};
		const next = [...this.events, ev];
		if (next.length > MAX_EVENTS) {
			// Preserve pinned events
			const pinned = next.filter((e) => e.pinned);
			const trimmed = next.slice(-MAX_EVENTS + pinned.length);
			this.events = [...pinned.filter((p) => !trimmed.includes(p)), ...trimmed];
		} else {
			this.events = next;
		}
	}

	info(source: EventSource, msg: string) { this.emit('info', source, msg); }
	warn(source: EventSource, msg: string) { this.emit('warn', source, msg); }
	err(source: EventSource, msg: string) { this.emit('err', source, msg); }
	critical(source: EventSource, msg: string) { this.emit('critical', source, msg); }
	debug(source: EventSource, msg: string) { this.emit('debug', source, msg); }

	get filtered(): LogEvent[] {
		const lvl = this.levelFilter;
		const search = this.searchFilter.toLowerCase();
		return this.events.filter((e) => {
			if (lvl !== 'all') {
				const order: EventLevel[] = ['debug', 'info', 'warn', 'err', 'critical'];
				if (order.indexOf(e.level) < order.indexOf(lvl)) return false;
			}
			if (search && !e.message.toLowerCase().includes(search) && !e.source.includes(search)) {
				return false;
			}
			return true;
		});
	}

	get lastEvent(): LogEvent | null {
		return this.events.length > 0 ? this.events[this.events.length - 1] : null;
	}

	pin(id: number): void {
		this.events = this.events.map((e) => (e.id === id ? { ...e, pinned: !e.pinned } : e));
	}

	clear(): void {
		this.events = this.events.filter((e) => e.pinned);
	}

	expand() { this.collapsed = false; }
	collapse() { this.collapsed = true; }
	toggleCollapse() { this.collapsed = !this.collapsed; }
}

export const eventLog = new EventLogStore();

// Seed a few synthetic events so the panel renders something on first paint
if (browser) {
	setTimeout(() => {
		eventLog.info('shell', 'Dashboard mounted');
		eventLog.info('decoder', 'Wire decoder ready (v1, v2, v3)');
		eventLog.info('ws', 'WebSocket attempting ws://127.0.0.1:8765');
		eventLog.warn('ws', 'WebSocket disconnected; falling back to mock source');
		eventLog.info('shell', 'Scenario loaded: AC Research — Behavioural Profiling (Clean)');
		eventLog.info('shell', 'Profile loaded: Default (dev)');
	}, 0);
}
