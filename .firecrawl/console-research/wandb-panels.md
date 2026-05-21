[Skip to main content](https://docs.wandb.ai/models/app/features/panels#content-area)

Help us improve these docs. [Take our quick survey.](https://forms.gle/a843ZFZu8oKqXYVDA)

[Weights & Biases Documentation home page![light logo](https://mintcdn.com/wb-21fd5541/C7PMWo9Jl9GZ2Zki/icons/Endorsed_primary_blackwhite.svg?fit=max&auto=format&n=C7PMWo9Jl9GZ2Zki&q=85&s=a9d999235518399090dd8ddd0a9e1e4a)![dark logo](https://mintcdn.com/wb-21fd5541/C7PMWo9Jl9GZ2Zki/icons/Endorsed_primary_goldwhite.svg?fit=max&auto=format&n=C7PMWo9Jl9GZ2Zki&q=85&s=f4eaf37e934c12e5fd3e806fe2984c2e)](https://docs.wandb.ai/)

English

Search...

Ctrl KAsk AI

Search...

Navigation

Panels

Panels

Products

[Get Started](https://docs.wandb.ai/get-started) [Reference](https://docs.wandb.ai/reference) [Release Notes](https://docs.wandb.ai/release-notes)
Support

- [W&B Models](https://docs.wandb.ai/models)

- [W&B Quickstart](https://docs.wandb.ai/models/quickstart)

- [Get Started with W&B Models](https://docs.wandb.ai/models/models_quickstart)

##### Guides

- Experiments

- Sweeps

- Tables

- Artifacts

- Registry

- Reports

- Automations

- LLM Evaluation Jobs

- W&B App UI



  - [Manage workspace, section, and panel settings](https://docs.wandb.ai/models/app/features/cascade-settings)
  - Panels



    - [Panels](https://docs.wandb.ai/models/app/features/panels)
    - Line plots

    - [Bar plots](https://docs.wandb.ai/models/app/features/panels/bar-plot)
    - [Parallel coordinates](https://docs.wandb.ai/models/app/features/panels/parallel-coordinates)
    - [Scatter plots](https://docs.wandb.ai/models/app/features/panels/scatter-plot)
    - [Media panels](https://docs.wandb.ai/models/app/features/panels/media)
    - [Save and diff code](https://docs.wandb.ai/models/app/features/panels/code)
    - [Parameter importance](https://docs.wandb.ai/models/app/features/panels/parameter-importance)
    - [Compare run metrics](https://docs.wandb.ai/models/app/features/panels/run-comparer)
    - Query panels
  - Custom charts

  - [Console logs](https://docs.wandb.ai/models/app/console-logs)
  - [Keyboard shortcuts](https://docs.wandb.ai/models/app/keyboard-shortcuts)

##### Integrations

- [Integrations overview](https://docs.wandb.ai/models/integrations)
- [Add W&B to a Python library](https://docs.wandb.ai/models/integrations/add-wandb-to-any-library)
- ML Frameworks and Libraries

- Cloud Platforms

- Other


##### Reference

- [Python SDK](https://docs.wandb.ai/models/ref/python)
- [Public API](https://docs.wandb.ai/models/ref/python/public-api)
- [CLI](https://docs.wandb.ai/models/ref/cli)
- [Query Expression Language](https://docs.wandb.ai/models/ref/query-panel)
- [Reports and Workspaces API](https://docs.wandb.ai/models/ref/wandb_workspaces/reports)

- [Support: Models](https://docs.wandb.ai/support/models)

On this page

- [Workspace modes](https://docs.wandb.ai/models/app/features/panels#workspace-modes)
- [Reset a workspace](https://docs.wandb.ai/models/app/features/panels#reset-a-workspace)
- [Configure the workspace layout](https://docs.wandb.ai/models/app/features/panels#configure-the-workspace-layout)
- [Configure a section’s layout](https://docs.wandb.ai/models/app/features/panels#configure-a-section%E2%80%99s-layout)
- [View a panel in full-screen mode](https://docs.wandb.ai/models/app/features/panels#view-a-panel-in-full-screen-mode)
- [Add panels](https://docs.wandb.ai/models/app/features/panels#add-panels)
- [Add a panel manually](https://docs.wandb.ai/models/app/features/panels#add-a-panel-manually)
- [Quick add panels](https://docs.wandb.ai/models/app/features/panels#quick-add-panels)
- [Share a panel](https://docs.wandb.ai/models/app/features/panels#share-a-panel)
- [Compose a panel’s full-screen link programmatically](https://docs.wandb.ai/models/app/features/panels#compose-a-panel%E2%80%99s-full-screen-link-programmatically)
- [Embed or share a panel on social media](https://docs.wandb.ai/models/app/features/panels#embed-or-share-a-panel-on-social-media)
- [Email a panel report](https://docs.wandb.ai/models/app/features/panels#email-a-panel-report)
- [Manage panels](https://docs.wandb.ai/models/app/features/panels#manage-panels)
- [Edit a panel](https://docs.wandb.ai/models/app/features/panels#edit-a-panel)
- [Move a panel](https://docs.wandb.ai/models/app/features/panels#move-a-panel)
- [Duplicate a panel](https://docs.wandb.ai/models/app/features/panels#duplicate-a-panel)
- [Remove panels](https://docs.wandb.ai/models/app/features/panels#remove-panels)
- [Manage sections](https://docs.wandb.ai/models/app/features/panels#manage-sections)
- [Add a section](https://docs.wandb.ai/models/app/features/panels#add-a-section)
- [Manage a section’s panels](https://docs.wandb.ai/models/app/features/panels#manage-a-section%E2%80%99s-panels)
- [Rename a section](https://docs.wandb.ai/models/app/features/panels#rename-a-section)
- [Delete a section](https://docs.wandb.ai/models/app/features/panels#delete-a-section)

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://docs.wandb.ai/llms.txt](https://docs.wandb.ai/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Use workspace panel visualizations to explore your [logged data](https://docs.wandb.ai/models/ref/python/experiments/run.md#method-runlog) by key, visualize the relationships between hyperparameters and output metrics, and more.

## [​](https://docs.wandb.ai/models/app/features/panels\#workspace-modes)  Workspace modes

W&B projects support two different workspace modes. The icon next to the workspace name shows its mode.

| Icon | Workspace mode |
| --- | --- |
| ![automated workspace icon](https://mintcdn.com/wb-21fd5541/qLByB7As1oRGQV3e/images/app_ui/automated_workspace.svg?fit=max&auto=format&n=qLByB7As1oRGQV3e&q=85&s=26e3b48cd9a1925c55ab3a28a38835da) | **Automated workspaces** automatically generate panels for all keys logged in the project. Choose an automatic workspace:<br>- To get started quickly by visualizing all available data for the project.<br>- For a smaller projects that log fewer keys.<br>- For more broad analysis.<br>If you delete a panel from an automatic workspace, you can use [Quick add](https://docs.wandb.ai/models/app/features/panels#quick-add) to recreate it. |
| ![manual workspace icon](https://mintcdn.com/wb-21fd5541/1haGbTiv4BKOT8E_/images/app_ui/manual_workspace.svg?fit=max&auto=format&n=1haGbTiv4BKOT8E_&q=85&s=04a2b6401c1203025a0c26215d2d936b) | **Manual workspaces** start as blank slates and display only those panels intentionally added by users. Choose a manual workspace:<br>- When you care mainly about a fraction of the keys logged in the project.<br>- For more focused analysis.<br>- To improve the performance of a workspace, avoiding loading panels that are less useful to you.<br>Use [Quick add](https://docs.wandb.ai/models/app/features/panels#quick-add) to easily populate a manual workspace and its sections with useful visualizations rapidly. |

To change how a workspace generates panels, [reset the workspace](https://docs.wandb.ai/models/app/features/panels#reset-a-workspace).

**Undo changes to your workspace**To undo changes to your workspace, click the Undo button (arrow that points left) or type **CMD + Z** (macOS) or **CTRL + Z** (Windows / Linux).

## [​](https://docs.wandb.ai/models/app/features/panels\#reset-a-workspace)  Reset a workspace

To reset a workspace:

1. At the top of the workspace, click the **action ()** menu.
2. Click **Reset workspace**.

## [​](https://docs.wandb.ai/models/app/features/panels\#configure-the-workspace-layout)  Configure the workspace layout

To configure the workspace layout, click **Settings** near the top of the workspace, then click **Workspace layout**.

- **Hide empty sections during search** (turned on by default)
- **Sort panels alphabetically** (turned off by default)
- **Section organization**(grouped by first prefix by default). To modify this setting:

1. Click the padlock icon.
2. Choose how to group panels within a section.

To configure defaults for the workspace’s line plots, refer to [Line plots](https://docs.wandb.ai/models/app/features/panels/line-plot).

### [​](https://docs.wandb.ai/models/app/features/panels\#configure-a-section%E2%80%99s-layout)  Configure a section’s layout

To configure the layout of a section, click its gear icon, then click **Display preferences**.

- **Turn on or off colored run names in tooltips** (turned on by default)
- **Only show highlighted run in companion chart tooltips** (turned off by default)
- **Number of runs shown in tooltips** (a single run, all runs, or **Default**)
- **Display full run names on the primary chart tooltip** (turned off by default)

## [​](https://docs.wandb.ai/models/app/features/panels\#view-a-panel-in-full-screen-mode)  View a panel in full-screen mode

In full-screen mode, the run selector displays and panels use full-fidelity sampling mode plots with 10,000 buckets, rather than 1000 buckets otherwise.To view a panel in full-screen mode:

1. Hover over the panel.
2. Click the panel’s **action ()** menu, then click the full-screen button, which looks like a viewfinder or an outline showing the four corners of a square.









![Full-screen panel](https://mintcdn.com/wb-21fd5541/UhAQoGpm-LvpH3-8/images/app_ui/panel_fullscreen.png?fit=max&auto=format&n=UhAQoGpm-LvpH3-8&q=85&s=3c4df45b1298d9af368b8275626a0d1c)

3. When you [share the panel](https://docs.wandb.ai/models/app/features/panels#share-a-panel) while viewing it in full-screen mode, the resulting link opens in full-screen mode automatically.

- To get back to a panel’s workspace from full-screen mode, click the left-pointing arrow at the top of the page.
- To navigate through a section’s panels without exiting full-screen mode, use either the **Previous** and **Next** buttons below the panel or the left and right arrow keys.
- To reclaim more space for the panel, minimize the run selector with **Cmd+.** (macOS) or **Ctrl+.** (Windows/Linux).
- When you view an image from a [media panel](https://docs.wandb.ai/models/app/features/panels/media) in full-screen mode, keyboard shortcuts can zoom in or out, reset zoom, or zoom to fit. See [Keyboard shortcuts](https://docs.wandb.ai/models/app/keyboard-shortcuts#media-panels).

See [Keyboard shortcuts](https://docs.wandb.ai/models/app/keyboard-shortcuts) for other full-screen and panel shortcuts.

## [​](https://docs.wandb.ai/models/app/features/panels\#add-panels)  Add panels

This section shows various ways to add panels to your workspace.

### [​](https://docs.wandb.ai/models/app/features/panels\#add-a-panel-manually)  Add a panel manually

Add panels to your workspace one at a time, either globally or at the section level.

1. To add a panel globally, click **Add panels** in the control bar near the panel search field.
2. To add a panel directly to a section instead, click the section’s **action ()** menu, then click **\+ Add panels**.
3. Select the type of panel to add, such as a chart. The panel’s configuration details appear, with defaults selected.
4. Optionally, customize the panel and its display preferences. Configuration options depend on the type of panel you select. To learn more about the options for each type of panel, refer to the relevant section below, such as [Line plots](https://docs.wandb.ai/models/app/features/panels/line-plot) or [Bar plots](https://docs.wandb.ai/models/app/features/panels/bar-plot).
5. Click **Apply**.

![Demo of adding a panel](https://mintcdn.com/wb-21fd5541/4kbs1cW6PdjDOqU3/images/app_ui/add_single_panel.gif?s=5aa84cadb20827b9035ad4da982bda82)

### [​](https://docs.wandb.ai/models/app/features/panels\#quick-add-panels)  Quick add panels

Use **Quick add** to add a panel automatically for each key you select, either globally or at the section level.

For an automated workspace with no deleted panels, the **Quick add** option is not visible because the workspace already includes panels for all logged keys. You can use **Quick add** to re-add a panel that you deleted.

1. To use **Quick add** to add a panel globally, click **Add panels** in the control bar near the panel search field, then click **Quick add**.
2. To use **Quick add** to add a panel directly to a section, click the section’s **action ()** menu, click **Add panels**, then click **Quick add**.
3. A list of panels appears. Each panel with a checkmark is already included in the workspace.
   - To add all available panels, click the **Add `<N>` panels** button at the top of the list. The **Quick Add** list closes and the new panels display in the workspace.
   - To add an individual panel from the list, hover over the panel’s row, then click **Add**. Repeat this step for each panel you want to add, then click the **X** at the top right to close the **Quick Add** list. The new panels display in the workspace.
4. Optionally, customize the panel’s settings.

## [​](https://docs.wandb.ai/models/app/features/panels\#share-a-panel)  Share a panel

This section shows how to share a panel using a link.To share a panel using a link, you can either:

- While viewing the panel in full-screen mode, copy the URL from the browser.
- Click the **action ()** menu and select **Copy panel URL**.

Share the link with the user or team. When they access the link, the panel opens in [full-screen mode](https://docs.wandb.ai/models/app/features/panels#view-a-panel-in-full-screen-mode).To return to a panel’s workspace from full-screen mode, click the left-pointing arrow at the top of the page.

### [​](https://docs.wandb.ai/models/app/features/panels\#compose-a-panel%E2%80%99s-full-screen-link-programmatically)  Compose a panel’s full-screen link programmatically

In certain situations, such as when [creating an automation](https://docs.wandb.ai/models/automations), it can be useful to include the panel’s full-screen URL. This section shows the format for a panel’s full-screen URL. In the following example, replace the entity, project, panel, and section names in brackets.

```
https://wandb.ai/<ENTITY_NAME>/<PROJECT_NAME>?panelDisplayName=<PANEL_NAME>&panelSectionName=<SECTON_NAME>
```

If multiple panels in the same section have the same name, this URL opens the first panel with the name.

### [​](https://docs.wandb.ai/models/app/features/panels\#embed-or-share-a-panel-on-social-media)  Embed or share a panel on social media

To embed a panel in a website or share it on social media, the panel must be viewable by anyone with the link. If a project is private, only members of the project can view the panel. If the project is public, anyone with the link can view the panel.To get the code to embed or share a panel on social media:

1. From the workspace, hover over the panel, then click its **action ()** menu.
2. Click the **Share** tab.
3. Change **Only those who are invited have access** to **Anyone with the link can view**. Otherwise, the choices in the next step are not available.
4. Choose **Share on Twitter**, **Share on Reddit**, **Share on LinkedIn**, or **Copy embed link**.

### [​](https://docs.wandb.ai/models/app/features/panels\#email-a-panel-report)  Email a panel report

To email a single panel as a stand-alone report:

1. Hover over the panel, then click the panel’s **action ()** menu.
2. Click **Share panel in report**.
3. Select the **Invite** tab.
4. Enter an email address or username.
5. Optionally, change **can view** to **can edit**.
6. Click **Invite**. W&B sends an email to the user with a clickable link to the report that contains only the panel you are sharing.

Unlike when you [share a panel](https://docs.wandb.ai/models/app/features/panels#share-a-panel), the recipient cannot get to the workspace from this report.

## [​](https://docs.wandb.ai/models/app/features/panels\#manage-panels)  Manage panels

### [​](https://docs.wandb.ai/models/app/features/panels\#edit-a-panel)  Edit a panel

To edit a panel:

1. Click its pencil icon.
2. Modify the panel’s settings.
3. To change the panel to a different type, select the type and then configure the settings.
4. Click **Apply**.

### [​](https://docs.wandb.ai/models/app/features/panels\#move-a-panel)  Move a panel

To move a panel to a different section, you can use the drag handle on the panel. To select the new section from a list instead:

1. If necessary, create a new section by clicking **Add section** after the last section.
2. Click the **action ()** menu for the panel.
3. Click **Move**, then select a new section.

You can also use the drag handle to rearrange panels within a section.

### [​](https://docs.wandb.ai/models/app/features/panels\#duplicate-a-panel)  Duplicate a panel

To duplicate a panel:

1. At the top of the panel, click the **action ()** menu.
2. Click **Duplicate**.

If desired, you can [customize](https://docs.wandb.ai/models/app/features/panels#edit-a-panel) or [move](https://docs.wandb.ai/models/app/features/panels#move-a-panel) the duplicated panel.

### [​](https://docs.wandb.ai/models/app/features/panels\#remove-panels)  Remove panels

To remove a panel:

1. Hover your mouse over the panel.
2. Select the **action ()** menu.
3. Click **Delete**.

To remove all panels from a manual workspace, click its **action ()** menu, then click **Clear all panels**.To remove all panels from an automatic or manual workspace, you can [reset the workspace](https://docs.wandb.ai/models/app/features/panels#reset-a-workspace). Select **Automatic** to start with the default set of panels, or select **Manual** to start with an empty workspace with no panels.

## [​](https://docs.wandb.ai/models/app/features/panels\#manage-sections)  Manage sections

By default, sections in a workspace reflect the logging hierarchy of your keys. However, in a manual workspace, sections appear only after you start adding panels.

### [​](https://docs.wandb.ai/models/app/features/panels\#add-a-section)  Add a section

To add a section, click **Add section** after the last section.To add a new section before or after an existing section, you can instead click the section’s **action ()** menu, then click **New section below** or **New section above**.

Do not name a section “Section”. Panels in this section will not render until the section is renamed, due to a known limitation.

### [​](https://docs.wandb.ai/models/app/features/panels\#manage-a-section%E2%80%99s-panels)  Manage a section’s panels

Sections with a large number of panels are paginated by default. The default number of panels on a page depend on the panel’s configuration and on the sizes of the panels in the section.

1. To resize a panel, hover over it, click the drag handle, and drag it to adjust the panel’s size. Resizing one panel resizes all panels in the section.
2. If a section is paginated, you can customize the number of panels to show on a page:
3. At the top of the section, click **1 to `<X>` of `<Y>`**, where `<X>` is the number of visible panels and `<Y>` is the total number of panels.
4. Choose how many panels to show per page, up to 100.
5. To delete a panel from a section:
6. Hover over the panel, then click its **action ()** menu.
7. Click **Delete**.

If you reset a workspace to an automated workspace, all deleted panels appear again.

### [​](https://docs.wandb.ai/models/app/features/panels\#rename-a-section)  Rename a section

To rename a section, click its **action ()** menu, then click **Rename section**.

Do not name a section “Section”. Panels in this section will not render until the section is renamed, due to a known limitation.

### [​](https://docs.wandb.ai/models/app/features/panels\#delete-a-section)  Delete a section

To delete a section, click the **action ()** menu, then click **Delete section**. This removes the section and its panels.

Was this page helpful?

YesNo

[Suggest edits](https://github.com/wandb/docs/edit/main/models/app/features/panels.mdx) [Raise issue](https://github.com/wandb/docs/issues/new?title=Issue%20on%20docs&body=Path:%20/models/app/features/panels)

[Manage workspace, section, and panel settings\\
\\
Previous](https://docs.wandb.ai/models/app/features/cascade-settings) [Line plots overview\\
\\
Next](https://docs.wandb.ai/models/app/features/panels/line-plot)

Ctrl+I

Assistant

Responses are generated using AI and may contain mistakes.

Cookie settings

![Full-screen panel](https://mintcdn.com/wb-21fd5541/UhAQoGpm-LvpH3-8/images/app_ui/panel_fullscreen.png?w=840&fit=max&auto=format&n=UhAQoGpm-LvpH3-8&q=85&s=927701d97526ed6ec6e086aaba51b503)

![Demo of adding a panel](https://mintcdn.com/wb-21fd5541/4kbs1cW6PdjDOqU3/images/app_ui/add_single_panel.gif?s=5aa84cadb20827b9035ad4da982bda82)

![](https://a.usbrowserspeed.com/ncs?pid=c59fd74cbd71a58d3d354f836099a3cf5a29e8461d70dad1ea560c3559765bfc&puid=usergems-ZCKXTHyLNZTV90gD-617b4b7bcb2a4a4883cf76f17d239372-617b4b7b_cb2a_4a48_83cf_76f17d239372-%252Fmodels%252Fapp%252Ffeatures%252Fpanels)