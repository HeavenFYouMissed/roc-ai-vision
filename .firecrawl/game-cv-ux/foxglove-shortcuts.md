[Skip to main content](https://docs.foxglove.dev/docs/visualization/shortcuts#__docusaurus_skipToContent_fallback)

On this page

Summary of keyboard shortcuts and mouse interactions.

## Playback controls [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#playback-controls "Direct link to Playback controls")

| Shortcut | Description | Command |
| --- | --- | --- |
| `Space` | Toggle playback | `player.togglePlayback` |
| `←` | Seek backward 100ms | `player.seekBackwardMedium` |
| `→` | Seek forward 100ms | `player.seekForwardMedium` |
| `Shift` \+ `←` | Seek backward 10ms | `player.seekBackwardSmall` |
| `Shift` \+ `→` | Seek forward 10ms | `player.seekForwardSmall` |
| `Alt` \+ `←` (Windows and Linux) <br>`Option` \+ `←` (macOS) | Seek backward 500ms | `player.seekBackwardLarge` |
| `Alt` \+ `→` (Windows and Linux) <br>`Option` \+ `→` (macOS) | Seek forward 500ms | `player.seekForwardLarge` |
| `Home` | Seek to the start of the data | `player.seekStart` |
| `End` | Seek to the end of the data | `player.seekEnd` |
| `Shift` \+ `↑` | Increase playback speed | `player.speedUp` |
| `Shift` \+ `↓` | Decrease playback speed | `player.speedDown` |
| `Ctrl` \+ `e` (Windows and Linux) <br>`Cmd` \+ `e` (macOS) | Open the create event dialog | `player.createEvent` |
| `Shift` \+ `Alt` \+ `←` (Windows and Linux) <br>`Shift` \+ `Option` \+ `←` (macOS) | Snap event start time to current playback time (while editing an event) | `player.snapEventStartTime` |
| `Shift` \+ `Alt` \+ `→` (Windows and Linux) <br>`Shift` \+ `Option` \+ `→` (macOS) | Snap event end time to current playback time (while editing an event) | `player.snapEventEndTime` |

[View full documentation](https://docs.foxglove.dev/docs/visualization/playback)

## General [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#general "Direct link to General")

### Global [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#global "Direct link to Global")

| Shortcut | Description | Command |
| --- | --- | --- |
| `Ctrl` \+ `K` (Windows and Linux) <br>`Cmd` \+ `K` (macOS) | Open the [command palette](https://docs.foxglove.dev/docs#command-palette) |  |
| `Ctrl` \+ `O` (Windows and Linux) <br>`Cmd` \+ `O` (macOS) | Open the [file picker](https://docs.foxglove.dev/docs/visualization/connecting/local-data) | `global.datasource.openFile` |
| `Ctrl` \+ `Shift` \+ `O` (Windows and Linux) <br>`Cmd` \+ `Shift` \+ `O` (macOS) | Open the [connection picker](https://docs.foxglove.dev/docs/visualization/connecting/live) | `global.datasource.openConnection` |

### Panel selection [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#panel-selection "Direct link to Panel selection")

| Shortcut | Description | Command |
| --- | --- | --- |
| `Ctrl` \+ `A` (Windows and Linux) <br>`Cmd` \+ `A` (macOS) | Select all panels in the current layout or tab. | `panel.selectAll` |
| Hover on panel + ````` (backtick) | Show panel shortcuts (remove from layout or split) |  |

### Panel settings [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#panel-settings "Direct link to Panel settings")

| Shortcut | Description | Command |
| --- | --- | --- |
| `,` | Toggle the panel settings sidebar | `panel.settings.toggleOpen` |
| While inside input + `↑` | Increment numeric panel setting values |  |
| While inside input + `↓` | Decrement numeric panel setting values |  |
| Click input + drag right | Increment numeric panel setting values |  |
| Click input + drag left | Decrement numeric panel setting values |  |

### Sidebar navigation [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#sidebar-navigation "Direct link to Sidebar navigation")

| Shortcut | Description | Command |
| --- | --- | --- |
| `[` | Toggle left sidebar | `sidebar.left.toggleVisibility` |\
| `]` | Toggle right sidebar | `sidebar.right.toggleVisibility` |

## Desktop tabs [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#desktop-tabs "Direct link to Desktop tabs")

The [desktop app](https://foxglove.dev/download) supports browser-style tabs for working with multiple data sources in a single window.

| Shortcut | Description |
| --- | --- |
| `Ctrl` \+ `t` (Windows and Linux) <br>`Cmd` \+ `t` (macOS) | New tab |
| `Ctrl` \+ `w` (Windows and Linux) <br>`Cmd` \+ `w` (macOS) | Close tab |
| `Ctrl` \+ `Tab` | Next tab |
| `Ctrl` \+ `Shift` \+ `Tab` | Previous tab |
| `Ctrl` \+ `/` (Windows and Linux) <br>`Cmd` \+ `/` (macOS) | Go to last active tab |
| `Ctrl` \+ `[` (Windows and Linux) <br>`Cmd` \+ `[` (macOS) | Navigate back |\
| `Ctrl` \+ `]` (Windows and Linux) <br>`Cmd` \+ `]` (macOS) | Navigate forward |

## Panels [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#panels "Direct link to Panels")

### 3D [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#3d "Direct link to 3D")

| Shortcut | Description | Command |
| --- | --- | --- |
| `KeyW` | Move camera forward | `panel.threeDee.moveForward` |
| `KeyA` | Move camera to the left | `panel.threeDee.moveLeft` |
| `KeyS` | Move camera backward | `panel.threeDee.moveBackward` |
| `KeyD` | Move camera to the right | `panel.threeDee.moveRight` |
| `Shift`+`KeyW` | Rotate camera up | `panel.threeDee.rotateUp` |
| `Shift`+`KeyA` | Rotate camera left | `panel.threeDee.rotateLeft` |
| `Shift`+`KeyS` | Rotate camera down | `panel.threeDee.rotateDown` |
| `Shift`+`KeyD` | Rotate camera right | `panel.threeDee.rotateRight` |
| Scroll | Zoom in and out |  |
| Drag | Move camera parallel to the ground (x-y plane) |  |
| `Alt` \+ drag (Windows and Linux) <br>`Option` \+ drag (macOS) | Move camera parallel to the screen (enable z-axis) |  |
| `Shift` \+ drag, or right-click-drag | Rotate the camera around the target position |  |
| `1` | Re-center the camera on the chosen target frame | `panel.threeDee.resetCamera` |
| `3` | Toggle between 2D bird's-eye view and 3D perspective view | `panel.threeDee.togglePerspective` |
| `i` | Show or hide the object inspector | `panel.threeDee.toggleInspector` |

[View full documentation](https://docs.foxglove.dev/docs/visualization/panels/3d)

### Image [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#image "Direct link to Image")

|     |     |
| --- | --- |
| Scroll | Zoom |

[View full documentation](https://docs.foxglove.dev/docs/visualization/panels/image)

### Plot [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#plot "Direct link to Plot")

| Shortcut | Description | Command |
| --- | --- | --- |
| Scroll | Default zoom for plot type |  |
| Scroll with mouse over y-axis <br>Scroll + `Shift` | Zoom the y-axis only |  |
| Scroll with mouse over x-axis <br>Scroll + `Alt` (Windows and Linux) <br>Scroll + `Option` (macOS) | Zoom the x-axis only |  |
| Scroll + `Shift` \+ `Alt` (Windows and Linux) <br>Scroll + `Shift` \+ `Option` (macOS) | Zoom both axes |  |
| `r` (when panel is selected) | Reset view to original viewport | `panel.plot.resetView` |
| `z` (when panel is selected) | Toggle zoom tool |  |

[View full documentation](https://docs.foxglove.dev/docs/visualization/panels/plot)

### State Transitions [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#state-transitions "Direct link to State Transitions")

| Shortcut | Description | Command |
| --- | --- | --- |
| Scroll | Zoom |  |
| Scroll + `Shift` | Scroll vertically |  |
| `r` (when panel is selected) | Reset view to original viewport | `panel.stateTransitions.resetView` |
| `z` (when panel is selected) | Toggle zoom tool |  |

[View full documentation](https://docs.foxglove.dev/docs/visualization/panels/state-transitions)

### Topic Graph [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#topic-graph "Direct link to Topic Graph")

| Shortcut | Description |
| --- | --- |
| Scroll | Zoom |

[View full documentation](https://docs.foxglove.dev/docs/visualization/panels/topic-graph)

### User Scripts [​](https://docs.foxglove.dev/docs/visualization/shortcuts\#user-scripts "Direct link to User Scripts")

| Shortcut | Description |
| --- | --- |
| `Cmd` \+ `s` | Save script changes |

[View full documentation](https://docs.foxglove.dev/docs/visualization/user-scripts)

- [Playback controls](https://docs.foxglove.dev/docs/visualization/shortcuts#playback-controls)
- [General](https://docs.foxglove.dev/docs/visualization/shortcuts#general)
  - [Global](https://docs.foxglove.dev/docs/visualization/shortcuts#global)
  - [Panel selection](https://docs.foxglove.dev/docs/visualization/shortcuts#panel-selection)
  - [Panel settings](https://docs.foxglove.dev/docs/visualization/shortcuts#panel-settings)
  - [Sidebar navigation](https://docs.foxglove.dev/docs/visualization/shortcuts#sidebar-navigation)
- [Desktop tabs](https://docs.foxglove.dev/docs/visualization/shortcuts#desktop-tabs)
- [Panels](https://docs.foxglove.dev/docs/visualization/shortcuts#panels)
  - [3D](https://docs.foxglove.dev/docs/visualization/shortcuts#3d)
  - [Image](https://docs.foxglove.dev/docs/visualization/shortcuts#image)
  - [Plot](https://docs.foxglove.dev/docs/visualization/shortcuts#plot)
  - [State Transitions](https://docs.foxglove.dev/docs/visualization/shortcuts#state-transitions)
  - [Topic Graph](https://docs.foxglove.dev/docs/visualization/shortcuts#topic-graph)
  - [User Scripts](https://docs.foxglove.dev/docs/visualization/shortcuts#user-scripts)