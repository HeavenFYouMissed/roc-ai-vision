[Skip to main content](https://docs.foxglove.dev/docs/visualization/layouts#__docusaurus_skipToContent_fallback)

On this page

note

Authoring and editing layouts requires a [developer seat](https://docs.foxglove.dev/docs/security/seat-types). Members with a basic seat can view shared layouts.

With layouts, you can reuse a workspace for a recurring task or share it with a teammate solving a similar problem.

A perception engineer may develop layouts for calibrating various sensors, a planning engineer may have a few for visualizing different routing algorithm outputs, and a controls engineer may build one for plotting robot kinematics.

Use the "Layouts" menu to create, edit, and share layouts.

### Personal layouts [​](https://docs.foxglove.dev/docs/visualization/layouts\#personal-layouts "Direct link to Personal layouts")

Personal layouts are accessible to only you – they cannot be viewed, loaded, or edited by anyone else.

Saved changes to your personal layouts are synced across all your devices, and can be shared with your organization.

#### Create [​](https://docs.foxglove.dev/docs/visualization/layouts\#create "Direct link to Create")

Use the "Layouts" menu's "Create new layout" option to start building a custom workspace. Add and position [panels](https://docs.foxglove.dev/docs/visualization/panels), configure their settings, adjust [playback settings](https://docs.foxglove.dev/docs/visualization/playback), and set [variable](https://docs.foxglove.dev/docs/visualization/variables) values.

tip

To organize multiple panels in your layout, use the [Tab panel](https://docs.foxglove.dev/docs/visualization/panels/tab) feature. Select multiple panels and choose "Group in tab" to instantly organize them without manual dragging.

#### Unsaved changes [​](https://docs.foxglove.dev/docs/visualization/layouts\#unsaved-changes "Direct link to Unsaved changes")

Changes you make to layouts are automatically tracked both in the desktop app and per-browser on web, but layouts must be saved to sync across different environments. When you have unsaved local changes to your layout you'll see a purple 'unsaved' indicator appear on the layout menu. From your layout's context menu you can:

- **"Save changes"** – Save your locally stored changes.
- **"Revert"** – Discard your unsaved changes and revert to the last explicitly saved layout version

#### Import and export [​](https://docs.foxglove.dev/docs/visualization/layouts\#import-and-export "Direct link to Import and export")

Use a layout's context menu to "Export..." it as a JSON file.

Use the "Layouts" menu's "Import from file..." option to load an exported layout JSON file.

tip

You can also copy individual tabs between layouts using the right-click context menu. See [Tab panel - Copying tabs between layouts](https://docs.foxglove.dev/docs/visualization/panels/tab#copying-tabs-between-layouts) for details.

Use a layout's context menu to "Share with team..." – this will make a personal layout accessible to your entire organization.

#### Other [​](https://docs.foxglove.dev/docs/visualization/layouts\#other "Direct link to Other")

Use each layout's context menu to view a history of changes, rename, duplicate, or delete any personal layout.

To perform a batch action on multiple layouts:

- Use `Cmd` to multi-select individual layouts
- Use `Shift` to select a range of adjacent layouts
- Right-click any selected layout's context menu to select a batch action

### Organization layouts [​](https://docs.foxglove.dev/docs/visualization/layouts\#organization-layouts "Direct link to Organization layouts")

Organization layouts allow teams to curate a set of canonical layouts to accomplish common tasks – e.g. for calibrating radar sensors, visualizing planning algorithm outputs, or viewing logs. Instead of maintaining marginally different setups for different tasks, organization members can use layouts pre-configured by workflow experts to avoid redundant work and accelerate development.

Organization layouts work very similarly to personal layouts – i.e. you can view their history, rename, copy, export, and delete them – but operate more as templates than evolving snapshots of a workspace.

#### Unsaved changes [​](https://docs.foxglove.dev/docs/visualization/layouts\#unsaved-changes-1 "Direct link to Unsaved changes")

Changes you make to layouts are automatically tracked both in the desktop app and per-browser on web, but layouts must be saved to sync across different environments and to update the shared layout for your organization. When you have unsaved local changes to your layout you'll see an 'unsaved' indicator appear on the layout menu. From your layout's context menu you can:

- **"Save changes"** – Overwrite the organization layout with your changes
- **"Revert"** – Discard your local changes and revert to the last explicitly saved layout version
- **"Make a personal copy"** – Save your changes as a separate personal layout (reverts changes to organization layout)

#### Share [​](https://docs.foxglove.dev/docs/visualization/layouts\#share-1 "Direct link to Share")

Use a personal layout's context menu to "Share with team..." – this will make it accessible to your entire organization.

Organization layouts can be edited, renamed, or deleted by any team member.

### Layout History [​](https://docs.foxglove.dev/docs/visualization/layouts\#layout-history "Direct link to Layout History")

In the layout history sidebar you can compare your current draft to the most recently saved version of a layout and see who last saved the layout when.

[Pro](https://foxglove.dev/pricing)

[Enterprise](https://foxglove.dev/pricing)

[Academic](https://foxglove.dev/pricing)

On Pro, Enterprise, and Academic plans, you can also browse a history of changes to your layouts and
restore an older version if needed.

### Layout Management [​](https://docs.foxglove.dev/docs/visualization/layouts\#layout-management "Direct link to Layout Management")

The [layout manager](https://app.foxglove.dev/~/layouts) allows for individual or batch changes to layouts:

#### Add [​](https://docs.foxglove.dev/docs/visualization/layouts\#add "Direct link to Add")

Use the **"Add"** button to either create a new layout or import an existing one.

- **"New Layout"** will prompt for a name, personal or organizational, and an optional folder to store the layout under:

- **"Import Personal Layout"** will prompt to select a layout JSON file from the computer's file system.


#### Navigation [​](https://docs.foxglove.dev/docs/visualization/layouts\#navigation "Direct link to Navigation")

Browsing layout folders can be performed directly from the left panel. They are categorized at the top level by: all, personal, and organizational. Additionally, navigation can be performed from the right panel by clicking a folder row to move into it, or by clicking the breadcrumb at the top of the right panel to move out of it.

#### Searching & Filtering [​](https://docs.foxglove.dev/docs/visualization/layouts\#searching--filtering "Direct link to Searching & Filtering")

Searching and filtering can be performed from the top of the right panel. Providing values to these controls will limit the shown layouts in the currently selected folder or top level category to those that match.

#### Individual Actions [​](https://docs.foxglove.dev/docs/visualization/layouts\#individual-actions "Direct link to Individual Actions")

To perform similar actions from the layout menu, clicking the **"ellipsis"** button will provide a dropdown revealing them. The actions listed will depend on the layout type and status. Clicking **"Open"** will navigate you to visualization with the opened layout preselected.

#### Batch Actions [​](https://docs.foxglove.dev/docs/visualization/layouts\#batch-actions "Direct link to Batch Actions")

Batch actions are performed by selecting at least one row using the checkboxes in the left most column. Once selected the actions will appear above the table.

##### Move to Folder [​](https://docs.foxglove.dev/docs/visualization/layouts\#move-to-folder "Direct link to Move to Folder")

Only layouts of the same type, personal or organizational, can be selected to move to a new folder. Additionally, only a folder that exists under that same layout type can be chosen as a destination.

### Layouts in Python notebooks [​](https://docs.foxglove.dev/docs/visualization/layouts\#layouts-in-python-notebooks "Direct link to Layouts in Python notebooks")

You can create and use layouts programmatically in your [Jupyter-like notebooks](https://docs.foxglove.dev/docs/notebook). See the [Layouts API documentation](https://docs.foxglove.dev/docs/notebook/layouts) for details.

- [Personal layouts](https://docs.foxglove.dev/docs/visualization/layouts#personal-layouts)
- [Organization layouts](https://docs.foxglove.dev/docs/visualization/layouts#organization-layouts)
- [Layout History](https://docs.foxglove.dev/docs/visualization/layouts#layout-history)
- [Layout Management](https://docs.foxglove.dev/docs/visualization/layouts#layout-management)
- [Layouts in Python notebooks](https://docs.foxglove.dev/docs/visualization/layouts#layouts-in-python-notebooks)