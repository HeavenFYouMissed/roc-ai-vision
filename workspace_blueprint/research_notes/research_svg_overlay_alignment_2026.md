# Research: SVG Bounding-Box Overlay Alignment Over Letterboxed Image/Video

## Problem statement

Render an `<img>` (MJPEG stream) or `<video>` (capture card) with `object-fit: contain` inside a flex/grid panel, then draw an SVG overlay (bboxes, labels, FOV ring, crosshair) such that:

1. Source-pixel coordinates (e.g. 640×480 detector output) map **1:1** to the rendered pixels of the visible image content.
2. No overlay primitive (bbox stroke, label tail, crosshair) is allowed to bleed into the dark gutters/letterbox bars.
3. The same DOM works for 4:3, 16:9, and 21:9 source aspect ratios without runtime code paths.

## Canonical answer: **Option A — `aspect-ratio` stage + `viewBox` SVG**

The canonical, JS-free pattern that all serious annotation tools converge on is:

1. Wrap the image/video and the SVG in a **stage div** whose intrinsic dimensions are dictated by the **source aspect ratio**, not the parent panel.
2. Bound the stage with `max-width: 100%; max-height: 100%` and center it with `margin: auto` (or via the parent's flex/grid alignment). The browser's intrinsic sizing algorithm then computes the same letterboxed rect that `object-fit: contain` would.
3. Position `<img>`/`<video>` and `<svg>` as siblings inside the stage with `position: absolute; inset: 0; width: 100%; height: 100%`.
4. Set `viewBox="0 0 W H"` on the SVG where `W×H` is the **source pixel resolution**. Default `preserveAspectRatio="xMidYMid meet"` is fine — because the stage already has the source aspect ratio, `meet` is a no-op (no internal letterboxing inside the SVG).
5. Set `overflow: hidden` on the stage so any SVG element drawn outside `[0,W]×[0,H]` is clipped at the image edge — it cannot enter the gutters.

### Minimal canonical example

```html
<div class="panel">
  <div class="stage" style="aspect-ratio: 640 / 480;">
    <img src="/mjpeg" alt="" />
    <svg viewBox="0 0 640 480" preserveAspectRatio="xMidYMid meet">
      <rect x="120" y="80" width="200" height="160"
            fill="none" stroke="lime" stroke-width="2"
            vector-effect="non-scaling-stroke" />
    </svg>
  </div>
</div>

<style>
.panel {
  display: flex;            /* or grid: place-items: center */
  align-items: center;
  justify-content: center;
  width: 100%;
  height: 100%;
  background: #000;
  overflow: hidden;
}
.stage {
  position: relative;
  max-width: 100%;
  max-height: 100%;
  overflow: hidden;         /* clip overlay to image rect */
  /* aspect-ratio is set inline from the source W/H */
}
.stage > img,
.stage > svg {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  display: block;
}
.stage > img {
  /* object-fit not strictly needed — stage already matches source AR.
     Keep it as a defensive default if the <img>'s natural size disagrees. */
  object-fit: fill;
}
</style>
```

Key detail: because the **stage** carries the aspect ratio (not `object-fit` on the image), the image is no longer letterboxed *within* the stage; instead the **stage itself** is letterboxed within the parent panel by `max-width/max-height + margin:auto`. The gutters live outside the stage, the SVG lives inside the stage, and `overflow: hidden` prevents the SVG from ever crossing into the gutters.

### Why this works (intrinsic sizing rules)

- `aspect-ratio: W/H` on a block with `max-width: 100%` and `max-height: 100%` triggers the same constraint solver the browser uses for `<img>`/`<video>` with `object-fit: contain`: it picks the largest box that fits inside the parent and has aspect ratio `W:H`. This is specified in CSS Sizing 4 and CSS Box Sizing 4. Browser support is Chrome 88+, Firefox 89+, Safari 15+ — all current.
- An SVG with `viewBox="0 0 W H"` and `preserveAspectRatio="xMidYMid meet"` establishes a coordinate system in **source pixels**. Because the SVG's CSS box is exactly `W×H`-shaped (it shares the stage), `meet` performs a uniform scale of factor `renderedW / W = renderedH / H`. A bbox at `(x, y, w, h)` in source pixels lands on exactly those rendered pixels.
- `vector-effect="non-scaling-stroke"` keeps stroke width in CSS pixels regardless of zoom, which is what every annotation UI wants.

## How the reference tools solve it

| Tool | Pattern | Notes |
|---|---|---|
| **CVAT** (`cvat-canvas`) | Single root `<svg>` with `viewBox` equal to image natural size; `<image>` element is the first child of the SVG (id `cvat_canvas_image`); all shapes are siblings inside the same SVG using the same coordinate space. | Effectively the same idea, except the raster is rendered as an SVG `<image>` instead of an HTML `<img>`. No alignment problem because there is no second element to sync. ([cvat-ai/cvat](https://github.com/cvat-ai/cvat/tree/develop/cvat-canvas)) |
| **Label Studio** | Annotations are stored as percentages of the original image, then re-projected onto the displayed image rect at render time. Has a known bug (#7379) when image is cropped at the viewport — confirms percentages **must** be mapped against the actual rendered content rect. | ([labelstud.io/tags/image](https://labelstud.io/tags/image), [issue #7379](https://github.com/HumanSignal/label-studio/issues/7379)) |
| **Foxglove Studio** image panel | `ImageAnnotations` are in source pixel coordinates; panel composites the raster and the annotation layer at the source resolution then scales the composite. Equivalent to the SVG-with-viewBox approach. | ([docs.foxglove.dev/docs/visualization/panels/image](https://docs.foxglove.dev/docs/visualization/panels/image)) |
| **Roboflow InferenceJS** | Returns `bbox{x,y,w,h}` either in source pixels or normalized 0–1, lets the consumer draw onto a canvas/SVG sized to the source. The official React example draws to a canvas overlaid on the `<img>`/`<video>` with explicit width/height set to the source dimensions. | ([github.com/roboflow/inferencejs-react-example](https://github.com/roboflow/inferencejs-react-example)) |
| **Sunone Aimbot / Aimmy** (desktop, not web) | Both render into a transparent **borderless top-most window** whose client rect equals the detection capture rect (typically a centered 320×320 / 384×216 region of the screen). ImGui/DirectX draws bboxes in detection-window pixel space directly, so coordinates are already 1:1 with the source pixels they were detected from. | This is the desktop equivalent of "stage div = source AR; everything drawn in source pixels". ([deepwiki.com/SunOner/sunone_aimbot/4-system-architecture](https://deepwiki.com/SunOner/sunone_aimbot/4-system-architecture)) |

The unifying principle across all five: **pick a single coordinate space equal to the source resolution, render everything (raster + annotations) into that space, then let one outer transform scale the whole composite for display.** The web realization of that principle is `aspect-ratio` stage + SVG `viewBox`.

## Edge cases

- **Vertical-fit vs horizontal-fit**: handled automatically. If the parent is wider than `W/H` × parentHeight, the stage hits `max-height: 100%` and gutters appear left/right. Otherwise it hits `max-width: 100%` and gutters appear top/bottom. No JS measurement needed.
- **Parent narrower than any reasonable min size**: add `min-width: 0; min-height: 0` to the panel and any flex ancestors so `max-width: 100%` on the stage is actually honored. Flex items default to `min-width: auto` which can break shrink.
- **Parent has no defined height** (e.g. inside a column flex that's `height: auto`): the stage will collapse. Give the panel an explicit height (`flex: 1`, `height: 100%`, or a grid row of `1fr`).
- **HiDPI / fractional scaling**: rendered stage size can be a non-integer number of CSS pixels. `vector-effect="non-scaling-stroke"` and integer-or-half-integer bbox coordinates avoid sub-pixel blur on strokes. The SVG itself handles fractional scaling losslessly.
- **Stage smaller than 1 source pixel per CSS pixel** (downscaling): fine, scale factor < 1. The bboxes shrink in lockstep with the image, alignment is preserved.
- **Annotation rounding**: don't round bbox coordinates client-side before passing into the SVG — let the SVG transform handle the float math.

## MJPEG `<img>` vs `<video>` pitfalls

- **MJPEG `<img>`**: the natural size is known after the first frame loads, but the `<img>` keeps that natural size across frame replacements (MJPEG is the same `<img>` element receiving new bytes). Set `aspect-ratio` on the stage from the **known source resolution** (camera config), not from `naturalWidth`, to avoid a layout shift on first frame. If you must read it dynamically, attach a one-shot `load` listener and set the stage's `style.aspectRatio = naturalWidth / naturalHeight` once.
- **MJPEG over `<img>` does not fire `onload` per frame** in Chromium — only on the initial connection. Don't try to drive overlay updates from `img.onload`.
- **`<video>`**: `videoWidth`/`videoHeight` are available after `loadedmetadata`. Set stage `aspect-ratio` from those. `<video>` honors `object-fit: contain` natively, but again the stage-based pattern means you don't depend on that.
- **`<video>` poster letterboxing**: if `poster` has a different aspect ratio than the stream, you'll briefly see misalignment. Either omit `poster` or pre-render it at the source resolution.
- **CORS for MJPEG**: drawing the image to a `<canvas>` taints it without `crossorigin="anonymous"` + server CORS headers. SVG overlay sidesteps this entirely — `<img>` and `<svg>` are separate elements, no pixel readback needed.
- **MJPEG frame tearing / async drop**: SVG overlay updates can race ahead of the next MJPEG frame. The bbox visually "leads" the image by up to one frame interval. Mitigation: stamp each bbox payload with the frame ID, buffer overlay updates, render on the next `requestAnimationFrame` aligned to a known MJPEG frame boundary if you have one. For `<video>`, use `requestVideoFrameCallback` to align bbox draws to the actual presented frame.
- **`<video>` with `controls`**: control bar overlays the bottom of the video and will visually overlap bboxes. Either hide controls or shrink the stage's effective bottom margin to exclude the control strip.

## Why **not** option B (JS measurement of `getBoundingClientRect`)

- Adds a `ResizeObserver` + layout-read tax on every panel resize and parent reflow.
- Creates a one-frame lag window where the overlay is the wrong size.
- Forces a render synchronization path between layout and React state.
- Becomes incorrect under CSS transforms / zoom unless you also normalize for `getBoundingClientRect`'s scaled output.

Option A is strictly superior: zero JS, zero layout reads, zero sync bugs, and the same source-of-truth (source W×H) drives both the stage size and the SVG viewBox.

## Final canonical pattern (for ROC AI Vision)

```html
<div class="video-panel">
  <div class="stage"
       style="aspect-ratio: {{ srcW }} / {{ srcH }};">
    <img class="stream" src="/mjpeg" alt="" />
    <svg class="overlay"
         viewBox="0 0 {{ srcW }} {{ srcH }}"
         preserveAspectRatio="xMidYMid meet">
      {#each detections as d}
        <rect x={d.x} y={d.y} width={d.w} height={d.h}
              fill="none" stroke="lime" stroke-width="2"
              vector-effect="non-scaling-stroke" />
        <text x={d.x} y={d.y - 4} font-size="12" fill="lime">{d.label}</text>
      {/each}
      <!-- FOV ring, crosshair, etc. all in source-pixel space -->
    </svg>
  </div>
</div>
```

```css
.video-panel {
  display: grid;
  place-items: center;
  width: 100%;
  height: 100%;
  min-width: 0;
  min-height: 0;
  background: #0b0b0b;
  overflow: hidden;
}
.stage {
  position: relative;
  max-width: 100%;
  max-height: 100%;
  overflow: hidden;
}
.stage > .stream,
.stage > .overlay {
  position: absolute;
  inset: 0;
  width: 100%;
  height: 100%;
  display: block;
  pointer-events: none;
}
.stage > .overlay { pointer-events: none; }
.stage > .stream  { object-fit: fill; }
```

This is the production pattern.

## Citations

1. CSS-Tricks — *Aspect Ratios with SVG* (viewBox + grid stack pattern):
   https://css-tricks.com/aspect-ratios-svg/
2. W3C SVG WG — *Coordinate systems, viewBox and preserveAspectRatio*:
   https://www.w3.org/TR/SVG11/coords.html (and `svgwg/master/coords.html`)
3. CVAT canvas source — single-SVG-with-viewBox annotation surface:
   https://github.com/cvat-ai/cvat/tree/develop/cvat-canvas
4. Foxglove Studio image panel docs — pixel-coord `ImageAnnotations`:
   https://docs.foxglove.dev/docs/visualization/panels/image
5. Label Studio image tag docs + bug confirming percentage→displayed-rect mapping:
   https://labelstud.io/tags/image
   https://github.com/HumanSignal/label-studio/issues/7379
6. Sunone Aimbot architecture (desktop overlay equivalent):
   https://deepwiki.com/SunOner/sunone_aimbot/4-system-architecture
