[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels/image#__docusaurus_skipToContent_fallback)

On this page

Display raw and compressed images, as well as compressed videos, with 2D annotations like text labels, circles, and points. Superimpose 3D markers for additional context.

![image panel](https://docs.foxglove.dev/assets/images/panel-3221e15366825b86674fb08e1e8fc62f.jpeg)

### Supported encodings [​](https://docs.foxglove.dev/docs/visualization/panels/image\#supported-encodings "Direct link to Supported encodings")

| Raw images | Compressed images | Compressed videos |
| --- | --- | --- |
| `8UC1` | `webp` | `h264` |
| `8UC3` | `jpeg` | `h265` (HEVC) |
| `16UC1` | `jpg` | `vp9` |
| `32FC1` | `png` | `av1` |
| `bayer_bggr8` | `avif` |  |
| `bayer_gbrg8` |  |  |
| `bayer_grbg8` |  |  |
| `bayer_rggb8` |  |  |
| `bgr8` |  |  |
| `bgra8` |  |  |
| `mono8` |  |  |
| `mono16` |  |  |
| `nv12` |  |  |
| `rgb8` |  |  |
| `rgba8` |  |  |
| `uyvy` or `yuv422` |  |  |
| `yuyv` or `yuv422_yuy2` |  |  |

## Supported messages [​](https://docs.foxglove.dev/docs/visualization/panels/image\#supported-messages "Direct link to Supported messages")

To use this panel, your data source must provide messages conforming to [3D marker message types](https://docs.foxglove.dev/docs/visualization/panels/3d#supported-messages) or one of the following supported schemas.

### `RawImage` [​](https://docs.foxglove.dev/docs/visualization/panels/image\#rawimage "Direct link to rawimage")

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/Image`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/Image.html) |
| ROS 2 | [`sensor_msgs/msg/Image`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/Image.msg) |
| Custom | [`foxglove.RawImage`](https://docs.foxglove.dev/docs/sdk/schemas/raw-image) |

### `CompressedImage` [​](https://docs.foxglove.dev/docs/visualization/panels/image\#compressedimage "Direct link to compressedimage")

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/CompressedImage`](https://docs.ros.org/en/api/sensor_msgs/html/msg/CompressedImage.html) |
| ROS 2 | [`sensor_msgs/msg/CompressedImage`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/CompressedImage.msg) |
| Custom | [`foxglove.CompressedImage`](https://docs.foxglove.dev/docs/sdk/schemas/compressed-image) |

### `CompressedVideo` [​](https://docs.foxglove.dev/docs/visualization/panels/image\#compressedvideo "Direct link to compressedvideo")

note

See the [`CompressedVideo` schema documentation](https://docs.foxglove.dev/docs/sdk/schemas/compressed-video) for information about how to correctly encode video messages.

warning

Compressed video support is subject to hardware limitations and patent licensing, so not all encodings may be supported on all platforms. See more about [H.265 support](https://caniuse.com/hevc), [VP9 support](https://caniuse.com/webm), and [AV1 support](https://caniuse.com/av1).

| framework | schema |
| --- | --- |
| ROS 1 | [`foxglove_msgs/CompressedVideo`](https://github.com/foxglove/foxglove-sdk/blob/main/schemas/ros1/CompressedVideo.msg) |
| ROS 2 | [`foxglove_msgs/msg/CompressedVideo`](https://github.com/foxglove/foxglove-sdk/blob/main/schemas/ros2/CompressedVideo.msg) |
| Custom | [`foxglove.CompressedVideo`](https://docs.foxglove.dev/docs/sdk/schemas/compressed-video) |

### `CameraCalibration` [​](https://docs.foxglove.dev/docs/visualization/panels/image\#cameracalibration "Direct link to cameracalibration")

Provide optional camera calibration data to render 3D entities in the Image panel, or render images in the 3D panel. Calibration data is not required to display `ImageAnnotations` since they use pixel coordinates.

Foxglove supports the following distortion models: `plumb_bob`, `rational_polynomial`, `kannala_brandt`, and `fisheye62`. The `equidistant` model is also supported when using ROS `sensor_msgs/CameraInfo` schema, where it is treated as equivalent to `kannala_brandt`.

note

Following ROS conventions, Foxglove treats a camera as uncalibrated when `K[0] === 0`. In this case, the image is rendered without applying camera calibration.

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/CameraInfo`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/CameraInfo.html) |
| ROS 2 | [`sensor_msgs/msg/CameraInfo`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/CameraInfo.msg) |
| Custom | [`foxglove.CameraCalibration`](https://docs.foxglove.dev/docs/sdk/schemas/camera-calibration) |

### `ImageAnnotations` [​](https://docs.foxglove.dev/docs/visualization/panels/image\#imageannotations "Direct link to imageannotations")

| framework | schema |
| --- | --- |
| ROS 1 | [`visualization_msgs/ImageMarker`](http://docs.ros.org/en/noetic/api/visualization_msgs/html/msg/ImageMarker.html) |
| ROS 2 | [`visualization_msgs/msg/ImageMarker`](https://github.com/ros2/common_interfaces/blob/master/visualization_msgs/msg/ImageMarker.msg) |
| Custom | [`foxglove.ImageAnnotations`](https://docs.foxglove.dev/docs/sdk/schemas/image-annotations) |

Hovering over an annotation displays a tooltip with the annotation's metadata. Individual annotations (`CircleAnnotation`, `PointsAnnotation`, `TextAnnotation`) can include their own `metadata` field. Per-annotation metadata is merged with the top-level `ImageAnnotations``metadata`; when both contain the same key, the per-annotation value takes precedence.

### `SceneEntity` [​](https://docs.foxglove.dev/docs/visualization/panels/image\#sceneentity "Direct link to sceneentity")

3D scene entities can be superimposed on images when a calibration topic is configured. See [3D panel scene entity docs](https://docs.foxglove.dev/docs/visualization/panels/3d#scene-entity) for more details.

| framework | schema |
| --- | --- |
| Custom | [`foxglove.SceneEntity`](https://docs.foxglove.dev/docs/sdk/schemas/scene-entity) |
| Custom | [`foxglove.SceneUpdate`](https://docs.foxglove.dev/docs/sdk/schemas/scene-update) |

## Settings [​](https://docs.foxglove.dev/docs/visualization/panels/image\#settings "Direct link to Settings")

### General [​](https://docs.foxglove.dev/docs/visualization/panels/image\#general "Direct link to General")

| field | description |
| --- | --- |
| **Topic** | Camera image or video topic to display |
| **Calibration** | Calibration topic to use for distortion and 3D markers |
| **Sync timestamps** | Enables [timestamp synchronization](https://docs.foxglove.dev/docs/visualization/panels/image#timestamp-synchronization) for the image and its associated topics. See the dedicated section below for details. |
| **Flip horizontal** | Flip image across the vertical axis |
| **Flip vertical** | Flip image across the horizontal axis |
| **Rotation** | Amount of image rotation (0°, 90°, 180°, or 270°) |
| **Color mode** | Only supported for single-channel images: `mono8`, `8UC1`, `mono16`, `16UC1`, and `32FC1`.<br>- **Color map** – Pre-defined color palette<br>- **Gradient** – Smooth transition between two custom colors |
| **Color map** | Only shown if "Color mode" is set to "Color map".<br>For mapping `mono8`, `8UC1`, `mono16`, `16UC1`, and `32FC1` image values to colors:<br>- "Turbo" (Google)<br>- "Rainbow" (RViz) |
| **Gradient** | Only shown if "Color mode" is set to "Gradient".<br>Specifies gradient color values for `mono8`, `8UC1`, `mono16`, `16UC1`, and `32FC1` images. |
| **Value min** | Minimum scaling value for `mono8`, `8UC1`, `mono16`, `16UC1`, and `32FC1` images (default: 0). |
| **Value max** | Maximum scaling value for `mono8`, `8UC1`, `mono16`, `16UC1`, and `32FC1` images. Defaults: 255 for `mono8`/`8UC1`, 10000 for `mono16`/`16UC1`, and 1.0 for `32FC1`. For `32FC1`, `NaN`/`Inf` pixels are rendered as black. |

### Image overlays [​](https://docs.foxglove.dev/docs/visualization/panels/image\#image-overlays "Direct link to Image overlays")

Superimpose one or more image topics on top of the base image. You can add overlays, reorder them, toggle visibility, and adjust how each overlay is composited.

Overlay topics must use the same `frame_id` and image dimensions as the base image topic. Foxglove shows an error when an overlay does not match.

| field | description |
| --- | --- |
| **Topic** | Image topic to composite on top of the base image |
| **Opacity** | Alpha strength for the overlay |
| **Blend mode** | How the overlay is composited onto the base image. `Alpha` uses standard alpha blending. `Add` adds overlay pixel values to the base image |
| **Pixel alpha** | Only supported for single-channel raw image topics. `White is transparent` makes pixels rendered as white fully transparent, which is useful for mask-style overlays |
| **Schema** | Message schema to use when a topic can be converted to multiple supported image schemas |

### Scene [​](https://docs.foxglove.dev/docs/visualization/panels/image\#scene "Direct link to Scene")

| field | description |
| --- | --- |
| **Render stats** | Display rendering performance statistics in panel corner |
| **Background** | Color of background behind the image |
| **Label scale** | Size of text labels |
| **Ignore COLLADA `<up_axis>`** | Ignore the `<up_axis>` tag in COLLADA files |
| **Mesh up-axis** | Direction of “up” when loading meshes (STL, OBJ) without orientation info ("Y-up", "Z-up") |

### Image annotations [​](https://docs.foxglove.dev/docs/visualization/panels/image\#image-annotations "Direct link to Image annotations")

2D image annotation marker topics to display.

### Transforms [​](https://docs.foxglove.dev/docs/visualization/panels/image\#transforms "Direct link to Transforms")

List of transform messages to display.

See the [3D panel docs](https://docs.foxglove.dev/docs/visualization/panels/3d#transforms-1) for more information on possible settings.

### Topics [​](https://docs.foxglove.dev/docs/visualization/panels/image\#topics "Direct link to Topics")

List of [3D marker topics](https://docs.foxglove.dev/docs/visualization/panels/3d#topics) to superimpose on the image.

### Custom layers [​](https://docs.foxglove.dev/docs/visualization/panels/image\#custom-layers "Direct link to Custom layers")

See the [3D panel docs](https://docs.foxglove.dev/docs/visualization/panels/3d) for more information on possible settings.

### Publish [​](https://docs.foxglove.dev/docs/visualization/panels/image\#publish "Direct link to Publish")

Configure publishing settings for the Image panel.

note

The cursor hover publish feature uses different message schemas for ROS and non-ROS data sources.

The message types used for ROS and non-ROS data sources are:

| Feature | Corresponding ROS message type | Foxglove message |
| --- | --- | --- |
| **Cursor click** | [geometry\_msgs/PointStamped](http://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PointStamped.html) | [foxglove.Point3InFrame](https://docs.foxglove.dev/docs/sdk/schemas/point-3-in-frame) |
| **Cursor hover** | [geometry\_msgs/PointStamped](http://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PointStamped.html) | [foxglove.Point3InFrame](https://docs.foxglove.dev/docs/sdk/schemas/point-3-in-frame) |

| Field | Description |
| --- | --- |
| **Click topic** | Topic on which to publish pixel coordinates when clicking on the image (default: `/foxglove/cursor/click`). |
| **Hover topic** | Topic on which to publish pixel coordinates when hovering over the image (default: `/foxglove/cursor/hover`). |

## Controls and shortcuts [​](https://docs.foxglove.dev/docs/visualization/panels/image\#controls-and-shortcuts "Direct link to Controls and shortcuts")

- Scroll – Zoom in and out
- Drag – Pan around the image
- `1` – Reset zoom and pan
- `i` – Show or hide the object inspector

Hover on the image to view the mouse coordinates and numeric value of a pixel. The same pixel will also be indicated on other Image panels displaying images with the same `frame_id` and pixel dimensions.

Right-click on the image to download it as a PNG file.

Click any displayed image marker to view its details.

Scroll to zoom, and drag to pan. Annotations will re-render on zoom to remain sharp.

## Troubleshooting video delay [​](https://docs.foxglove.dev/docs/visualization/panels/image\#troubleshooting-video-delay "Direct link to Troubleshooting video delay")

You may see errors for `foxglove.CompressedVideo` topics stating that the frame being displayed is delayed a number of milliseconds from the most recent frame. If you're seeing the delay grow during playback, then this could be due to the video decoder on your platform. Video decoding behavior can vary across platforms, and some platforms may experience delays while others do not. It is possible to configure your video stream to optimize for low-latency decoding, but it may be impossible to _guarantee_ that a video stream can be decoded with zero latency on all platforms.

Some tips on how configure the video encoding to best achieve low-latency decoding:

- Use the `BASELINE` profile for `h264` encoded streams. This profile is preferred because it does not support B frames. Profiles which support B frames may introduce decoding delay, even if the encoded stream doesn't actually contain B frames.
- Disable frame reordering on your encoder.
- Lower profile levels (`level_idc`) usually require smaller buffers, resulting in lower-latency decoding.
- If your encoder allows it, use a `bitstream_restriction` in the VUI parameters to limit the size of the buffer (`max_dec_frame_buffering`) and also disable frame reordering (`max_num_reorder_frames`).

If you see that the video delay increases significantly during playback that likely means that the video decoder can't keep up with your stream, and you'll want to reduce the number of your panels displaying video streams and/or check that your platform supports video acceleration. Read on to see more information.

System CPU load and power consumption may also contribute to decoding delays. Hardware-accelerated decoding is generally faster and more energy-efficient. Make sure to check that it is enabled.

These steps can help your platform achieve low-latency decoding, but there is no guarantee depending on the platform and device being used.

## Checking video acceleration [​](https://docs.foxglove.dev/docs/visualization/panels/image\#checking-video-acceleration "Direct link to Checking video acceleration")

You can check whether your platform supports hardware-accelerated video decoding by opening Google Chrome and entering `chrome://gpu` in the address bar. If you do not see `Video Decode: Hardware accelerated` or the `Video Acceleration` section is empty on this page, but you believe that it should be supported, then you may need to take additional platform-specific steps to enable it.

The desktop app uses Electron (based on Chromium), and should automatically use hardware acceleration when available. If you're seeing issues here, we recommend following the steps above to see if the same issue exists in Chrome. If so, there's a good chance it's affecting our application for the same reasons. We recommend getting things working in Chrome first, and if the desktop app is still having issues afterward please [let us know](https://foxglove.dev/chat) and we'll look into it.

## Enabling video acceleration [​](https://docs.foxglove.dev/docs/visualization/panels/image\#enabling-video-acceleration "Direct link to Enabling video acceleration")

Tips for Foxglove web in Chromium browsers:

- Go to `chrome://settings`, search your settings for `acceleration` and ensure that the "Use graphics acceleration when available" toggle is enabled.
- Go to `chrome://flags`, search for `video` and ensure that "Hardware-accelerated video decode" is enabled.

MacOS:

- For web, significant performance gains have been observed when changing the graphics backend to use "Metal" in Chromium-based browsers. You can do this in `about://flags`.

On Linux:

- Note that Chrome/Chromium doesn't support video acceleration on Linux by default and custom Chrome/Chromium flags may need to be used to enable it with your GPU.
- Ensure that your graphics drivers are up-to-date and installed properly.

## Timestamp synchronization [​](https://docs.foxglove.dev/docs/visualization/panels/image\#timestamp-synchronization "Direct link to Timestamp synchronization")

The **Sync timestamps** setting gates rendering on exact timestamp alignment across the image topic and its associated topics. When enabled, the panel buffers incoming messages and only renders a frame when every participating topic has a message with the same timestamp.

### How it works [​](https://docs.foxglove.dev/docs/visualization/panels/image\#how-it-works "Direct link to How it works")

1. Enable **Sync timestamps** in the General settings section.
2. The primary image topic is always included in synchronization.
3. All visible [`ImageAnnotations`](https://docs.foxglove.dev/docs/sdk/schemas/image-annotations) topics are automatically included — there is no separate opt-out.
4. For 3D overlay topics, enable the **Sync** toggle in the topic's settings to include them.
5. At least **one** additional topic (a visible annotation or a Sync-enabled overlay) must participate alongside the image topic — otherwise there is nothing to match against.

The panel retains up to **250 unique timestamps** in its synchronization buffer. If a match is not found within that window, the oldest timestamps are discarded. For best results, ensure your topics publish at the same rate and with aligned timestamps.

Calibration topics always use the most recently received message and do not participate in synchronization.

### Timestamp extraction [​](https://docs.foxglove.dev/docs/visualization/panels/image\#timestamp-extraction "Direct link to Timestamp extraction")

Timestamps are extracted from `header.stamp` for ROS messages or the `timestamp` field for Foxglove messages — not [log time](https://docs.foxglove.dev/docs/visualization/playback#message-ordering). For scene updates and marker arrays, the timestamp of the first entity or marker in the message is used. For `ImageAnnotations`, the root-level `timestamp` is used; if absent, the `timestamp` of the first annotation (circles, points, or texts) is used.

### What is not synchronized [​](https://docs.foxglove.dev/docs/visualization/panels/image\#what-is-not-synchronized "Direct link to What is not synchronized")

Transforms and URDFs are never registered for synchronization. Transforms continue to build the transform tree normally, and all entities compute their position at the synced timestamp.

## Links and resources [​](https://docs.foxglove.dev/docs/visualization/panels/image\#links-and-resources "Direct link to Links and resources")

- [Annotate camera images with image markers](https://foxglove.dev/blog/annotate-your-robots-camera-images-with-image-markers)

- [Supported encodings](https://docs.foxglove.dev/docs/visualization/panels/image#supported-encodings)
- [Supported messages](https://docs.foxglove.dev/docs/visualization/panels/image#supported-messages)
  - [`RawImage`](https://docs.foxglove.dev/docs/visualization/panels/image#rawimage)
  - [`CompressedImage`](https://docs.foxglove.dev/docs/visualization/panels/image#compressedimage)
  - [`CompressedVideo`](https://docs.foxglove.dev/docs/visualization/panels/image#compressedvideo)
  - [`CameraCalibration`](https://docs.foxglove.dev/docs/visualization/panels/image#cameracalibration)
  - [`ImageAnnotations`](https://docs.foxglove.dev/docs/visualization/panels/image#imageannotations)
  - [`SceneEntity`](https://docs.foxglove.dev/docs/visualization/panels/image#sceneentity)
- [Settings](https://docs.foxglove.dev/docs/visualization/panels/image#settings)
  - [General](https://docs.foxglove.dev/docs/visualization/panels/image#general)
  - [Image overlays](https://docs.foxglove.dev/docs/visualization/panels/image#image-overlays)
  - [Scene](https://docs.foxglove.dev/docs/visualization/panels/image#scene)
  - [Image annotations](https://docs.foxglove.dev/docs/visualization/panels/image#image-annotations)
  - [Transforms](https://docs.foxglove.dev/docs/visualization/panels/image#transforms)
  - [Topics](https://docs.foxglove.dev/docs/visualization/panels/image#topics)
  - [Custom layers](https://docs.foxglove.dev/docs/visualization/panels/image#custom-layers)
  - [Publish](https://docs.foxglove.dev/docs/visualization/panels/image#publish)
- [Controls and shortcuts](https://docs.foxglove.dev/docs/visualization/panels/image#controls-and-shortcuts)
- [Troubleshooting video delay](https://docs.foxglove.dev/docs/visualization/panels/image#troubleshooting-video-delay)
- [Checking video acceleration](https://docs.foxglove.dev/docs/visualization/panels/image#checking-video-acceleration)
- [Enabling video acceleration](https://docs.foxglove.dev/docs/visualization/panels/image#enabling-video-acceleration)
- [Timestamp synchronization](https://docs.foxglove.dev/docs/visualization/panels/image#timestamp-synchronization)
  - [How it works](https://docs.foxglove.dev/docs/visualization/panels/image#how-it-works)
  - [Timestamp extraction](https://docs.foxglove.dev/docs/visualization/panels/image#timestamp-extraction)
  - [What is not synchronized](https://docs.foxglove.dev/docs/visualization/panels/image#what-is-not-synchronized)
- [Links and resources](https://docs.foxglove.dev/docs/visualization/panels/image#links-and-resources)