[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels/teleop#__docusaurus_skipToContent_fallback)

On this page

Teleoperate your robot by publishing [`geometry_msgs/Twist`](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/Twist.html) or [`geometry_msgs/msg/Twist`](https://github.com/ros2/common_interfaces/blob/master/geometry_msgs/msg/Twist.msg) messages on a given topic.

The directional pad includes up, down, left, and right buttons for movement, as well as a center stop button for sending a stop command. Each button's field and value are configurable in the panel settings.

To use the Teleop panel, connect to a data source that supports publishing. For more information, check out the [supported data sources](https://docs.foxglove.dev/docs/visualization/connecting#data-sources).

## Settings [​](https://docs.foxglove.dev/docs/visualization/panels/teleop\#settings "Direct link to Settings")

| General |  |
| --- | --- |
| **Publish rate** | Rate (Hz) at which to publish messages (default: 1) |
| **Topic** | Topic on which to publish messages |
| **Stop on release** | When enabled, automatically sends the stop button's configured message when any directional button is released |
| **Up button** | Field (linear or angular x, y, or z) and value to publish when pressing the up button |
| **Down button** | Field (linear or angular x, y, or z) and value to publish when pressing the down button |
| **Left button** | Field (linear or angular x, y, or z) and value to publish when pressing the left button |
| **Right button** | Field (linear or angular x, y, or z) and value to publish when pressing the right button |
| **Stop button** | Field (linear or angular x, y, or z) and value to publish when pressing the center stop button |

## Supported messages [​](https://docs.foxglove.dev/docs/visualization/panels/teleop\#supported-messages "Direct link to Supported messages")

This panel publishes messages conforming to one of the following schemas.

### `Twist` [​](https://docs.foxglove.dev/docs/visualization/panels/teleop\#twist "Direct link to twist")

| framework | schema |
| --- | --- |
| ROS 1 | [`geometry_msgs/Twist`](https://docs.ros.org/en/noetic/api/geometry_msgs/html/msg/Twist.html) |
| ROS 2 | [`geometry_msgs/msg/Twist`](https://github.com/ros2/common_interfaces/blob/master/geometry_msgs/msg/Twist.msg) |

- [Settings](https://docs.foxglove.dev/docs/visualization/panels/teleop#settings)
- [Supported messages](https://docs.foxglove.dev/docs/visualization/panels/teleop#supported-messages)
  - [`Twist`](https://docs.foxglove.dev/docs/visualization/panels/teleop#twist)