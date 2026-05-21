[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels/topic-graph#__docusaurus_skipToContent_fallback)

On this page

Display a graph visualization of the current node and topic topology.

To use this panel, you must be connected to a live data source. For ROS, connect via a native, Foxglove Bridge, or Rosbridge connection; use the [Foxglove Bridge](https://docs.foxglove.dev/docs/visualization/connecting/live/ros-foxglove-bridge) for best results. For a custom data source using the Foxglove SDK or a Foxglove WebSocket connection, publish a Connection Graph update.

The graph will show nodes, topics, and services. Nodes implement services, and either subscribe or publish to topics. The direction of the graph's arrows will denote whether a node is subscribing to or publishing a topic.

- **Nodes** – blue rectangles
- **Topics** – purple diamonds
- **Services** – red rectangles

## Controls and shortcuts [​](https://docs.foxglove.dev/docs/visualization/panels/topic-graph\#controls-and-shortcuts "Direct link to Controls and shortcuts")

Scroll to zoom in and out.

If any items in the graph are overlapping, click and drag to reposition them for easier viewing.

Use the available controls to accomplish the following:

- **Zoom fit** – Fit the graph within the viewport

- **Orientation** – Toggle the graph's orientation

- **Showing services / Hiding services** – Toggle the visibility of the services

- **Showing x topics** – Toggle between displaying different groups of topics


- [Controls and shortcuts](https://docs.foxglove.dev/docs/visualization/panels/topic-graph#controls-and-shortcuts)