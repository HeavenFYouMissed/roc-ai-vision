[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels/3d#__docusaurus_skipToContent_fallback)

On this page

Display markers, entities, camera images, meshes, URDF models, and more in a 3D scene.

![3d panel](https://docs.foxglove.dev/assets/images/panel-2e51fd1b3cc2cb922c6e459ddcbfaaa4.png)

## Visualization [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#visualization "Direct link to Visualization")

Visualizing objects in the 3D scene correctly requires your data to conform to certain conventions. When you enable a topic for visualization, the 3D panel will subscribe to the topic, receive messages on the topic in **log time** order (see: [playback](https://docs.foxglove.dev/docs/visualization/playback#message-ordering)), and visualize the latest message on each enabled topic using the `timestamp` or `header.stamp` (ROS) information in the message.

To render objects into the scene, there must exist a transform path from the object's coordinate frame to the display frame _at or before the timestamp of the object being visualized_. For objects this timestamp comes from the `timestamp` or `header.stamp` (ROS) field within the message, NOT the log time (aka receive time). Like other messages, transform messages are received in log time order but stored and accessed by the `timestamp` or `header.stamp` field within the message.

This convention allows your robot to receive sensor data and transform state, perceive and identify objects in the scene, and produce a message describing the scene at some time that relates to the sensor data or processing rather than the log time which might come later. This provides a mechanism for more accurately representing the scene as the robot may have experienced it.

## Supported messages [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#supported-messages "Direct link to Supported messages")

The 3D panel can visualize an assortment of different messages.

To visualize a topic, the messages on that topic must conform to one of the known message schemas listed below.

### Camera field-of-view [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#camera-field-of-view "Direct link to Camera field-of-view")

Calibration parameters for your scene's camera.

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/CameraInfo`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/CameraInfo.html) |
| ROS 2 | [`sensor_msgs/msg/CameraInfo`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/CameraInfo.msg) |
| Custom | [`foxglove.CameraCalibration`](https://docs.foxglove.dev/docs/sdk/schemas/camera-calibration) |

### Grid [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#grid "Direct link to Grid")

2D colored grids.

![grid](https://docs.foxglove.dev/assets/images/occupancygrid-61558ef9ecb1463b3093a484fea56afb.png)

![gridmap](https://docs.foxglove.dev/assets/images/gridmap-b783e2453f8e76c0f0d6ee5a3ba37d26.png)

| framework | schema |
| --- | --- |
| ROS 1 | [`nav_msgs/OccupancyGrid`](https://docs.ros.org/en/noetic/api/nav_msgs/html/msg/OccupancyGrid.html), [`map_msgs/OccupancyGridUpdate`](https://docs.ros.org/en/noetic/api/map_msgs/html/msg/OccupancyGridUpdate.html), [`grid_map_msgs/GridMap`](http://docs.ros.org/en/jade/api/grid_map_msgs/html/msg/GridMap.html) |
| ROS 2 | [`nav_msgs/msg/OccupancyGrid`](https://github.com/ros2/common_interfaces/blob/master/nav_msgs/msg/OccupancyGrid.msg), [`map_msgs/msg/OccupancyGridUpdate`](https://github.com/ros-planning/navigation_msgs/blob/rolling/map_msgs/msg/OccupancyGridUpdate.msg), [`grid_map_msgs/msg/GridMap`](https://docs.ros.org/en/ros2_packages/jazzy/api/grid_map_msgs/msg/GridMap.html) |
| Custom | [`foxglove.Grid`](https://docs.foxglove.dev/docs/sdk/schemas/grid) |

#### `foxglove.Grid` and `GridMap` settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#foxglovegrid-and-gridmap-settings "Direct link to foxglovegrid-and-gridmap-settings")

| field | description |
| --- | --- |
| **Color mode** | One of: <br>**Flat**: solid color<br>**Color map**: pre-defined color palette<br>**Gradient**: smooth transition between two custom colors<br>**RGBA (separate fields)**: use embedded color from each cell's `red`, `green`, `blue`, and `alpha` fields ( [see below](https://docs.foxglove.dev/docs/visualization/panels/3d#rgba-separate-fields-color-mode)) <br>Note: only `Color Map` and `Gradient` are currently supported for `GridMap` messages |
| **Flat color** | Only shown if "Color mode" is set to "Flat"; hex code for color of each cell |
| **Color field**<br>**Color layer** | Only shown if "Color mode" is not set to "Flat"; numeric field in message used for coloring logic |
| **Color map** | Only shown if "Color mode" is set to "Color map"; "Turbo" (Google) or "Rainbow" (RViz); for mapping "Color field" values to colors |
| **Gradient** | Only shown if "Color mode" is set to "Gradient"; The two colors to interpolate between, based on the "color field" |
| **Opacity** | Only shown if "Color mode" is set to "Color map" or "BGR (packed)"; sets alpha value for all cells |
| **Value min** | Only shown if "Color mode" is not set to "Flat" and the message is not a `GridMap` schema; minimum value used to normalize incoming grid's "Color field" values |
| **Value max** | Only shown if "Color mode" is not set to "Flat" and the message is not a `GridMap` schema; maximum value used to normalize incoming grid's "Color field" values |
| **Elevation field**<br>**Elevation layer** | Optional field to displace each grid cell in the Z axis; if not set, elevation is 0 for all cells |
| **Interpolation method** | Only shown if "Elevation" is set; either **Linear** or **Nearest Neighbor**. Determines how elevation and color values are sampled spatially across the grid. |
| **Show lines** | Only shown if "Elevation" is set; whether to render subtle grid cell borders |
| **Line color** | Only shown if "Show lines" is enabled; color of the cell grid lines |
| **Lighting** | Only shown if "Elevation" is set; enables shading from simulated directional lighting, based on elevation surface normals |
| **Frame lock** | "On" means the grid is locked to the frame specified by its `frame_id`, and will move as that frame's transforms change. "Off" means the grid is relative to the fixed frame and will not move after it is first displayed |
| **Draw behind** | "On" means the grid is rendered before the rest of the scene, so it appears behind other scene elements regardless of their relative depth. "Off" means the grid is rendered normally |

##### RGBA (separate fields) color mode [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#rgba-separate-fields-color-mode "Direct link to RGBA (separate fields) color mode")

Each cell can contain color information in four separate fields. **All four fields must be present** with the exact names `red`, `green`, `blue`, and `alpha`, of any numeric type:

- **Floating-point values** — 0–1 range
- **Unsigned integer values** — Maximum possible range (e.g. 0–255 for a `UINT8` field)
- **Signed integer values** — `-max` to `max` (e.g. −127 to 127 for an `INT8` field; a value of −128 is treated as identical to −127)

tip

The field names must match exactly (though the order doesn't matter). When these fields are detected, additional "Color Mode" options will become available.

#### `nav_msgs/OccupancyGrid` settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#nav_msgsoccupancygrid-settings "Direct link to nav_msgsoccupancygrid-settings")

| field | description |
| --- | --- |
| **Color mode** | One of: <br>**Costmap**: pre-defined RViz color palette. Cannot customize settings further.<br>**Custom**: custom color palette using settings below |
| **Min color** | Color corresponding to minimum cell value (0) |
| **Max color** | Color corresponding to maximum cell value (100). Note that cells with value exactly 100 are displayed as fully transparent. |
| **Unknown color** | Color corresponding to unknown cell value (−1) |
| **Fallback color** | Color corresponding to cell values that fall outside the range from −1 to 100 |
| **Frame lock** | "On" means the grid is locked to the frame specified by its `frame_id`, and will move as that frame's transforms change. "Off" means the grid is relative to the fixed frame and will not move after it is first displayed. |

#### Incremental occupancy grid updates [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#incremental-occupancy-grid-updates "Direct link to Incremental occupancy grid updates")

The 3D panel supports `map_msgs/OccupancyGridUpdate` messages for incremental costmap updates. ROS's `costmap_2d` node publishes two topics per costmap:

- A **full grid** (`nav_msgs/OccupancyGrid`) at a low rate (typically 1–2 Hz)
- **Partial updates** (`map_msgs/OccupancyGridUpdate`) at a higher rate (typically 3–12 Hz)

The panel automatically pairs update topics to their base grid using the `_updates` suffix convention (e.g. `/costmap` and `/costmap_updates`). Update topics do not appear as separate entries in the settings sidebar — they are consumed internally.

Performance

Each update triggers a full texture rebuild of the entire grid. The cost scales linearly at approximately **13 ns per cell**. Grids up to ~1.28 million cells (~1130×1130) fit within a single frame at 60 fps. Larger grids (e.g. 2000×2000 global costmaps) may cause frame drops during updates.

### Image [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#image "Direct link to Image")

Images displayed in the 3D scene, using the corresponding [Camera field-of-view](https://docs.foxglove.dev/docs/visualization/panels/3d#camera-field-of-view) messages.

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/Image`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/Image.html) |
| ROS 2 | [`sensor_msgs/msg/Image`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/Image.msg) |
| ROS 1 | [`sensor_msgs/CompressedImage`](https://docs.ros.org/en/api/sensor_msgs/html/msg/CompressedImage.html) |
| ROS 2 | [`sensor_msgs/msg/CompressedImage`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/CompressedImage.msg) |
| Custom | [`foxglove.RawImage`](https://docs.foxglove.dev/docs/sdk/schemas/raw-image) |
| Custom | [`foxglove.CompressedImage`](https://docs.foxglove.dev/docs/sdk/schemas/compressed-image) |

#### Render modes [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#render-modes "Direct link to Render modes")

Image topics in the 3D panel support three render modes. Each mode has context-specific settings.

- [**Default**](https://docs.foxglove.dev/docs/visualization/panels/3d#default) – Projects the image onto a distorted plane at a fixed distance from the camera.
- [**Plane projection**](https://docs.foxglove.dev/docs/visualization/panels/3d#plane-projection) – Projects the image onto the XY plane of a selected coordinate frame.
- [**Depth map**](https://docs.foxglove.dev/docs/visualization/panels/3d#depth-map) – Renders depth images as point clouds.

#### Default [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#default "Direct link to Default")

The default render mode displays images using standard camera projection. This mode shows the following settings:

| field | description |
| --- | --- |
| **Render mode** | Select how the image is rendered: **Default**, **Plane projection**, or **Depth map**. |
| **Distance** | Distance from the camera at which the image is projected |
| **Planar projection factor** | Controls the planar projection effect, from zero to one. Zero results in a curved plane with all points equidistant to camera origin. One results in a flat plane orthogonal to the camera's z (depth) axis. |

#### Depth map [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#depth-map "Direct link to Depth map")

The 3D panel can render uncompressed depth images as point clouds, enabling visualization of depth camera data directly in 3D space.

To render a depth image as a point cloud, you need:

- An uncompressed depth image topic with a supported encoding (`32FC1`, `16UC1`, `MONO16`, `8UC1`, `MONO8`, or `8UC3`), or a compressed depth image topic using `compressedDepth` 16-bit grayscale PNG
- A corresponding [Camera field-of-view](https://docs.foxglove.dev/docs/visualization/panels/3d#camera-field-of-view) message with camera calibration data

Enable the depth image topic in the panel's topic settings, then set the **Render mode** to **Depth map**.

Compressed depth images must be 16-bit grayscale PNG payloads (for example, ROS `compressedDepth` transport using `sensor_msgs/CompressedImage` with a `compressedDepth` format string). Depth values are interpreted as millimeters (see **Distance type** below for how values are projected into 3D). Other compressed formats (JPEG, color PNG, or 8-bit PNG) are not supported and will show an error.

| field | description |
| --- | --- |
| **Render mode** | Select how the image is rendered: **Default**, **Plane projection**, or **Depth map**. Only one mode can be active at a time. Settings shown below are only visible when "Render mode" is set to "Depth map". |
| **Distance type** | How to interpret pixel values: **Z-axis** (default) interprets each pixel as the distance along the camera's optical axis, while **Euclidean** interprets each pixel as the straight-line distance from the sensor origin. Use **Euclidean** for sensors that report range (e.g., time-of-flight cameras). |
| **Point size** | Size of each rendered point in pixels |
| **Depth scale** | Scale factor to convert depth values to meters. Defaults to 1.0 for 32-bit float images (`32FC1`) and 0.001 for all other encodings (`16UC1`, `MONO16`, `8UC1`, `MONO8`, `8UC3`). Customize this to match your camera system's depth scale configuration. |
| **RGB topic** | Sibling RGB image topic to colorize the depth point cloud. When set, each point in the cloud is colored using the corresponding pixel from the RGB image. The RGB image can have a different resolution than the depth image, and Foxglove uses bilinear interpolation to sample colors accurately. Select "None" to use distance-based coloring instead. |
| **Color mode** | Only shown when **RGB topic** is set to "None". One of:<br>**Color map**: color points by depth using a pre-defined color palette<br>**Gradient**: color points by depth using a smooth transition between two custom colors<br>**Flat**: render every point with the same custom color |
| **Flat color** | Only shown if **Color mode** is set to **Flat**. Sets the point color and alpha. |
| **Color map** | Only shown if **Color mode** is set to **Color map**. Maps depth values to "Turbo" (Google) or "Rainbow" (RViz). |
| **Gradient** | Only shown if **Color mode** is set to **Gradient**. Sets the start and end colors for depth-based coloring. |
| **Opacity** | Only shown if **Color mode** is set to **Color map**. Sets alpha for all points. |
| **Value min** | Only shown if **Color mode** is **Color map** or **Gradient**. Minimum depth value used to normalize point colors. |
| **Value max** | Only shown if **Color mode** is **Color map** or **Gradient**. Maximum depth value used to normalize point colors. |

For accurate depth-to-RGB colorization, the depth and RGB images should use the same camera frame, or have matching intrinsics with only a rigid transform between frames. The current implementation does not perform stereo rectification, so you may see subtle colorization errors when the sensors are offset and have different intrinsics.

#### Plane projection [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#plane-projection "Direct link to Plane projection")

The 3D panel can project camera images onto a reference plane in the 3D scene. This is especially useful for ground-plane projection, where the elements on the ground-plane in the image can be lined up with objects in the 3D scene. However, things that are protruding or not on the plane will appear distorted.

To use plane projection, enable the camera image topic in the panel's topic settings, then set the **Render mode** to **Plane projection** and select a **Projection frame** from the available transform frames.

| field | description |
| --- | --- |
| **Render mode** | Select how the image is rendered: **Default**, **Plane projection**, or **Depth map**. Only one mode can be active at a time. Settings shown below are only visible when "Render mode" is set to "Plane projection". |
| **Projection frame** | The coordinate frame to project into. The image will be projected onto this frame's XY plane. There must be a valid transform between this frame and the camera's frame. |

### Laser scan [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#laser-scan "Direct link to Laser scan")

A single scan from a planar laser range-finder. Laser scans share the same color and display settings as [point clouds](https://docs.foxglove.dev/docs/visualization/panels/3d#point-cloud), including color mode, color map, and gradient.

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/LaserScan`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/LaserScan.html) |
| ROS 2 | [`sensor_msgs/msg/LaserScan`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/LaserScan.msg) |
| Custom | [`foxglove.LaserScan`](https://docs.foxglove.dev/docs/sdk/schemas/laser-scan) |

### Location fix [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#location-fix "Direct link to Location fix")

A latitude, longitude, and altitude (LLA) coordinate, used to globally position [the scene](https://docs.foxglove.dev/docs/visualization/panels/3d#frame) and [the maps](https://docs.foxglove.dev/docs/visualization/panels/3d#map-custom-layer) within it. Your location fix messages must use a `frame_id` in your transform tree.

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/NavSatFix`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/NavSatFix.html) |
| ROS 2 | [`sensor_msgs/msg/NavSatFix`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/NavSatFix.msg) |
| Custom | [`foxglove.LocationFix`](https://docs.foxglove.dev/docs/sdk/schemas/location-fix) [`foxglove.LocationFixes`](https://docs.foxglove.dev/docs/sdk/schemas/location-fixes) |

### ROS polygons [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#ros-polygons "Direct link to ROS polygons")

Timestamped polygons made up of a series of connected points.

| framework | schema |
| --- | --- |
| ROS 1 | [`geometry_msgs/PolygonStamped`](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PolygonStamped.html) |
| ROS 2 | [`geometry_msgs/msg/PolygonStamped`](https://github.com/ros2/common_interfaces/blob/master/geometry_msgs/msg/PolygonStamped.msg) |

### ROS markers [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#ros-markers "Direct link to ROS markers")

Similar to [scene entities](https://docs.foxglove.dev/docs/visualization/panels/3d#scene-entity), these `Marker` messages describe primitive shapes or meshes.

| framework | schema |
| --- | --- |
| ROS 1 | [`visualization_msgs/Marker`](https://docs.ros.org/en/noetic/api/visualization_msgs/html/msg/Marker.html) |
| ROS 2 | [`visualization_msgs/msg/Marker`](https://github.com/ros2/common_interfaces/blob/master/visualization_msgs/msg/Marker.msg) |
| ROS 1 | [`visualization_msgs/MarkerArray`](https://docs.ros.org/en/noetic/api/visualization_msgs/html/msg/MarkerArray.html) |
| ROS 2 | [`visualization_msgs/msg/MarkerArray`](https://github.com/ros2/common_interfaces/blob/master/visualization_msgs/msg/MarkerArray.msg) |

#### Mesh markers [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#mesh-markers "Direct link to Mesh markers")

Markers with a `mesh_resource` field support the following URL schemes:

- `http(s)://`
- `package://` (Desktop app only)
- `file://` (Desktop app only)

And file formats:

- glTF
- STL
- COLLADA
- Wavefront OBJ

**glTF (`.glb`)**

This is the preferred format, as it enjoys the best performance of all supported file types.

Binary glTF files bundle all required assets into a single file, with support for embedded meshes, compression, and the same physically-based material system used in Foxglove. As a result, your model should appear in Foxglove similarly to how it appears in other 3D programs.

**STL (`.stl`)**

STL files are well supported in Foxglove, but lack some of glTF's visualization features. The main advantage to STL is the ability to share the same files between your hardware manufacturing process and robot visualization tooling.

STL was designed for 3D printing and CAD applications, and does not include materials or hierarchies of meshes. While they can be represented in a binary encoding, STL files are commonly represented with ASCII characters, which leads to larger files.

**COLLADA (`.dae`)**

As a predecessor to glTF, COLLADA has a similar feature set. With that said, it does have larger XML-based files, no compression, and additional processing overhead.

There is a [bug in RViz](https://github.com/ros-visualization/rviz/issues/1045) where the up-axis metadata is ignored, resulting in incorrect orientations for many `.dae` files in ROS environments. To work around this, the 3D panel has a `Ignore COLLADA <up_axis>` setting to toggle between observing the `<up-axis>` tag or ignoring it like RViz.

**Wavefront OBJ (`.obj`)**

OBJ is a simple ASCII format predating all other supported formats. It has large file sizes, no material support, no mesh hierarchies, no compression, and additional processing overhead.

Material support was added to the OBJ format as separate `.mtl` files, which Foxglove does not read.

### Path [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#path "Direct link to Path")

An array of timestamped poses in a named coordinate frame, denoting an object's path through space.

| framework | schema |
| --- | --- |
| ROS 1 | [`nav_msgs/Path`](https://docs.ros.org/en/noetic/api/nav_msgs/html/msg/Path.html) |
| ROS 2 | [`nav_msgs/msg/Path`](https://github.com/ros2/common_interfaces/blob/master/nav_msgs/msg/Path.msg) |
| Custom | [`foxglove.PosesInFrame`](https://docs.foxglove.dev/docs/sdk/schemas/poses-in-frame) |

### Point cloud [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#point-cloud "Direct link to Point cloud")

A collection of N-dimensional points, which may contain additional fields with information like normals, intensity, etc.

![pointcloud](https://docs.foxglove.dev/assets/images/pointcloud-1413f0a1e671c7c18f96a01345f50242.png)

| framework | schema |
| --- | --- |
| ROS 1 | [`sensor_msgs/PointCloud2`](https://docs.ros.org/en/noetic/api/sensor_msgs/html/msg/PointCloud2.html) |
| ROS 2 | [`sensor_msgs/msg/PointCloud2`](https://github.com/ros2/common_interfaces/blob/master/sensor_msgs/msg/PointCloud2.msg) |
| Custom | [`foxglove.PointCloud`](https://docs.foxglove.dev/docs/sdk/schemas/point-cloud) |
| Custom | [`foxglove.CompressedPointCloud`](https://docs.foxglove.dev/docs/sdk/schemas/compressed-point-cloud) |

note

Foxglove supports `foxglove.CompressedPointCloud` messages encoded with Draco.

#### Settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#settings "Direct link to Settings")

| field | description |
| --- | --- |
| **Point shape** | Shape of each rendered point: "Circle", "Square", or "Cube". Default: "Circle". |
| **Point size** | Size of each rendered point in pixels. Only shown when the point shape is "Circle" or "Square". |
| **Cube size** | Size of each cube in 3D scene units. Only shown when the point shape is "Cube". |
| **Show outline** | Toggle whether to display wireframe outline around cubes. Only shown when the point shape is "Cube". Default: "Off" |
| **Decay time** | Duration of time (in sec) that each point stays rendered |
| **Color mode** | One of: <br>**Flat**: solid color<br>**Color map**: pre-defined color palette<br>**Gradient**: smooth transition between two custom colors<br>**BGR (packed)**: `sensor_msgs/PointCloud2` only; use embedded color from each point's `rgb` field ( [see below](https://docs.foxglove.dev/docs/visualization/panels/3d#rgba-color-modes))<br>**BGRA (packed)**: `sensor_msgs/PointCloud2` only; use embedded color from each point's `rgba` field ( [see below](https://docs.foxglove.dev/docs/visualization/panels/3d#rgba-color-modes))<br>**RGBA (separate fields)**: `foxglove.PointCloud` and `foxglove.CompressedPointCloud` only; use embedded color from each point's `red`, `green`, `blue`, and `alpha` fields ( [see below](https://docs.foxglove.dev/docs/visualization/panels/3d#rgba-color-modes)) |
| **Flat color** | Only shown if "Color mode" is set to "Flat"; hex code for color of each point |
| **Color field** | Only shown if "Color mode" is not set to "Flat"; value used for "Color map" coloring logic; any numeric field in message such as `x`, `y`, `z`, `<distance>` (L2 norm of coordinates), or custom defined field |
| **Color map** | Only shown if "Color mode" is set to "Color map"; "Turbo" (Google) or "Rainbow" (RViz); for mapping "Color field" values to colors |
| **Opacity** | Only shown if "Color mode" is set to "Color map" or "BGR (packed)"; sets alpha value for all points |
| **Value min** | Only shown if "Color mode" is not set to "Flat"; minimum value used to normalize incoming points' "Color field" values |
| **Value max** | Only shown if "Color mode" is not set to "Flat"; maximum value used to normalize incoming points' "Color field" values |
| **Stixel view** | Visualize points as stixels that extend from the point's z location to 0 |

#### RGBA color modes [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#rgba-color-modes "Direct link to RGBA color modes")

When using the "BGR (packed)", "BGRA (packed)", and "RGBA (separate fields)" color modes, your point cloud message must contain certain fields to display color information for each point.

##### RGBA (separate fields) [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#rgba-separate-fields "Direct link to RGBA (separate fields)")

For `foxglove.PointCloud` and `foxglove.CompressedPointCloud` messages, each point can contain color information in four separate fields, named `red`, `green`, `blue`, and `alpha`, of any numeric type:

- **Floating-point values** — 0–1 range
- **Unsigned integer values** — Maximum possible range (e.g. 0–255 for a `UINT8` field)
- **Signed integer values** — `-max` to `max` (e.g. −127 to 127 for an `INT8` field; a value of −128 is treated as identical to −127)

##### BGR (packed) and BGRA (packed) [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#bgr-packed-and-bgra-packed "Direct link to BGR (packed) and BGRA (packed)")

For `sensor_msgs/PointCloud2` messages, each point can contain color information in a single field named `rgb` or `rgba`:

- Must use a 4-byte type from [sensor\_msgs/PointField](http://docs.ros.org/en/melodic/api/sensor_msgs/html/msg/PointField.html) (`UINT32`, value 6, is recommended)
- Each red, green, blue, and alpha value is represented by one byte in the 0–255 range
- Bytes must be packed in `[0xBB, 0xGG, 0xRR, 0xAA]` order (i.e. `(0xAA << 24) | (0xRR << 16) | (0xGG << 8) | 0xBB` in little-endian order). This order is compatible with RViz.

If using the "BGR" mode, the alpha value must still be present, but is ignored.

### Pose [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#pose "Direct link to Pose")

Poses in a named coordinate frame.

| framework | schema |
| --- | --- |
| ROS 1 | [`geometry_msgs/PoseArray`](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PoseArray.html) |
| ROS 2 | [`geometry_msgs/msg/PoseArray`](https://github.com/ros2/common_interfaces/blob/master/geometry_msgs/msg/PoseArray.msg) |
| ROS 1 | [`geometry_msgs/PoseStamped`](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PoseStamped.html) |
| ROS 2 | [`geometry_msgs/msg/PoseStamped`](https://github.com/ros2/common_interfaces/blob/master/geometry_msgs/msg/PoseStamped.msg) |
| Custom | [`foxglove.PosesInFrame`](https://docs.foxglove.dev/docs/sdk/schemas/poses-in-frame) |

### Scene entity [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#scene-entity "Direct link to Scene entity")

A collection of primitive shapes (cubes, spheres, text, meshes, lines, etc) used to display anything from a basic bounding box to a complex 3D decision tree or road network.

Scene entities must be wrapped in a `SceneUpdate` message.

Model primitives (`foxglove.ModelPrimitive`) support both a `url` and `data` field. When both are provided and the URL fails to load (e.g., network error), the 3D panel falls back to the embedded `data` so the model can still be displayed.

| framework | schema |
| --- | --- |
| Custom | [`foxglove.SceneEntity`](https://docs.foxglove.dev/docs/sdk/schemas/scene-entity) |
| Custom | [`foxglove.SceneUpdate`](https://docs.foxglove.dev/docs/sdk/schemas/scene-update) |

#### Settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#settings-1 "Direct link to Settings")

| field | description |
| --- | --- |
| **Color** | Render all entities under this topic with this color, overriding colors defined in the entities. |
| **Show outlines** | Display a wireframe outline around rendered entities. |
| **Selection variable** | Set a global variable with this name to the selected entity ID. |
| **Vertex normals** | Toggle calculation of vertex normals. Normals improve lighting and visual appearance but incur a significant performance penalty in some cases. |

tip

To control entity visibility, publish them on separate [topics](https://docs.foxglove.dev/docs/visualization/panels/3d#topics) (for example, `/scene/walls`, `/scene/floor`). You can then toggle each topic independently.

### Transforms [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#transforms "Direct link to Transforms")

A transform (translation and rotation) between two reference frames in 3D space.

| framework | schema |
| --- | --- |
| ROS 1 | [`tf/tfMessage`](https://docs.ros.org/en/noetic/api/tf/html/msg/tfMessage.html) |
| ROS 1 | [`tf2_msgs/TFMessage`](https://docs.ros.org/en/noetic/api/tf2_msgs/html/msg/TFMessage.html) |
| ROS 2 | [`tf2_msgs/msg/TFMessage`](https://github.com/ros2/geometry2/blob/ros2/tf2_msgs/msg/TFMessage.msg) |
| Custom | [`foxglove.FrameTransform`](https://docs.foxglove.dev/docs/sdk/schemas/frame-transform) |

### Velodyne scan [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#velodyne-scan "Direct link to Velodyne scan")

Velodyne Lidar scan packets from the Velodyne ROS driver.

| framework | schema |
| --- | --- |
| ROS 1 | [`velodyne_msgs/VelodyneScan`](https://docs.ros.org/en/noetic/api/velodyne_msgs/html/msg/VelodyneScan.html) |
| ROS 2 | [`velodyne_msgs/msg/VelodyneScan`](https://github.com/ros-drivers/velodyne/blob/ros2/velodyne_msgs/msg/VelodyneScan.msg) |

### Video [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#video "Direct link to Video")

Compressed videos displayed in the 3D scene, using the corresponding [Camera field-of-view](https://docs.foxglove.dev/docs/visualization/panels/3d#camera-field-of-view) messages.

| framework | schema |
| --- | --- |
| Custom | [`foxglove.CompressedVideo`](https://docs.foxglove.dev/docs/sdk/schemas/compressed-video) |

### Voxel Grid [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#voxel-grid "Direct link to Voxel Grid")

A 3D volumetric grid used to represent occupancy information in a spatial environment, typically used in robot navigation and obstacle avoidance. Each voxel can be marked (obstacle), present (observed), or unknown, and the grid is used to compute traversability.

![voxelgrid](https://docs.foxglove.dev/assets/images/voxelgrid-9299fef02401dc77c99631c1201cadfa.png)

| framework | schema |
| --- | --- |
| ROS 1 | [costmap\_2d/VoxelGrid](http://docs.ros.org/en/api/costmap_2d/html/msg/VoxelGrid.html) |
| ROS 2 | [nav2\_msgs/msg/VoxelGrid](https://github.com/ros-navigation/navigation2/blob/main/nav2_msgs/msg/VoxelGrid.msg) |
| Custom | [`foxglove.VoxelGrid`](https://docs.foxglove.dev/docs/sdk/schemas/voxel-grid) |

#### ROS Settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#ros-settings "Direct link to ROS Settings")

| **field** | **description** |
| --- | --- |
| **Color mode** | How to color each voxel. Options: <br>• **By type** (based on occupancy classification) <br>• **Gradient** (numeric field mapped to a gradient) <br>• **Rainbow** (predefined colormap) |
| **Gradient** | Only shown if "Color mode" is set to "Gradient". User-defined color mapping for voxel values. |
| **Marked and present** | Only shown if "Color mode" is set to "By type". RGBA color for voxels that are both marked and present. |
| **Present** | Only shown if "Color mode" is set to "By type". RGBA color for voxels that are present but not marked. |
| **Marked** | Only shown if "Color mode" is set to "By type". RGBA color for voxels that are marked but not present. |
| **Use lighting** | Enables lighting for voxel shading in 3D view. |

#### ROS Voxel classification [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#ros-voxel-classification "Direct link to ROS Voxel classification")

When using **By type** coloring:

- **Marked** voxels represent obstacles or areas flagged as non-traversable.
- **Present** voxels represent observed but unmarked areas.
- **Marked and present** voxels are both observed and flagged.

Each voxel is rendered as a cube in 3D space and can be shaded based on lighting settings. The resolution and origin of the grid are defined in the message metadata (`resolutions`, `origin`, and `data` fields).

#### `foxglove.VoxelGrid` settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#foxglovevoxelgrid-settings "Direct link to foxglovevoxelgrid-settings")

| field | description |
| --- | --- |
| **Color mode** | One of: <br>**Flat**: solid color<br>**Color map**: pre-defined color palette<br>**Gradient**: smooth transition between two custom colors<br>**RGBA (separate fields)**: use embedded color from each cell's `red`, `green`, `blue`, and `alpha` fields ( [see below](https://docs.foxglove.dev/docs/visualization/panels/3d#rgba-separate-fields-color-mode)) |
| **Color field** | Only shown if "Color mode" is not set to "Flat"; numeric field in message used for coloring logic |
| **Color map** | Only shown if "Color mode" is set to "Color map"; "Turbo" (Google) or "Rainbow" (RViz); for mapping "Color field" values to colors |
| **Gradient** | Only shown if "Color mode" is set to "Gradient"; The two colors to interpolate between, based on the "color field" |
| **Opacity** | Only shown if "Color mode" is set to "Color map" or "BGR (packed)"; sets alpha value for all cells |
| **Value min** | Only shown if "Color mode" is not set to "Flat"; minimum value used to normalize incoming grid's "Color field" values |
| **Value max** | Only shown if "Color mode" is not set to "Flat"; maximum value used to normalize incoming grid's "Color field" values |
| **Lighting** | Only shown if "Elevation" is set; enables shading from simulated directional lighting, based on elevation surface normals |
| **Frame lock** | "On" means the grid is locked to the frame specified by its `frame_id`, and will move as that frame's transforms change. "Off" means the grid is relative to the fixed frame and will not move after it is first displayed |
| **Draw behind** | "On" means the grid is rendered before the rest of the scene, so it appears behind other scene elements regardless of their relative depth. "Off" means the grid is rendered normally |

##### RGBA (separate fields) color mode [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#rgba-separate-fields-color-mode-1 "Direct link to RGBA (separate fields) color mode")

Each cell can contain color information in four separate fields, named `red`, `green`, `blue`, and `alpha`, of any numeric type:

- **Floating-point values** — 0–1 range
- **Unsigned integer values** — Maximum possible range (e.g. 0–255 for a `UINT8` field)
- **Signed integer values** — `-max` to `max` (e.g. −127 to 127 for an `INT8` field; a value of −128 is treated as identical to −127)

## Settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#settings-2 "Direct link to Settings")

### Frame [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#frame "Direct link to Frame")

Select your 3D scene's references frames and target following mode.

| field | description |
| --- | --- |
| **Fixed frame** | The stationary world reference relative to which all other objects in the scene are located. Elements that accumulate over time (for example, point clouds) are stored and displayed in this frame, and in 2D mode the camera faces this frame's x-y plane. By default the `<Root frame>` option is selected, which uses the TF tree root relative to the selected display frame. Choosing a fixed frame closer to the robot can help reduce floating-point precision issues when transforms span large distances. |
| **Display frame** | The coordinate frame the camera follows. The camera position and orientation are defined relative to the origin of this frame. |
| **Follow mode** | How the camera moves relative to the display frame during playback:<br>- **Pose** follows the display frame's position, roll, pitch, and yaw<br>- **Heading** follows the display frame's position and yaw (heading), while pitch and roll are removed so the horizon stays level (the fixed frame's Z-axis stays pointing up)<br>- **Position** follows the display frame's position only, with orientation aligned to the fixed frame<br>- **Off** does not update viewport |
| **Sync timestamps** | Enables [timestamp synchronization](https://docs.foxglove.dev/docs/visualization/panels/3d#timestamp-synchronization) for selected topics. See the dedicated section below for details. |
| **Global location** | Defines the center of [map layers](https://docs.foxglove.dev/docs/visualization/panels/3d#map-custom-layer) using a [Location fix](https://docs.foxglove.dev/docs/visualization/panels/3d#location-fix) topic with a valid `frame_id` in the TF tree. **Auto** will use the first available topic. (Hidden when there are no location fix topics.) |
| **ENU frame** | Defines the East-North-Up orientation of the scene and the alignment of [map layers](https://docs.foxglove.dev/docs/visualization/panels/3d#map-custom-layer). (Hidden when there are no location fix topics.) |

### Scene [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#scene "Direct link to Scene")

Configure generic rendering properties and viewport properties.

| field | description |
| --- | --- |
| **Show render stats** | Display rendering performance statistics in panel |
| **Background** | Background color of the scene |
| **Label scale** | Scale factor to apply to all rendered labels |
| **Ignore COLLADA `<up_axis>`** | Match the behavior of RViz by ignoring the `<up_axis>` tag in COLLADA files |
| **Mesh "up" axis** | The direction to use as "up" when loading meshes without orientation info (e.g. STL, OBJ) |

### View [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#view "Direct link to View")

Configure the camera settings.

| field | description |
| --- | --- |
| **Sync camera** | Sync the camera with other panels that also have this setting enabled |
| **Distance** | Camera distance from the display frame origin |
| **3D view** | Toggles between 3D and 2D views of the scene. The 2D view looks down the z-axis of the fixed frame and flattens the scene against its x-y plane. |
| **Target** | Translational offset from the origin of the display frame |
| **Theta** | Azimuthal angle offset from the origin of the display frame in degrees |
| **Phi** | Polar angle offset from the origin of the display frame in degrees |
| **Y-axis FOV** | Vertical field of view in degrees |
| **Near** | Near clipping plane distance |
| **Far** | Far clipping plane distance |
| **Log depth** | Enable [logarithmic depth buffer](https://www.gamedeveloper.com/programming/logarithmic-depth-buffer) for more uniform depth precision. May result in rendering artifacts or performance degradation. |

#### Interaction of log depth and near/far plane settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#interaction-of-log-depth-and-nearfar-plane-settings "Direct link to Interaction of log depth and near/far plane settings")

The **Near**, **Far**, and **Log depth** settings are interrelated. When using a very small **Near** value or a large **Far** value, enabling **Log depth** may reduce visual artifacts. For example, in the video below:

- The left panel does not use logarithmic depth, and it has a near clipping plane of 1.0. The grids render smoothly, but objects close to the viewer are clipped.
- The middle panel does not use logarithmic depth, and has a near plane of 0.01. The viewer can get closer to objects without clipping, but rendering artifacts (z-fighting) appear between multiple grids.
- The right panel enables the logarithmic depth setting. This avoids z-fighting by providing more depth precision for the grids, while still allowing a near plane value of 0.01 for reduced clipping.

### Transforms [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#transforms-1 "Direct link to Transforms")

A robotics system produces many messages describing its observations of the world around it. These messages may exist in one or more coordinate frames, or on different parts of the robot.

Transforms define the spacial relationships between two coordinate frames at a given time. To render objects into the scene, there must exist a transform path from the object's coordinate frame to the display frame.

Visualize [transform frames](https://docs.foxglove.dev/docs/visualization/panels/3d#transforms) to debug why elements may not be rendering where expected.

#### History [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#history "Direct link to History")

The panel stores coordinate frame relationships over time in a transform history store. Each coordinate frame stores up to 10,000 transform messages before it begins to remove the oldest messages as new messages arrive.

#### Preloading [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#preloading "Direct link to Preloading")

Transform preloading in the 3D panel ensures an accurate scene by loading transform messages into memory. Transform preloading may impact 3D panel performance during preloading and seeking. You can disable preloading in the 3D panel settings. However, when disabled, the 3D panel may not properly render your scene in certain circumstances where infrequent coordinate frames are involved. To maintain reasonable memory usage, there is an upper limit on the number of transforms that can be preloaded per topic.

#### Globally-located frames [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#globally-located-frames "Direct link to Globally-located frames")

A frame in the scene can be associated with global longitude, latitude, and altitude (LLA) coordinates using the **Global location** field in the [frame settings](https://docs.foxglove.dev/docs/visualization/panels/3d#frame). This allows you to position the frame [relative to a map](https://docs.foxglove.dev/docs/visualization/panels/3d#map-custom-layer).

Globally-located frames are annotated with a globe, both in the settings and in the scene:

#### Troubleshooting [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#troubleshooting "Direct link to Troubleshooting")

When working with live sources where time synchronicity cannot be achieved, it is recommended to have the server publish its time (using the [SDK](https://docs.foxglove.dev/docs/sdk/websocket-server?lang=cpp#broadcasting-time) or [Foxglove Bridge's](https://github.com/foxglove/foxglove-sdk/blob/main/ros/src/foxglove_bridge/README.md#capabilities)`time` capability). Otherwise if the server time is ahead of the visualization app, certain messages in the 3D panel will display immediately but coordinate frame state can lag behind as the app uses its wall time instead of the server time.

#### Settings [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#settings-3 "Direct link to Settings")

| field | description |
| --- | --- |
| **Values relative to** | Controls transform sidebar readouts only. Shows **Translation** and **Rotation** relative to the **Parent**, **Fixed frame**, or **Display frame**. This does not change 3D scene artifacts. |
| **Editable offsets** | Toggles in-app editing of transform frames. Editable **Translation offset** and **Rotation offset** values are always relative to the parent frame, regardless of the **Values relative to** setting. When "on", you can update those values to tweak transform frames, see the impact on the scene, and debug how you may want to adjust the frames on your robot. When turned "off", it reverts to the original translation and rotation values. |
| **Labels** | Toggle the display of transform labels. |
| **Axis scale** | Scale of transform axis (displayed as an arrow marker) |
| **Line width** | Width of transform (displayed as a line marker) |
| **Line color** | Color of transform (displayed as a line marker) |
| **Enable preloading** | Preload transform messages from the data source. |

### Topics [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#topics "Direct link to Topics")

Lists topics containing [supported messages](https://docs.foxglove.dev/docs/visualization/panels/3d#supported-messages) that can be displayed in the scene. Toggle individual topics.

Each topic provides settings to configure how the visualization appears in the scene.

Any topic with messages matching the [supported messages](https://docs.foxglove.dev/docs/visualization/panels/3d#supported-messages) will show up in the topics list. Each supported format has format specific visualization settings. These settings will show up under the topic.

Use the top-level menu to toggle all topics, or click the eye icon next to a topic name to toggle it individually.

### Grid custom layer [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#grid-custom-layer "Direct link to Grid custom layer")

A grid is 2D square with a fixed size and number of divisions. You can create any number of grids. Grids can be rendered relative to the fixed frame, the display frame or any other transform frame.

Use the grid layer settings to change the frame, size, color, divisions, and other properties of the grid.

### URDF custom layer [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#urdf-custom-layer "Direct link to URDF custom layer")

[URDF](https://wiki.ros.org/urdf) robot models are loaded automatically if your data source supports parameters (i.e. a native ROS 1 or ROS 2 connection) and the `/robot_description` parameter is set to valid URDF XML.

You can add URDF models with a custom layer. Use each layer's menu to duplicate or delete the custom layer.

| field | description |
| --- | --- |
| **Source** | Source of the URDF<br>- **URL**– URL to the source URDF file in one of the following schemes:<br>  - `http(s)://` – Over an HTTP(S) connection<br>  - `package://` – Package path. Only supported when using the desktop app or a live connection that supports asset fetching, such as [`foxglove_bridge`](https://docs.foxglove.dev/docs/visualization/connecting/live/ros-foxglove-bridge).<br>- **File path** – Absolute file path to the source URDF file. Only supported when using the desktop app.<br>- **Parameteter** – Parameter containing the source URDF file.<br>- **Topic** – Topic containing the source URDF file. |
| **Control mode** | Control mode of the URDF<br>- **Transforms** – URDF is controlled using transform data<br>- **Joint states** – URDF is controlled by a joint states topic |
| **Label** | (Optional) Label with which the custom layer will appear in the sidebar. |
| **Frame prefix** | (Optional) Prefix for robot's transforms. Also commonly known as a TF prefix. |
| **Display mode** | Robot link geometries to display <br>- **Auto** – Defaults to visual geometries, but falls back to collision geometries if there are no visuals<br>- **Visual** – Visual geometries<br>- **Collision** – Collision geometries |
| **Color** | Fallback color for the URDF model, if the source doesn't contain colors. |
| **Opacity** | Opacity of link geometries on a scale from 0 (fully transparent) to 1 (fully opaque). |
| **Show outlines** | Visualize edges of URDF link geometries. |
| **Show axis** | Visualize coordinate frame axis for each URDF link. |
| **Axis scale** | Scale of link coordinate frame axis. |
| **Joint states** | Select the topic providing joint state data, enabled when **Control mode** is **Joint states**. Each joint states message must list every joint in the URDF — Foxglove does not merge partial updates across messages. |
| **Links** | Toggle visiblity of each URDF link or set a custom offset with respect to the parent link. The latter is disabled when the transform to the parent link is provided by the datasource. |

#### Resolution of URDF assets with `package://` URLs [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#resolution-of-urdf-assets-with-package-urls "Direct link to resolution-of-urdf-assets-with-package-urls")

Foxglove is able fetch meshes and other assests referenced by `package://` in a number of different ways:

1. When using the desktop app, package paths can be resolved by finding packages in directories specified by the [ROS\_PACKAGE\_PATH](https://docs.foxglove.dev/docs/settings#desktop-app) setting
2. When using a live connection that supports asset fetching such as [Foxglove Bridge](https://docs.foxglove.dev/docs/visualization/connecting/live/ros-foxglove-bridge), assets can be fetched over the WebSocket connection
3. When the URDF is given as http(s) or local file path, package paths can be resolved when the URL or file path contains the package name. Consider the following example where the package name `leo_description` is contained in the URDF URL:

   - URDF URL: `https://raw.githubusercontent.com/LeoRover/leo_common-ros2/humble/leo_description/urdf/leo.urdf.xacro`
   - Asset URL: `package://leo_description/models/Rocker.dae`
   - Resolved asset URL: `https://raw.githubusercontent.com/LeoRover/leo_common-ros2/humble/leo_description/models/Rocker.dae`

### Map custom layer [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#map-custom-layer "Direct link to Map custom layer")

You can add a maps with street or satellite imagery to the 3D panel.

Use the 3D panel's [Frame settings](https://docs.foxglove.dev/docs/visualization/panels/3d#frame) to configure how your map layers are positioned and oriented:

- The center of maps are determined by the **Global location** setting
- The orientation of maps are determined by the **ENU frame** setting

Each map is a fixed 500m square positioned relative to the XY-plane of the ENU frame:

| field | description |
| --- | --- |
| **Type** | - **Street** (© OpenStreetMap contributors)<br>- **Satellite** (© [Esri](https://www.esri.com/) — Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community)<br>- **Shaded relief** – GEBCO grayscale bathymetry basemap (© GEBCO / NCEI)<br>- **Sea marks** – OpenSeaMap sea mark chart overlay (© OpenSeaMap contributors). Stack over a base map layer such as Street, Satellite, or Shaded relief.<br>- **Isobaths** – OpenSeaMap transparent depth contour overlay (© OpenSeaMap contributors). Stack over a base map layer such as Street, Satellite, or Shaded relief.<br>- **Custom** – Load from a custom URL (Pro and Enterprise plans only). Provide a URL to your custom map tiles following the [Tile Map Service specs](https://wiki.osgeo.org/wiki/Tile_Map_Service_Specification), e.g. `https://my.custom.url/{x}/{y}/{z}`. |
| **Opacity** | Opacity of the map |
| **Z-position** | The z-position of the map layer in meters relative to the ENU frame's XY-plane. When creating layouts through the layout API, set a non-zero z-position for Sea marks and Isobaths overlays so they render above the base map. |
| **Draw behind** | **On** means the map is rendered before the rest of the scene, so it appears behind other scene elements regardless of their relative depth. **Off** means the map is rendered normally |

### Publish [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#publish "Direct link to Publish")

Configure click-to-publish behavior for the 3D panel.

The click-to-publish feature requires a data source that supports publishing, such as the [Foxglove Bridge](https://docs.foxglove.dev/docs/visualization/connecting/live/ros-foxglove-bridge) or a [custom WebSocket server](https://docs.foxglove.dev/docs/getting-started/custom#live-data) with the `clientPublish` capability enabled.

note

The click-to-publish feature operates differently for ROS and non-ROS data sources.

The message types used for ROS and non-ROS data sources are:

| Click-to-publish tool | Corresponding ROS message type | Foxglove message (non-ROS) |
| --- | --- | --- |
| **2D pose estimate** | [geometry\_msgs/PoseWithCovarianceStamped](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PoseWithCovarianceStamped.html) | _not supported_ |
| **2D pose** | [geometry\_msgs/PoseStamped](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PoseStamped.html) | [foxglove.PoseInFrame](https://docs.foxglove.dev/docs/sdk/schemas/pose-in-frame) |
| **2D point** | [geometry\_msgs/PointStamped](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/PointStamped.html) | _not supported_ |

| Field | Description |
| --- | --- |
| **Topic** | Topic on which to publish |
| **X deviation**<br>**Y deviation**<br>**Theta deviation** | Only available for the **2D pose estimate** tool. Standard deviation values used to create the published covariance matrix. |

## Controls and shortcuts [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#controls-and-shortcuts "Direct link to Controls and shortcuts")

Click any object in the scene to display its relevant details in the selected object popup.

Click the icons next to the topic to open its settings in the sidebar or to display its messages in a new [Raw Messages panel](https://docs.foxglove.dev/docs/visualization/panels/raw-messages).

Selecting an object will set a `$selected_id` variable to the clicked object's `id` value, if it has one. For [ROS markers](https://docs.foxglove.dev/docs/visualization/panels/3d#ros-markers) and [scene entities](https://docs.foxglove.dev/docs/visualization/panels/3d#scene-entity), you can also set another variable on click.

The panel controls on the right can be used to do the following:

- **Select** — toggle object select mode
- **3D** — Toggle between 3D and 2D views of the scene
- **Measure** — measure the distance between two points
- **Publish** — publish a message based on a clicked location in the 3D view

By default, measurement endpoints are placed at the free-space cursor position on the ground plane.

Hold `Alt` (Windows and Linux) or `Option` (macOS) while placing an endpoint to snap it to measurable objects in the scene — like cube faces, point cloud points, and line segments — when the cursor is near one. This is useful for measuring distances between known features in the scene.

note

The click-to-publish feature requires a data source that supports publishing, such as the [Foxglove Bridge](https://docs.foxglove.dev/docs/visualization/connecting/live/ros-foxglove-bridge) or a [custom WebSocket server](https://docs.foxglove.dev/docs/getting-started/custom#live-data) with the `clientPublish` capability enabled.

Activate the currently-selected publish tool by clicking on the icon. Right-click or click and hold to switch between tools.

|     |     |
| --- | --- |
| **Keyboard and mouse shortcuts** |
| `w` | Move camera forward |
| `a` | Move camera to the left |
| `s` | Move camera backward |
| `d` | Move camera to the right |
| `Shift`+`w` | Rotate camera up |
| `Shift`+`a` | Rotate camera left |
| `Shift`+`s` | Rotate camera down |
| `Shift`+`d` | Rotate camera right |
| Scroll | Zoom in and out |
| Drag | Move camera parallel to the ground (x-y plane) |
| `Alt` \+ drag (Windows and Linux) <br>`Option` \+ drag (macOS) | Move camera parallel to the screen (enable z-axis) |
| `Shift` \+ drag, or right-click-drag | Rotate the camera around the target position |
| `1` | Re-center the camera on the chosen display frame |
| `3` | Toggle between 2D bird's-eye view and 3D perspective view |
| `i` | Show or hide the object inspector |
| Hold `Alt` (Windows and Linux) <br>Hold `Option` (macOS) | Temporarily enable measurement snapping while measuring |

## Performance troubleshooting [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#performance-troubleshooting "Direct link to Performance troubleshooting")

Here are some tips for dealing with performance issues you may encounter.

### Hardware acceleration [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#hardware-acceleration "Direct link to Hardware acceleration")

If you are experiencing poor performance while interacting with the 3D panel, it may be because hardware acceleration is not enabled. Hardware acceleration will significantly improve performance by rendering on the GPU rather than the CPU.

If you're using Google Chrome, you can check that hardware graphics acceleration is enabled by going to `chrome://gpu` and seeing that "WebGL" and/or "WebGL2" are "Hardware accelerated". If not, go to `chrome://settings`, search your settings for `acceleration` and ensure that the "Use graphics acceleration when available" toggle is enabled. If after these steps you still see that WebGL is still not "Hardware accelerated" in `chrome://gpu` then you might have to investigate platform-specific steps for enabling your GPU.

The desktop app uses Electron (based on Chromium), and should automatically use hardware acceleration when available. If you're seeing issues here, we recommend following the steps above to see if the same issue exists in Chrome. If so, there's a good chance it's affecting our application for the same reasons. Therefore we recommend getting things working in Chrome first, and if the desktop app is still having issues afterward please [let us know](https://foxglove.dev/chat) and we'll look into it.

Some reasons hardware acceleration might be disabled:

- Your graphics drivers are out of date or not installed such that Chrome can make use of them.
- Your system configuration could be unsupported for hardware acceleration in Chrome. A way to solve this is to enable "Override software rendering list" in `about://flags`, which can allow Chrome to use hardware acceleration on unsupported systems. This can result in an unstable Chrome experience and is not advised unless necessary.

### Video acceleration [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#video-acceleration "Direct link to Video acceleration")

If you are experiencing poor performance with the 3D panel while using `foxglove.CompressedVideo` topics, check that video acceleration is enabled. [Docs here](https://docs.foxglove.dev/docs/visualization/panels/image#checking-video-acceleration).

### Triangle primitives [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#triangle-primitives "Direct link to Triangle primitives")

If you have large `foxglove.TriangleListPrimitive` entities in your scene updates that are added and removed often causing noticable lag, consider toggling the ["Vertex normals" setting](https://docs.foxglove.dev/docs/visualization/panels/3d#scene-entity).

### SceneEntity structure [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#sceneentity-structure "Direct link to SceneEntity structure")

For maximum efficiency, prefer fewer SceneEntities each with many primitives, rather than many SceneEntities each with fewer primitives.

### Floating point precision [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#floating-point-precision "Direct link to Floating point precision")

To reduce displayed precision issues the 3D panel converts the transform tree coordinates to camera relative equivalents before sending them to the GPU. This allows transforms to have large displacements between them; these displacements can exceed the seven significant digits that 32-bit floating point numbers usually provide. However, this does not prevent precision issues when a scene entity is large itself and has vertices far away from the transform's origin. It is recommended that if a scene entity will be far from its transform origin, then a new intermediate transform should be created near the scene entity, to keep the large displacements inside the transform tree and prevent perceived distortion.

## Timestamp synchronization [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#timestamp-synchronization "Direct link to Timestamp synchronization")

The **Sync timestamps** setting gates rendering on exact timestamp alignment across selected topics. When enabled, the panel buffers incoming messages and only renders a frame when every participating topic has a message with the same timestamp.

### How it works [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#how-it-works "Direct link to How it works")

1. Enable **Sync timestamps** in the General settings section.
2. For each topic you want to synchronize, enable the **Sync** toggle in that topic's settings. At least **two** topics must have Sync enabled — otherwise there is nothing to match against.
3. The panel collects messages from all Sync-enabled topics and looks for a complete set where every topic has a message at the same timestamp.
4. When a match is found, those messages are rendered together. Topics without Sync enabled continue rendering independently.

The panel retains up to **250 unique timestamps** in its synchronization buffer. If a match is not found within that window, the oldest timestamps are discarded. For best results, ensure your topics publish at the same rate and with aligned timestamps.

### Timestamp extraction [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#timestamp-extraction "Direct link to Timestamp extraction")

Timestamps are extracted from `header.stamp` for ROS messages or the `timestamp` field for Foxglove messages — not [log time](https://docs.foxglove.dev/docs/visualization/playback#message-ordering). For scene updates and marker arrays, the timestamp of the first entity or marker in the message is used.

### What is never synchronized [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#what-is-never-synchronized "Direct link to What is never synchronized")

Transforms and URDFs are never registered for synchronization. Transforms continue to build the transform tree normally, and all entities compute their position at the synced timestamp.

## Links and resources [​](https://docs.foxglove.dev/docs/visualization/panels/3d\#links-and-resources "Direct link to Links and resources")

- [Visualize point clouds with custom colors](https://foxglove.dev/blog/visualizing-point-clouds-with-custom-colors)
- [Visualize ROS mesh markers](https://foxglove.dev/blog/how-to-visualize-ros-mesh-markers)
- [Write a message converter extension (3D panel)](https://foxglove.dev/blog/using-message-converters-to-display-3d-markers-in-foxglove-studio)

- [Visualization](https://docs.foxglove.dev/docs/visualization/panels/3d#visualization)
- [Supported messages](https://docs.foxglove.dev/docs/visualization/panels/3d#supported-messages)
  - [Camera field-of-view](https://docs.foxglove.dev/docs/visualization/panels/3d#camera-field-of-view)
  - [Grid](https://docs.foxglove.dev/docs/visualization/panels/3d#grid)
  - [Image](https://docs.foxglove.dev/docs/visualization/panels/3d#image)
  - [Laser scan](https://docs.foxglove.dev/docs/visualization/panels/3d#laser-scan)
  - [Location fix](https://docs.foxglove.dev/docs/visualization/panels/3d#location-fix)
  - [ROS polygons](https://docs.foxglove.dev/docs/visualization/panels/3d#ros-polygons)
  - [ROS markers](https://docs.foxglove.dev/docs/visualization/panels/3d#ros-markers)
  - [Path](https://docs.foxglove.dev/docs/visualization/panels/3d#path)
  - [Point cloud](https://docs.foxglove.dev/docs/visualization/panels/3d#point-cloud)
  - [Pose](https://docs.foxglove.dev/docs/visualization/panels/3d#pose)
  - [Scene entity](https://docs.foxglove.dev/docs/visualization/panels/3d#scene-entity)
  - [Transforms](https://docs.foxglove.dev/docs/visualization/panels/3d#transforms)
  - [Velodyne scan](https://docs.foxglove.dev/docs/visualization/panels/3d#velodyne-scan)
  - [Video](https://docs.foxglove.dev/docs/visualization/panels/3d#video)
  - [Voxel Grid](https://docs.foxglove.dev/docs/visualization/panels/3d#voxel-grid)
- [Settings](https://docs.foxglove.dev/docs/visualization/panels/3d#settings-2)
  - [Frame](https://docs.foxglove.dev/docs/visualization/panels/3d#frame)
  - [Scene](https://docs.foxglove.dev/docs/visualization/panels/3d#scene)
  - [View](https://docs.foxglove.dev/docs/visualization/panels/3d#view)
  - [Transforms](https://docs.foxglove.dev/docs/visualization/panels/3d#transforms-1)
  - [Topics](https://docs.foxglove.dev/docs/visualization/panels/3d#topics)
  - [Grid custom layer](https://docs.foxglove.dev/docs/visualization/panels/3d#grid-custom-layer)
  - [URDF custom layer](https://docs.foxglove.dev/docs/visualization/panels/3d#urdf-custom-layer)
  - [Map custom layer](https://docs.foxglove.dev/docs/visualization/panels/3d#map-custom-layer)
  - [Publish](https://docs.foxglove.dev/docs/visualization/panels/3d#publish)
- [Controls and shortcuts](https://docs.foxglove.dev/docs/visualization/panels/3d#controls-and-shortcuts)
- [Performance troubleshooting](https://docs.foxglove.dev/docs/visualization/panels/3d#performance-troubleshooting)
  - [Hardware acceleration](https://docs.foxglove.dev/docs/visualization/panels/3d#hardware-acceleration)
  - [Video acceleration](https://docs.foxglove.dev/docs/visualization/panels/3d#video-acceleration)
  - [Triangle primitives](https://docs.foxglove.dev/docs/visualization/panels/3d#triangle-primitives)
  - [SceneEntity structure](https://docs.foxglove.dev/docs/visualization/panels/3d#sceneentity-structure)
  - [Floating point precision](https://docs.foxglove.dev/docs/visualization/panels/3d#floating-point-precision)
- [Timestamp synchronization](https://docs.foxglove.dev/docs/visualization/panels/3d#timestamp-synchronization)
  - [How it works](https://docs.foxglove.dev/docs/visualization/panels/3d#how-it-works)
  - [Timestamp extraction](https://docs.foxglove.dev/docs/visualization/panels/3d#timestamp-extraction)
  - [What is never synchronized](https://docs.foxglove.dev/docs/visualization/panels/3d#what-is-never-synchronized)
- [Links and resources](https://docs.foxglove.dev/docs/visualization/panels/3d#links-and-resources)