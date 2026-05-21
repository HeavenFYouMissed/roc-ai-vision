[Skip to main content](https://docs.foxglove.dev/docs/visualization/panels#__docusaurus_skipToContent_fallback)

On this page

Panels are modular visualization interfaces that can be configured and arranged into Foxglove layouts.

Find the full list of available panels in the "Add panel" menu.

![panels thumbnail](https://docs.foxglove.dev/assets/images/thumbnail-b1f9a341075cb5080868a1ae2c40d495.png)

### Add to layout [​](https://docs.foxglove.dev/docs/visualization/panels\#add-to-layout "Direct link to Add to layout")

Click any panel name to add it to your current layout. Alternatively, drag and drop a panel name into your current layout to add it to a specific location.

Each panel's top bar contains the following:

- **Menu** – Common panel actions, like splitting the panel or changing it to another panel type
- **Settings** – Click the cog icon or press `,` to open the panel's settings

Use the top bar to easily drag a panel around the layout.

### Edit settings [​](https://docs.foxglove.dev/docs/visualization/panels\#edit-settings "Direct link to Edit settings")

Click the cog icon in each panel's top bar to view and edit its settings in the sidebar. The selected panel will be designated with a purple border.

![panel settings tab](https://docs.foxglove.dev/assets/images/settings-tab-bb0ebd651ff0565a80294188f8bc44bb.png)

Clicking different panels in your layout while the sidebar's panel settings are open will switch to its settings.

### Drag-and-drop topics [​](https://docs.foxglove.dev/docs/visualization/panels\#drag-and-drop-topics "Direct link to Drag-and-drop topics")

Filter your data source's topics – and their contained [message paths](https://docs.foxglove.dev/docs/visualization/message-path-syntax) – using the sidebar's Topics tab:

![panel settings tab](https://docs.foxglove.dev/assets/images/filter-topics-babe53aa6ad27c36da29144cfbfeddac.png)

Drag-and-drop filter results into certain panels for easy instant visualization:

- **All topics** – [Raw Messages](https://docs.foxglove.dev/docs/visualization/panels/raw-messages) and [Table](https://docs.foxglove.dev/docs/visualization/panels/table) panels
- **Image and image annotation topics** – [Image](https://docs.foxglove.dev/docs/visualization/panels/image) panel
- **Message paths** – [Plot](https://docs.foxglove.dev/docs/visualization/panels/plot) and [State Transitions](https://docs.foxglove.dev/docs/visualization/panels/state-transitions) panels

To drag-and-drop multiple message paths, use `Shift` to select a range of adjacent items, or `Ctrl` (`Cmd` on macOS) to select multiple non-adjacent items:

![panel settings tab](https://docs.foxglove.dev/assets/images/multi-drag-and-drop-84017fc34d65a5d4f3fc1c1e0390a164.png)

You can also drag topics and message paths from a panel's settings sidebar. For example, drag a series from a Plot panel's settings to quickly duplicate it into another Plot panel, or drag a topic from a 3D panel's settings into a Raw Messages panel to inspect the data.

## Controls and shortcuts [​](https://docs.foxglove.dev/docs/visualization/panels\#controls-and-shortcuts "Direct link to Controls and shortcuts")

- `,` – Open the panel settings sidebar
- `Cmd` \+ `a` – Select all panels in the current layout
- `Cmd` \+ `b` – Close sidebar if open
- Hover on panel + ````` – Show panel shortcuts (remove from layout or split)
- Click input + Drag right – Increment numeric panel setting values
- Click input + Drag left – Decrement numeric panel setting values

## Import/export settings [​](https://docs.foxglove.dev/docs/visualization/panels\#importexport-settings "Direct link to Import/export settings")

Each panel stores its settings (also known as panel state) as a JSON object. When developing panels, it can be helpful to manually inspect or adjust the settings of a given panel.

To view or edit a panel's settings, click "Import/Export Settings" in the panel settings menu:

This will open a modal displaying the panel's settings.

Note that these settings are only for the current panel. To import or export an entire layout, see [Layouts](https://docs.foxglove.dev/docs/visualization/layouts#import-and-export).

- [Add to layout](https://docs.foxglove.dev/docs/visualization/panels#add-to-layout)
- [Edit settings](https://docs.foxglove.dev/docs/visualization/panels#edit-settings)
- [Drag-and-drop topics](https://docs.foxglove.dev/docs/visualization/panels#drag-and-drop-topics)
- [Controls and shortcuts](https://docs.foxglove.dev/docs/visualization/panels#controls-and-shortcuts)
- [Import/export settings](https://docs.foxglove.dev/docs/visualization/panels#importexport-settings)