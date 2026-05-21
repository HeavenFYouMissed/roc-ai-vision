[Skip to content](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst#start-of-content)

You signed in with another tab or window. [Reload](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst) to refresh your session.You signed out in another tab or window. [Reload](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst) to refresh your session.You switched accounts on another tab or window. [Reload](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst) to refresh your session.Dismiss alert

{{ message }}

[hailo-ai](https://github.com/hailo-ai)/ **[hailo-apps-core](https://github.com/hailo-ai/hailo-apps-core)** Public

- [Notifications](https://github.com/login?return_to=%2Fhailo-ai%2Fhailo-apps-core) You must be signed in to change notification settings
- [Fork\\
78](https://github.com/login?return_to=%2Fhailo-ai%2Fhailo-apps-core)
- [Star\\
188](https://github.com/login?return_to=%2Fhailo-ai%2Fhailo-apps-core)


## Collapse file tree

## Files

master

Search this repository(forward slash)` forward slash/`

/

# TAPPAS\_architecture.rst

Copy path

Blame

More file actions

Blame

More file actions

## Latest commit

[![HailoRT-Automation](https://avatars.githubusercontent.com/u/98901220?v=4&size=40)](https://github.com/HailoRT-Automation)[HailoRT-Automation](https://github.com/hailo-ai/hailo-apps-core/commits?author=HailoRT-Automation)

[v5.3.0](https://github.com/hailo-ai/hailo-apps-core/commit/70c2d5bf2ad31fa47e4c1dc06fd5574055ea8772)

2 months agoMar 31, 2026

[70c2d5b](https://github.com/hailo-ai/hailo-apps-core/commit/70c2d5bf2ad31fa47e4c1dc06fd5574055ea8772) · 2 months agoMar 31, 2026

## History

[History](https://github.com/hailo-ai/hailo-apps-core/commits/master/docs/TAPPAS_architecture.rst)

Open commit details

[View commit history for this file.](https://github.com/hailo-ai/hailo-apps-core/commits/master/docs/TAPPAS_architecture.rst) History

55 lines (37 loc) · 4.12 KB

/

# TAPPAS\_architecture.rst

Top

## File metadata and controls

- Preview

- Code

- Blame


55 lines (37 loc) · 4.12 KB

[Raw](https://github.com/hailo-ai/hailo-apps-core/raw/refs/heads/master/docs/TAPPAS_architecture.rst)

Copy raw file

Download raw file

Outline

Edit and raw actions

# TAPPAS Framework

[Permalink: TAPPAS Framework](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst#tappas-framework)

TAPPAS is a GStreamer based library of plug-ins. It enables using a Hailo devices within gstreamer pipelines to create intelligent video processing applications.

## What is GStreamer?

[Permalink: What is GStreamer?](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst#what-is-gstreamer)

GStreamer is a framework for creating streaming media applications.

GStreamer's development framework makes it possible to write any type of streaming multimedia application. The GStreamer framework is designed to simplify for the user how to write applications that handle audio or video or both. It isn't restricted to audio and video and can process any kind of data flow. ​The framework is based on plugins that will provide various codecs and other functionalities. The plugins can be linked and arranged in a pipeline. This pipeline defines the flow of the data. ​The GStreamer core function is to provide a framework for plugins, data flow, and media type handling/negotiation. It also provides an API to write applications using the various plugins.​

For additional details check [GStreamer overview](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/terminology.rst#gstreamer-framework)

## Hailo GStreamer Concepts

[Permalink: Hailo GStreamer Concepts](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst#hailo-gstreamer-concepts)

The functionality that Hailo brings into the GStreamer-framework allows us to infer video frames easily and intuitively without compromising on performance and flexibility.

### Hailo Concepts

[Permalink: Hailo Concepts](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst#hailo-concepts)

- **Network encapsulation** \- Since in a configured network group, there are only input and output layers a GstHailoNet will be associated to a "Network" by its configured input and output pads​
- **Network independent elements** \- The GStreamer elements will be network independent, so the same infrastructure elements can be used for different applicative pipelines that use different NN functionality, configuration, activation, and pipelines​. Using the new API we can better decouple network configuration and activation stages and thus better support network switch​
- **GStreamer Hailo decoupling** \- Applicative code will use Hailo API and as such will be GStreamer independent. This will help us build and develop the NN and postprocessing functionality in a controlled environment (with all modern IDE and debugging capabilities).
- **Context control** \- Hailo elements will be contextless and thus leave the context (thread) control to the pipeline builder​
- **GStreamer reuse** \- The pipeline will use many off the shelf GStreamer elements​

### Hailo GStreamer Elements

[Permalink: Hailo GStreamer Elements](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/TAPPAS_architecture.rst#hailo-gstreamer-elements)

- [HailoNet](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_net.rst) \- Element for sending and receiving data from Hailo-8/Hailo-10H chip
- [HailoFilter](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_filter.rst) \- Element that enables the user to apply a postprocess or drawing operation to a frame and its tensors
- [HailoPython](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_python.rst) \- Element that enables the user to apply a postprocess or drawing operation to a frame and its tensors via python.
- [HailoMuxer](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_muxer.rst) \- Muxer element used for Multi-Hailo-8 setups
- [HailoDeviceStats](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_device_stats.rst) \- Hailodevicestats is an element that samples power and temperature
- [HailoAggregator](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_aggregator.rst) \- HailoAggregator is an element designed for applications with cascading networks. It has 2 sink pads and 1 source
- [HailoCropper](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_cropper.rst) \- HailoCropper is an element designed for applications with cascading networks. It has 1 sink and 2 sources
- [HailoTileAggregator](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_tile_aggregator.rst) \- HailoTileAggregator is an element designed for applications with tiles. It has 2 sink pads and 1 source
- [HailoTileCropper](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_tile_cropper.rst) \- HailoTileCropper is an element designed for applications with tiles. It has 1 sink and 2 sources
- [HailoTracker](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_tracker.rst) \- HailoTracker is an element that applies Joint Detection and Embedding (JDE) model with Kalman filtering to track object instances.
- [HailoRoundRobin](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_roundrobin.rst) \- HailoRoundRobin is an element that provides muxing functionality in roundrobin method.
- [HailoStreamRouter](https://github.com/hailo-ai/hailo-apps-core/blob/master/docs/elements/hailo_stream_router.rst) \- HailoStreamRouter is an element that provides de-muxing functionality.

You can’t perform that action at this time.