[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels/plot#__docusaurus_skipToContent_fallback)

On this page

Plot values from topic [message paths](https://docs.foxglove.dev/docs/visualization/message-path-syntax) which point to numeric fields.

## Settings [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#settings "Direct link to Settings")

### General [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#general "Direct link to General")

| field | description |
| --- | --- |
| **X-axis value type** | Type and source of data for values plotted on the x-axis:<br>- **Timestamp:** x-values for time series data<br>- **Message path:** numeric message fields for XY plots<br>- **Array index:** integer indices of an array in the latest message<br>These options are described in more detail below. |
| **Sync with other plots** | For **Timestamp** plots:<br>Sync timeline to other Plot and State Transitions panels. |
| **Time range** | For **Message path** plots:<br>- **All** shows data from the full time range<br>- **Latest** shows data from the most recent message. |
| **Axis scales** | For **Message path** plots:<br>- **Independent** (the default) lets you configure min and max values for the scales independently.<br>- **1:1** is useful for when axes have the same units, and will keep the scales locked together. It hides the min/max axis settings and always zooms both axes simultaneously. |

#### Time series plots [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#time-series-plots "Direct link to Time series plots")

The default **Timestamp** option for the x-axis value type plots y-values against messages timestamps. The zoom state of time series Plot and State Transitions panels in a given layout can be kept in sync for easy comparison.

Current playback time is indicated by a vertical gray bar.

#### Path-based XY plots [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#path-based-xy-plots "Direct link to Path-based XY plots")

The **Message path** mode produces XY scatter and trajectory plots where both y-values and x-values are extracted from message paths. You can specify x-values either per-series, or set a default for all series. The path can point to a single value or an array of values in the message.

The "Data range" setting controls which messages are plotted.

#### Index-based plots [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#index-based-plots "Direct link to Index-based plots")

The **Array index** mode plots instantaneous data from an array in the latest playback message using the array's index as the x-axis value. For example, if `/some_topic.some_array` contained the values `[5, 10, 15]` in a particular message, the resulting points on the chart would be `[0, 5]`, `[1, 10]`, and `[2, 15]` at that point in time.

### Legend [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#legend "Direct link to Legend")

| field | description |
| --- | --- |
| **Position** | Position of the legend in relation to the chart (Floating, Left, Top) |
| **Show legend** | Display the legend |
| **Show values** | Show the corresponding y value next to each series in the legend (either at the current playback time or at a point on user hover) |

### X-axis [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#x-axis "Direct link to X-axis")

| field | description |
| --- | --- |
| **Default values path** | For path-based values: the message path containing default x-axis values for the series. You can also set these individually for each series. |
| **Axis label** | Label displayed for the x-axis |
| **Show tick labels** | Whether or not to display numeric values for x-axis tick marks |
| **Timestamp format** | For **Timestamp** plots:<br>- **Relative** (default) shows seconds elapsed since the start of the data source<br>- **Absolute** shows timestamps formatted according to the app's time format and timezone settings |
| **Time window** | For **Timestamp** plots:<br>- **Automatic** Last 30 seconds for live data, or the full time range for recorded data<br>- **Sliding** Specify a sliding time window that follows the current playback time<br>- **Fixed** Set fixed min and max values for the time axis |
| **Window size** | For **Sliding** time window: range of time in seconds |
| **Playhead position** | For **Sliding** time window: either at the center or the right edge |
| **Min** | For **Fixed** time window: min value in seconds |
| **Max** | For **Fixed** time window: max value in seconds |

### Y-axis [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#y-axis "Direct link to Y-axis")

| field | description |
| --- | --- |
| **Axis label** | Label displayed for the y-axis |
| **Show tick labels** | Whether or not to display numeric values for y-axis tick marks |
| **Min** | Fixed minimum value for y-axis |
| **Max** | Fixed maximum value for y-axis |

### Series [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#series "Direct link to Series")

| field | description |
| --- | --- |
| **Label** | Label displayed for the series in the legend |
| **X-value path** | For **Message path** plots: message path containing x-axis values for the series. Overrides default x-axis values if set. |
| **Y-value path** | Message path containing y-axis values for the series |
| **Color** | Color used to plot the series |
| **Line size** | Width of line connecting data points |
| **Show line** | Show line connecting data points. (Not applicable to [reference lines](https://docs.foxglove.dev/docs/visualization/panels/plot#reference-lines).) |
| **Timestamp** | For **Timestamp** plots, set which source of time information is used for message ordering:<br>- **Log time** is the standard timestamp used for [playback message ordering](https://docs.foxglove.dev/docs/visualization/playback#message-ordering) across the app. For live connections, this is when the message was received; for recorded files, this is when the message was originally recorded<br>- **Custom field** is a message path which can point to any field within the message data containing `sec` and `nsec` integers<br>- **Header stamp** is a `header.stamp` [ROS 1](http://docs.ros.org/en/noetic/api/std_msgs/html/msg/Header.html) or [ROS 2](https://docs.ros2.org/latest/api/std_msgs/msg/Header.html) field containing `sec` and `nsec` integers (and special case of 'custom field', above). This typically represents when sensor data was captured<br>- **Publish time** is an optional [MCAP-specific field](https://mcap.dev/spec#message-op0x05) representing when the event occurred |

Use the panel settings to add, edit, remove, or reorder data series:

Drag a series to a new position or use its action menu to move it up or down. Reordering changes the series order in the settings and plot legend while preserving the color associated with each series.

Enter the field or slice of data you would like plotted on the y-axis using [message path syntax](https://docs.foxglove.dev/docs/visualization/message-path-syntax) as a **Series**.

If you enter a message path that points to multiple values (for example, `/some_topic/some_array[:].x`), the plot will display multiple values each x-axis tick.

## Toolbar buttons [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#toolbar-buttons "Direct link to Toolbar buttons")

### Toggle legend [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#toggle-legend "Direct link to Toggle legend")

Hide and show the plot legend:

### Point inspection [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#point-inspection "Direct link to Point inspection")

You can highlight two points on the chart and measure the distance between them using the **Inspection tool** in the panel toolbar. The point values and the distances between them along each axis are shown in place of the legend:

Press `Esc` to deselect any selected points. Press it again to turn off the inspection tool.

tip

For path-based XY plots, the inspection tool will also display the straight-line (hypotenuse) distance between selected points.

### Zoom tool [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#zoom-tool "Direct link to Zoom tool")

The zoom tool allows you to precisely specify a region of the plot to zoom in on. Press the button in the toolbar or press `z` on your keyboard to enable it.

You can use `Alt`/`Option` and `Shift` to specify the x- and y-axis, respectively:

### Reset view [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#reset-view "Direct link to Reset view")

Reset the plot to the original viewport, as defined by your panel's range settings:

## Controls and shortcuts [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#controls-and-shortcuts "Direct link to Controls and shortcuts")

### Pan and zoom [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#pan-and-zoom "Direct link to Pan and zoom")

- To pan, click and drag using the mouse or trackpad.
- To zoom, scroll using the mouse wheel or trackpad, or use the **Zoom tool** as described above.
- Click the **Reset view** button, press `r` (when the panel is selected), or double-click the panel to reset to the original viewport.

By default, path-based XY plots will zoom both axes simultaneously, while time series and index-based plots will zoom just the x-axis:

Path-based XY plots with 1:1 axis scales enabled will always zoom both axes simultaneously.

Zoom each axis independently by **hovering over the target axis** while scrolling with the mouse wheel or trackpad.

You can also use keyboard modifiers to control zoom behavior while scrolling or using the **Zoom tool**:

- Y-axis only, hold `Shift`
- X-axis only, hold:
  - `Alt` (Windows and Linux)
  - `Option` (macOS)
- Both axes, hold:
  - `Shift` \+ `Alt` (Windows and Linux)
  - `Shift` \+ `Option` (macOS)

### Click-to-seek [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#click-to-seek "Direct link to Click-to-seek")

Hover over any point on the plot to see its details in a tooltip. You'll see a vertical yellow bar appear, as well as a corresponding yellow marker at the same on the playback timeline. Click to seek playback to the yellow marker on the timeline.

note

Only available for recorded data.

### Download CSV data [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#download-csv-data "Direct link to Download CSV data")

Right-click the plot to download the plotted data as a `.csv` file.

## Message path functions [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#message-path-functions "Direct link to Message path functions")

All message path functions are supported in the Plot panel, including time-series functions when using **timestamp** as the x-axis. Append functions to message paths using `.@name` syntax. See the [message path function reference](https://docs.foxglove.dev/docs/visualization/message-path-syntax#message-path-functions) for the full list and descriptions.

Scalar(`.@abs`, `.@acos`, `.@asin`, and more)

| Function | Description |
| --- | --- |
| `.@abs` | Absolute value |
| `.@acos` | Arc cosine (radians) |
| `.@asin` | Arc sine (radians) |
| `.@atan` | Arc tangent (radians) |
| `.@ceil` | Round up to nearest integer |
| `.@cos` | Cosine (input in radians) |
| `.@degrees` | Convert radians to degrees |
| `.@log` | Natural logarithm |
| `.@log1p` | Natural log of 1 + value |
| `.@log2` | Base-2 logarithm |
| `.@log10` | Base-10 logarithm |
| `.@negative` | Negate the value |
| `.@radians` | Convert degrees to radians |
| `.@round` | Round to nearest integer |
| `.@sign` | Sign of value (-1, 0, or 1) |
| `.@sin` | Sine (input in radians) |
| `.@sqrt` | Square root |
| `.@tan` | Tangent (input in radians) |
| `.@trunc` | Truncate to integer |

Operand(`.@add(number)`, `.@sub(number)`, `.@mul(number)`, and more)

| Function | Description |
| --- | --- |
| `.@add(number)` | Add a number |
| `.@sub(number)` | Subtract a number |
| `.@mul(number)` | Multiply by a number |
| `.@div(number)` | Divide by a number |

Vector(`.@norm`)

| Function | Description |
| --- | --- |
| `.@norm` | Euclidean norm (magnitude) of a vector object or numeric array |

Struct(`.@rpy`, `.@quat`)

| Function | Description |
| --- | --- |
| `.@rpy` | Convert quaternion to roll/pitch/yaw (radians) |
| `.@quat` | Convert roll/pitch/yaw to quaternion |

Time-series(`.@delta`, `.@derivative`, `.@timedelta`)

| Function | Description |
| --- | --- |
| `.@delta` | Change between consecutive samples |
| `.@derivative` | Rate of change between consecutive samples |
| `.@timedelta` | Elapsed time in seconds between consecutive samples |

Time-series functions (`.@delta`, `.@derivative`, `.@timedelta`) are available only when using **timestamp** as the x-axis (Log time, custom timestamp, etc.). They don't work with index-based or path-based x-axis plots.

For time-series modifiers:

- `.@delta` plots the change in y-value between consecutive samples (`y[n] - y[n-1]`).
- `.@derivative`plots the rate of change in y-value (\`(y\[n\] - y\[n-1\]) / (t\[n\]
  - t\[n-1\])\`).
- `.@timedelta` plots elapsed time in seconds between consecutive samples
(`t[n] - t[n-1]`).

For `.@derivative` and `.@timedelta`, `t` comes from the series' **Timestamp** setting (for example, Log time, ROS header stamp, Publish time, or Custom field timestamp path).

When a message path includes filters, time-series modifiers use consecutive samples that match the filter. For example, `/imu{sensor_id==3}.@timedelta` uses the elapsed time between successive `/imu` messages where `sensor_id` is `3` (see [`@timedelta`](https://docs.foxglove.dev/docs/visualization/message-path-syntax#message-path-function-timedelta)).

Time-series functions can be followed by scalar functions, but only one time-series function can appear in a chain:

- Valid: `/foo.value.@derivative.@abs`
- Valid: `/foo.value.@delta.@negative`
- Invalid: `/foo.value.@derivative.@derivative`
- Invalid: `/foo.value.@derivative.@norm`

See [JavaScript Math](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math) for details on the corresponding built-in math functions.

The operand functions `.@add(number)`, `.@sub(number)`, `.@div(number)`, and `.@mul(number)` accept a single numeric operand. These numeric values can also be passed in via [variable reference](https://docs.foxglove.dev/docs/visualization/variables).

`.@norm` computes the Euclidean norm (magnitude) of a 2D or 3D vector object
with `x`, `y` (and optionally `z`) numeric properties, or of any numeric array:

```json
/imu.linear_acceleration.@norm
/gps.velocity.@norm
/joint_states.velocity.@norm
```

`.@rpy` and `.@quat` are struct conversion functions using
XYZ Euler order (intrinsic rotations):

- `.@rpy` converts quaternion fields (`x`, `y`, `z`, `w`) to `roll`, `pitch`,
and `yaw` values in radians (rotations around X, Y, and Z axes respectively).
- `.@quat` converts `roll`, `pitch`, and `yaw` fields (radians) to quaternion
`x`, `y`, `z`, and `w`, as the inverse of `.@rpy`.

In Plot, use field access after `.@rpy` or `.@quat` to select a numeric value:

```json
/odom.orientation.@rpy.roll
/pose.euler.@quat.w
```

## Reference lines [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#reference-lines "Direct link to Reference lines")

To add a reference line at a constant value in a plot panel, add a series and set the message path to the constant value you would like plotted.

## Downsampling [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#downsampling "Direct link to Downsampling")

To improve performance, the Plot panel will downsample data when points are too dense to meaningfully display.

### Times series plots [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#times-series-plots "Direct link to Times series plots")

In timestamp mode, the Plot panel will downsample data using a min/max/first/last downsampling algorithm. This downsampling will preserve extremes so even when downsampled, the plot will still show the highest and lowest values for each time period. The points kept during downsampling will always be from the original data — downsampling does not interpolate or create new points.

When data is downsampled, the Plot panel will automatically hide the dots for your data and only show lines. You can zoom until you see dots indicating you are seeing the full original data.

### XY plots [​](https://docs.foxglove.dev/docs/visualization/panels/plot\#xy-plots "Direct link to XY plots")

In path-based modes, the Plot panel will use different downsampling approaches depending on the "Show lines" setting for the series.

If the series does not have lines enabled, downsampling will remove points which occur at the same pixel location as other points. The rendered plot will look the same as one with no downsampling.

If the series has lines enabled, downsampling will preserve the shape of the data and only remove points which would not alter the shape. When data is downsampled, the Plot panel will automatically hide the dots for your data and only show lines. You can zoom until you see dots indicating you are seeing the full original data.

- [Settings](https://docs.foxglove.dev/docs/visualization/panels/plot#settings)
  - [General](https://docs.foxglove.dev/docs/visualization/panels/plot#general)
  - [Legend](https://docs.foxglove.dev/docs/visualization/panels/plot#legend)
  - [X-axis](https://docs.foxglove.dev/docs/visualization/panels/plot#x-axis)
  - [Y-axis](https://docs.foxglove.dev/docs/visualization/panels/plot#y-axis)
  - [Series](https://docs.foxglove.dev/docs/visualization/panels/plot#series)
- [Toolbar buttons](https://docs.foxglove.dev/docs/visualization/panels/plot#toolbar-buttons)
  - [Toggle legend](https://docs.foxglove.dev/docs/visualization/panels/plot#toggle-legend)
  - [Point inspection](https://docs.foxglove.dev/docs/visualization/panels/plot#point-inspection)
  - [Zoom tool](https://docs.foxglove.dev/docs/visualization/panels/plot#zoom-tool)
  - [Reset view](https://docs.foxglove.dev/docs/visualization/panels/plot#reset-view)
- [Controls and shortcuts](https://docs.foxglove.dev/docs/visualization/panels/plot#controls-and-shortcuts)
  - [Pan and zoom](https://docs.foxglove.dev/docs/visualization/panels/plot#pan-and-zoom)
  - [Click-to-seek](https://docs.foxglove.dev/docs/visualization/panels/plot#click-to-seek)
  - [Download CSV data](https://docs.foxglove.dev/docs/visualization/panels/plot#download-csv-data)
- [Message path functions](https://docs.foxglove.dev/docs/visualization/panels/plot#message-path-functions)
- [Reference lines](https://docs.foxglove.dev/docs/visualization/panels/plot#reference-lines)
- [Downsampling](https://docs.foxglove.dev/docs/visualization/panels/plot#downsampling)
  - [Times series plots](https://docs.foxglove.dev/docs/visualization/panels/plot#times-series-plots)
  - [XY plots](https://docs.foxglove.dev/docs/visualization/panels/plot#xy-plots)