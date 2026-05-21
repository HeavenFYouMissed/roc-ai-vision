# [Rerun](https://rerun.io/)

- [GitHub](https://github.com/rerun-io/rerun)
- [Discord](https://discord.gg/PXtCgFBSmH)
- [X](https://twitter.com/rerundotio)
- [LinkedIn](https://www.linkedin.com/company/rerun-io/)

Search documentation `/`

`esc`

[**Getting Started** Docs\\
\\
The fastest way to get started is with a quick start guide for C++, Python or Rust. If you are looking for other language support, search for an open issue on GitHub to find the…\\
\\
Docs›Overview›What Is Rerun](https://rerun.io/docs/overview/what-is-rerun) [**Troubleshooting** Docs\\
\\
You can set RUST\_LOG=debug before running to get some verbose logging output. If you run into any issues don't hesitate to open a ticket or join our Discord.\\
\\
Docs›Getting Started›Troubleshooting](https://rerun.io/docs/getting-started/troubleshooting) [**Examples** Docs\\
\\
The examples serve to showcase the Rerun logging SDK, and demonstrate the usage of Rerun to create visual walkthroughs of papers. To run these examples, make sure you have the…\\
\\
Examples](https://rerun.io/examples) [**Archetypes** Docs\\
\\
Rerun comes with built-in support for a number of different types that can be logged via the Python and Rust Logging APIs and then visualized in the Viewer. The top-level types…\\
\\
Docs›Reference›Types›Archetypes](https://rerun.io/docs/reference/types/archetypes)

```` Navigate`` Open

Browse docs

# Views

Use with LLMUse with LLMCopy as MarkdownOpen in ChatGPTOpen in Claude [Edit page](https://github.com/rerun-io/rerun/blob/main/docs/content/reference/types/views.md) [Leave feedback](https://rerun.io/docs/reference/types/views#docs-feedback)

Views are the panels shown in the viewer's viewport and the primary means of inspecting & visualizing previously logged data. This page lists all built-in views.

- [`BarChartView`](https://rerun.io/docs/reference/types/views/bar_chart_view): A bar chart view.
- [`DataframeView`](https://rerun.io/docs/reference/types/views/dataframe_view): A view to display any data in a tabular form.
- [`GraphView`](https://rerun.io/docs/reference/types/views/graph_view): A graph view to display time-variying, directed or undirected graph visualization.
- [`MapView`](https://rerun.io/docs/reference/types/views/map_view): A 2D map view to display geospatial primitives.
- [`Spatial2DView`](https://rerun.io/docs/reference/types/views/spatial2d_view): For viewing spatial 2D data.
- [`Spatial3DView`](https://rerun.io/docs/reference/types/views/spatial3d_view): For viewing spatial 3D data.
- [`StateTimelineView`](https://rerun.io/docs/reference/types/views/state_timeline_view): A view for displaying state transitions over time, for use with [`archetypes.StateChange`](https://rerun.io/docs/reference/types/archetypes/state_change).
- [`TensorView`](https://rerun.io/docs/reference/types/views/tensor_view): A view on a tensor of any dimensionality.
- [`TextDocumentView`](https://rerun.io/docs/reference/types/views/text_document_view): A view of a single text document, for use with [`archetypes.TextDocument`](https://rerun.io/docs/reference/types/archetypes/text_document).
- [`TextLogView`](https://rerun.io/docs/reference/types/views/text_log_view): A view of a text log, for use with [`archetypes.TextLog`](https://rerun.io/docs/reference/types/archetypes/text_log).
- [`TimeSeriesView`](https://rerun.io/docs/reference/types/views/time_series_view): A time series view for scalars over time, for use with [`archetypes.Scalars`](https://rerun.io/docs/reference/types/archetypes/scalars).

Feedback?

Send feedback