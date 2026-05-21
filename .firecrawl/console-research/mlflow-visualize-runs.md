[Skip to main content](https://docs.databricks.com/aws/en/mlflow/visualize-runs#__docusaurus_skipToContent_fallback)

On this page

Last updated on **Feb 25, 2026**

This page describes how to use the visualizations that are available for comparing runs in the MLflow UI. The chart view page shows a collection of charts comparing the runs of an experiment. You can customize this page by selecting runs to include, modifying charts, and creating new charts. With [MLflow 3](https://docs.databricks.com/aws/en/mlflow/mlflow-3-install), all of these features are available for models from the **Models** tab as well. For more details, see [Track and compare models using MLflow Logged Models](https://docs.databricks.com/aws/en/mlflow/logged-model).

MLflow metadata for experiments and runs is also available in system tables, where you can leverage [Databricks SQL](https://docs.databricks.com/aws/en/sql/) and all the lakehouse tooling Databricks offers to visualize your experiment data. See [MLflow system tables reference](https://docs.databricks.com/aws/en/admin/system-tables/mlflow) for further details.

To display the chart view page, click the **Chart view** icon on the experiment details page.

![Chart View button on experiment details page.](https://docs.databricks.com/aws/en/assets/images/chart-view-button-e2d30c8f59686aca8b076eb2914b0f99.png)

For information about the runs list page, see [View training results with MLflow runs](https://docs.databricks.com/aws/en/mlflow/runs). To display runs from multiple experiments, see [Compare runs from multiple experiments](https://docs.databricks.com/aws/en/mlflow/runs#compare-runs-from-multiple-expts).

## Chart overview [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#chart-overview "Direct link to Chart overview")

By default, charts on this page show the most recent 10 runs. As you roll your cursor over the lines on a chart, details for that run appear.

![Video showing that lines highlight and display details as you roll over them.](https://docs.databricks.com/aws/en/assets/images/chart-details-44dfcf8f9f8491a6797f1b26a57aa931.gif)

You can move or resize a chart, or enlarge it to full screen. A kebab menu at the upper-right of the chart lets you edit, delete, or download the chart.

![Chart controls including move, resize, and kebab menu.](https://docs.databricks.com/aws/en/assets/images/chart-controls-c801d1423fad98b81eec9e2250438b9e.png)

## Select runs to display [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#select-runs-to-display "Direct link to Select runs to display")

To select the number of runs to display, click ![Show run icon](<Base64-Image-Removed>) at the top of the list of runs.

![Use the show-hide run menu to show the first 10, 20, or all runs.](<Base64-Image-Removed>)

Runs that are shown on the charts are indicated by ![Show run icon](<Base64-Image-Removed>) and a colored dot. Runs not shown on the charts are indicated by ![Hide run icon](<Base64-Image-Removed>) and a grayed-out dot.

![Displayed and hidden runs.](https://docs.databricks.com/aws/en/assets/images/hidden-runs-2925ecb7e5efcd1559c9be8c3e4876f4.png)

## Manage runs [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#manage-runs "Direct link to Manage runs")

To delete, compare, or add or remove tags from a run, check the box next to the left of the run(s). When one or more runs is checked, the **Delete**, **Compare**, and **Add tags** buttons appear.

![Select runs to delete, compare, or add/edit tags.](https://docs.databricks.com/aws/en/assets/images/manage-run-buttons-4c3e71d9ee787ee73f0745c1e16559d1.png)

For details about the comparing runs page, see [Compare runs](https://docs.databricks.com/aws/en/mlflow/runs#compare-n-runs).

## Filter runs [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#filter-runs "Direct link to Filter runs")

Use the search field to the right of the **Chart view** icon to filter runs based on parameter or metric values or by tag. For details, see [Filter runs](https://docs.databricks.com/aws/en/mlflow/runs#filter-runs).

## Sort runs [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#sort-runs "Direct link to Sort runs")

To change the sort order of runs shown in the charts, select the parameter to sort by from the **Sort** dropdown menu.

![Use the sort dropdown menu to sort runs by a parameter.](https://docs.databricks.com/aws/en/assets/images/sort-dropdown-67b06bbf6d0451803a590401aae8a352.png)

## Group runs [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#group-runs "Direct link to Group runs")

To group runs by parameter value, select one or more parameters from the **Group by** dropdown menu.

![Use the Group by dropdown menu to group runs by parameter value.](https://docs.databricks.com/aws/en/assets/images/groupby-dropdown-5c85dd5f2bb300f3e76b30e662d9c9f3.png)

## Create new visualizations [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#create-new-visualizations "Direct link to Create new visualizations")

To add a chart, click **Add chart**, and select the type of chart to add from the dropdown menu.

![Add a new chart.](https://docs.databricks.com/aws/en/assets/images/add-chart-menu-9f4b19c9a05b417e2e9fafa3ecbb132c.png)

### Parallel coordinates chart [​](https://docs.databricks.com/aws/en/mlflow/visualize-runs\#parallel-coordinates-chart "Direct link to Parallel coordinates chart")

A parallel coordinates plot is useful in understanding the effect of parameter settings on model performance and investigating relationships between parameters and metrics. To create a parallel coordinates plot, select **Parallel coordinates** from the menu. In the dialog, select the parameters and metrics to investigate.

![Dialog to set up parallel coordinates plot.](https://docs.databricks.com/aws/en/assets/images/parallel-coord-dialog-34db3e1c86b31d6017afb3c85b0932aa.png)

In this example, the runs highlighted in the black boxes suggest that lower values for `max_depth` result in higher values for the metric `auc`.

![Example parallel coordinates plot.](https://docs.databricks.com/aws/en/assets/images/parallel-coord-plot-f39e2027c85d0bfbce0c6c44b79d9e49.png)

On this page

- [Chart overview](https://docs.databricks.com/aws/en/mlflow/visualize-runs#chart-overview)
- [Select runs to display](https://docs.databricks.com/aws/en/mlflow/visualize-runs#select-runs-to-display)
- [Manage runs](https://docs.databricks.com/aws/en/mlflow/visualize-runs#manage-runs)
- [Filter runs](https://docs.databricks.com/aws/en/mlflow/visualize-runs#filter-runs)
- [Sort runs](https://docs.databricks.com/aws/en/mlflow/visualize-runs#sort-runs)
- [Group runs](https://docs.databricks.com/aws/en/mlflow/visualize-runs#group-runs)
- [Create new visualizations](https://docs.databricks.com/aws/en/mlflow/visualize-runs#create-new-visualizations)
  - [Parallel coordinates chart](https://docs.databricks.com/aws/en/mlflow/visualize-runs#parallel-coordinates-chart)

Was this page helpful?

YesNo

Send feedback

Ask Genie

Open Genie

|     |     |
| --- | --- |
|  |  |