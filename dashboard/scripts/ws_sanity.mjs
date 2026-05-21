// One-shot CLI sanity check for the live bridge: connect to ws://127.0.0.1:8765
// for 2 seconds, count BINARY frames, verify the 12-byte header signature
// (version byte == 1, num_tracks <= 64), report bytes/sec + frames/sec.
//
// Run:  node scripts/ws_sanity.mjs
import WebSocket from 'ws';

const URL = 'ws://127.0.0.1:8765';
const DURATION_MS = 2000;

let frames = 0;
let bytes = 0;
let badHeaders = 0;
let trackCountHistogram = new Map();

const t0 = Date.now();
const ws = new WebSocket(URL, { origin: 'http://api.roc.internal' });
ws.binaryType = 'arraybuffer';

ws.on('open', () => {
  console.log(`[ws-sanity] connected to ${URL} at +${Date.now() - t0}ms`);
});

ws.on('message', (data, isBinary) => {
  if (!isBinary) {
    console.log('[ws-sanity] WARN: non-binary frame received');
    return;
  }
  // ws module gives Buffer for binary frames; convert to ArrayBuffer view safely.
  const buf = Buffer.isBuffer(data) ? data : Buffer.from(data);
  if (buf.length < 12) {
    badHeaders++;
    return;
  }
  const version = buf.readUInt8(0);
  const numTracks = buf.readUInt16LE(6);
  if (version !== 1 || numTracks > 64) {
    badHeaders++;
    return;
  }
  const expectedSize = 12 + numTracks * 86;
  if (buf.length !== expectedSize) {
    badHeaders++;
    return;
  }
  frames++;
  bytes += buf.length;
  trackCountHistogram.set(numTracks, (trackCountHistogram.get(numTracks) || 0) + 1);
});

ws.on('error', (err) => {
  console.error(`[ws-sanity] ERROR: ${err.message}`);
});

ws.on('close', (code, reason) => {
  console.log(`[ws-sanity] socket closed (code=${code}, reason="${reason}")`);
});

setTimeout(() => {
  ws.close();
  const elapsed = Date.now() - t0;
  console.log('==== ws sanity report ====');
  console.log(`elapsed:           ${elapsed} ms`);
  console.log(`frames received:   ${frames}`);
  console.log(`bytes received:    ${bytes}`);
  console.log(`bad headers:       ${badHeaders}`);
  console.log(`frames/sec:        ${(frames / (elapsed / 1000)).toFixed(2)}`);
  console.log(`bytes/sec:         ${(bytes / (elapsed / 1000)).toFixed(0)}`);
  console.log(`track-count histogram (num_tracks → frames):`);
  for (const [k, v] of [...trackCountHistogram.entries()].sort((a, b) => a[0] - b[0])) {
    console.log(`  ${k.toString().padStart(3)} tracks: ${v} frames`);
  }
  const verdict =
    badHeaders === 0 && frames >= 30 && trackCountHistogram.size > 0
      ? 'PASS'
      : 'FAIL';
  console.log(`verdict:           ${verdict}`);
  process.exit(verdict === 'PASS' ? 0 : 1);
}, DURATION_MS);
