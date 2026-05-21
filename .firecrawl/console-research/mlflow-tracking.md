[Skip to main content](https://mlflow.org/docs/latest/ml/tracking/#__docusaurus_skipToContent_fallback)

Copy Page

Copy Raw MarkdownCopy page as Markdown for LLMs

View MarkdownView markdown file in new tab

Reference in ChatGPTAsk questions about this page

Reference in ClaudeAsk questions about this page

On this page

The MLflow Tracking is an API and UI for logging parameters, code versions, metrics, and output files
when running your machine learning code and for later visualizing the results.
MLflow Tracking provides [Python](https://mlflow.org/docs/latest/api_reference/python_api/index.html#)
, [REST](https://mlflow.org/docs/latest/api_reference/rest-api.html#)
, [R](https://mlflow.org/docs/latest/api_reference/R-api.html#),
and [Java](https://mlflow.org/docs/latest/api_reference/java_api/index.html#) APIs.

![](https://mlflow.org/docs/latest/assets/images/tracking-metrics-ui-temp-ffc0da57b388076730e20207dbd7f9c4.png)

A screenshot of the MLflow Tracking UI, showing a plot of validation loss metrics during model training.

## Quickstart [​](https://mlflow.org/docs/latest/ml/tracking/\#quickstart "Direct link to Quickstart")

If you haven't used MLflow Tracking before, we strongly recommend going through the following quickstart tutorial.

[MLflow Tracking Quickstart\\
\\
A great place to start to learn the fundamentals of MLflow Tracking! Learn in 5 minutes how to log, register, and load a model for inference.](https://mlflow.org/docs/latest/ml/tracking/quickstart/)

## Concepts [​](https://mlflow.org/docs/latest/ml/tracking/\#concepts "Direct link to Concepts")

### Runs [​](https://mlflow.org/docs/latest/ml/tracking/\#runs "Direct link to Runs")

MLflow Tracking is organized around the concept of **runs**, which are executions of some piece of
data science code, for example, a single `python train.py` execution. Each run records metadata
(various information about your run such as metrics, parameters, start and end times) and artifacts
(output files from the run such as model weights, images, etc).

### Models [​](https://mlflow.org/docs/latest/ml/tracking/\#models "Direct link to Models")

Models represent the trained machine learning artifacts that are produced during your runs. Logged Models contain their own metadata and artifacts similar to runs.

### Experiments [​](https://mlflow.org/docs/latest/ml/tracking/\#experiments "Direct link to Experiments")

An experiment groups together runs and models for a specific task. You can create an experiment using the CLI, API, or UI.
The MLflow API and UI also let you create and search for experiments. See [Organizing Runs into Experiments](https://mlflow.org/docs/latest/ml/tracking/tracking-api/#experiment-organization)
for more details on how to organize your runs into experiments.

## Tracking Runs [​](https://mlflow.org/docs/latest/ml/tracking/\#start-logging "Direct link to Tracking Runs")

[MLflow Tracking APIs](https://mlflow.org/docs/latest/ml/tracking/tracking-api/) provide a set of functions to track your runs. For example, you can call [`mlflow.start_run()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.start_run) to start a new run,
then call [Logging Functions](https://mlflow.org/docs/latest/ml/tracking/tracking-api/) such as [`mlflow.log_param()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.log_param) and [`mlflow.log_metric()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.log_metric) to log parameters and metrics respectively.
Please visit the [Tracking API documentation](https://mlflow.org/docs/latest/ml/tracking/tracking-api/) for more details about using these APIs.

python

```python
import mlflow

with mlflow.start_run():

    mlflow.log_param("lr", 0.001)

    # Your ml code

    ...

    mlflow.log_metric("val_loss", val_loss)
```

Alternatively, [Auto-logging](https://mlflow.org/docs/latest/ml/tracking/autolog/) offers an ultra-quick setup for starting MLflow tracking.
This powerful feature allows you to log metrics, parameters, and models without the need for explicit log statements -
all you need to do is call [`mlflow.autolog()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.autolog) before your training code. Auto-logging supports popular
libraries such as [Scikit-learn](https://mlflow.org/docs/latest/ml/tracking/autolog/#autolog-sklearn), [XGBoost](https://mlflow.org/docs/latest/ml/tracking/autolog/#autolog-xgboost), [PyTorch](https://mlflow.org/docs/latest/ml/tracking/autolog/#autolog-pytorch),
[Keras](https://mlflow.org/docs/latest/ml/tracking/autolog/#autolog-keras), [Spark](https://mlflow.org/docs/latest/ml/tracking/autolog/#autolog-spark), and more. See [Automatic Logging Documentation](https://mlflow.org/docs/latest/ml/tracking/autolog/)
for supported libraries and how to use auto-logging APIs with each of them.

python

```python
import mlflow

mlflow.autolog()

# Your training code...
```

note

By default, without any particular server/database configuration, MLflow Tracking logs data to the local `mlruns` directory.
If you want to log your runs to a different location, such as a remote database and cloud storage, in order to share your results with
your team, follow the instructions in the [Set up MLflow Tracking Environment](https://mlflow.org/docs/latest/ml/tracking/#tracking-setup) section.

### Searching Logged Models Programmatically [​](https://mlflow.org/docs/latest/ml/tracking/\#search_logged_models "Direct link to Searching Logged Models Programmatically")

MLflow 3 introduces powerful model search capabilities through [`mlflow.search_logged_models()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.search_logged_models). This API allows you to find specific models across your experiments based on performance metrics, parameters, and model attributes using SQL-like syntax.

python

```python
import mlflow

# Find high-performing models across experiments

top_models = mlflow.search_logged_models(

    experiment_ids=["1", "2"],

    filter_string="metrics.accuracy > 0.95 AND params.model_type = 'RandomForest'",

    order_by=[{"field_name": "metrics.f1_score", "ascending": False}],

    max_results=5,

)

# Get the best model for deployment

best_model = mlflow.search_logged_models(

    experiment_ids=["1"],

    filter_string="metrics.accuracy > 0.9",

    max_results=1,

    order_by=[{"field_name": "metrics.accuracy", "ascending": False}],

    output_format="list",

)[0]

# Load the best model directly

loaded_model = mlflow.pyfunc.load_model(f"models:/{best_model.model_id}")
```

**Key Features:**

- **SQL-like filtering**: Use `metrics.`, `params.`, and attribute prefixes to build complex queries
- **Dataset-aware search**: Filter metrics based on specific datasets for fair model comparison
- **Flexible ordering**: Sort by multiple criteria to find the best models
- **Direct model loading**: Use the new `models:/<model_id>` URI format for immediate model access

For comprehensive examples and advanced search patterns, see the [Search Logged Models Guide](https://mlflow.org/docs/latest/ml/search/search-models/).

### Querying Runs Programmatically [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking_query_api "Direct link to Querying Runs Programmatically")

You can also access all of the functions in the Tracking UI programmatically with [MlflowClient](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.client.html#mlflow.client.MlflowClient).

For example, the following code snippet search for runs that has the best validation loss among all runs in the experiment.

python

```python
client = mlflow.tracking.MlflowClient()

experiment_id = "0"

best_run = client.search_runs(experiment_id, order_by=["metrics.val_loss ASC"], max_results=1)[0]

print(best_run.info)

# {'run_id': '...', 'metrics': {'val_loss': 0.123}, ...}
```

## Tracking Models [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking-models "Direct link to Tracking Models")

MLflow 3 introduces enhanced model tracking capabilities that allow you to log multiple model checkpoints within a single run and track their performance against different datasets. This is particularly useful for deep learning workflows where you want to save and compare model checkpoints at different training stages.

### Logging Model Checkpoints [​](https://mlflow.org/docs/latest/ml/tracking/\#logging-model-checkpoints "Direct link to Logging Model Checkpoints")

You can log model checkpoints at different steps during training using the `step` parameter in model logging functions. Each logged model gets a unique model ID that you can use to reference it later.

python

```python
import mlflow

import mlflow.pytorch

with mlflow.start_run() as run:

    for epoch in range(100):

        # Train your model

        train_model(model, epoch)

        # Log model checkpoint every 10 epochs

        if epoch % 10 == 0:

            model_info = mlflow.pytorch.log_model(

                pytorch_model=model,

                name=f"checkpoint-epoch-{epoch}",

                step=epoch,

                input_example=sample_input,

            )

            # Log metrics linked to this specific model checkpoint

            accuracy = evaluate_model(model, validation_data)

            mlflow.log_metric(

                key="accuracy",

                value=accuracy,

                step=epoch,

                model_id=model_info.model_id,  # Link metric to specific model

                dataset=validation_dataset,

            )
```

### Linking Metrics to Models and Datasets [​](https://mlflow.org/docs/latest/ml/tracking/\#linking-metrics-to-models-and-datasets "Direct link to Linking Metrics to Models and Datasets")

MLflow 3 allows you to link metrics to specific model checkpoints and datasets, providing better traceability of model performance:

python

```python
# Create a dataset reference

train_dataset = mlflow.data.from_pandas(train_df, name="training_data")

# Log metric with model and dataset links

mlflow.log_metric(

    key="f1_score",

    value=0.95,

    step=epoch,

    model_id=model_info.model_id,  # Links to specific model checkpoint

    dataset=train_dataset,  # Links to specific dataset

)
```

### Searching and Ranking Model Checkpoints [​](https://mlflow.org/docs/latest/ml/tracking/\#searching-and-ranking-model-checkpoints "Direct link to Searching and Ranking Model Checkpoints")

Use [`mlflow.search_logged_models()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.search_logged_models) to search and rank model checkpoints based on their performance metrics:

python

```python
# Search for all models in a run, ordered by accuracy

ranked_models = mlflow.search_logged_models(

    filter_string=f"source_run_id='{run.info.run_id}'",

    order_by=[{"field_name": "metrics.accuracy", "ascending": False}],

    output_format="list",

)

# Get the best performing model

best_model = ranked_models[0]

print(f"Best model: {best_model.name}")

print(f"Accuracy: {best_model.metrics[0].value}")

# Load the best model for inference

loaded_model = mlflow.pyfunc.load_model(f"models:/{best_model.model_id}")
```

### Model URIs in MLflow 3 [​](https://mlflow.org/docs/latest/ml/tracking/\#model-uris-in-mlflow-3 "Direct link to Model URIs in MLflow 3")

MLflow 3 introduces a new model URI format that uses model IDs instead of run IDs, providing more direct model referencing:

python

```python
# New MLflow 3 model URI format

model_uri = f"models:/{model_info.model_id}"

loaded_model = mlflow.pyfunc.load_model(model_uri)

# This replaces the older run-based URI format:

# model_uri = f"runs:/{run_id}/model_path"
```

This new approach provides several advantages:

- **Direct model reference**: No need to know the run ID and artifact path
- **Better model lifecycle management**: Each model checkpoint has its own unique identifier
- **Improved model comparison**: Easily compare different checkpoints within the same run
- **Enhanced traceability**: Clear links between models, metrics, and datasets

## Tracking Datasets [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking-datasets "Direct link to Tracking Datasets")

MLflow offers the ability to track datasets that are associated with model training events. These metadata associated with the Dataset can be stored through the use of the [`mlflow.log_input()`](https://mlflow.org/docs/latest/api_reference/python_api/mlflow.html#mlflow.log_input) API.
To learn more, please visit the [MLflow data documentation](https://mlflow.org/docs/latest/ml/dataset/) to see the features available in this API.

## Explore Runs, Models, and Results [​](https://mlflow.org/docs/latest/ml/tracking/\#explore-runs-models-and-results "Direct link to Explore Runs, Models, and Results")

### Tracking UI [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking_ui "Direct link to Tracking UI")

The Tracking UI lets you visually explore your experiments, runs, and models, as shown on top of this page.

- Experiment-based run listing and comparison (including run comparison across multiple experiments)
- Searching for runs by parameter or metric value
- Visualizing run metrics
- Downloading run results (artifacts and metadata)

These features are available for models as well, as shown below.

![MLflow UI Experiment view page models tab](https://mlflow.org/docs/latest/assets/images/tracking-models-ui-0f88d40c517e103cdead462aab12781a.png)

A screenshot of the MLflow Tracking UI on the models tab, showing a list of models under the experiment.

If you log runs to a local `mlruns` directory, run the following command in the directory above it,
then access [http://127.0.0.1:5000](http://127.0.0.1:5000/) in your browser.

bash

```bash
mlflow server --port 5000
```

Alternatively, the [MLflow Tracking Server](https://mlflow.org/docs/latest/ml/tracking/#tracking_server) serves the same UI and enables remote
storage of run artifacts. In that case, you can view the UI at `http://<IP address of your MLflow tracking server>:5000`
from any machine that can connect to your tracking server.

## Set up the MLflow Tracking Environment [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking-setup "Direct link to Set up the MLflow Tracking Environment")

note

If you just want to log your experiment data and models to local files, you can skip this section.

MLflow Tracking supports many different scenarios for your development workflow. This section will guide you through how to set up the MLflow Tracking environment for your particular use case.
From a bird's-eye view, the MLflow Tracking environment consists of the following components.

### Components [​](https://mlflow.org/docs/latest/ml/tracking/\#components "Direct link to Components")

#### [MLflow Tracking APIs](https://mlflow.org/docs/latest/ml/tracking/tracking-api/) [​](https://mlflow.org/docs/latest/ml/tracking/\#mlflow-tracking-apis "Direct link to mlflow-tracking-apis")

You can call MLflow Tracking APIs in your ML code to log runs and communicate with the MLflow Tracking Server if necessary.

#### [Backend Store](https://mlflow.org/docs/latest/self-hosting/architecture/backend-store/) [​](https://mlflow.org/docs/latest/ml/tracking/\#backend-store "Direct link to backend-store")

The backend store persists various metadata for each [Run](https://mlflow.org/docs/latest/ml/tracking/#runs), such as run ID, start and end times, parameters, metrics, etc.
MLflow supports two types of storage for the backend: **file-system-based** like local files and **database-based** like PostgreSQL.

Additionally, if you are interfacing with a managed service (such as Databricks or Azure Machine Learning), you will be interfacing with a
REST-based backend store that is externally managed and not directly accessible.

#### [Artifact Store](https://mlflow.org/docs/latest/self-hosting/architecture/artifact-store/) [​](https://mlflow.org/docs/latest/ml/tracking/\#artifact-stores "Direct link to artifact-stores")

Artifact store persists (typically large) artifacts for each run, such as model weights (e.g. a pickled scikit-learn model),
images (e.g. PNGs), model and data files (e.g. [Parquet](https://parquet.apache.org/) file). MLflow stores artifacts in a
local file (`mlruns`) by default, but also supports different storage options such as Amazon S3 and Azure Blob Storage.

For models which are logged as MLflow artifacts, you can refer the model through a model URI of format: `models:/<model_id>`,
where 'model\_id' is the unique identifier assigned to the logged model. This replaces the older `runs:/<run_id>/<artifact_path>` format
and provides more direct model referencing.

If the model is registered in the [MLflow Model Registry](https://mlflow.org/docs/latest/ml/model-registry/),
you can also refer to the model through a model URI of format: `models:/<model-name>/<model-version>`,
see [MLflow Model Registry](https://mlflow.org/docs/latest/ml/model-registry/) for details.

#### [MLflow Tracking Server](https://mlflow.org/docs/latest/self-hosting/architecture/tracking-server/) (Optional) [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking_server "Direct link to tracking_server")

MLflow Tracking Server is a stand-alone HTTP server that provides REST APIs for accessing backend and/or artifact store.
Tracking server also offers flexibility to configure what data to serve, govern access control, versioning, and etc. Read
[MLflow Tracking Server documentation](https://mlflow.org/docs/latest/self-hosting/) for more details.

### Common Setups [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking_setup "Direct link to Common Setups")

By configuring these components properly, you can create an MLflow Tracking environment suitable for your team's development workflow.
The following diagram and table show a few common setups for the MLflow Tracking environment.

![](https://mlflow.org/docs/latest/assets/images/tracking-setup-overview-3d8cfd511355d9379328d69573763331.png)

|  | 1\. Localhost (default) | 2\. Local Tracking with Local Database | 3\. Remote Tracking with [MLflow Tracking Server](https://mlflow.org/docs/latest/ml/tracking/#tracking_server) |
| --- | --- | --- | --- |
| Scenario | Solo development | Solo development | Team development |
| Use Case | By default, MLflow records metadata and artifacts for each run to a local directory, `mlruns`. This is the simplest way to get started with MLflow Tracking, without setting up any external server, database, and storage. | The MLflow client can interface with a SQLAlchemy-compatible database (e.g., SQLite, PostgreSQL, MySQL) for the [backend](https://mlflow.org/docs/latest/self-hosting/architecture/backend-store/). Saving metadata to a database allows you cleaner management of your experiment data while skipping the effort of setting up a server. | MLflow Tracking Server can be configured with an artifacts HTTP proxy, passing artifact requests through the tracking server to store and retrieve artifacts without having to interact with underlying object store services. This is particularly useful for team development scenarios where you want to store artifacts and experiment metadata in a shared location with proper access control. |
| Tutorial | [QuickStart](https://mlflow.org/docs/latest/ml/tracking/quickstart/) | [Tracking Experiments using a Local Database](https://mlflow.org/docs/latest/ml/tracking/tutorials/local-database/) | [Remote Experiment Tracking with MLflow Tracking Server](https://mlflow.org/docs/latest/ml/tracking/tutorials/remote-server/) |

## Other Configuration with [MLflow Tracking Server](https://mlflow.org/docs/latest/ml/tracking/\#tracking_server) [​](https://mlflow.org/docs/latest/ml/tracking/\#other-tracking-setup "Direct link to other-tracking-setup")

MLflow Tracking Server provides customizability for other special use cases. Please follow
[Remote Experiment Tracking with MLflow Tracking Server](https://mlflow.org/docs/latest/ml/tracking/tutorials/remote-server/) for learning
the basic setup and continue to the following materials for advanced configurations to meet your needs.

- Local Tracking Server
- Artifacts-only Mode
- Direct Access to Artifacts

#### Using MLflow Tracking Server Locally [​](https://mlflow.org/docs/latest/ml/tracking/\#using-mlflow-tracking-server-locally "Direct link to Using MLflow Tracking Server Locally")

You can of course run MLflow Tracking Server locally. While this doesn't provide much additional benefit over directly using
the local files or database, might useful for testing your team development workflow locally or running your machine learning
code on a container environment.

![](https://mlflow.org/docs/latest/assets/images/tracking-setup-local-server-cd51180e89bfd0a18c52f5b33e0f188d.png)

#### Running MLflow Tracking Server in Artifacts-only Mode [​](https://mlflow.org/docs/latest/ml/tracking/\#running-mlflow-tracking-server-in-artifacts-only-mode "Direct link to Running MLflow Tracking Server in Artifacts-only Mode")

MLflow Tracking Server has an `--artifacts-only` option which allows the server to handle (proxy) exclusively artifacts, without permitting
the processing of metadata. This is particularly useful when you are in a large organization or are training extremely large models. In these scenarios, you might have high artifact
transfer volumes and can benefit from splitting out the traffic for serving artifacts to not impact tracking functionality. Please read
[Optionally using a Tracking Server instance exclusively for artifact handling](https://mlflow.org/docs/latest/self-hosting/architecture/tracking-server/#tracking-server-artifacts-only)
for more details on how to use this mode.

![](https://mlflow.org/docs/latest/assets/images/tracking-setup-artifacts-only-f9630e7e6dc87eab52eea8f85a706382.png)

#### Disable Artifact Proxying to Allow Direct Access to Artifacts [​](https://mlflow.org/docs/latest/ml/tracking/\#disable-artifact-proxying-to-allow-direct-access-to-artifacts "Direct link to Disable Artifact Proxying to Allow Direct Access to Artifacts")

MLflow Tracking Server, by default, serves both artifacts and only metadata. However, in some cases, you may want
to allow direct access to the remote artifacts storage to avoid the overhead of a proxy while preserving the functionality
of metadata tracking. This can be done by disabling artifact proxying by starting server with `--no-serve-artifacts` option.
Refer to [Use Tracking Server without Proxying Artifacts Access](https://mlflow.org/docs/latest/self-hosting/architecture/tracking-server/#tracking-server-no-proxy)
for how to set this up.

![](https://mlflow.org/docs/latest/assets/images/tracking-setup-no-serve-artifacts-9e21c03b857275a42dc667e4454fba37.png)

## FAQ [​](https://mlflow.org/docs/latest/ml/tracking/\#faq "Direct link to FAQ")

### Can I launch multiple runs in parallel? [​](https://mlflow.org/docs/latest/ml/tracking/\#can-i-launch-multiple-runs-in-parallel "Direct link to Can I launch multiple runs in parallel?")

Yes, MLflow supports launching multiple runs in parallel e.g. multi processing / threading.
See [Launching Multiple Runs in One Program](https://mlflow.org/docs/latest/ml/tracking/tracking-api/#parallel-execution-strategies) for more details.

### How can I organize many MLflow Runs neatly? [​](https://mlflow.org/docs/latest/ml/tracking/\#how-can-i-organize-many-mlflow-runs-neatly "Direct link to How can I organize many MLflow Runs neatly?")

MLflow provides a few ways to organize your runs:

- [Organize runs into experiments](https://mlflow.org/docs/latest/ml/tracking/tracking-api/#experiment-organization) \- Experiments are logical containers for your runs. You can create an experiment using the CLI, API, or UI.
- [Create child runs](https://mlflow.org/docs/latest/ml/tracking/tracking-api/#hierarchical-runs-with-parent-child-relationships) \- You can create child runs under a single parent run to group them together. For example, you can create a child run for each fold in a cross-validation experiment.
- [Add tags to runs](https://mlflow.org/docs/latest/ml/tracking/tracking-api/#smart-tagging-for-organization) \- You can associate arbitrary tags with each run, which allows you to filter and search runs based on tags.

### Can I directly access remote storage without running the Tracking Server? [​](https://mlflow.org/docs/latest/ml/tracking/\#can-i-directly-access-remote-storage-without-running-the-tracking-server "Direct link to Can I directly access remote storage without running the Tracking Server?")

Yes, while it is best practice to have the MLflow Tracking Server as a proxy for artifacts access for team development workflows, you may not need that
if you are using it for personal projects or testing. You can achieve this by following the workaround below:

1. Set up artifacts configuration such as credentials and endpoints, just like you would for the MLflow Tracking Server.
See [configure artifact storage](https://mlflow.org/docs/latest/self-hosting/architecture/artifact-store/#artifacts-store-supported-storages) for more details.
2. Create an experiment with an explicit artifact location,

python

```python
experiment_name = "your_experiment_name"

mlflow.create_experiment(experiment_name, artifact_location="s3://your-bucket")

mlflow.set_experiment(experiment_name)
```

Your runs under this experiment will log artifacts to the remote storage directly.

#### How to integrate MLflow Tracking with [Model Registry](https://mlflow.org/docs/latest/ml/model-registry/)? [​](https://mlflow.org/docs/latest/ml/tracking/\#tracking-with-model-registry "Direct link to tracking-with-model-registry")

To use the Model Registry functionality with MLflow tracking, you **must use database backed store** such as PostgresQL and log a model using the `log_model` methods of the corresponding model flavors.
Once a model has been logged, you can add, modify, update, or delete the model in the Model Registry through the UI or the API.
See [Backend Stores](https://mlflow.org/docs/latest/self-hosting/architecture/backend-store/) and [Common Setups](https://mlflow.org/docs/latest/self-hosting/architecture/overview/#common-setups) for how to configures backend store properly for your workflow.

#### How to include additional description texts about the run? [​](https://mlflow.org/docs/latest/ml/tracking/\#how-to-include-additional-description-texts-about-the-run "Direct link to How to include additional description texts about the run?")

A system tag `mlflow.note.content` can be used to add descriptive note about this run. While the other [system tags](https://mlflow.org/docs/latest/ml/tracking/tracking-api/#system-tags-reference) are set automatically,
this tag is **not set by default** and users can override it to include additional information about the run. The content will be displayed on the run's page under
the Notes section.

- [Quickstart](https://mlflow.org/docs/latest/ml/tracking/#quickstart)
- [Concepts](https://mlflow.org/docs/latest/ml/tracking/#concepts)
  - [Runs](https://mlflow.org/docs/latest/ml/tracking/#runs)
  - [Models](https://mlflow.org/docs/latest/ml/tracking/#models)
  - [Experiments](https://mlflow.org/docs/latest/ml/tracking/#experiments)
- [Tracking Runs](https://mlflow.org/docs/latest/ml/tracking/#start-logging)
  - [Searching Logged Models Programmatically](https://mlflow.org/docs/latest/ml/tracking/#search_logged_models)
  - [Querying Runs Programmatically](https://mlflow.org/docs/latest/ml/tracking/#tracking_query_api)
- [Tracking Models](https://mlflow.org/docs/latest/ml/tracking/#tracking-models)
  - [Logging Model Checkpoints](https://mlflow.org/docs/latest/ml/tracking/#logging-model-checkpoints)
  - [Linking Metrics to Models and Datasets](https://mlflow.org/docs/latest/ml/tracking/#linking-metrics-to-models-and-datasets)
  - [Searching and Ranking Model Checkpoints](https://mlflow.org/docs/latest/ml/tracking/#searching-and-ranking-model-checkpoints)
  - [Model URIs in MLflow 3](https://mlflow.org/docs/latest/ml/tracking/#model-uris-in-mlflow-3)
- [Tracking Datasets](https://mlflow.org/docs/latest/ml/tracking/#tracking-datasets)
- [Explore Runs, Models, and Results](https://mlflow.org/docs/latest/ml/tracking/#explore-runs-models-and-results)
  - [Tracking UI](https://mlflow.org/docs/latest/ml/tracking/#tracking_ui)
- [Set up the MLflow Tracking Environment](https://mlflow.org/docs/latest/ml/tracking/#tracking-setup)
  - [Components](https://mlflow.org/docs/latest/ml/tracking/#components)
  - [Common Setups](https://mlflow.org/docs/latest/ml/tracking/#tracking_setup)
- [Other Configuration with MLflow Tracking Server](https://mlflow.org/docs/latest/ml/tracking/#other-tracking-setup)
- [FAQ](https://mlflow.org/docs/latest/ml/tracking/#faq)
  - [Can I launch multiple runs in parallel?](https://mlflow.org/docs/latest/ml/tracking/#can-i-launch-multiple-runs-in-parallel)
  - [How can I organize many MLflow Runs neatly?](https://mlflow.org/docs/latest/ml/tracking/#how-can-i-organize-many-mlflow-runs-neatly)
  - [Can I directly access remote storage without running the Tracking Server?](https://mlflow.org/docs/latest/ml/tracking/#can-i-directly-access-remote-storage-without-running-the-tracking-server)

![brand logo](https://mlflow.org/img/mlflow-favicon.ico)Ask AI