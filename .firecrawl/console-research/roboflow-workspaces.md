Everything in Roboflow follows this structure:

Workspace → Projects → Dataset Versions → Models → Workflows → Deployments

### [hashtag](https://docs.roboflow.com/workspaces/key-concepts\#workspaces)    **Workspaces**

A [Workspace](https://docs.roboflow.com/workspaces/roboflow-workspaces) is the top-level container.

- It's where you and your team collaborate.

- All Projects and Workflows live inside a Workspace

- Billing and [subscription plansarrow-up-right](https://roboflow.com/pricing) are managed at the Workspace level


Think of it like a company folder that holds all your computer vision work.

### [hashtag](https://docs.roboflow.com/workspaces/key-concepts\#projects)    Projects

A [Project](https://docs.roboflow.com/datasets/create-a-project) lives inside a Workspace. Each Project is built around a computer vision dataset. This is where you manage:

- Images

- Annotations

- Dataset updates over time


When you create a Project, you have to choose the Project type - one of the computer vision task type:

- Object detection

- Classification

- Instance segmentation

- Keypoint detection

- Semantic segmentation

- Multimodal


This determines how your data is structured and which model architectures you can train.

### [hashtag](https://docs.roboflow.com/workspaces/key-concepts\#dataset-versions)    **Dataset Versions**

A [Dataset Version](https://docs.roboflow.com/datasets/dataset-versions) is a snapshot of your dataset at a specific moment in time.

- You create a Version from the current state of your Project

- Once created, it does not change

- Any future edits to images or annotations will not affect existing Versions


Which ensures reproducibility, clear tracking, and helps with model comparison.

### [hashtag](https://docs.roboflow.com/workspaces/key-concepts\#models)    Models

[Models](https://docs.roboflow.com/train/train) are trained using Dataset Versions.

- You select a specific Dataset Version which will be used to train a model

- That model is permanently linked to that version

- Available model architectures for training will depend on your Project type


You can also [upload trained models to Roboflow](https://docs.roboflow.com/deploy/upload-custom-weights).

After you have a model, [create a workflow](https://docs.roboflow.com/workflows/create-a-workflow), or go [right to deploy](https://docs.roboflow.com/deploy/deployment-overview)!

[PreviousView Project Analyticschevron-left](https://docs.roboflow.com/universe/view-project-analytics) [NextCreate a Workspacechevron-right](https://docs.roboflow.com/workspaces/roboflow-workspaces)

Last updated 7 days ago

Was this helpful?

This site uses cookies to deliver its service and to analyze traffic. By browsing this site, you accept the [privacy policy](https://policies.gitbook.com/privacy/cookies).

close

AcceptReject

![Project Logo](https://media.roboflow.com/chat.png)

Ask AI