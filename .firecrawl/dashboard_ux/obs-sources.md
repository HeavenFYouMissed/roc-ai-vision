[Knowledge Base](https://obsproject.com/kb) >
[Sources & Filters](https://obsproject.com/kb/category/6) >
Sources Guide

# Sources Guide

2022-01-11

#### Table of Contents

- [Adding/Removing Sources](https://obsproject.com/kb/sources-guide#adding-removing-sources)
- [Ordering Sources](https://obsproject.com/kb/sources-guide#ordering-sources)
- [Hiding/Showing Sources](https://obsproject.com/kb/sources-guide#hiding-showing-sources)
- [Positioning and Sizing Sources](https://obsproject.com/kb/sources-guide#positioning-and-sizing-sources)
- [Advanced Sizing](https://obsproject.com/kb/sources-guide#advanced-sizing)
- [Hotkeys](https://obsproject.com/kb/sources-guide#hotkeys)

Scenes and Sources are the meat of OBS Studio. These are where you set up your stream layout, add your games, webcams, and any other devices or media that you want in the output.

* * *

| [![Audio Sources](https://obsproject.com/media/pages/kb/sources-guide/29d816eab7-1767489976/microphone.png)](https://obsproject.com/kb/audio-sources)[Audio Sources](https://obsproject.com/kb/audio-sources)<br>Capture audio, including microphones, desktop audio, and application audio | [![Browser Source](https://obsproject.com/media/pages/kb/sources-guide/f38a4adf05-1767489966/globe.png)](https://obsproject.com/kb/browser-source)[Browser Source](https://obsproject.com/kb/browser-source)<br>Add a web page to your scene. This is primarily used for [stream alert overlays and chat boxes](https://obsproject.com/kb/stream-tutorial-2-alert) | [![Color Source](https://obsproject.com/media/pages/kb/sources-guide/c1705497d0-1767490019/brush.png)](https://obsproject.com/kb/color-source)[Color Source](https://obsproject.com/kb/color-source)<br>Add a block of colour to a scene |
| [![Display Capture Sources](https://obsproject.com/media/pages/kb/sources-guide/bd78c452c5-1767490019/window.png)](https://obsproject.com/kb/display-capture-sources)[Display Capture Sources](https://obsproject.com/kb/display-capture-sources)<br>Capture an entire display on Windows, Linux, and macOS 12.6 and prior. On macOS 13, use [macOS Screen Capture](https://obsproject.com/kb/macos-screen-capture-source) | [![Game Capture Source](https://obsproject.com/media/pages/kb/sources-guide/0689ae4c4f-1767489984/gamepad.png)](https://obsproject.com/kb/game-capture-source)[Game Capture Source](https://obsproject.com/kb/game-capture-source)<br>Capture hardware-accelerated games with high performance (Windows only; on other platforms, use a [Display](https://obsproject.com/kb/display-capture-sources), [Window](https://obsproject.com/kb/window-capture-sources), or [macOS Screen Capture](https://obsproject.com/kb/macos-screen-capture-source) Source | [![Image Sources](https://obsproject.com/media/pages/kb/sources-guide/c8203e6052-1767489960/image.png)](https://obsproject.com/kb/image-sources)[Image Sources](https://obsproject.com/kb/image-sources)<br>Add an image, or slideshow of images, to a scene |
| [![macOS Screen Capture Source](https://obsproject.com/media/pages/kb/sources-guide/bd78c452c5-1767490019/window.png)](https://obsproject.com/kb/macos-screen-capture-source)[macOS Screen Capture Source](https://obsproject.com/kb/macos-screen-capture-source)<br>Capture a display, window, or application — including desktop audio (macOS 13 and later) | [![Media Sources](https://obsproject.com/media/pages/kb/sources-guide/c97860e936-1767489965/media.png)](https://obsproject.com/kb/media-sources)[Media Sources](https://obsproject.com/kb/media-sources)<br>Add audio/video files to a scene. If VLC Media Player is installed, add a playlist | [![Text Sources](https://obsproject.com/media/pages/kb/sources-guide/79198fccf3-1767489984/text.png)](https://obsproject.com/kb/text-sources)[Text Sources](https://obsproject.com/kb/text-sources)<br>Add customisable text to a source, including from a text file |
| [![Video Capture Sources](https://obsproject.com/media/pages/kb/sources-guide/55c7938324-1767489965/camera.png)](https://obsproject.com/kb/video-capture-sources)[Video Capture Sources](https://obsproject.com/kb/video-capture-sources)<br>Capture a webcam, capture card, or other video device (including Blackmagic) | [![Window Capture Sources](https://obsproject.com/media/pages/kb/sources-guide/bd78c452c5-1767490019/window.png)](https://obsproject.com/kb/window-capture-sources)[Window Capture Sources](https://obsproject.com/kb/window-capture-sources)<br>Capture a single window from your display on Windows, Linux, and macOS 12.6 and prior. On macOS 13, use [macOS Screen Capture](https://obsproject.com/kb/macos-screen-capture-source) |  |

## Adding/Removing Sources

To add a Source, click on the + button at the bottom of the Sources dock (figure 1.1). From the Add Sources list (figure 1.2), you can choose a Source to add to your scene.

Whether it's a specific window, a capture card or game, image, text or your entire display that you want to capture, there are several different sources available in OBS Studio for you to choose from. Try them out!

To remove a Source, select the Source in the Sources dock and click the - button.

| ![](https://obsproject.com/media/pages/kb/sources-guide/d93bf42eb6-1767489963/obs-sources.png)<br> Figure 1.1: _Sources dock_ | ![](https://obsproject.com/media/pages/kb/sources-guide/f016374ea6-1767489956/obs-sources-list.png)<br> Figure 1.2: _Add Sources list_ |

## Ordering Sources

Sources that are above others in the Sources list (Figure 1.1) are also located above other Sources in the preview. You can re-align sources in the preview and change their order by using drag and drop in the list, or using the up and down arrow buttons.

A Source that is listed above another Source in the list will be on top and might hide what's beneath it. This can also be useful for situations where you want something on top of another source, like a webcam to show over your game play.

## Hiding/Showing Sources

To hide a Source temporarily, you can click the eye icon to hide the associated Source with it; the eye will become greyed out.

To show the Source again, click the eye another time; the eye will become white.

| ![](https://obsproject.com/media/pages/kb/sources-guide/361ee159e3-1767489956/cnmeq1a-1.png)<br> Figure 2.1: _Visible Source_ | ![](https://obsproject.com/media/pages/kb/sources-guide/1f2ca6f432-1767489984/ofefwok-1.png)<br> Figure 2.2: _Hidden Source_ |

## Positioning and Sizing Sources

![](https://obsproject.com/media/pages/kb/sources-guide/802cfe190d-1767489984/obs-bounding-box.png)
Figure 3.1: _Bounding box_

When a Source is selected in the Sources list, you will see a red box that shows up around it (Figure 3.1). This is the bounding box, and can be used to position sources within the preview as well as make the source larger or smaller.

Drag Sources around to position them within the Scene. Drag the red drag handles on the corners and edges of the Source to resize it to the desired size.

To place and size Sources with the greatest accuracy, use the **Edit Transform** window (figure 3.2): click on the Source, then click the Edit menu → Transform → Edit Transform… (Ctrl-E/Cmd-E). There are also options available in the **Transform list** (figure 3.3).

| ![](https://obsproject.com/media/pages/kb/sources-guide/f63ddc75a7-1767489960/obs-transform-window.png)<br> Figure 3.2: _Edit Transform window_ | ![](https://obsproject.com/media/pages/kb/sources-guide/dcbc5df2a7-1767489965/obs-transform.png)<br> Figure 3.3: _Transform menu_ |

## Advanced Sizing

![](https://obsproject.com/media/pages/kb/sources-guide/c2e3f6b595-1767490018/obs-crop-box.png)
Figure 3.4: _Cropping a source_

To show only part of a Source, you can crop it to the part of the image you wish to show.

You can crop a Source using the Edit Transform window (Figure 3.1) or by holding the Alt/Option key and dragging the bounding box's drag handles. The edges will change to green to show it's being cropped. You can see both techniques being used in Figure 3.4 above to crop and enlarge only the part of the screen we want to show.

## Hotkeys

The following Hotkeys are available in the preview to tweak the source position and size:

| Function | Windows | macOS |
| --- | --- | --- |
| Edit Transform | Ctrl-E | Cmd-E |
| Reset Transform | Ctrl-R | Cmd-R |
| Fit to Screen | Ctrl-F | Cmd-F |
| Stretch to Screen | Ctrl-S | Cmd-S |
| Center to Screen | Ctrl-D | Cmd-D |
| Crop | Hold Alt and drag the bounding box | Hold Option and drag the bounding box |
| Disable Source/Edge snapping | Hold Ctrl | Hold Cmd |

#### Table of Contents

- [Adding/Removing Sources](https://obsproject.com/kb/sources-guide#adding-removing-sources)
- [Ordering Sources](https://obsproject.com/kb/sources-guide#ordering-sources)
- [Hiding/Showing Sources](https://obsproject.com/kb/sources-guide#hiding-showing-sources)
- [Positioning and Sizing Sources](https://obsproject.com/kb/sources-guide#positioning-and-sizing-sources)
- [Advanced Sizing](https://obsproject.com/kb/sources-guide#advanced-sizing)
- [Hotkeys](https://obsproject.com/kb/sources-guide#hotkeys)

#### Related Articles

[OBS Studio Overview Guide](https://obsproject.com/kb/obs-studio-overview)

#### Similar Articles

[Track Matte Stinger Transitions](https://obsproject.com/kb/track-matte-stinger-transitions)

[Media Sources](https://obsproject.com/kb/media-sources)

[Hardware Encoding](https://obsproject.com/kb/hardware-encoding)