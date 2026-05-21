[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#__docusaurus_skipToContent_fallback)

On this page

Display the status of seen nodes (i.e. stale, error, warn, or OK) from ROS [`DiagnosticArray`](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#diagnosticarray) topic messages in a feed. Drill down into data for a given diagnostic name and hardware ID (e.g.`diagnostic_name/hardware_id`).

Open a Diagnostics – Summary (ROS) panel to see the status of seen nodes (i.e. stale, error, warn, or OK) in a live feed.

To pin a node to the top of your feed, hover on an entry and click the pin icon that appears.

Use the dropdown menu to select the minimum level by which you want to filter your diagnostics.

Use the search field to fuzzy filter entries by `hardware_id` and node name. Results will be ordered by how early the search text appears in the matched label.

Click any entry to open a corresponding Diagnostics – Detail (ROS) panel with the clicked node’s relevant details. Alternatively, you can manually open a new Diagnostics – Detail (ROS) panel and input a diagnostic to inspect. The displayed keys and values support rich formatting with basic HTML tags – e.g. `<b>`, `<u>`, `<table>`, etc.

## Supported messages [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#supported-messages "Direct link to Supported messages")

To use this panel, your data source must provide messages conforming to one of the following supported schemas.

### `DiagnosticArray` [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#diagnosticarray "Direct link to diagnosticarray")

| framework | schema |
| --- | --- |
| ROS 1 | [`diagnostic_msgs/DiagnosticArray`](https://docs.ros.org/en/noetic/api/diagnostic_msgs/html/msg/DiagnosticArray.html) |
| ROS 2 | [`diagnostic_msgs/msg/DiagnosticArray`](https://github.com/ros2/common_interfaces/blob/master/diagnostic_msgs/msg/DiagnosticArray.msg) |

## Settings [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#settings "Direct link to Settings")

### Diagnostics – Summary (ROS) [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#diagnostics-summary-ros "Direct link to Diagnostics – Summary (ROS)")

#### General [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#general "Direct link to General")

| field | description |
| --- | --- |
| **Topic** | ROS [`DiagnosticArray`](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#diagnosticarray) topic to subscribe to |
| **Sort by level** | Sort visible nodes by level |
| **Stale timeout** | Number of seconds after which entries will be marked as stale if no new messages are received |

### Diagnostics – Detail (ROS) [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#diagnostics-detail-ros "Direct link to Diagnostics – Detail (ROS)")

#### General [​](https://docs.foxglove.dev/docs/visualization/panels/diagnostics\#general-1 "Direct link to General")

| field | description |
| --- | --- |
| **Topic** | ROS [`DiagnosticArray`](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#diagnosticarray) topic to subscribe to |
| **Numeric precision** | Number of decimal places to display for numeric values |
| **Stale timeout** | Number of seconds after which entries will be marked as stale if no new messages are received |

- [Supported messages](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#supported-messages)
  - [`DiagnosticArray`](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#diagnosticarray)
- [Settings](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#settings)
  - [Diagnostics – Summary (ROS)](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#diagnostics-summary-ros)
  - [Diagnostics – Detail (ROS)](https://docs.foxglove.dev/docs/visualization/panels/diagnostics#diagnostics-detail-ros)