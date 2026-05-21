[![logo](https://services.dev.arxiv.org/html/static/arxiv-logomark-small-white.svg)Back to arXiv](https://arxiv.org/)

[Back to abstract page](https://arxiv.org/abs/2504.21415v1)

[![logo](https://services.dev.arxiv.org/html/static/arxiv-logo-one-color-white.svg)Back to arXiv](https://arxiv.org/)

This is **experimental HTML** to improve accessibility. We invite you to report rendering errors. Use Alt+Y to toggle on accessible reporting links and Alt+Shift+Y to toggle off. Learn more [about this project](https://info.arxiv.org/about/accessible_HTML.html) and [help improve conversions](https://info.arxiv.org/help/submit_latex_best_practices.html).


[Why HTML?](https://info.arxiv.org/about/accessible_HTML.html) [Report Issue](https://arxiv.org/html/2504.21415v1/#myForm) [Back to Abstract](https://arxiv.org/abs/2504.21415v1) [Download PDF](https://arxiv.org/pdf/2504.21415v1)

## Table of Contents

01. [Abstract](https://arxiv.org/html/2504.21415v1#abstract "Abstract")
02. [I Introduction](https://arxiv.org/html/2504.21415v1#S1 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
03. [II Related Work](https://arxiv.org/html/2504.21415v1#S2 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
04. [III Analysis of Mouse Dynamic Data](https://arxiv.org/html/2504.21415v1#S3 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
05. [IV Appropriate Volume of Mouse Dynamic Data Determination](https://arxiv.org/html/2504.21415v1#S4 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
06. [V Mouse Authentication Unit Length Determination](https://arxiv.org/html/2504.21415v1#S5 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
07. [VI Local-Time Mouse Authentication (LTMouseAuthen)](https://arxiv.org/html/2504.21415v1#S6 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")    1. [VI-AResNet Block for Local Features](https://arxiv.org/html/2504.21415v1#S6.SS1 "In VI Local-Time Mouse Authentication (LTMouseAuthen) ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
    2. [VI-BGRU for Time Series Context Information](https://arxiv.org/html/2504.21415v1#S6.SS2 "In VI Local-Time Mouse Authentication (LTMouseAuthen) ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
    3. [VI-CTransferring and Training](https://arxiv.org/html/2504.21415v1#S6.SS3 "In VI Local-Time Mouse Authentication (LTMouseAuthen) ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
08. [VII Experiment](https://arxiv.org/html/2504.21415v1#S7 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")    1. [VII-AExperimental Setting](https://arxiv.org/html/2504.21415v1#S7.SS1 "In VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
    2. [VII-BProper Volume of Data](https://arxiv.org/html/2504.21415v1#S7.SS2 "In VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
    3. [VII-CMouse Authentication Unit Length and Model Performance Trade-off](https://arxiv.org/html/2504.21415v1#S7.SS3 "In VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
    4. [VII-DComparison with Other Models](https://arxiv.org/html/2504.21415v1#S7.SS4 "In VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
    5. [VII-EAttack Model](https://arxiv.org/html/2504.21415v1#S7.SS5 "In VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
09. [VIII Discussion](https://arxiv.org/html/2504.21415v1#S8 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
10. [IX Conclusion](https://arxiv.org/html/2504.21415v1#S9 "In Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges")
11. [References](https://arxiv.org/html/2504.21415v1#bib "References")

[License: CC BY-NC-ND 4.0](https://info.arxiv.org/help/license/index.html#licenses-available)

arXiv:2504.21415v1 \[cs.CR\] 30 Apr 2025

# Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges

Report issue for preceding element

Yi Wang
The University of Tokyo

Tokyo, Japan

yiwangyww@gmail.com
Chenyv Wu
Wuhan, China

wcy981021@gmail.com
Yang Liao
Xi’an Jiaotong Univeristy

Xi’an, China

ly905650639@gmail.com
Maowei You

Beijing, China

maowei.you@foxmail.com

Report issue for preceding element

###### Abstract

Report issue for preceding element

User authentication is essential to ensure secure access to computer systems, yet traditional methods face limitations in usability, cost, and security. Mouse dynamics authentication, based on the analysis of users’ natural interaction behaviors with mouse devices, offers a cost-effective, non-intrusive, and adaptable solution. However, challenges remain in determining the optimal data volume, balancing accuracy and practicality, and effectively capturing temporal behavioral patterns. In this study, we propose a statistical method using Gaussian kernel density estimate (KDE) and Kullback-Leibler (KL) divergence to estimate the sufficient data volume for training authentication models. We introduce the Mouse Authentication Unit (MAU), leveraging Approximate Entropy (ApEn) to optimize segment length for efficient and accurate behavioral representation. Furthermore, we design the Local-Time Mouse Authentication (LT-AMouse) framework, integrating 1D-ResNet for local feature extraction and GRU for modeling long-term temporal dependencies. Taking the Balabit and DFL datasets as examples, we significantly reduced the data scale, particularly by a factor of 10 for the DFL dataset, greatly alleviating the training burden. Additionally, we determined the optimal input recognition unit length for the user authentication system on different datasets based on the slope of Approximate Entropy. Training with imbalanced samples, our model achieved a successful defense AUC 98.52%percent98.5298.52\\%98.52 % for blind attack on the DFL dataset and 94.65%percent94.6594.65\\%94.65 % on the Balabit dataset, surpassing the current sota performance.

Report issue for preceding element

###### Index Terms:

Report issue for preceding element
User Authentication, Pattern Recognition

## I Introduction

Report issue for preceding element

User authentication is essential to ensure secure access to computer systems and prevent unauthorized usage\[ [1](https://arxiv.org/html/2504.21415v1#bib.bib1 "")\]. Traditional authentication methods, such as passwords, auxiliary devices, and biometric recognition, have several limitations. Passwords are susceptible to being guessed, forgotten, or reused across multiple accounts, leading to security vulnerabilities \[ [2](https://arxiv.org/html/2504.21415v1#bib.bib2 "")\]. Auxiliary devices, such as security tokens, can be costly, prone to loss or theft, and add complexity to the user experience. Biometric recognition, such as facial recognition, while more secure, faces challenges such as being tricked by photos or fake videos, high costs, privacy concerns, and varying accuracy due to environmental factors and user appearance changes. In contrast, mouse dynamics, which involves analyzing user behavior through mouse movement patterns, offers a promising alternative. As a means of secondary auxiliary authentication, mouse dynamics authentication is difficult to replicate, protects user privacy, and does not require additional hardware, making it a cost-effective, non-intrusive, and highly adaptable solution. The collection of mouse dynamics data is imperceptible to users and does not disrupt their normal operations or user experience \[ [3](https://arxiv.org/html/2504.21415v1#bib.bib3 "")\]. Using the natural interactive behaviors of users, this approach eliminates the need for additional user actions. Consequently, it improves the convenience and fluency of the user experience while simultaneously ensuring system security.

Report issue for preceding element

Using computers or other devices equipped with touchpads or mouse input systems, we define authorized users of these devices as legitimate users, while all other individuals are considered unauthorized users. To achieve mouse-dynamics-based authentication, researchers worldwide have invested significant effort and resources into collecting data sets related to mouse dynamics\[ [39](https://arxiv.org/html/2504.21415v1#bib.bib39 "")\], in order to identify user behavior patterns and design user authentication systems. Mouse dynamics datasets typically record behavioral data in time series when users interact with mouse devices. The datasets include cursor positions, kinematic features such as speed and acceleration, and event data such as single-click actions, double-click actions, and scroll wheel events. Given these datasets, existing research has proposed various methods for identifying unauthorized users based on mouse dynamics\[ [9](https://arxiv.org/html/2504.21415v1#bib.bib9 ""), [10](https://arxiv.org/html/2504.21415v1#bib.bib10 ""), [11](https://arxiv.org/html/2504.21415v1#bib.bib11 ""), [40](https://arxiv.org/html/2504.21415v1#bib.bib40 ""), [26](https://arxiv.org/html/2504.21415v1#bib.bib26 ""), [12](https://arxiv.org/html/2504.21415v1#bib.bib12 ""), [19](https://arxiv.org/html/2504.21415v1#bib.bib19 ""), [21](https://arxiv.org/html/2504.21415v1#bib.bib21 ""), [22](https://arxiv.org/html/2504.21415v1#bib.bib22 ""), [23](https://arxiv.org/html/2504.21415v1#bib.bib23 ""), [24](https://arxiv.org/html/2504.21415v1#bib.bib24 ""), [25](https://arxiv.org/html/2504.21415v1#bib.bib25 ""), [41](https://arxiv.org/html/2504.21415v1#bib.bib41 "")\],. These methods generally involve two steps: first, extracting hand-crafted features from mouse dynamic sequences; second, applying machine learning or deep learning techniques to classify these features for user authentication.

Report issue for preceding element

However, existing mouse dynamics-based behavioral authentication systems face the following key challenges:

Report issue for preceding element

(i) Determining the appropriate amount of data for effective user authentication remains unresolved. Similar questions have been explored in other fields \[ [43](https://arxiv.org/html/2504.21415v1#bib.bib43 ""), [44](https://arxiv.org/html/2504.21415v1#bib.bib44 ""), [45](https://arxiv.org/html/2504.21415v1#bib.bib45 "")\], but not in mouse dynamics authentication. To address this, we propose a method for estimating the required dataset size, avoiding issues of insufficient or excessive data, and providing guidance for experiment design.

Report issue for preceding element

(ii) The length of data segments significantly affects recognition accuracy and real-time performance. Short segments (1–2 seconds) improve responsiveness but lack sufficient behavioral information, reducing accuracy. Longer segments (30 seconds or more) capture richer features but are impractical in scenarios requiring real-time performance.

Report issue for preceding element

(iii) Mouse dynamics data includes dimensions like time, speed, acceleration, and direction, often noisy and redundant. Traditional models, such as SVMs and Decision Trees, rely on manually extracted features, which are limited to basic statistics and fail to capture complex behavioral patterns.

Report issue for preceding element

As shown in Figure [1](https://arxiv.org/html/2504.21415v1#S1.F1 "Figure 1 ‣ I Introduction ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), in this study, we employ statistical methods to address the data volume required for mouse user behavior authentication, aiming to achieve a balance between accuracy and practicality. We propose Local-Time Mouse Authentication (LT-MAuthen), a mouse user verification framework that integrates both local and long-term temporal information.

Report issue for preceding element

To construct a user authentication model based on mouse dynamics, we discuss and analyze the raw data content, collection environments, and dataset sizes of different types of mouse dynamics datasets. To ensure adaptability across various devices, reduce model parameter complexity, and enhance inference speed, we compute mouse movement velocity as an input variable for the user authentication system. Furthermore, we propose a general statistical method to determine the appropriate total data volume for modeling user mouse behavior. This method utilizes Gaussian Kernel Density Estimation (KDE) and evaluates the similarity between two density functions with different data volumes using the Kullback–Leibler (KL) divergence. If adding more data results in only minimal changes to the density function—indicated by a KL divergence below a predefined threshold, it suggests that the additional data contributes no new information, and the current data volume is deemed sufficient.

Report issue for preceding element

In practical model training and system deployment, the collected mouse dynamics data must be segmented into multiple short sequences, which serve as inputs for user authentication. We define a Mouse Authentication Unit (MAU) as a smaller, independent temporal sequence segment extracted from continuous mouse trajectory data. Each MAU represents an analyzable behavioral fragment containing sufficient dynamic information to support feature extraction and pattern recognition for user identity verification. To ensure that each MAU encapsulates adequate information without being excessively long—thus compromising system efficiency—we introduce the concept of Approximate Entropy (ApEn) to measure the information content of authentication units. As the length of the MAU increases, approximate entropy decreases while the information content rises, enhancing the discriminability of user behavior. This approach enables a flexible trade-off between authentication speed and recognition accuracy, dynamically determining the optimal MAU length to meet system performance requirements across various application scenarios.

Report issue for preceding element

Additionally, to effectively integrate both local and global features of mouse movement sequences, we design a two-step scheme called Local-Time Mouse Authentication. In the first step, a 1D-ResNet is employed to analyze the local features of mouse velocity sequences. In the second step, the trained blocks of the 1D-ResNet are transferred and combined with a GRU (Gated Recurrent Unit) to capture the temporal characteristics within mouse velocity sequences.

Report issue for preceding element

Our main contributions are shown in below:

Report issue for preceding element

1. 1.


We propose a statistical method using KDE and KL divergence to determine the optimal data volume for mouse dynamics authentication model training.

Report issue for preceding element

2. 2.


We introduce the Mouse Authentication Unit (MAU) with Approximate Entropy (ApEn) to balance authentication accuracy and efficiency.

Report issue for preceding element

3. 3.


We design the LTMouseAuthen framework, combining 1D-ResNet for local feature extraction and GRU for temporal pattern modeling.

Report issue for preceding element


The remainder of this paper is organized as follows: Section 2 reviews related work in biometric authentication and mouse dynamics. Section 3 presents our analysis of mouse dynamic data. Sections 4 and 5 detail our methods for determining appropriate data volume and MAU length, respectively. Section 6 describes the proposed LTMouseAuthen framework. Section 7 presents comprehensive experimental results and comparisons. Section 8 discusses limitations and future directions, followed by conclusions in Section 9.

Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/structure.png)Figure 1: Structure of the PaperReport issue for preceding elementTABLE I: MAJOR MOUSE DYNAMICS DATASETS

|     |     |     |     |     |
| --- | --- | --- | --- | --- |
| Dataset Name | Time | Content | User Amount | Environment |
| Mouse-Behavior Data forContinuous Authentication \[ [49](https://arxiv.org/html/2504.21415v1#bib.bib49 "")\] | 2012 | Timestamp, Click, State, ID, X and Y | 28 | Experiment Collection Software |
| Balabit \[ [39](https://arxiv.org/html/2504.21415v1#bib.bib39 "")\] | 2016 | Timestamp, Click, State, X and Y | 10 | Daily Usage |
| DFL \[ [40](https://arxiv.org/html/2504.21415v1#bib.bib40 "")\] | 2018 | Timestamp, Click, State, X and Y | 21 | Experiment Collection Software |
| Minecraft-Mouse-Dynamic-Dataset \[ [22](https://arxiv.org/html/2504.21415v1#bib.bib22 "")\] | 2022 | Timestamp, Click, Scroll, State, ID, X and Y | 10 | Daily Usage |

Report issue for preceding element

## II Related Work

Report issue for preceding element

Biometric-based User Authentication Biometric features like keystroke dynamics, mouse movements, and user-system interactions \[ [51](https://arxiv.org/html/2504.21415v1#bib.bib51 "")\] have been widely used for user authentication. Early work, such as typing patterns \[ [52](https://arxiv.org/html/2504.21415v1#bib.bib52 "")\], dates back to the 1990s. With the growth of large datasets, machine learning methods have enhanced biometric identification. Bailey \[ [15](https://arxiv.org/html/2504.21415v1#bib.bib15 "")\] used a GUI to collect keystroke and mouse dynamics data, applying deep learning for identity verification. Meng \[ [16](https://arxiv.org/html/2504.21415v1#bib.bib16 "")\] proposed touch gesture-based authentication on mobile devices, using dynamic training to adapt to data variations. Buriro \[ [17](https://arxiv.org/html/2504.21415v1#bib.bib17 "")\] combined micro-movements, touch strokes, and facial features with random forests and MLPs for classification.

Report issue for preceding element

Other biometrics, such as fingerprint recognition \[ [59](https://arxiv.org/html/2504.21415v1#bib.bib59 "")\] and voice biometrics \[ [60](https://arxiv.org/html/2504.21415v1#bib.bib60 "")\], also offer reliable authentication. Recent studies combine these with behavioral features, like keystroke dynamics and facial recognition, to improve robustness \[ [61](https://arxiv.org/html/2504.21415v1#bib.bib61 "")\].

Report issue for preceding element

User Authentication on Mouse Dynamics Ahmed et al. \[ [24](https://arxiv.org/html/2504.21415v1#bib.bib24 "")\] first applied machine learning to mouse dynamics, achieving a FAR of 2.46%percent\\%% and FRR of 2.46%percent\\%%. Shen et al. \[ [41](https://arxiv.org/html/2504.21415v1#bib.bib41 ""), [49](https://arxiv.org/html/2504.21415v1#bib.bib49 "")\] used PCA and stream learning to address behavior interference, while Xu et al. \[ [50](https://arxiv.org/html/2504.21415v1#bib.bib50 "")\] applied random forests to reduce overfitting. More recent approaches, such as \[ [25](https://arxiv.org/html/2504.21415v1#bib.bib25 "")\], categorize users into groups to reduce authentication time, and \[ [26](https://arxiv.org/html/2504.21415v1#bib.bib26 "")\] used CNNs for deeper feature extraction. Penny et al. \[ [10](https://arxiv.org/html/2504.21415v1#bib.bib10 "")\] combined CNNs and RNNs for enhanced feature capture. Margit et al. \[ [12](https://arxiv.org/html/2504.21415v1#bib.bib12 "")\] released the SapiMouse dataset, and Siddiqui et al. \[ [22](https://arxiv.org/html/2504.21415v1#bib.bib22 "")\] introduced a dataset from Minecraft to avoid dataset homogeneity. However, challenges remain, including the complexity of multidimensional data, non-standardized sequence lengths, and the need for better feature extraction methods.

Report issue for preceding element

## III Analysis of Mouse Dynamic Data

Report issue for preceding element

Mouse dynamics datasets shown in Table [I](https://arxiv.org/html/2504.21415v1#S1.T1 "TABLE I ‣ I Introduction ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") are widely utilized in behavioral biometric authentication as well as in cognitive and psychological research, which are primarily collected from two distinct environments:

Report issue for preceding element

1. 1.


Daily usage environments

Report issue for preceding element

2. 2.


Controlled laboratory environments, with standardized mouse task software

Report issue for preceding element


![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/data_count_balabite_before.png)Figure 2: Amount of Individual User Mouse Behavior in the Balabit DatasetReport issue for preceding element![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/data_count_DFL_before.png)Figure 3: Amount of Individual User Mouse Behavior in the DFL DatasetReport issue for preceding element

Compared to laboratory environments, collecting mouse or touchpad usage data from daily usage environments can more accurately reflect the natural characteristics of user behavior. However, this approach is more costly and time-consuming. As shown in Figure [2](https://arxiv.org/html/2504.21415v1#S3.F2 "Figure 2 ‣ III Analysis of Mouse Dynamic Data ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") and Figure [3](https://arxiv.org/html/2504.21415v1#S3.F3 "Figure 3 ‣ III Analysis of Mouse Dynamic Data ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), datasets collected from users’ daily usage environments, such as the Balabit dataset, are significantly smaller than those obtained from laboratory environments, such as the DFL dataset. To avoid issues of insufficient datasets failing to adequately represent data characteristics, or excessive data leading to wasted time and resources, determining a reasonable data volume is essential.

Report issue for preceding element

Due to differences in application scenarios and data collection methods, the mouse dynamics datasets presented in the Table [I](https://arxiv.org/html/2504.21415v1#S1.T1 "TABLE I ‣ I Introduction ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") have varying feature contents, particularly in the State attribute. For instance, Balabit\[ [39](https://arxiv.org/html/2504.21415v1#bib.bib39 "")\] and DFL\[ [40](https://arxiv.org/html/2504.21415v1#bib.bib40 "")\] datasets’ states include dragging, double-click and so on, while Mouse-Behavior Data for Continuous Authentication \[ [49](https://arxiv.org/html/2504.21415v1#bib.bib49 "")\] contains left/right button clicks and movement with left or right button held down, etc. However, all datasets share the basic fundamental movement state. Therefore, before determine a reasonable data volume, we need to choose the variables to describe the mouse dynamic behaviors for our authentication task. To make our method more generalized and reduce the complexity of data processing, we utilize the moving velocity information of user mouse movements as the input data for the authentication system. Furthermore, relying solely on velocity data without directly storing mouse position information enhances user privacy protection and minimizes the risk of sensitive information leakage.

Report issue for preceding element

Given mouse movement trajectory of one specific user as x𝑥xitalic\_x={(xi,yi)}i=1Nabsentsubscriptsuperscriptsuperscript𝑥𝑖superscript𝑦𝑖𝑁𝑖1=\\{(x^{i},y^{i})\\}^{N}\_{i=1}= { ( italic\_x start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT , italic\_y start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT ) } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT, where xisuperscript𝑥𝑖x^{i}italic\_x start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT and yisuperscript𝑦𝑖y^{i}italic\_y start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT represent the mouse coordinates at the i−limit-from𝑖i-italic\_i -th time point. First, to convert {(xi,yi)}i=1Nsubscriptsuperscriptsuperscript𝑥𝑖superscript𝑦𝑖𝑁𝑖1\\{(x^{i},y^{i})\\}^{N}\_{i=1}{ ( italic\_x start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT , italic\_y start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT ) } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT to velocity sequence v={vi}i=1N𝑣subscriptsuperscriptsuperscript𝑣𝑖𝑁𝑖1v=\\{v^{i}\\}^{N}\_{i=1}italic\_v = { italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT, we calculate the Euclidean distance disuperscript𝑑𝑖d^{i}italic\_d start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT between adjacent points:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | di=(xi−xi−1)2+(yi−yi−1)2superscript𝑑𝑖superscriptsuperscript𝑥𝑖superscript𝑥𝑖12superscriptsuperscript𝑦𝑖superscript𝑦𝑖12d^{i}=\\sqrt{(x^{i}-x^{i-1})^{2}+(y^{i}-y^{i-1})^{2}}italic\_d start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT = square-root start\_ARG ( italic\_x start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT - italic\_x start\_POSTSUPERSCRIPT italic\_i - 1 end\_POSTSUPERSCRIPT ) start\_POSTSUPERSCRIPT 2 end\_POSTSUPERSCRIPT + ( italic\_y start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT - italic\_y start\_POSTSUPERSCRIPT italic\_i - 1 end\_POSTSUPERSCRIPT ) start\_POSTSUPERSCRIPT 2 end\_POSTSUPERSCRIPT end\_ARG |  | (1) |

Ignoring device latency, the time interval at each time point is fixed and denoted as Δ⁢tΔ𝑡\\Delta troman\_Δ italic\_t. Therefore, the mouse velocity sequence visuperscript𝑣𝑖v^{i}italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT is given by:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | vi=diΔ⁢tsuperscript𝑣𝑖superscript𝑑𝑖Δ𝑡v^{i}=\\frac{d^{i}}{\\Delta t}italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT = divide start\_ARG italic\_d start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT end\_ARG start\_ARG roman\_Δ italic\_t end\_ARG |  | (2) |

## IV Appropriate Volume of Mouse Dynamic Data Determination

Report issue for preceding element

Overly small volume of mouse dynamic data may fail to capture the user’s unique operational patterns, while excessively big amount of data may introduce redundant information and high cost. Therefore, determining the optimal volume of mouse dynamics data is crucial for real user authentication task. To establish a unified determination paradigm, we can assume that the user’s mouse behavior is largely influenced by the uncertainty caused by the surrounding environment (such as desktop space and current tasks) and the user themselves (such as emotions), but over a long period of use, the mouse dynamics data of individual users will tend to converge, i.e., the user’s mouse dynamics data will contain unique and stable identity characteristics.

Report issue for preceding element

Based on the this assumption, we estimate the appropriate data quantity by calculating the convergence point of the density function derived from the collected data\[7959200\]. We use F⁢(v;n)𝐹𝑣𝑛F(v;n)italic\_F ( italic\_v ; italic\_n ) to express the distribution of userj𝑗jitalic\_j mouse velocity data sequence v={vi}i=1N𝑣subscriptsuperscriptsuperscript𝑣𝑖𝑁𝑖1v=\\{v^{i}\\}^{N}\_{i=1}italic\_v = { italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT, and its density is f⁢(v;n)=dd⁢x⁢f⁢(v;n)𝑓𝑣𝑛𝑑𝑑𝑥𝑓𝑣𝑛f(v;n)=\\frac{d}{dx}f(v;n)italic\_f ( italic\_v ; italic\_n ) = divide start\_ARG italic\_d end\_ARG start\_ARG italic\_d italic\_x end\_ARG italic\_f ( italic\_v ; italic\_n ) under n𝑛nitalic\_n time. The density of distribution f⁢(v;n)𝑓𝑣𝑛f(v;n)italic\_f ( italic\_v ; italic\_n ) will vary with different data quantities n𝑛nitalic\_n. If a sufficient amount of data is provided, the observed density will no longer exhibit significant changes with the addition of extra data. For example, if the quantity n^^𝑛\\hat{n}over^ start\_ARG italic\_n end\_ARG represents a sufficient amount of mouse dynamics data for user authentication, then the density p(x,n^p(x,\\hat{n}italic\_p ( italic\_x , over^ start\_ARG italic\_n end\_ARG should exhibit only minor changes when supplemented with an additional m𝑚mitalic\_m length of data, i.e.,

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | f⁢(x;n^)≈f⁢(x;n^+m)𝑓𝑥^𝑛𝑓𝑥^𝑛𝑚f(x;\\hat{n})\\approx f(x;\\hat{n}+m)italic\_f ( italic\_x ; over^ start\_ARG italic\_n end\_ARG ) ≈ italic\_f ( italic\_x ; over^ start\_ARG italic\_n end\_ARG + italic\_m ) |  | (3) |

If adding more data does not alter the original data distribution too much, the additional data is considered redundant. Therefore, the observed data quantity may be appropriate because: i𝑖iitalic\_i) This data volume can capture almost all the potential features of user mouse behavior; i⁢i𝑖𝑖iiitalic\_i italic\_i) Adding more data cannot provide additional useful information but will increase data collection difficulty and computational overhead for the model. As shown in Fig [4](https://arxiv.org/html/2504.21415v1#S4.F4 "Figure 4 ‣ IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), We generate random data following a normal distribution 𝒩⁢(0,1)𝒩01\\mathcal{N}(0,1)caligraphic\_N ( 0 , 1 ) for specified sample sizes n,m,r,s𝑛𝑚𝑟𝑠n,m,r,sitalic\_n , italic\_m , italic\_r , italic\_s, and t𝑡titalic\_t, where the intervals are identical, but the sample sizes r,s𝑟𝑠r,sitalic\_r , italic\_s and t𝑡titalic\_t are significantly larger than n𝑛nitalic\_n and m𝑚mitalic\_m. In the figure [4](https://arxiv.org/html/2504.21415v1#S4.F4 "Figure 4 ‣ IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), the distributions for sample sizes n𝑛nitalic\_n and m𝑚mitalic\_m are similar, whereas the distributions for sample sizes r,s𝑟𝑠r,sitalic\_r , italic\_s and t𝑡titalic\_t exhibit minimal differences, almost overlapping. It indicates that when the dataset is sufficiently large, adding more data has a negligible impact on altering the overall distribution of the dataset.

Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/output_kde.png)Figure 4: Illustrationsof the different distribution density with different amount of data for normal distribution N⁢(0,1)𝑁01N(0,1)italic\_N ( 0 , 1 ) exampleReport issue for preceding element

We use Gaussian mixture model (GMM)\[ [46](https://arxiv.org/html/2504.21415v1#bib.bib46 "")\] to define f⁢(x;n)𝑓𝑥𝑛f(x;n)italic\_f ( italic\_x ; italic\_n ), regarding the complex probability distribution of mouse velocity data as a weighted combination of multiple Gaussian distributions to fit the complex data distribution flexibly. Given a mouse velocity dataset {vi}i=1Nsubscriptsuperscriptsuperscript𝑣𝑖𝑁𝑖1\\{v^{i}\\}^{N}\_{i=1}{ italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT with the density function f⁢(x;n)𝑓𝑥𝑛f(x;n)italic\_f ( italic\_x ; italic\_n ), the density function from data sample v𝑣vitalic\_v can be estimated by \[ [47](https://arxiv.org/html/2504.21415v1#bib.bib47 "")\]

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | f⁢(x;n)=1n⁢∑i=1n1(2⁢π)D2⁢\|Σ\|12⁢exp⁡(−12⁢(v−vi)⊤⁢Σ−1⁢(v−vi))𝑓𝑥𝑛1𝑛superscriptsubscript𝑖1𝑛1superscript2𝜋𝐷2superscriptΣ1212superscript𝑣superscript𝑣𝑖topsuperscriptΣ1𝑣superscript𝑣𝑖f(x;n)=\\frac{1}{n}\\sum\_{i=1}^{n}\\frac{1}{(2\\pi)^{\\frac{D}{2}}\|\\Sigma\|^{\\frac{1%<br>}{2}}}\\exp\\left(-\\frac{1}{2}(v-v^{i})^{\\top}\\Sigma^{-1}(v-v^{i})\\right)italic\_f ( italic\_x ; italic\_n ) = divide start\_ARG 1 end\_ARG start\_ARG italic\_n end\_ARG ∑ start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT start\_POSTSUPERSCRIPT italic\_n end\_POSTSUPERSCRIPT divide start\_ARG 1 end\_ARG start\_ARG ( 2 italic\_π ) start\_POSTSUPERSCRIPT divide start\_ARG italic\_D end\_ARG start\_ARG 2 end\_ARG end\_POSTSUPERSCRIPT \| roman\_Σ \| start\_POSTSUPERSCRIPT divide start\_ARG 1 end\_ARG start\_ARG 2 end\_ARG end\_POSTSUPERSCRIPT end\_ARG roman\_exp ( - divide start\_ARG 1 end\_ARG start\_ARG 2 end\_ARG ( italic\_v - italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT ) start\_POSTSUPERSCRIPT ⊤ end\_POSTSUPERSCRIPT roman\_Σ start\_POSTSUPERSCRIPT - 1 end\_POSTSUPERSCRIPT ( italic\_v - italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT ) ) |  | (4) |

where D𝐷Ditalic\_D is the dimension of the mouse velocity data; ΣΣ\\Sigmaroman\_Σ is the covariance matrix, i.e., bandwidth in 1D data, \|Σ\|Σ\|\\Sigma\|\| roman\_Σ \| is the determinant of bandwidth. In this work, we estimated the bandwidth by\[ [42](https://arxiv.org/html/2504.21415v1#bib.bib42 "")\]

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | h=1.06⋅σ^⋅n−15ℎ⋅1.06^𝜎superscript𝑛15h=1.06\\cdot\\hat{\\sigma}\\cdot n^{-\\frac{1}{5}}italic\_h = 1.06 ⋅ over^ start\_ARG italic\_σ end\_ARG ⋅ italic\_n start\_POSTSUPERSCRIPT - divide start\_ARG 1 end\_ARG start\_ARG 5 end\_ARG end\_POSTSUPERSCRIPT |  | (5) |

where σ^^𝜎\\hat{\\sigma}over^ start\_ARG italic\_σ end\_ARG is the standard deviation of the dataset {vi}i=1Nsubscriptsuperscriptsuperscript𝑣𝑖𝑁𝑖1\\{v^{i}\\}^{N}\_{i=1}{ italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT.

Report issue for preceding element

Thus, we can drive the density function f⁢(v;n)𝑓𝑣𝑛f(v;n)italic\_f ( italic\_v ; italic\_n ) from the mouse velocity data {vi}i=1Nsubscriptsuperscriptsuperscript𝑣𝑖𝑁𝑖1\\{v^{i}\\}^{N}\_{i=1}{ italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT } start\_POSTSUPERSCRIPT italic\_N end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT. We will assess the similarity between two adjacent kernel functions estimated from n𝑛nitalic\_n and n+m𝑛𝑚n+mitalic\_n + italic\_m data observations. Utilizing Kullback-Liebler (KL) divergence \[ [48](https://arxiv.org/html/2504.21415v1#bib.bib48 "")\], we can evaluate the difference from different density:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | KL(f(v;n+m)\|\|f(v;n))=∫\[f(v;n+m)×logf⁢(v;n+m)f⁢(v;n)\] KL(f(v;n+m)\|\|f(v;n))=\\int\\big{\[}f(v;n+m)\\times\\log\\frac{f(v;n+m)}{f(v;n)}\\big%<br>{\]}italic\_K italic\_L ( italic\_f ( italic\_v ; italic\_n + italic\_m ) \| \| italic\_f ( italic\_v ; italic\_n ) ) = ∫ \[ italic\_f ( italic\_v ; italic\_n + italic\_m ) × roman\_log divide start\_ARG italic\_f ( italic\_v ; italic\_n + italic\_m ) end\_ARG start\_ARG italic\_f ( italic\_v ; italic\_n ) end\_ARG \] |  | (6) |

Therefore, the KL can qualify the level of similarity between two density given by mouse velocity data with different length. When KL(f(v;n+m)\|\|f(v;n))KL(f(v;n+m)\|\|f(v;n))italic\_K italic\_L ( italic\_f ( italic\_v ; italic\_n + italic\_m ) \| \| italic\_f ( italic\_v ; italic\_n ) ) approaches 0, it indicates that f⁢(v;n)𝑓𝑣𝑛f(v;n)italic\_f ( italic\_v ; italic\_n ) is extremely close to f⁢(v;n+m)𝑓𝑣𝑛𝑚f(v;n+m)italic\_f ( italic\_v ; italic\_n + italic\_m ), where the additional data would not supply more useful information to density function. Furthermore, it is essential to calculate the KL divergence between f⁢(v;n+m)𝑓𝑣𝑛𝑚f(v;n+m)italic\_f ( italic\_v ; italic\_n + italic\_m ) and f⁢(v;n+2⁢m)𝑓𝑣𝑛2𝑚f(v;n+2m)italic\_f ( italic\_v ; italic\_n + 2 italic\_m ), as we aim to ensure that the decrease in KL divergence is not abrupt but instead shows a smooth and convergent behavior to ensure our data is enough.

Report issue for preceding element

We thus determine the proper amount n𝑛nitalic\_n of mouse velocity data so that KL(f(v;n+m)\|\|f(v;n))KL(f(v;n+m)\|\|f(v;n))italic\_K italic\_L ( italic\_f ( italic\_v ; italic\_n + italic\_m ) \| \| italic\_f ( italic\_v ; italic\_n ) ) itself is small and change very slightly, even more data added:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | {\|K⁢L⁢(f⁢(v;n+m)∥f⁢(v;n))\|≤ϵ1,\|K⁢L⁢(f⁢(v;n+2⁢m)∥f⁢(v;n+m))−K⁢L⁢(f⁢(v;n+m)∥f⁢(v;n))\|≤ϵ2cases𝐾𝐿conditional𝑓𝑣𝑛𝑚𝑓𝑣𝑛subscriptitalic-ϵ1otherwise𝐾𝐿conditional𝑓𝑣𝑛2𝑚𝑓𝑣𝑛𝑚𝐾𝐿conditional𝑓𝑣𝑛𝑚𝑓𝑣𝑛subscriptitalic-ϵ2otherwise\\begin{cases}\\lvert KL\\bigl{(}f(v;n+m)\\\|f(v;n)\\bigr{)}\\rvert\\leq\\epsilon\_{1},%<br>\\\\[6.0pt\]<br>\\lvert KL\\bigl{(}f(v;n+2m)\\\|f(v;n+m)\\bigr{)}-KL\\bigl{(}f(v;n+m)\\\|f(v;n)\\bigr{)%<br>}\\rvert\\leq\\epsilon\_{2}\\end{cases}{ start\_ROW start\_CELL \| italic\_K italic\_L ( italic\_f ( italic\_v ; italic\_n + italic\_m ) ∥ italic\_f ( italic\_v ; italic\_n ) ) \| ≤ italic\_ϵ start\_POSTSUBSCRIPT 1 end\_POSTSUBSCRIPT , end\_CELL start\_CELL end\_CELL end\_ROW start\_ROW start\_CELL \| italic\_K italic\_L ( italic\_f ( italic\_v ; italic\_n + 2 italic\_m ) ∥ italic\_f ( italic\_v ; italic\_n + italic\_m ) ) - italic\_K italic\_L ( italic\_f ( italic\_v ; italic\_n + italic\_m ) ∥ italic\_f ( italic\_v ; italic\_n ) ) \| ≤ italic\_ϵ start\_POSTSUBSCRIPT 2 end\_POSTSUBSCRIPT end\_CELL start\_CELL end\_CELL end\_ROW |  | (7) |

where ϵitalic-ϵ\\epsilonitalic\_ϵ is a small positive value. It is obvious that a larger value of ϵ1subscriptitalic-ϵ1\\epsilon\_{1}italic\_ϵ start\_POSTSUBSCRIPT 1 end\_POSTSUBSCRIPT and ϵ2subscriptitalic-ϵ2\\epsilon\_{2}italic\_ϵ start\_POSTSUBSCRIPT 2 end\_POSTSUBSCRIPT can lead a small amount of required mouse velocity data.

Report issue for preceding element

## V Mouse Authentication Unit Length Determination

Report issue for preceding element

Having established the amount of data required for the user authentication system to learn mouse movement patterns, we now determine the size of the Mouse Authentication Unit (MAU). Each MAU represents a time-bounded segment of mouse movement data, serving as the foundational element for user verification models. Adjusting the length of the MAU based on each user’s mouse behavior pattern preserves as much valuable information as possible while minimizing the introduction of redundant data, thereby improving the efficiency of the authentication system.

Report issue for preceding element

Because information and data predictability are closely linked to data complexity, i.e., entropy, we employ Approximate Entropy (ApEn) \[ [5](https://arxiv.org/html/2504.21415v1#bib.bib5 "")\] in this study to estimate the information complexity and determine an appropriate MAU length.

Report issue for preceding element

Given one individual user’s mouse velocity data sequence {vi}i=1nsubscriptsuperscriptsuperscript𝑣𝑖𝑛𝑖1\\{v^{i}\\}^{n}\_{i=1}{ italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT } start\_POSTSUPERSCRIPT italic\_n end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT with certain n𝑛nitalic\_n-dimension, we form a length-m𝑚mitalic\_m MAU v⁢(i)𝑣𝑖v(i)italic\_v ( italic\_i ):

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | v⁢(i)=(vi,vi+1,…,vi+m−1),i=1,2,…,n−m+1formulae-sequence𝑣𝑖superscript𝑣𝑖superscript𝑣𝑖1…superscript𝑣𝑖𝑚1𝑖12…𝑛𝑚1v(i)=(v^{i},v^{i+1},...,v^{i+m-1}),i=1,2,...,n-m+1italic\_v ( italic\_i ) = ( italic\_v start\_POSTSUPERSCRIPT italic\_i end\_POSTSUPERSCRIPT , italic\_v start\_POSTSUPERSCRIPT italic\_i + 1 end\_POSTSUPERSCRIPT , … , italic\_v start\_POSTSUPERSCRIPT italic\_i + italic\_m - 1 end\_POSTSUPERSCRIPT ) , italic\_i = 1 , 2 , … , italic\_n - italic\_m + 1 |  | (8) |

Therefore, we create a set {v⁢(i)}i=1n−m+1={v⁢(1),v⁢(2),…,v⁢(n−m+1)}superscriptsubscript𝑣𝑖𝑖1𝑛𝑚1𝑣1𝑣2…𝑣𝑛𝑚1\\{v(i)\\}\_{i=1}^{n-m+1}=\\{v(1),v(2),...,v(n-m+1)\\}{ italic\_v ( italic\_i ) } start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT start\_POSTSUPERSCRIPT italic\_n - italic\_m + 1 end\_POSTSUPERSCRIPT = { italic\_v ( 1 ) , italic\_v ( 2 ) , … , italic\_v ( italic\_n - italic\_m + 1 ) } containing all length-m𝑚mitalic\_m MAU.

Report issue for preceding element

Next, for each pair of length-m𝑚mitalic\_m MAU v⁢(p)𝑣𝑝v(p)italic\_v ( italic\_p ) and v⁢(q)𝑣𝑞v(q)italic\_v ( italic\_q ), we use Chebyshev distance to measure how ”close” they are:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | ⁢d⁢\[v⁢(p),v⁢(q)\]=maxs=1,2,…,m⁡\|vp+s−1−vq+s−1\|⁢⁢⁢⁢𝑑𝑣𝑝𝑣𝑞subscript𝑠12…𝑚superscript𝑣𝑝𝑠1superscript𝑣𝑞𝑠1 d\[v(p),v(q)\]=\\max\_{s=1,2,\\ldots,m}\|v^{p+s-1}-v^{q+s-1}\|    italic\_d \[ italic\_v ( italic\_p ) , italic\_v ( italic\_q ) \] = roman\_max start\_POSTSUBSCRIPT italic\_s = 1 , 2 , … , italic\_m end\_POSTSUBSCRIPT \| italic\_v start\_POSTSUPERSCRIPT italic\_p + italic\_s - 1 end\_POSTSUPERSCRIPT - italic\_v start\_POSTSUPERSCRIPT italic\_q + italic\_s - 1 end\_POSTSUPERSCRIPT \| |  | (9) |

In ApEn analysis, each length-m𝑚mitalic\_m MAU vj⁢(p)subscript𝑣𝑗𝑝v\_{j}(p)italic\_v start\_POSTSUBSCRIPT italic\_j end\_POSTSUBSCRIPT ( italic\_p ) is compared against all others v⁢(q)𝑣𝑞v(q)italic\_v ( italic\_q ) in time series to determine how many of the distances between them lie within a specified tolerance r𝑟ritalic\_r. Formally,

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | ⁢Cpm⁢(r)=#⁢{p≠q\|d⁢\[v⁢(p),v⁢(q)\]≤r}N−m+1⁢⁢⁢⁢subscriptsuperscript𝐶𝑚𝑝𝑟#conditional-set𝑝𝑞𝑑𝑣𝑝𝑣𝑞𝑟𝑁𝑚1 C^{m}\_{p}(r)=\\frac{\\#\\{p\\neq q\\,\|\\,d\[v(p),v(q)\]\\leq r\\}}{N-m+1}    italic\_C start\_POSTSUPERSCRIPT italic\_m end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_p end\_POSTSUBSCRIPT ( italic\_r ) = divide start\_ARG # { italic\_p ≠ italic\_q \| italic\_d \[ italic\_v ( italic\_p ) , italic\_v ( italic\_q ) \] ≤ italic\_r } end\_ARG start\_ARG italic\_N - italic\_m + 1 end\_ARG |  | (10) |

where d⁢\[·,·\]𝑑··d\[\\textperiodcentered,\\textperiodcentered\]italic\_d \[ · , · \] denotes the Chebyshev distance metric, and the numerator #⁢{p≠q\|d⁢\[v⁢(p),v⁢(q)\]≤r}#conditional-set𝑝𝑞𝑑𝑣𝑝𝑣𝑞𝑟\\#\\{p\\neq q\\,\|\\,d\[v(p),v(q)\]\\leq r\\}\# { italic\_p ≠ italic\_q \| italic\_d \[ italic\_v ( italic\_p ) , italic\_v ( italic\_q ) \] ≤ italic\_r } counts how many mouse velocity windows remain sufficiently close to v⁢(p)𝑣𝑝v(p)italic\_v ( italic\_p ) under the threshold r𝑟ritalic\_r. Consequently, Cpm⁢(r)subscriptsuperscript𝐶𝑚𝑝𝑟C^{m}\_{p}(r)italic\_C start\_POSTSUPERSCRIPT italic\_m end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_p end\_POSTSUBSCRIPT ( italic\_r ) serves as a measure of the local similarity or “cohesion” for each mouse velocity windows v⁢(p)𝑣𝑝v(p)italic\_v ( italic\_p ) and forms the basis for evaluating the overall regularity or predictability of the mouse velocity sequence when computing ApEn.

Report issue for preceding element

Then, we repeat the same procedure for another length-m+1𝑚1m+1italic\_m + 1 MAUs {v′⁢(i)}i=1n−m={v⁢(1),v⁢(2),…,v⁢(n−m)}superscriptsubscriptsuperscript𝑣′𝑖𝑖1𝑛𝑚𝑣1𝑣2…𝑣𝑛𝑚\\{v^{\\prime}(i)\\}\_{i=1}^{n-m}=\\{v(1),v(2),...,v(n-m)\\}{ italic\_v start\_POSTSUPERSCRIPT ′ end\_POSTSUPERSCRIPT ( italic\_i ) } start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT start\_POSTSUPERSCRIPT italic\_n - italic\_m end\_POSTSUPERSCRIPT = { italic\_v ( 1 ) , italic\_v ( 2 ) , … , italic\_v ( italic\_n - italic\_m ) } again. ApEn takes the ratio of these similarity measures at length m𝑚mitalic\_m and m+1𝑚1m+1italic\_m + 1 as below:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | ApEn⁢(m)=1n−m+1⁢∑i=1n−m+1log⁡Cim⁢(r)ApEn𝑚1𝑛𝑚1superscriptsubscript𝑖1𝑛𝑚1subscriptsuperscript𝐶𝑚𝑖𝑟\\displaystyle\\text{ApEn}(m)=\\frac{1}{n-m+1}\\sum\_{i=1}^{n-m+1}\\log C^{m}\_{i}(r)ApEn ( italic\_m ) = divide start\_ARG 1 end\_ARG start\_ARG italic\_n - italic\_m + 1 end\_ARG ∑ start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT start\_POSTSUPERSCRIPT italic\_n - italic\_m + 1 end\_POSTSUPERSCRIPT roman\_log italic\_C start\_POSTSUPERSCRIPT italic\_m end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i end\_POSTSUBSCRIPT ( italic\_r ) |  |
|  | ⁢⁢−1N−m⁢∑i=1n−mlog⁡Cim+1⁢(r)⁢⁢⁢UNKNOWN1𝑁𝑚superscriptsubscript𝑖1𝑛𝑚subscriptsuperscript𝐶𝑚1𝑖𝑟UNKNOWN\\displaystyle   -\\frac{1}{N-m}\\sum\_{i=1}^{n-m}\\log C^{m+1}\_{i}(r)   \- divide start\_ARG 1 end\_ARG start\_ARG italic\_N - italic\_m end\_ARG ∑ start\_POSTSUBSCRIPT italic\_i = 1 end\_POSTSUBSCRIPT start\_POSTSUPERSCRIPT italic\_n - italic\_m end\_POSTSUPERSCRIPT roman\_log italic\_C start\_POSTSUPERSCRIPT italic\_m + 1 end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_i end\_POSTSUBSCRIPT ( italic\_r ) UNKNOWN |  | (11) |

where m𝑚mitalic\_m is length for MAU. The MAU of different lengths possess varying levels of information complexity, which can be quantified by approximate entropy ApEn⁢(m)ApEn𝑚\\text{ApEn}(m)ApEn ( italic\_m ).

Report issue for preceding element

Approximate entropy decreases as the length of MAU increases, which means it contains more information. But as the length of MAU increase, the race of the increasing of approximate entropy will be slower. It indicates that the data’s predictability does not increase much and is insufficient to compensate for the increased collection time required for mouse dynamic sequences. In the subsequent experiments, we aimed to balance the trade-off between the sequence collection time and accuracy. Generally, we selected the length of the sequence with a slow rate of decrease in approximate entropy as the segmentation length for the mouse dynamic sequence.

Report issue for preceding element

## VI Local-Time Mouse Authentication (LTMouseAuthen)

Report issue for preceding element

In order to effectively extract deep features from mouse velocity sequences that can distinguish between different users, we propose a user authentication framework that integrates ResNet residual blocks and a GRU to fully exploit both local and global temporal information. Specifically, the input to the proposed LT-AMouse model is of fixed length. First, a 1D-CNN plus ResNet block module is employed to progressively extract and refine local features. Next, a GRU is utilized to capture contextual correlations among these features. Finally, a fully connected network performs binary classification on the extracted deep features to determine whether the input MAU belongs to the corresponding legitimate user (e.g., user j) or not.

Report issue for preceding element

### VI-AResNet Block for Local Features

Report issue for preceding element

Compared to commonly used multi-modal mouse data (e.g., data with timestamps, (x,y) coordinates, and interaction types), the mouse movement velocity sequence provides only single-channel velocity information. This results in a lower input dimensionality, which accelerates inference and reduces the burden of model deployment. Nevertheless, the reduced dimensionality makes it difficult for conventional manual feature engineering to adequately capture the potential temporal and local detail features. Therefore, we employ a one-dimensional convolutional neural network (1D-CNN) to extract local features from the mouse velocity sequence.

Report issue for preceding element

Specifically, as illustrated in Figure 1, we encode the input N-dimensional mouse velocity sequence using one-dimensional convolution while preserving the sequence length as much as possible to retain its temporal encoding. To further capture deeper local representations of the mouse data, we adopt a ResNet architecture, thereby deepening the convolutional layers and introducing residual connections. This design enables the model to refine key velocity variation patterns while preserving the complete temporal context, laying a solid foundation for subsequent classification or identity verification tasks.

Report issue for preceding element

### VI-BGRU for Time Series Context Information

Report issue for preceding element![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/user_authentication_model.png)Figure 5: User Authentication ModelReport issue for preceding element

After completing the local convolution and residual encoding, the resulting feature maps primarily emphasize patterns of local velocity changes. To capture global trends and longer-range temporal dependencies, we use a Gated Recurrent Unit (GRU) as the temporal feature extractor. The GRU module can remember past states in the sequence and selectively retain or update crucial information through its gating mechanism, which is particularly important for modeling the temporal evolution of mouse velocity.

Report issue for preceding element

In the overall network architecture, the dimensionality of the features fed into the GRU remains similar with the original sequence length, owing to the ResNet design that preserves sequence length. This allows the GRU to fully utilize the complete temporal information of the original sequence, leading to more effective modeling of velocity patterns.

Report issue for preceding element

After extracting both local and global features of the MAU, we feed the resulting hidden states into a fully connected network (FN) for final classification or identity verification. Similar to typical binary classification tasks, this fully connected layer can employ the Softmax function to output a probability distribution, thereby determining whether the input sample belongs to the legitimate user.

Report issue for preceding element

### VI-CTransferring and Training

Report issue for preceding element

During the training process, we formulate user identity verification as a binary classification problem and use cross-entropy loss\[ [53](https://arxiv.org/html/2504.21415v1#bib.bib53 "")\] as the objective function:

Report issue for preceding element

|     |     |     |     |
| --- | --- | --- | --- |
|  | ℒC=−1N⁢∑c=1Cyc⁢l⁢o⁢g⁢yc^⁢subscriptℒ𝐶1𝑁subscriptsuperscript𝐶𝑐1subscript𝑦𝑐𝑙𝑜𝑔^subscript𝑦𝑐\\mathcal{L}\_{C}=-\\frac{1}{N}\\sum^{C}\_{c=1}y\_{c}log\\hat{y\_{c}} caligraphic\_L start\_POSTSUBSCRIPT italic\_C end\_POSTSUBSCRIPT = - divide start\_ARG 1 end\_ARG start\_ARG italic\_N end\_ARG ∑ start\_POSTSUPERSCRIPT italic\_C end\_POSTSUPERSCRIPT start\_POSTSUBSCRIPT italic\_c = 1 end\_POSTSUBSCRIPT italic\_y start\_POSTSUBSCRIPT italic\_c end\_POSTSUBSCRIPT italic\_l italic\_o italic\_g over^ start\_ARG italic\_y start\_POSTSUBSCRIPT italic\_c end\_POSTSUBSCRIPT end\_ARG |  | (12) |

where N𝑁Nitalic\_N is the number of mouse velocity segmentation sequences, ycsubscript𝑦𝑐y\_{c}italic\_y start\_POSTSUBSCRIPT italic\_c end\_POSTSUBSCRIPT is the true value for user authentication, and yc^^subscript𝑦𝑐\\hat{y\_{c}}over^ start\_ARG italic\_y start\_POSTSUBSCRIPT italic\_c end\_POSTSUBSCRIPT end\_ARG is the probability of each velocity segmentation sample after softmax operation.

Report issue for preceding element

For the optimization algorithm, we select the Adam optimizer\[ [7](https://arxiv.org/html/2504.21415v1#bib.bib7 "")\] to balance convergence speed and training stability. The parameter β1subscript𝛽1\\beta\_{1}italic\_β start\_POSTSUBSCRIPT 1 end\_POSTSUBSCRIPT is 0.9 and β2subscript𝛽2\\beta\_{2}italic\_β start\_POSTSUBSCRIPT 2 end\_POSTSUBSCRIPT is 0.999.

Report issue for preceding element

## VII Experiment

Report issue for preceding element

### VII-AExperimental Setting

Report issue for preceding element

Dataset In this study, the Balabit and DFL datasets were selected as representatives of data collected from daily usage environments and laboratory environments, respectively, to investigate the trade-offs between data sufficiency, efficiency, and accuracy, as well as model performance. We first determine the appropriate amount of data for one individual user to be used as positive samples based on Equation [7](https://arxiv.org/html/2504.21415v1#S4.E7 "In IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), and then randomly select the remaining other users as negative samples to include in the training and testing sets. To verify whether the model is overfitting and to simulate a blind attack scenario, we randomly select unseen users from the remaining samples as unknown samples to be added to the testing set. Considering practical scenarios, user identification should be treated as an imbalanced classification problem, where the amount of positive sample data exceeds that of negative samples. For the DFL dataset, the ratio of positive to negative samples in the training set is 8:1, while for the Balabit dataset, the ratio is 5:1.

Report issue for preceding element

Metrics
We employ the following evaluation metrics to assess the performance of our user authentication system based on mouse velocity sequences and its robustness against attacks:

Report issue for preceding element

1. 1.


F1 Score: The harmonic mean of precision and recall, used to balance the trade-off between false positives and false negatives in imbalanced classification tasks.

Report issue for preceding element

2. 2.


Area Under the Curve (AUC): The area under the Receiver Operating Characteristic (ROC) curve, which reflects the system’s ability to differentiate between legitimate and unauthorized users across varying classification thresholds.

Report issue for preceding element

3. 3.


Equal Error Rate (ERR): The point at which the False Acceptance Rate (FAR) and False Rejection Rate (FRR) are equal, representing a balance between security and usability in the authentication process.

Report issue for preceding element

4. 4.


Defense Success Rate (DSR): A metric used to evaluate the effectiveness of the model in defending against adversarial attacks. It is defined as the percentage of attack attempts that fail to bypass the authentication system.

Report issue for preceding element


These metrics collectively ensure a comprehensive evaluation of the system’s performance, particularly in the context of imbalanced classification tasks and its robustness under adversarial conditions.

Report issue for preceding element

### VII-BProper Volume of Data

Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/kde_200_400_high_res.png)(a)KDE of Small U12Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/kde_17800_18000_18200_high_res.png)(b)KDE of Enough U12Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/dflbefore_high_res.png)(c)KDE of Small U19Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/kde_dflafter_high_res.png)(d)KDE of Enough U19Report issue for preceding element

Figure 6: Comparison of KDE of Different Volume of Mouse Velocity Data for Balabit and DFL Dataset ExampleReport issue for preceding element

In this study, mouse velocity sequences were extracted from two mouse dynamics datasets, Balabit and DFL mentioned in Section 3. Each user in the Balabit and DFL datasets is associated with multiple CSV files, each representing a distinct session unit. There is a clear discontinuity between the ending timestamp of one CSV file and the starting and ending timestamps of the subsequent file. Considering the potential variations in mouse operation habits across different time periods, which may result in differing data distributions, we first calculate the optimal data quantity for each individual CSV file and then sum these quantities to obtain the total data volume.

Report issue for preceding element

After calculating the mouse velocity sequences, due to the dataset being divided into multiple CSV files with clear temporal discontinuities, we computed the velocity sequences for each CSV file. To calculate the optimal data volume for a single session for each user, based on Equation [4](https://arxiv.org/html/2504.21415v1#S4.E4 "In IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), we first generate KDEs for different data volumes within each session, using a step size of 200. As shown in Figure [6](https://arxiv.org/html/2504.21415v1#S7.F6 "Figure 6 ‣ VII-B Proper Volume of Data ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), taking User 12 in Balabit and User 19 in DFL as example, the kernel density changes significantly with increasing data when the mouse velocity data volume is relatively small. At data volumes of n=200𝑛200n=200italic\_n = 200 and n=400𝑛400n=400italic\_n = 400, the kernel densities for two velocity sequence lengths differ markedly. However, when the data volume is large, even substantial data increases result in minimal kernel density differences. For example, when the data volume reaches sufficient value, mouse velocity data captures all variations, and the kernel density is not significantly different around it.

Report issue for preceding element

To better define the differences between kernel densities, we calculate the KL divergence between kernel densities represented by data of two different sample sizes using Equation [6](https://arxiv.org/html/2504.21415v1#S4.E6 "In IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"). Additionally, we use Equation [7](https://arxiv.org/html/2504.21415v1#S4.E7 "In IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") to determine when the KL divergence converges to a sufficiently small value, indicating that the distributions of the two datasets exhibit only minimal changes. If the two thresholds in Equation [7](https://arxiv.org/html/2504.21415v1#S4.E7 "In IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") are set to larger values, it results in smaller data volume; conversely, smaller thresholds yield larger data volume. To obtain conservative results, we set ϵ1subscriptitalic-ϵ1\\epsilon\_{1}italic\_ϵ start\_POSTSUBSCRIPT 1 end\_POSTSUBSCRIPT to 1×10−41superscript1041\\times 10^{-4}1 × 10 start\_POSTSUPERSCRIPT - 4 end\_POSTSUPERSCRIPT in this study. For the Balabit dataset, considering the complexity of mouse dynamics in real-world scenarios, we adopt a more conservative threshold ϵ2subscriptitalic-ϵ2\\epsilon\_{2}italic\_ϵ start\_POSTSUBSCRIPT 2 end\_POSTSUBSCRIPT as 1×10−71superscript1071\\times 10^{-7}1 × 10 start\_POSTSUPERSCRIPT - 7 end\_POSTSUPERSCRIPT; for the DFL dataset, we adopted a more aggressive strategy, setting ϵ2subscriptitalic-ϵ2\\epsilon\_{2}italic\_ϵ start\_POSTSUBSCRIPT 2 end\_POSTSUBSCRIPT as 1×10−61superscript1061\\times 10^{-6}1 × 10 start\_POSTSUPERSCRIPT - 6 end\_POSTSUPERSCRIPT.

Report issue for preceding element

Taking User 12 from the Balabit dataset and User 9 from the DFL dataset as examples, both of which have relatively large original data volumes, we present the variation trends of the KL divergence values corresponding to Equation [6](https://arxiv.org/html/2504.21415v1#S4.E6 "In IV Appropriate Volume of Mouse Dynamic Data Determination ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") across different session data volumes. As illustrated in Figure [9](https://arxiv.org/html/2504.21415v1#S7.F9 "Figure 9 ‣ VII-B Proper Volume of Data ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), the KL divergence decreases and stabilizes as the data volume grows, suggesting diminishing distributional differences between the mouse dynamic datasets with different volume. Furthermore, adding more mouse velocity data contributes minimal additional information, indicating saturation in the data’s informational content.

Report issue for preceding element

In the end, as shown in Figure [7](https://arxiv.org/html/2504.21415v1#S7.F7 "Figure 7 ‣ VII-B Proper Volume of Data ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") and Figure [8](https://arxiv.org/html/2504.21415v1#S7.F8 "Figure 8 ‣ VII-B Proper Volume of Data ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), the average suitable mouse dynamic dataset volume 73,778.7 of Balabit dataset is smaller than orginal one 114,114.2. For the DFL dataset, our method reduces the amount of data required for each user by a factor of ten, from 0.6726×1070.6726superscript1070.6726\\times 10^{7}0.6726 × 10 start\_POSTSUPERSCRIPT 7 end\_POSTSUPERSCRIPT to 0.0691×1070.0691superscript1070.0691\\times 10^{7}0.0691 × 10 start\_POSTSUPERSCRIPT 7 end\_POSTSUPERSCRIPT.

Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/compare_balabite_data_count.png)Figure 7: Proper and Total Volume of Balabit DatasetReport issue for preceding element![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/compare_DFL_data_count.png)Figure 8: Proper and Total Volume of DFL DatasetReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit21.png)(a)Balabit User12 Session1Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit52.png)(b)Balabit User12 Session2Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit58.png)(c)Balabit User12 Session3Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit74.png)(d)Balabit User12 Session4Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit88.png)(e)Balabit User12 Session5Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit90.png)(f)Balabit User12 Session6Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetbalabit98.png)(g)Balabit User12 Session7Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/plot_with_insetdfl19.png)(h)DFL User9Report issue for preceding element

Figure 9: KL divergence convergence performance of User12 in the Balabit and User 9 in DFL datasetsReport issue for preceding element

### VII-CMouse Authentication Unit Length and Model Performance Trade-off

Report issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/DFL_AUC_1-10.png)(a)AUC ComparisonReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/DFL_EER_1-11.png)(b)EER ComparisonReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/users_DFL_user1_10.png)(c)User DistributionReport issue for preceding element

DFL Dataset User 1-10

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/DFL_AUC_11-21.png)(d)AUC ComparisonReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/DFL_EER_11-21.png)(e)EER ComparisonReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/users_DFL_user11_21.png)(f)User DistributionReport issue for preceding element

DFL Dataset User 11-21

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/balabit_auc_comparison_all.png)(g)AUC ComparisonReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/balabit_eer_comparison_all.png)(h)EER ComparisonReport issue for preceding element

![Refer to caption](https://arxiv.org/html/2504.21415v1/extracted/6396421/users_balabite.png)(i)User DistributionReport issue for preceding element

Balabit DatasetFigure 10: Performance Metrics and ApEn Analysis on DFL and Balabit DatasetReport issue for preceding element

In this section, We analyzed the performance impact of different MAU (Mouse Action Unit) lengths on the authentication model LT-AMouse in Balabit and DFL datasets. The results show that as the MAU length increases, the AUC index of the model gradually increases and the EER decreases accordingly. This is because longer MAUs provide richer mouse dynamics features, which help to portray user behavior more comprehensively. However, the performance of the model does not increase linearly. In the short MAU length range, the AUC and EER improve significantly, and when the length exceeds a certain threshold, the performance improvement tends to slow down.

Report issue for preceding element

Through the analysis of Approximate Entropy (ApEn), we find that this trend is consistent with the pattern of sequence randomness reduction. With a short MAU length, the data is not enough to fully characterize the user; as the length increases, the uncertainty of the sequence decreases, and the model can quickly accumulate discriminative features. However, when the data volume reaches a certain scale, the additional information gain decreases, and the change amplitude of both ApEn and performance indicators tends to level off.

Report issue for preceding element

Therefore, the choice of MAU length needs to be a trade-off between security and real-time: high security requirement scenarios can appropriately increase the MAU length to improve accuracy, while applications with high real-time requirements need to shorten the MAU length to realize fast response. This study provides a reference basis for the practical deployment of mouse dynamics authentication system.

Report issue for preceding element

By further analyzing the slope of the change of entropy, we find that when the absolute value of the slope of the entropy is close to or less than 1×10−41superscript1041\\times 10^{-4}1 × 10 start\_POSTSUPERSCRIPT - 4 end\_POSTSUPERSCRIPT, the increase of the MAU length tends to moderate the enhancement of the model performance. At this point, the decrease in entropy is small, indicating that the accumulated feature information is close to saturation, and continuing to increase the MAU length has limited gain in model performance, while the computational cost may increase significantly. Therefore, an absolute value of the entropy slope less than 1×10−41superscript1041\\times 10^{-4}1 × 10 start\_POSTSUPERSCRIPT - 4 end\_POSTSUPERSCRIPT is defined as the optimal balance between efficiency and model performance.

Report issue for preceding element

It can be observed in Figure [10](https://arxiv.org/html/2504.21415v1#S7.F10 "Figure 10 ‣ VII-C Mouse Authentication Unit Length and Model Performance Trade-off ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") that the optimal equilibrium point for the Balabit dataset lies between 90-130 MAU lengths, while the DFL dataset lies between 110-160. This analysis provides a theoretical basis for the selection of experimental data in Section 7.4, helping us to optimize the efficiency while ensuring the model performance.

Report issue for preceding element

### VII-DComparison with Other Models

Report issue for preceding element

To ensure a fair comparison with existing state-of-the-art methods, we evaluate our proposed user authentication model on the DFL and Balabit datasets under identical experimental settings. For consistency, we use the same hyper-parameter configurations, including the network architecture, loss function, and training epochs, as employed by baseline methods. This ensures that performance improvements are attributed solely to our model’s design rather than differing experimental conditions.

Report issue for preceding element

Table [II](https://arxiv.org/html/2504.21415v1#S7.T2 "TABLE II ‣ VII-D Comparison with Other Models ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") and table [III](https://arxiv.org/html/2504.21415v1#S7.T3 "TABLE III ‣ VII-D Comparison with Other Models ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") summarizes the performance of our model compared to other commonly used methods, including CNN, LSTM, RF, and SVM. On the DFL dataset, our model achieves the highest F1 score (97.24%) and AUC (98.52%), while maintaining a low equal error rate (EER) of 5.05%. Similarly, on the Balabit dataset, our model outperforms all baselines, achieving an F1 score of 94.65%, AUC of 97.73%, and an EER of 6.14%. These results demonstrate that our model not only provides state-of-the-art accuracy but also ensures robustness across different datasets.

Report issue for preceding element

Moreover, compared to traditional machine learning models such as RF and SVM, our model achieves significant improvements in both accuracy and efficiency. Specifically, our model is more than 8 times accurate than RF in terms of EER on the DFL dataset and reduces the error rate by over 40% compared to SVM on the Balabit dataset. These enhancements highlight the strength of our approach in capturing the underlying dynamics of user-specific mouse behavior.

Report issue for preceding element

Finally, our method exhibits superior generalizability across datasets, as evidenced by consistent improvements in both precision and recall. This capability is particularly critical for real-world applications, where datasets often vary significantly in terms of user behavior and interaction patterns.

Report issue for preceding element

TABLE II: User-Averaged Models Performance Comparison on DFL Dataset

|     |     |     |     |
| --- | --- | --- | --- |
| Model | F1 | AUC | EER |
| Our Model | 97.24% | 98.52% | 5.05% |
|  | (0.13%) | (0.01%) | (0.08%) |
| CNN | 96.01% | 97.07% | 7.56% |
|  | (0.68%) | (0.07%) | (0.18%) |
| LSTM | 94.22% | 83.48% | 23.11% |
|  | (0.07%) | (1.66%) | (1.45%) |
| RF | 79.53% | 89.85% | 14.74% |
|  | (2.56%) | (1.43%) | (1.71%) |
| SVM | 88.92% | 59.40% | 42.97% |
|  | (0.00%) | (2.77%) | (2.21%) |

Report issue for preceding elementTABLE III: User-Averaged Models Performance Comparison on Balabit Dataset

|     |     |     |     |
| --- | --- | --- | --- |
| Model | F1 | AUC | EER |
| Our Model | 94.65%percent\\%% | 97.73%percent\\%% | 6.14%percent\\%% |
| (0.71%percent\\%%) | (0.03%percent\\%%) | (0.11%percent\\%%) |
| CNN | 93.01%percent\\%% | 93.15%percent\\%% | 13.79%percent\\%% |
| (0.28%percent\\%%) | (0.09%percent\\%%) | (0.20%percent\\%%) |
| LSTM | 89.79%percent\\%% | 80.18%percent\\%% | 25.85%percent\\%% |
| (0.24%percent\\%%) | (1.35%percent\\%%) | (1.21%percent\\%%) |
| RF | 54.62%percent\\%% | 72.18%percent\\%% | 33.36%percent\\%% |
| (0.43%percent\\%%) | (1.63%percent\\%%) | (1.21%percent\\%%) |
| SVM | 86.10%percent\\%% | 44.08%percent\\%% | 54.04%percent\\%% |
| (0.07%percent\\%%) | (2.47%percent\\%%) | (1.48%percent\\%%) |

Report issue for preceding element

### VII-EAttack Model

Report issue for preceding element

We assume that the attacker is familiar with the authentication mechanism of LT-AMouse. Depending on whether the adversary can access the parameters of the LT-AMouse model and whether they can obtain partial mouse movement data from legitimate users (e.g., through phishing emails or other malicious means), we classify attacks into the following two types:

Report issue for preceding element

Blind Attack In a blind attack, the adversary possesses no prior knowledge of the legitimate user’s mouse movement patterns. To carry out the attack, the adversary interacts with the system by controlling the mouse, attempting to bypass the authentication mechanism using their own mouse dynamics.

Report issue for preceding element

Result Blind attack results are summarized in Table [II](https://arxiv.org/html/2504.21415v1#S7.T2 "TABLE II ‣ VII-D Comparison with Other Models ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") and Table [III](https://arxiv.org/html/2504.21415v1#S7.T3 "TABLE III ‣ VII-D Comparison with Other Models ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"). By introducing unseen samples from the LT-AMouse dataset into the test set, which were not present in the training set, our model achieved notable performance across all evaluation metrics. Specifically, on , the model achieved an F1 Score of 97.24

Report issue for preceding element

Imitation Attack In an imitation attack, we assume that the adversary can observe, record, and analyze the dynamic mouse trajectories of legitimate users. Leveraging this data, the adversary employs generative models\[ [55](https://arxiv.org/html/2504.21415v1#bib.bib55 "")\] to create highly realistic forged mouse trajectories, aiming to deceive the authentication system and impersonate a legitimate user.

Report issue for preceding element

In this study, we employ a tailored attack model designed to circumvent a mouse dynamics authentication system. This model leverages the Wasserstein Conditional Deep Convolutional Generative Adversarial Network (WCDCGAN) \[ [54](https://arxiv.org/html/2504.21415v1#bib.bib54 "")\] to generate realistic and high-quality adversarial samples that closely mimic genuine mouse dynamics, making them challenging for the authentication system to distinguish from legitimate inputs.

Report issue for preceding element

Result
In the simulation attack scenario, this study employs DSR as the core evaluation metric to assess LT-AMouse’s capability in distinguishing between legitimate users and generated fraudulent data under different parameter configurations. Table [IV](https://arxiv.org/html/2504.21415v1#S7.T4 "TABLE IV ‣ VII-E Attack Model ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges") demonstrates the system’s defensive performance against imitation attacks when the MAU length increases from 110 to 160.

Report issue for preceding element

TABLE IV: Defense Success Rates for Different MAU Lengths

|     |     |     |
| --- | --- | --- |
| Dataset | MAU Length | Defense Success Rate (%) |
| DFL | 110 | 89.22 |
| 120 | 99.02 |
| 130 | 88.56 |
| 140 | 95.87 |
| 150 | 94.44 |
| 160 | 93.68 |
| Balabit | 90 | 47.51 |
| 100 | 70.00 |
| 110 | 79.69 |
| 120 | 59.90 |
| 130 | 60.00 |

Report issue for preceding element

As shown in Table [IV](https://arxiv.org/html/2504.21415v1#S7.T4 "TABLE IV ‣ VII-E Attack Model ‣ VII Experiment ‣ Optimizing Mouse Dynamics for User Authentication by Machine Learning: Addressing Data Sufficiency, Accuracy-Practicality Trade-off, and Model Performance Challenges"), for the DFL dataset, when the MAU length is 120, the system achieves a defense success rate of up to 99.02%. At other lengths the defense success rate remains between 88.56% and 95.87%. For the Balabit dataset, our model achieves the best defense success rate of 79.69% at MAU length 110, which demonstrates a significant improvement compared to the original attack success rate of 94% reported in the WC-DCGAN attack \[ [54](https://arxiv.org/html/2504.21415v1#bib.bib54 "")\] . These findings suggest that the proposed LT-AMouse model can accurately differentiate genuine from forged mouse trajectories, even with high-fidelity imitation data, thereby demonstrating strong robustness and security.

Report issue for preceding element

## VIII Discussion

Report issue for preceding element

Environmental interference and minimal user input In practical applications, mouse dynamics-based authentication relies on the consistent capture of movement trajectories and click patterns. However, significant fluctuations in surface friction or hardware sensitivity caused by environmental factors, as well as minimal and low-amplitude mouse operations, pose challenges to accurate modeling of these sparse and weak dynamic signals, leading to performance degradation. Compared to continuous and pronounced mouse inputs, unstable or low-amplitude operations are more susceptible to environmental factors and usage posture, increasing the likelihood of errors in authentication.

Report issue for preceding element

Biometric Authentication Mouse operation habits are susceptible to variations in hardware, environment, and users’ states (e.g. fatigue and emotions), leading to significant shifts in data distribution over time and weak generalization. Additionally, in cross-platform or cross-context deployments, mouse movement features can vary substantially due to differences in operating systems or the mouse’s settings, posing challenges to model transferability. Furthermore, while mouse-dynamics-based approaches offer advantages in data collection and privacy protection compared to biometic methods, they remain vulnerable to some threats. For example, attackers could potentially infer user behavior patterns and compromise system security by intercepting portions of mouse trajectories through malicious software or remote monitoring. Lastly, to continuously enhance security and robustness, further research is needed in privacy-protection technologies, such as on-device computation, federated learning, and secure multiparty computation, along with in-depth model adaptation and optimization tailored to diverse application scenarios.

Report issue for preceding element

Attack Model In this study, we assessed our method’s resilience against the advanced WCDCGAN \[ [54](https://arxiv.org/html/2504.21415v1#bib.bib54 "")\] attack, which has demonstrated up to a 94% success rate in compromising mouse dynamics authentication systems. Notably, our approach can operate effectively with smaller datasets, thus enabling faster and more efficient defense performance—albeit without achieving complete immunity. Future research will focus on integrating advanced strategies and novel techniques to further fortify the system’s resistance. Empirical results show that our method achieved defense success rates exceeding 88% on the DFL dataset, whereas the Balabit dataset peaked at 79.69%, suggesting that dataset properties significantly influence how well the system can withstand sophisticated adversarial threats.

Report issue for preceding element

## IX Conclusion

Report issue for preceding element

This study presents a robust and efficient mouse dynamics-based authentication framework, addressing challenges in data sufficiency, practicality, and security. By introducing the Mouse Authentication Unit (MAU) and leveraging Approximate Entropy, our method optimizes data segmentation for accurate behavioral representation. The Local-Time Mouse Authentication (LT-MAuthen) framework achieved state-of-the-art performance, with AUCs of 98.52%percent98.5298.52\\%98.52 % on the DFL dataset and 94.65%percent94.6594.65\\%94.65 % on the Balabit dataset, while demonstrating resilience against advanced adversarial attacks. These findings highlight the framework’s potential for real-world applications, with future work focused on enhancing cross-platform adaptability and robust defense mechanisms.

Report issue for preceding element

## References

Report issue for preceding element

## References

Report issue for preceding element

- \[1\]↑
Z. Erlich and M. Zviran, “Authentication methods for computer systems security,” in _Encyclopedia of Information Science and Technology, Second Edition_.   IGI Global, 2009, pp. 288–293.

- \[2\]↑
R. W. Proctor, M.-C. Lien, K.-P. L. Vu, E. E. Schultz, and G. Salvendy, “Improving computer security for authentication of users: Influence of proactive password restrictions,” _Behavior Research Methods, Instruments, & Computers_, vol. 34, pp. 163–169, 2002.

- \[3\]↑
K. Revett, H. Jahankhani, S. T. D. Magalhaes, and H. M. D. Santos, “A survey of user authentication based on mouse dynamics,” in _Global E-Security: 4th International Conference, ICGeS 2008, London, UK, June 23-25, 2008. Proceedings_.   Springer, 2008, pp. 210–219.

- \[4\]↑
D. Haussler, “A general minimax result for relative entropy,” _IEEE Transactions on Information Theory_, vol. 43, no. 4, pp. 1276–1280, 1997.

- \[5\]↑
C. Villani, “A short proof of the ”concavity of entropy power”,” _IEEE Transactions on Information Theory_, vol. 46, no. 4, pp. 1695–1696, 2000.

- \[6\]↑
K. He, X. Zhang, S. Ren, and J. Sun, “Deep residual learning for image recognition,” in _Proceedings of the IEEE Conference on Computer Vision and Pattern Recognition_, 2016, pp. 770–778.

- \[7\]↑
D. P. Kingma and J. Ba, “Adam: A method for stochastic optimization,” _arXiv preprint arXiv:1412.6980_, 2014.

- \[8\]↑
K. Wang, C. Ma, Y. Qiao, X. Lu, W. Hao, and S. Dong, “A hybrid deep learning model with 1dcnn-lstm-attention networks for short-term traffic flow prediction,” _Physica A: Statistical Mechanics and its Applications_, vol. 583, p. 126293, 2021.

- \[9\]↑
S. Fu, D. Qin, D. Qiao, and G. T. Amariucai, “Rumba-mouse: Rapid user mouse-behavior authentication using a cnn-rnn approach,” in _2020 IEEE Conference on Communications and Network Security (CNS)_.   IEEE, 2020, pp. 1–9.

- \[10\]↑
P. Chong, Y. Elovici, and A. Binder, “User authentication based on mouse dynamics using deep neural networks: A comprehensive study,” _IEEE Transactions on Information Forensics and Security_, vol. 15, pp. 1086–1101, 2019.

- \[11\]↑
Q. Yao, J. Zhao, Z. Yang, R. Fei, L. Yan, and Y. Wang, “Identity authentication based on user mouse behavior,” in _2020 International Conference on Virtual Reality and Intelligent Systems (ICVRIS)_.   IEEE, 2020, pp. 571–577.

- \[12\]↑
M. Antal, N. Fejér, and K. Buza, “Sapimouse: Mouse dynamics-based user authentication using deep feature learning,” in _2021 IEEE 15th International Symposium on Applied Computational Intelligence and Informatics (SACI)_.   IEEE, 2021, pp. 61–66.

- \[13\]↑
D. Polemi, “Biometric techniques: Review and evaluation of biometric techniques for identification and authentication, including an appraisal of the areas where they are most applicable,” _Reported Prepared for the European Commission DG XIIIC_, vol. 4, 1997.

- \[14\]↑
R. Joyce and G. Gupta, “Identity authentication based on keystroke latencies,” _Communications of the ACM_, vol. 33, no. 2, pp. 168–176, 1990.

- \[15\]↑
K. O. Bailey, J. S. Okolica, and G. L. Peterson, “User identification and authentication using multi-modal behavioral biometrics,” _Computers & Security_, vol. 43, pp. 77–89, 2014.

- \[16\]↑
Y. Meng, D. S. Wong, R. Schlegel, and L. for Kwok, “Touch gestures based biometric authentication scheme for touchscreen mobile phones,” in _Information Security and Cryptology: 8th International Conference, Inscrypt 2012, Beijing, China, November 28-30, 2012, Revised Selected Papers 8_.   Springer, 2013, pp. 331–350.

- \[17\]↑
A. Buriro, B. Crispo, F. D. Frari, and K. Wrona, “Touchstroke: Smartphone user authentication based on touch-typing biometrics,” in _New Trends in Image Analysis and Processing–ICIAP 2015 Workshops: ICIAP 2015 International Workshops, BioFor, CTMR, RHEUMA, ISCA, MADiMa, SBMI, and QoEM, Genoa, Italy, September 7-8, 2015, Proceedings 18_.   Springer, 2015, pp. 27–34.

- \[18\]↑
A. M. Amarasinghe, I. Malassri, K. Weerasinghe, I. Jayasingha, P. K. Abeygunawardhana, and S. Silva, “Stress analysis and care prediction system for online workers,” in _2021 3rd International Conference on Advancements in Computing (ICAC)_.   IEEE, 2021, pp. 329–334.

- \[19\]↑
Q. Yi, W. Li, S. ping Yi, and J. dong Xie, “Trustworthy identity authentication based on joint time-frequency analysis of mouse behavior,” _Journal of Beijing University of Posts and Telecommunications_, vol. 44, no. 4, p. 121.

- \[20\]↑
X. Ding, C. Peng, H. Ding, M. Wang, H. Yang, and Q. Yu, “User identity authentication and identification based on multi-factor behavior features,” in _2019 IEEE Globecom Workshops (GC Wkshps)_.   IEEE, 2019, pp. 1–6.

- \[21\]↑
S. J. Quraishi and S. Bedi, “Secure system of continuous user authentication using mouse dynamics,” in _2022 3rd International Conference on Intelligent Engineering and Management (ICIEM)_.   IEEE, 2022, pp. 138–144.

- \[22\]↑
N. Siddiqui, R. Dave, and N. Seliya, “Continuous user authentication using mouse dynamics, machine learning, and minecraft,” in _2021 International Conference on Electrical, Computer and Energy Technologies (ICECET)_.   IEEE, 2021, pp. 1–6.

- \[23\]↑
C. Shen, Z. Cai, X. Guan, C. Fang, and Y. Du, “User authentication and monitoring based on mouse behavioural features,” _Journal on Communications_, vol. 31, no. 7, pp. 68–75, 2010.

- \[24\]↑
A. A. E. Ahmed and I. Traore, “A new biometric technology based on mouse dynamics,” _IEEE Transactions on Dependable and Secure Computing_, vol. 4, no. 3, pp. 165–179, 2007.

- \[25\]↑
H. Jun and M. Kang, “Three-way identity authentication method based on mouse behavior,” _Journal of Nanjing University of Science and Technology_, no. 4, pp. 474–480, 2019.

- \[26\]↑
M. Antal and N. Fejér, “Mouse dynamics based user recognition using deep learning,” _Acta Universitatis Sapientiae, Informatica_, vol. 12, no. 1, pp. 39–50, 2020.

- \[27\]↑
O. V. Komogortsev, A. Karpov, and C. D. Holland, “Attack of mechanical replicas: Liveness detection with eye movements,” _IEEE Transactions on Information Forensics and Security_, vol. 10, no. 4, pp. 716–725, 2015.

- \[28\]↑
J. Dai, Y. Li, K. He, and J. Sun, “R-fcn: Object detection via region-based fully convolutional networks,” _Advances in Neural Information Processing Systems_, vol. 29, 2016.

- \[29\]↑
H. Li, A. Kadav, I. Durdanovic, H. Samet, and H. P. Graf, “Pruning filters for efficient convnets,” _arXiv Preprint arXiv:1608.08710_, 2016.

- \[30\]↑
Y. Huang, C. Du, Z. Xue, X. Chen, H. Zhao, and L. Huang, “What makes multi-modal learning better than single (provably),” _Advances in Neural Information Processing Systems_, vol. 34, pp. 10 944–10 956, 2021.

- \[31\]↑
S. Pincus, “Approximate entropy (apen) as a complexity measure,” _Chaos: An Interdisciplinary Journal of Nonlinear Science_, vol. 5, no. 1, pp. 110–117, 1995.

- \[32\]↑
E. Chiu, J. Lin, B. McFerron, N. Petigara, and S. Seshasai, “Mathematical theory of claude shannon. a study of the style and context of his work up to the genesis of information theory,” _Submitted for The Structure of Engineering Revolutions (MIT course 6.933 J/STS. 420J), nd_, 2018.

- \[33\]↑
P. Paysarvi-Hoseini and N. C. Beaulieu, “Optimal wideband spectrum sensing framework for cognitive radio systems,” _IEEE Transactions on Signal Processing_, vol. 59, no. 3, pp. 1170–1182, 2010.

- \[34\]↑
E. Fredkin, “An introduction to digital philosophy,” _International Journal of Theoretical Physics_, vol. 42, pp. 189–247, 2003.

- \[35\]↑
M. Boedihardjo, T. Strohmer, and R. Vershynin, “Privacy of synthetic data: A statistical framework,” _IEEE Transactions on Information Theory_, vol. 69, no. 1, pp. 520–527, 2022.

- \[36\]↑
J. A. O’Sullivan, R. E. Blahut, and D. L. Snyder, “Information-theoretic image formation,” _IEEE Transactions on Information Theory_, vol. 44, no. 6, pp. 2094–2123, 1998.

- \[37\]↑
A. Bradlev, “The use of the area under the roc curve in the evaluation of machine learning algorithms,” _Pattern Recognition_, vol. 30, no. 7, pp. 1145–1159, 1997.

- \[38\]↑
L. Araujo, L. Sucupira, M. Lizarraga, L. Ling, and J. Yabu-Uti, “User authentication through typing biometrics features,” _IEEE Transactions on Signal Processing_, vol. 53, no. 2, pp. 851–855, 2005.

- \[39\]↑
K. W. Fülöp, Á., Kovács, L., “Balabit mouse dynamics challenge data set,” 2016, https://github.com/balabit/Mouse-Dynamics-Challenge/, Last accessed on 03-01-2025.

- \[40\]↑
M. Antal and L. Denes-Fazakas, “User verification based on mouse dynamics: a comparison of public data sets,” in _2019 IEEE 13th International Symposium on Applied Computational Intelligence and Informatics (SACI)_.   IEEE, 2019, pp. 143–148.

- \[41\]↑
C. Shen, Z. Cai, X. Guan, Y. Du, and R. A. Maxion, “User authentication through mouse dynamics,” _IEEE Transactions on Information Forensics and Security_, vol. 8, no. 1, pp. 16–30, 2013.

- \[42\]↑
B. W. Silverman, _Density estimation for statistics and data analysis_.   Routledge, 2018.

- \[43\]↑
R. E. Heyman, B. R. Chaudhry, D. Treboux, J. Crowell, C. Lord, D. Vivian, and E. B. Waters, “How much observational data is enough? an empirical test using marital interaction coding,” _Behavior Therapy_, vol. 32, no. 1, pp. 107–122, 2001. \[Online\]. Available: https://doi.org/10.1016/S0005-7894(01)80047-2
- \[44\]↑
A. H. Wortley, P. J. Rudall, D. J. Harris, and R. W. Scotland, “How much data are needed to resolve a difficult phylogeny?: case study in lamiales,” _Systematic biology_, vol. 54, no. 5, pp. 697–709, October 2005. \[Online\]. Available: https://academic.oup.com/sysbio/article-pdf/54/5/697/26543993/10635150500221028.pdf
- \[45\]↑
K. D. Splinter, I. L. Turner, and M. A. Davidson, “How much data is enough? the importance of morphological sampling interval and duration for calibration of empirical shoreline models,” _Coastal Engineering_, vol. 77, pp. 14–27, 2013. \[Online\]. Available: https://www.sciencedirect.com/science/article/pii/S0378383913000495
- \[46\]↑
C. Stauffer and W. Grimson, “Adaptive background mixture models for real-time tracking,” in _Proceedings. 1999 IEEE Computer Society Conference on Computer Vision and Pattern Recognition (Cat. No PR00149)_, vol. 2, 1999, pp. 246–252 Vol. 2.

- \[47\]↑
B. W. Silverman, _Density Estimation for Statistics and Data Analysis_.   Chapman and Hall/CRC, 1986.

- \[48\]↑
S. Kullback and R. A. Leibler, “On information and sufficiency,” _The Annals of Mathematical Statistics_, vol. 22, no. 1, pp. 79–86, 1951.

- \[49\]↑
C. Shen, Z. Cai, and X. Guan, “Continuous authentication for mouse dynamics: A pattern-growth approach,” in _IEEE/IFIP International Conference on Dependable Systems and Networks (DSN 2012)_, 2012, pp. 1–12.

- \[50\]↑
J. Xu, M. Li, F. Zhou, and R. Xue, “Identity authentication method based on user’s mouse behavior,” _Computer Science_, vol. 43, no. 2, pp. 148–154, 2016.

- \[51\]↑
S. Bleha, C. Slivinsky, and B. Hussien, “Computer-access security systems using keystroke dynamics,” _IEEE Transactions on Pattern Analysis and Machine Intelligence_, vol. 12, no. 12, pp. 1217–1222, 1990.

- \[52\]↑
M. S. Obaidat and B. Sadoun, “Verification of computer users using keystroke dynamics,” _IEEE Transactions on Systems, Man, and Cybernetics, Part B (Cybernetics)_, vol. 27, no. 2, pp. 261–269, 1997.

- \[53\]↑
A. Krizhevsky, I. Sutskever, and G. E. Hinton, “Imagenet classification with deep convolutional neural networks,” _Communications of the ACM_, vol. 60, no. 6, pp. 84–90, June 2017. \[Online\]. Available: https://doi.org/10.1145/3065386
- \[54\]↑
A. Roy, K. Wong, and R. C.-W. Phan, “Attacking mouse dynamics authentication using novel wasserstein conditional dcgan,” _IEEE Transactions on Information Forensics and Security_, vol. 18, pp. 3622–3631, 2023.

- \[55\]↑
Y. X. Marcus Tan, A. Iacovazzi, I. Homoliak, Y. Elovici, and A. Binder, “Adversarial attacks on remote user authentication using behavioural mouse dynamics,” in _2019 International Joint Conference on Neural Networks (IJCNN)_, 2019, pp. 1–10.

- \[56\]↑
Y. X. M. Tan, A. Iacovazzi, I. Homoliak, Y. Elovici, and A. Binder, “Adversarial attacks on remote user authentication using behavioural mouse dynamics,” 2019.

- \[57\]↑
M. Arjovsky, S. Chintala, and L. Bottou, “Wasserstein generative adversarial networks,” in _Proceedings of the 34th International Conference on Machine Learning (ICML 2017)_, ser. ICML’17.   JMLR.org, 2017, pp. 214–223.

- \[58\]↑
M. Mirza and S. Osindero, “Conditional generative adversarial nets,” 2014.

- \[59\]↑
A. K. Jain, S. Prabhakar, L. Hong, and S. Pankanti, “Filterbank-based fingerprint matching,” _IEEE Transactions on Image Processing_, vol. 9, no. 5, pp. 846–859, 2000.

- \[60\]↑
T. Kinnunen and H. Li, “An overview of text-independent speaker recognition: From features to supervectors,” _Speech Communication_, vol. 52, no. 1, pp. 12–40, 2010. \[Online\]. Available: https://www.sciencedirect.com/science/article/pii/S0167639309001289
- \[61\]↑
A. Gupta, A. Khanna, A. Jagetia, D. Sharma, S. Alekh, and V. Choudhary, “Combining keystroke dynamics and face recognition for user verification,” in _Proceedings of the 2015 IEEE 18th International Conference on Computational Science and Engineering (CSE)_, 2015, pp. 294–299.


Report Issue

##### Report GitHub Issue

Title:

Content selection saved. Describe the issue below:

Description:

Submit without GitHubSubmit in GitHub

Report Issue for Selection

Generated by
[L\\
A\\
T\\
Exml![[LOGO]](<Base64-Image-Removed>)](https://math.nist.gov/~BMiller/LaTeXML/)

## Instructions for reporting errors

We are continuing to improve HTML versions of papers, and your feedback helps enhance accessibility and mobile support. To report errors in the HTML that will help us improve conversion and rendering, choose any of the methods listed below:

- Click the "Report Issue" button.
- Open a report feedback form via keyboard, use " **Ctrl + ?**".
- Make a text selection and click the "Report Issue for Selection" button near your cursor.
- You can use Alt+Y to toggle on and Alt+Shift+Y to toggle off accessible reporting links at each section.

Our team has already identified [the following issues](https://github.com/arXiv/html_feedback/issues). We appreciate your time reviewing and reporting rendering errors we may not have found yet. Your efforts will help us improve the HTML versions for all readers, because disability should not be a barrier to accessing research. Thank you for your continued support in championing open access for all.

Have a free development cycle? Help support accessibility at arXiv! Our collaborators at LaTeXML maintain a [list of packages that need conversion](https://github.com/brucemiller/LaTeXML/wiki/Porting-LaTeX-packages-for-LaTeXML), and welcome [developer contributions](https://github.com/brucemiller/LaTeXML/issues).