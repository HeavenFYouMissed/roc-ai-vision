[Jump to content](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_Dataflow_Compiler#bodyContent)

From RidgeRun Developer Wiki

< [Hailo](https://developer.ridgerun.com/wiki/index.php/Hailo "Hailo") ‎ \| [Hailo-8](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8 "Hailo/Hailo-8")

Follow us on: [![YouTube](https://developer.ridgerun.com/wiki/images/4/4f/YouTube_icon.svg)](https://www.youtube.com/@ridgerunengineering6760)[![Twitter](https://developer.ridgerun.com/wiki/images/6/63/Twitter-New.png)](https://x.com/RidgeRunX)[![LinkedIn](https://developer.ridgerun.com/wiki/images/c/cf/LinkedIn.jpeg)](https://www.linkedin.com/company/ridgerun/posts/?feedView=all)[![Email](https://developer.ridgerun.com/wiki/images/5/5b/EmailIcon3.png)](https://www.ridgerun.com/contact)![Share this page](https://developer.ridgerun.com/wiki/images/5/55/ShareIcon1.png)

### Share This Page

Facebook
Twitter
LinkedIn
WhatsApp

Email
Copy Link

Close

### Send Email Using

Gmail (Web)

Thunderbird (default mail app)


Close


[![Logo](https://developer.ridgerun.com/wiki/images/7/7f/Icon_FC.png)](https://developer.ridgerun.com/wiki/index.php/Hailo)

[⟵](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite) [⟶](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite_Installation)

‎

![Logo](https://developer.ridgerun.com/wiki/images/thumb/f/f3/Ridgerun-logo.svg/200px-Ridgerun-logo.svg.png)

[Hailo AI Platform Wiki](https://developer.ridgerun.com/wiki/index.php/Hailo)

Table of Contents
\[Sticky\]

- [Hailo-8](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8)

  - [Hailo-8 Overview](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8)
  - [Software and Tools](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite)

    - [Hailo AI Software Suite](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite)
    - [Hailo Dataflow Compiler](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_Dataflow_Compiler)
    - [Hailo AI Software Suite Installation](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite_Installation)
    - [Hailo TAPPAS Installation](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_TAPPAS_Installation)
    - [Hailo Commands](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_Commands)
    - [Hailo Model Scripts](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_Model_Scripts)

- [Hailo-15](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15)

  - [Hailo-15 Overview](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15)
  - [Hardware Setup](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Hardware_Setup/Hailo-15_SBC_(Single-Board-Computer))

    - [Hailo-15 SBC (Single-Board-Computer)](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Hardware_Setup/Hailo-15_SBC_(Single-Board-Computer))
    - [UART Connection Setup and Serial Monitoring](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Hardware_Setup/Serial_Monitoring)

  - [Image Building](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Image_Building/Building_a_Yocto_Image_for_Hailo-15)

    - [Building a Yocto Image for Hailo-15](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Image_Building/Building_a_Yocto_Image_for_Hailo-15)
    - [Creating a Bootable Micro SD Card for Hailo-15](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Image_Building/Creating_a_Bootable_Micro_SD_Card_for_Hailo-15)
    - [Booting a Yocto Image on the Hailo-15 from an SD Card](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/Image_Building/Booting_a_Yocto_Image_on_the_Hailo-15_from_an_SD_Card)

  - [AI Applications](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/AI_Applications/Run_an_Example_Application)

    - [Run an Example Application](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-15/AI_Applications/Run_an_Example_Application)

- [**Hailo Available Source Code**](https://developer.ridgerun.com/wiki/index.php/Hailo/Available_Source_Code)
- [**Contact Us**](https://www.ridgerun.com/contact)
- [**Sponsor your Favorite Feature**](https://developer.ridgerun.com/wiki/index.php/Sponsor_Projects)

## Hailo Dataflow Compiler

The Hailo Dataflow Compiler toolchain enables users to generate a Hailo Executable Format (HEF) binary file from various inputs, including TensorFlow checkpoints, frozen TensorFlow graphs, TFLite files, and ONNX files. The build process consists of several stages: translating the original model into a Hailo-compatible format, optimizing model parameters, and compiling the model into the final binary file.

The diagram illustrates the model-building process, starting with a TensorFlow or ONNX model and culminating in the Hailo binary (HEF).

[![](https://developer.ridgerun.com/wiki/images/f/fe/Dataflowcompiler.png)](https://developer.ridgerun.com/wiki/index.php/File:Dataflowcompiler.png) Model building process using the Hailo Dataflow Compiler. Source: Hailo Dataflow Compiler User Guide

As illustrated in the figure, the model-building process consists of several key steps:

### 1\. TensorFlow and ONNX Translation

The process begins by converting the user’s original model into a Hailo-compatible format. The translation API takes the model and generates an internal Hailo representation (HAR compressed file), which includes an HN model file (in JSON format) and a NumPy NPZ file containing the weights.

### 2\. Profiler

The Profiler tool utilizes the HAR file to assess the model’s expected performance on the hardware. This profiling evaluates the required devices, hardware resource utilization, throughput (frames per second), and provides a detailed breakdown for each layer of the model.

### 3\. Emulator

The emulator allows users to perform inference on their model without needing actual hardware. It operates in two modes:

#### Native Mode

Runs the model with float32 parameters for validating the translation process and calibration.

#### Quantized Mode

Simulates hardware implementation to analyze the accuracy of the optimized model.

### 4\. Model Optimization

During model optimization, parameters are converted from float32 to int8. This is achieved by running the model in native mode on a small set of images to collect activation statistics. The calibration module then generates a new network configuration for the 8-bit representation, including int8 weights, biases, scaling, and hardware configuration.

### 5\. Compiling the Model into a Binary Image

The model is compiled into a hardware-compatible binary format (HEF). The Dataflow Compiler Tool allocates hardware resources to maximize frames per second while balancing resource allocation. The compilation process, including microcode generation, is automated and can be initiated with a single API call.

### 6\. Dataflow Compiler Studio (Preview - Parsing Stage Only)

The Dataflow Compiler Studio allows users to parse and visualize neural network graphs. Users can upload ONNX or TFLite files, and the tool suggests start and end nodes for parsing. The GUI offers a side-by-side comparison of Hailo’s parsed graph and the original graph, enabling users to adjust and re-parse as necessary to meet specific requirements.

### 7\. Deployment Process

After compilation, the model is ready for inference on the target device. The HailoRT library, accessible via C/C++, Python APIs, and command-line tools, provides the necessary interface to load and run the model. Depending on the device and connection type (e.g., PCIe or Ethernet), the library employs various communication methods to interact with the device. The HailoRT library can be installed on the same machine as the Dataflow Compiler or on a separate machine, with a Yocto layer provided for easy integration into embedded environments.

[![Logo](https://developer.ridgerun.com/wiki/images/7/7f/Icon_FC.png)](https://developer.ridgerun.com/wiki/index.php/Hailo)

[⟵](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite) [⟶](https://developer.ridgerun.com/wiki/index.php/Hailo/Hailo-8/AI_Software_and_Tools/Hailo_AI_Software_Suite_Installation)

❯

[Have a Question? Ask Us!](https://www.ridgerun.com/contact "Contact Our Experts") [Explore Our Engineering Services](https://www.ridgerun.com/engineering-services "Discover RidgeRun Engineering Solutions")

Retrieved from " [https://developer.ridgerun.com/wiki/index.php?title=Hailo/Hailo-8/AI\_Software\_and\_Tools/Hailo\_Dataflow\_Compiler&oldid=64219](https://developer.ridgerun.com/wiki/index.php?title=Hailo/Hailo-8/AI_Software_and_Tools/Hailo_Dataflow_Compiler&oldid=64219)"

[Category](https://developer.ridgerun.com/wiki/index.php/Special:Categories "Special:Categories"):

- [Hailo](https://developer.ridgerun.com/wiki/index.php/Category:Hailo "Category:Hailo")

Cookies help us deliver our services. By using our services, you agree to our use of cookies.

OK

Toggle limited content width