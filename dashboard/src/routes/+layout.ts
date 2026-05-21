// =============================================================================
// roc-ai-vision · dashboard · routes · +layout.ts
//
// SPA-mode configuration.
//
// The ROC dashboard runs exclusively against a local C++ orchestrator over
// a loopback WebSocket (ws://127.0.0.1:8765 / ws://api.roc.internal:8765).
// SSR has no value for this deployment — it would only run the page lifecycle
// in Node, where browser-only APIs (`requestAnimationFrame`, `WebSocket`,
// `SVGElement`, etc.) don't exist. Disabling SSR here makes the entire app
// a client-rendered single-page app, which matches the operator-console
// deployment topology and eliminates a whole class of "X is not defined" SSR
// errors when adding new browser-only code paths.
//
// `prerender = false` is the default for non-static routes; we leave it
// implicit. CSR remains on (the default) so the page actually hydrates.
// =============================================================================

export const ssr = false;
