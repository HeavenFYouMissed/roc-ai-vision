- [Home](https://learnopencv.com/ "Home")
- \>
- [Computer Vision](https://learnopencv.com/category/computer-vision/ "Computer Vision")
- \>
- Introduction to OpenVINO Deep Learning Workbench

[Sovit Rath](https://learnopencv.com/author/sovit/), [Aditya Sharma](https://learnopencv.com/author/adityasharma/)

- on August 23, 2021

# Introduction to OpenVINO Deep Learning Workbench

The Intel-OpenVINO Toolkit provides many great functionalities for Deep-Learning model optimization, inference and deployment. Perhaps the most interesting and practical tool among them is the Deep-Learning (DL) workbench. Not only does model optimization, calibration, and quantization get easier, but the OpenVINO Deep Learning Workbench also makes the final model deployment-ready

- [Computer Vision](https://learnopencv.com/category/computer-vision/), [Deep Learning](https://learnopencv.com/category/deep-learning/), [Intel OpenVINO Toolkit](https://learnopencv.com/category/intel-openvino-toolkit/), [Model Optimization](https://learnopencv.com/category/model-optimization/), [Object Detection](https://learnopencv.com/category/object-detection/), [Post Training Quantization](https://learnopencv.com/category/post-training-quantization/)

![](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093449/Pg1-Introduction-to-OpenVINO-Deep-Learning-Workbench-FeatureImage-New-768x432.jpg)

The Intel-OpenVINO Toolkit provides many great functionalities for Deep-Learning model optimization, inference and deployment. Perhaps the most interesting and practical tool among them is the Deep-Learning (DL) workbench. Not only does model optimization, calibration, and quantization get easier, but the **_OpenVINO Deep Learning Workbench_** also makes the final model deployment-ready in a matter of minutes.

This post is the fourth in the **OpenVINO** series, which consists of the following posts:

1. [Introduction to OpenVINO Toolkit](https://learnopencv.com/introduction-to-intel-openvino-toolkit/)
2. [Post Training Quantization with OpenVino Toolkit](https://learnopencv.com/post-training-quantization-with-openvino-toolkit/)
3. [Running OpenVino Models on Intel Integrated GPU](https://learnopencv.com/running-openvino-models-on-intel-integrated-gpu/)
4. Introduction to OpenVino Deep Learning Workbench

1. [Introduction to the OpenVINO Deep-Learning Workbench](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#introduction "#introduction")
1. [Functionalities and Components](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#functionalities)
2. [Workflow of the Deep-Learning Workbench](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#workflow "#workflow")
2. [Installing the Deep-Learning Workbench through Docker](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#installing "#installing")
3. [AccuracyAwareQuantization Using DL Workbench](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#accuracy-aware-quantization "#accuracy-aware-quantization")
1. [Applying AccuracyAwareQuantization to Tiny YOLOv4, Using DL Workbench](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#applying-accuracy-aware-quantization "#applying-accuracy-aware-quantization")
4. [Run Inference, Using INT8-Calibrated Tiny-YOLOv4 model](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#run-inference "#run-inference")
5. [Comparing Performance of FP32 With INT8 DefaultQuantization and AccuracyAwareQuantization Model](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#comparison "#comparison")
6. [COCO Evaluation Results](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#coco-evaluation "#coco-evaluation")
7. [Summary](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#summary "#summary")

After going through this final post in the OpenVINO series, you should be able to use the Deep-Learning workbench for your own projects, and optimize your Deep-Learning models as required.

## Introduction to the OpenVINO Deep Learning Workbench

First, let’s understand what exactly is the DL workbench and why it’s important. It is a web-based application provided by the [Intel](https://learnopencv.com/introduction-to-intel-openvino-toolkit/ "") [OpenVINO toolkit](https://learnopencv.com/introduction-to-intel-openvino-toolkit/ "") that essentially runs in the browser. And it’s goal is to minimize the inference-to-deployment workflow timing for Deep-Learning models.

What makes it so useful?

- The DL workbench strongly integrates many of the optimization, quantization and deployment processes that OpenVINO supports, but are done manually.
- Its easy-to-use Graphical-User Interface lets you access almost everything, no need to bother what’s going on below the hood.
- You can not only import models and datasets, but also visualize and optimize these models.
- Even compare accuracies across various runs and parameters.
- Also, you can export the final model, which will be deployment-ready.

That’s not all, to learn what more you can do with the DL workbench, come let’s explore it well.

### Functionalities and Components of the OpenVINO Deep Learning Workbench

Okay, so let’s study the functionalities and components that make the DL workbench so special.

[![workflow of the Deep Learning workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093612/dl-workbnech-workflow-1024x384.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093612/dl-workbnech-workflow.jpg) [Source](https://docs.openvinotoolkit.org/latest/workbench_docs_Workbench_DG_Introduction.html)

The general workflow of the DL workbench showing the basic functionalities

Have a look at the above image to understand  the general workflow of the DL workbench and know the basic functionalities that come integrated with the following components:

1. Model Evaluator
2. Model Optimizer
3. Model Quantization Tool
4. Accuracy Checker
5. Deployment Package Manager

Now, let’s go over them in a bit more detail.

### 1\. Evaluating Model Performance

Like we already told you, the DL workbench allows you to import any model from its list of supported frameworks, which includes TensorFlow, ONNX, Caffe, MXNet and Kaldi. But did you know you can evaluate their performance too? Also, you can simply import a dataset of your choice, like the MS COCO or the PASCAL VOC dataset, and run an evaluation on them. Even if you do not have a standardized dataset at hand, you can always generate random data in the DL workbench itself.

Not only that, along with models from various frameworks, you can also import and evaluate models that are already in the OpenVINO-IR format.

### 2\. Analyzing the Model

Furthermore, you get a great environment and visualization tools to analyze the architecture of your imported models.

[![Analyzing quantized model layers in the openvino Deep Learning Workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093608/layer-wise-kernel-level-performance-fp32-1024x497.png)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093608/layer-wise-kernel-level-performance-fp32.png) Analyzing quantized model layers in the Deep Learning Workbench

You can check each layer, each operation in the layers, and even how much time each layer took for every single operation. Then use these insights to further optimize your model and make it all the better.

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2025/02/PyTorch_Bootcamp.jpg)\\
\\
10K+ Learners\\
\\
8 Hours of Learning\\
\\
**Join Free PyTorch Bootcamp**](https://opencv.org/university/free-pytorch-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/All-CV-Courses-Thumbnails-3.jpg)\\
\\
100K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free OpenCV Bootcamp**](https://opencv.org/university/free-opencv-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2025/02/PyTorch_Bootcamp.jpg)\\
\\
10K+ Learners\\
\\
8 Hours of Learning\\
\\
**Join Free PyTorch Bootcamp**](https://opencv.org/university/free-pytorch-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/All-CV-Courses-Thumbnails-3.jpg)\\
\\
100K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free OpenCV Bootcamp**](https://opencv.org/university/free-opencv-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=introduction-to-openvino-deep-learning-workbench)

[View all AI Free Courses](https://opencv.org/university/free-courses/?utm_source=lopcv&utm_medium=blog)

### 3\. Accuracy Evaluation

You can evaluate the accuracy of your imported models on various datasets, and this perhaps is the most important functionality available in the DL workbench.

![Initial accuracy evaluation by openvino Deep Learning Workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093611/first-eval-in-progess.jpg) Initial accuracy evaluation done by the Deep Learning Workbench

Simply choose the model, the target environment and the dataset on which to run the evaluation. And the workbench handles the rest. It will give you not only the accuracy, but also the FPS of the model for your chosen target environment. From there on, you decide whether to optimize it further or straightaway deploy.

### 4\. Model Tuning and Quantization

Be it DefaultQuantization or AccuracyAwareQuantization, the process of model quantization becomes easier and hassle-free with the DL workbench.

[![selection  of openvino quantization method.](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093613/accuracy-aware-screen-1024x558.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093613/accuracy-aware-screen.jpg) Selecting quantization method

You will recall that in our previous [Post Training Quantization](https://learnopencv.com/post-training-quantization-with-openvino-toolkit/) post, we followed a series of manual steps to convert an FP32-Tiny YOLOv4 model into an INT8-precision model. Besides having to take care of each step and configuration file, we also had to ensure that each path and command was correct. With the DL workbench, you need not worry about any of these. Just provide the FP32 model, the dataset, and the desired accuracy. Within minutes, you will have an optimizer and a quantized model ready for use.

**_Note:_** _Further down this post, you will learn how to quantize an FP32-Tiny YOLOv4 model into the INT8-precision format, using the AccuracyAwareQuantization with the DL workbench._

### 5\. Integration and Deployment

Lastly, the DL workbench also provides a final-deployable model which you can just click and export. The exported, deployable package will contain:

- the optimized model
- all the configuration files
- the results of the various runs and experiments that you carry out

With all this information, you can easily zero in on the best possible way to deploy your model, and know exactly how it will perform. You can integrate it within an application, deploy it on the edge, or simply decide to run inference using the model.

### Workflow of the Deep-Learning Workbench

The following diagram presents the workflow of the Deep-Learning workbench, illustrating all the steps, starting from model selection right up to model deployment:

[![general Workflow of the Deep-Learning Workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093610/general-workflow-1024x197.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093610/general-workflow-scaled.jpg) [Source](https://docs.openvinotoolkit.org/latest/workbench_docs_Workbench_DG_Introduction.html "https://docs.openvinotoolkit.org/latest/workbench_docs_Workbench_DG_Introduction.html")

As you can see, the general workflow consists of 7 steps. Let’s break these down into different components for greater clarity.

### 1\. Model selection

You always start by selecting the model you want to optimize.

![workflow select model](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093605/workflow-select-model.png) workflow select model

You are not constrained to choose a model from a specific framework, just ensure it comes from any of the OpenVINO-supported frameworks. You are even free to choose an OpenVINO-IR format model, provided you have already converted it using the Model Optimizer from any of the above frameworks.

### 2\. Selecting the Target Environment

Next, select the target environment. This is important because the DL workbench will then optimize the model to run best on this particular hardware environment.

![Select workflow environment in open vino Deep Learning workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093605/workflow-select-environment.png) Select workflow environment

The target environment can be:

- a CPU
- the Intel Integrated GPU
- even VPUs like the Myriad X

In fact, you can even target a remote environment, which is not local to the system in which you are running the DL workbench.

### 3\. Selecting the Dataset

Now that you’ve chosen the environment, select the dataset on which you want to run the evaluation and optimize the model.

![select dataset in open vino Deep Learning workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093605/workflow-select-dataset.png) Select Dataset

The DL workbench supports a number of datasets, including:

- Object-detection datasets like MS COCO, Pascal VOC
- Image-classification datasets like the ImageNet dataset
- Common Semantic Segmentation (CSS) dataset for semantic segmentation
- Common Super Resolution (CSR) dataset for super resolution, image inpainting and style transfer

### 4\. Model Optimization

After model and dataset selection, optimize your model.

![optimize dataset  in open vino Deep Learning workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093606/workflow-optimize-dataset.png) Optimize Dataset

Apply quantization to convert the FP32 models into INT8-precision models. Also, assess the precision of your model, so that you know for sure it will perform well in the real-world.

### 5\. Configure and Deploy

The final few steps will configure the model according to your requirements and deploy it.

![deploy workflow in open vino Deep Learning workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093606/workflow-deploy.png) deploy workflow

Experiment with different batch sizes and compare the throughput across different runs. When you get the desired tradeoff between speed and throughput:

- create the deployment package
- download it
- deploy it on an edge device

Don’t worry, we will be going through all these steps with you practically to ensure you get the required hands-on experience. We will convert the Tiny-YOLOv4 FP32 model into INT8 model, by applying AccuracyAwareQuantization, which itself will make most of these steps easy to grasp.

## Installing Deep-Learning Workbench Through Docker

Only after you install the Deep-Learning workbench in your system can you use it for optimization.

The easiest way to install it is through the Docker Hub. First, install the Docker Engine, that being a necessary prerequisite. **_Please follow the instructions_** [**_given here_**](https://docs.docker.com/engine/install/ubuntu/) **_to install the Docker Engine on Ubuntu_**.

Now, follow these steps to install the DL workbench through Docker Hub on Linux ( **_Ubuntu 20.04_**):

### 1\. Go inside the `workbench`Folder in the OpenVINO Installation Directory

|     |     |
| --- | --- |
| 1 | `cd /opt/intel/openvino_2021.3.394/deployment_tools/tools/workbench` |

### 2\. Download the Workbench Starting Script

To start the DL workbench, you need to download the script. Give the following command to download the starting script for DL workbench, inside the current working directory.

|     |     |
| --- | --- |
| 1 | `wget https://raw.githubusercontent.com/openvinotoolkit/workbench_aux/master/start_workbench.sh` |

### 3\. Enure the File is Executable

In many cases, execution permissions are disabled by default for security reasons. Execute the following command to ensure the script is executable.

|     |     |
| --- | --- |
| 1 | `chmod +x start_workbench.sh` |

### 4\. Run the DL Workbench

Finally, start the DL workbench through the terminal. `./start_workbench.sh`

You may need to wait for some time till the DL workbench is ready. After the script runs all the commands successfully, a prompt on the terminal informs you that the DL workbench is available on the local host at port 5665.

[http://127.0.0.1:5665/](http://127.0.0.1:5665/)

Open the link and you will see the following browser window:

[![Open vino workbench start page](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093607/workbench_start_page-1024x282.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093607/workbench_start_page.jpg) The initial window of the DL Workbench.

With this, you have completed the installation of DL workbench. Now, let’s quantize the models.

## AccuracyAwareQuantization Using Deep-Learning Workbench

The Deep-Learning workbench makes it really easy to quantize models from FP32-precision format to INT8 precision. In fact, it supports both DefaultQuantization and AccuracyAwareQuantization.

We already carried out DefaultQuantization in our [Post Training Quantization](https://learnopencv.com/post-training-quantization-with-openvino-toolkit/) post in this series. Here, we will use the DL workbench to apply AccuracyAwareQuantization to the TIny-YOLOv4 model. There are mainly two reasons for going this way:

1. The DL workbench will handle most of the heavy lifting that you otherwise would have to do manually.
2. As of now, the Tiny-YOLOv4 model is not fully supported by OpenVINO. So, using POT (Post Training Optimization Toolkit) to quantize the Tiny-YOLOv4 model from FP32 to INT8 will give error. You can however successfully complete this process using the DL workbench.

Even when using the DL workbench, there are a few caveats when it comes to applying AccuracyAwareQuantization to Tiny YOLOv4. One such issue is that the DL workbench will not give the accuracy of the quantized model or even the full-precision model due to partial support. The reasons will be clear as you carry out the process yourself.

### Applying AccuracyAwareQuantization to Tiny YOLOv4, Using DL Workbench

Just follow these steps:

#### 1\. Start the DL Workbench

First, open the DL workbench, by following all the steps discussed in the installation section.

![Openvino workbench active projects](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093606/workbench-active-projects.jpg) Checking active projects in the Deep Learning workbench

You will see the **Create** button at the top of the initial DL workbench window. Click on it.

#### 2\. Create Project

That takes you to the **Create Project** page, which should look similar to this:

![create project in the Openvino Deep Learning workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093612/create_project.jpg) Creating a project in the Deep Learning workbench

You need four things to successfully optimize a model, using the DL workbench:

1. The Deep-Learning model
2. The target environment
3. The target device or hardware
4. An evaluation dataset

Initially, all these options will be marked with red crosses (as in the above image), indicating that none of the requirements are met.

#### 3\. Import the Deep-Learning Model

Next, import the Deep-Learning model. For this example, we will be importing the Tiny-YOLOv4 FP32 model, which is already in the IR format.  So no need to run it through the model optimizer. Simply, click on the **Import** button, which should take you to the following screen:

![import model for quantization in Openvino workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093609/import-model.jpg) Importing the model for quantization

You will need both:

- The .`xml`file containing the network topology
- The .`bin`file containing the model weights

Finally, click on the **Import Model** button. The model might take some time to upload to the DL-workbench environment.

#### 4\. Select the Target Environment and Hardware

Then you need to select the target environment and hardware.

[![Select target hardware](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093607/target-hardware-1024x271.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093607/target-hardware.jpg) Selecting the target environment and hardware

For this example, we are using the **Local Environment** and the local CPU as our targets.

**_Note:_** _The CPU used here is an i7 8670H laptop, with a base clock_ speed of 2.3 GHz _and 16 GB of RAM._

When running on your local system, you should see your own CPU model and can choose accordingly.

#### 5\. Import the Evaluation Dataset

The final step before carrying out evaluation and optimization is to choose the evaluation dataset.

If you just needed to carry out the evaluation, you wouldn’t have to import any official dataset. Just creating a random dataset in the DL workbench would do. But here we need to quantize the model as well, so we will require a validation subset on which the quantized model can be evaluated.

![Import the dataset to evaluate the accuracy](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093610/import-data.jpg) Import the dataset to evaluate the accuracy

Here, we are importing the MS COCO 2017 validation dataset for evaluation purposes. This zip file contains 5000 validation images in total, along with their corresponding images. To download the dataset from the official website, click [here](https://cocodataset.org/#download).

After this, you should see a _green tick mark_ across all the requirements.

![workbench all tick marked](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093606/workbench-all-tick-marked.jpg)

These are all the things you need to start the model evaluation/optimization. Next, click on the **Create** button on the browser window to start the process.

#### **Initial Run Results**

After this, run one initial evaluation on the entire validation dataset, using the full-precision model we selected above. Check out the results in the following image:

![fp32 initial run results](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093611/fp32-initial-run-results.png) Initial run results

The above results are from the i7 CPU machine, as mentioned in the previous section. Your results may vary depending on the hardware. For the FP32 model, the initial is giving 27.6 FPS on average and latency of 35.17 milliseconds. It will be interesting to compare these results with the quantized model, after applying AccuracyAwareQuantization.

#### 6\. Optimize Performance

To start the AccuracyAwareQuantization:

- Click on the **Perform** tab on the current screen,
- Then click the **Optimize Performance** button. This should let you select the INT8-optimization method.

[![perform int8 qunatization](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093608/perform-int8-qunatization-1024x239.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093608/perform-int8-qunatization.jpg) Selecting the optimization method for quantization

- Next, click the **Optimize** button.

#### 7\. Configure the Accuracy Settings and Select Dataset-Subset Size

Now you need to configure the accuracy settings. It is safe to leave the settings to default value initially. You should see a screen similar to this:

[![configure accuracy](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093613/configure-accuracy-1024x256.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093613/configure-accuracy.jpg) Configuring the accuracy settings for AccuracyAwareQuantization

For the optimized configuration settings:

- The metric is mAP ( [mean Average Precision](https://learnopencv.com/mean-average-precision-map-object-detection-model-evaluation-metric/ ""))
- The dataset is COCO with 80 classes
- Both IOU ( [Intersection Over Union](https://learnopencv.com/intersection-over-union-iou-in-object-detection-and-segmentation/ "")) and NMS ( [Non Maximum Suppression](https://learnopencv.com/non-maximum-suppression-theory-and-implementation-in-pytorch/ "")) have 0.5 threshold
- The usage type is object detection, as we are using the Tiny YOLOv4 model
- The _Model Type_ is set to Tiny YOLOv2

As you must have noticed by now, instead of v3 or v4, the _Model Type_ is Tiny YOLOv2. Because the recent Tiny YOLO versions are not-fully supported by OpenVINO, we are bound to choose the Tiny YOLOv2 as the Model Type. Though this will not cause issues while quantizing the model, it will fail to give us any accuracy results. So, we will be checking the accuracy manually, by using the COCO evaluator to run evaluation manually on the entire COCO-validation set.

After completing the above settings, choose AccuracyAwareQuantization as the quantization method.

[![accuracy aware screen in Openvino workbench](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093614/accuracy_aware_screen_1-1024x558.jpg)](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093614/accuracy_aware_screen_1.jpg) Selecting the AccuracyAware method for quantization

For this example, we have set the _Max Accuracy Drop_ value to 0.5. This means, while doing INT8 calibration, if the accuracy drops below this specified threshold for any particular layer, then that layer will revert back to the original precision.

Also, take note of the dataset Subset Size. We are using just 20% of the 5000 images. So the calibration and evaluation will be done only on 1000 images. Selecting the whole dataset would have eaten up too much time, sometimes it takes hours to complete.

Finally, you can click on the **Optimize** button.

#### 8\. Check the Results

Let the calibration tool run the AccuracyAwareQuantization. It could take some time, depending on your hardware (CPU).

Here are the results from the i7 8th Generation 8670H CPU.

![accuracy aware int8 results](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093613/accuracy-aware-int8-results.png)

We got:

- 60 FPS, in terms of throughput
- The latency dropped to 17.66 milliseconds, after the INT8 calibration

This is a huge improvement compared to the 27 FPS and 35 milliseconds latency seen in the case of the full precision model.

We can also check the **Precision Distribution** in our calibrated model to ensure the model was actually converted to the INT8 format.

![precision distribution](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093608/precision-distribution.png)

You can see that:

- More than 97% of the execution time was spent in the INT8 layers
-  Around 2.7% of the time was spent in the FP32 layers

Okay, so this means most of the layers have been successfully converted to INT8 precision. Only a few layers reverted back to their original FP32-precision format, probably because they exceeded the accuracy-drop threshold of 0.5.

#### 9\. Export the Calibrated Model

The final step is to export your INT8-calibrated model so that you can run an inference with it.

![export package](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093611/export-package.jpg)

Go to the **Export Project** sub-tab on the Perform tab, choose the calibrated model, and click on the Export button. The downloaded file will contain the .xml as well as the .`bin`file.

## Run Inference Using INT8-Calibrated Tiny-YOLOv4 Model

Now that we have obtained the INT8 models from the above steps, let’s try running inference on the following video.

To run inference, we use almost the same commands as in the previous posts in this series. The only difference being the path to the INT8-calibrated model. As the input video also remains the same, we can compare performance with the FP32 model.

**Download Code**
To easily follow along this tutorial, please download code by clicking on the button below. It's FREE!

Download Code

![](https://cdn.learnopencv.com/wp-content/uploads/2021/10/04093413/cropped-favicon-512x512-1-150x150.png)

Click here to download the source code to this post

|     |     |
| --- | --- |
| 1 | `python object_detection_demo.py --model frozen_darknet_yolov4_model.xml -at yolo -i video_1.mp4 -t 0.5 -o acc_aw_int8_default.mp4` |

For the above run:

- the average FPS is 30.3
- latency is 27.3 milliseconds

This is very close to what the INT8 model with DefaultQuantization gave in the [**_Post Training Quantization post_**](https://learnopencv.com/post-training-quantization-with-openvino-toolkit/).

Now, check out the following video output:

The above results are really interesting. Not only are we getting throughput very similar to the Default Quantized INT8 model, but also the detections look exactly the same as the FP32 model. This means **we are getting good speed and accuracy at the same time**.

Let’s consider these graphs to double-check our speculations.

## Comparing Performance of FP32 With INT8 DefaultQuantization and AccuracyAwareQuantization Model

![fps comparison](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04104037/fps-comparison.png) FPS comparison between FP32 INT8 Default quantized model INT8 AccuracyAware quantized model![latency comparison](https://cdn.learnopencv.com/wp-content/uploads/2021/07/04093609/latency-comparison.png) Latency comparison between FP32 INT8 Default quantized model INT8 AccuracyAware quantized model

As you can see, the performance of INT8-DefaultQuantization Model and INT8-AccuracyAwareQuantization Model are running neck to neck. And in all the cases, the FPS is higher and latency is lower than the FP32 model.

Still, before we accept the AccuracyAwareQuantization-INT8 model as the final and best choice, it needs to pass one more test. So, let’s run the model evaluation on the entire COCO-evaluation dataset.

## COCO Evaluation Results

We already discussed the steps to configure and install the COCO evaluator in second post of the series.

After running the COCO evaluator, the 5000 images from the MS COCO validation set, we get the following results.

|     |     |
| --- | --- |
| 1<br>2<br>3 | `Average Precision  (AP) @[ IoU=0.50:0.95 | area=   all | maxDets=100 ] = 0.180`<br>`Average Precision  (AP) @[ IoU=0.50      | area=   all | maxDets=100 ] = 0.332`<br>`Average Precision  (AP) @[ IoU=0.75      | area=   all | maxDets=100 ] = 0.176` |

As you can see, **the mAP is 0.180,** which is:

- Even higher than the DefaultQuantization INT8 model that we saw in our Post-Training Quantization _post_.
- In fact, even higher than the FP32 model, which gave an mAP of 0.152.

This is quite amazing. Even the FPS in the above inference results almost matched that of the DefaultQuantization-INT8 model. It did fall short a bit, but the accuracy gain more than made up for that loss.

So, all in all, we can safely conclude that going with the AccuracyAwareQuantization Tiny YOLOv4 INT8 model turned out to be a good choice, both in terms of accuracy and speed.

## Summary

In this post, you got introduced to the OpenVINO-Deep Learning Workbench and learned to use it for model evaluation and optimization.

- First, you explored all the different components needed for optimizing a Deep-earning model, using the DL workbench.
- Next, you went through all the steps necessary for applying AccuracyAwareQuantization to the Tiny-YOLOv4 INT8 model.
- After that, you studied several inference-comparison graphs, with FP32 and the DefaultQuantization model
- Finally, you concluded that the AccuracyAware-quantized model is the best choice to go with. The COCO evaluation results also proved it to be the best model.

Now you have all the tools that you need to optimize your Deep-Learning models for best performance and accuracy. We cannot wait to see what cool applications you come up with, using the techniques you learned. While building that next big thing, if there’s a wall or gap in your knowledge, don’t lose heart. Invest in our [Deep Learning with PyTorch Course](https://opencv.org/courses/deep-learning-with-pytorch/) that takes care of basics as well as many advanced topics, and can make the next step easy for you.

Was This Article Helpful?

## Subscribe & Download Code

If you liked this article and would like to download code (C++ and Python) and example images used in this post, please click here. Alternately, sign up to receive a free Computer Vision Resource Guide. In our newsletter, we share OpenCV tutorials and examples written in C++/Python, and Computer Vision and Machine Learning algorithms and news.

Download Example Code

[PrevPreviousRunning OpenVINO Models on Intel Integrated GPU](https://learnopencv.com/running-openvino-models-on-intel-integrated-gpu/)

[NextReal-time style transfer in a Zoom meetingNext](https://learnopencv.com/real-time-style-transfer-in-a-zoom-meeting/)

Load Comments

### [How to Master YOLOE: Real-Time Open-Vocabulary Detection Made Easy](https://learnopencv.com/yoloe-tutorial-real-time-open-vocabulary-detection/)

Learn YOLOE for real-time open-vocabulary object detection and instance segmentation in Python with Ultralytics —

### [Vision Banana: How Image Generators Are Becoming Powerful Vision Models](https://learnopencv.com/vision-banana-explained/)

Vision Banana turns Nano Banana Pro into a powerful vision model for segmentation, depth estimation,

### [YOLO26 Keypoint Estimation: Real-Time Pose Estimation with Ultralytics](https://learnopencv.com/yolo26-pose-estimation-tutorial/)

Learn how to use YOLO26-pose with Python for real-time keypoint estimation on images and videos,

#### Table of Contents

1. [Introduction to the OpenVINO Deep Learning Workbench](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#introduction)

2. [Installing Deep-Learning Workbench Through Docker](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#installing)

3. [AccuracyAwareQuantization Using Deep-Learning Workbench](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#accuracy-aware-quantization)

4. [Run Inference Using INT8-Calibrated Tiny-YOLOv4 Model](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#run-inference)

5. [Comparing Performance of FP32 With INT8 DefaultQuantization and AccuracyAwareQuantization Model](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#comparison)

6. [COCO Evaluation Results](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#coco-evaluation)

7. [Summary](https://learnopencv.com/introduction-to-openvino-deep-learning-workbench/#summary)


Was This Article Helpful?

- [AccuracyAwareQuatization](https://learnopencv.com/tag/accuracyawarequatization/), [deep learning](https://learnopencv.com/tag/deep-learning/), [INT8](https://learnopencv.com/tag/int8/), [Model Quantization](https://learnopencv.com/tag/model-quantization/), [Object Detection](https://learnopencv.com/tag/object-detection/), [OpenVINO](https://learnopencv.com/tag/openvino/), [OpenVINO Deep Learning Workbench](https://learnopencv.com/tag/openvino-deep-learning-workbench/), [OpenVINO Toolkit](https://learnopencv.com/tag/openvino-toolkit/), [Tiny YOLOv4](https://learnopencv.com/tag/tiny-yolov4/)

## Read Next

[![YOLO26 Keypoint Estimation: Real-Time Pose Estimation with Ultralytics](https://cdn.learnopencv.com/wp-content/uploads/2026/04/04080555/Yolo26_Pose_Estimation.jpg)](https://learnopencv.com/yolo26-pose-estimation-tutorial/)

[Computer Vision](https://learnopencv.com/category/computer-vision/)[YOLO](https://learnopencv.com/category/yolo/)

[Sudip Chakrabarty](https://learnopencv.com/author/sudip/)
April 21, 2026

## [YOLO26 Keypoint Estimation: Real-Time Pose Estimation with Ultralytics](https://learnopencv.com/yolo26-pose-estimation-tutorial/)

Learn how to use YOLO26-pose with Python for real-time keypoint estimation on images and videos, understand its RLE-based architecture, and…

[![YOLO26 Instance Segmentation: Pixel-Perfect AI at Real-Time Speed](https://cdn.learnopencv.com/wp-content/uploads/2026/03/04103840/yolo26-instance-segmentation-hero-1.jpg)](https://learnopencv.com/yolo26-instance-segmentation-pixel-perfect-ai-at-real-time-speed/)

[Computer Vision](https://learnopencv.com/category/computer-vision/)[Image Segmentation](https://learnopencv.com/category/image-segmentation/)[Object Detection](https://learnopencv.com/category/object-detection/)[YOLO](https://learnopencv.com/category/yolo/)

[Sudip Chakrabarty](https://learnopencv.com/author/sudip/)
March 26, 2026

## [YOLO26 Instance Segmentation: Pixel-Perfect AI at Real-Time Speed](https://learnopencv.com/yolo26-instance-segmentation-pixel-perfect-ai-at-real-time-speed/)

Build a complete pipeline for YOLO26 instance segmentation, from image and video inference to custom dataset training and edge deployment.

[![Breaking the Bottleneck: Achieving Native NMS-Free Inference with YOLO26](https://cdn.learnopencv.com/wp-content/uploads/2026/02/04103842/yolo26.jpg)](https://learnopencv.com/yolo26-nms-free-inference/)

[YOLO](https://learnopencv.com/category/yolo/)

[Sudip Chakrabarty](https://learnopencv.com/author/sudip/)
February 10, 2026

## [Breaking the Bottleneck: Achieving Native NMS-Free Inference with YOLO26](https://learnopencv.com/yolo26-nms-free-inference/)

YOLO26 introduces a paradigm shift with native NMS-free inference. Discover how its One-to-One label assignment eliminates post-processing overhead for stable,…

## Subscribe to our Newsletter

Subscribe to our email newsletter to get the latest posts delivered right to your email.

Name

Email

Send