- [Home](https://learnopencv.com/ "Home")
- \>
- [GUI](https://learnopencv.com/category/gui/ "GUI")
- \>
- cvui: A GUI lib built on top of OpenCV drawing primitives

[Fernando Bevilacqua](https://learnopencv.com/author/dovyski/)

- on June 21, 2017

# cvui: A GUI lib built on top of OpenCV drawing primitives

Often the development of a computer vision project involves tweaking parameters of a technique to achieve the desired outcome. These parameters could be the thresholds of an edge detection algorithm or the brightness of an image, for instance. If you don’t use any graphical user interface (GUI) for tweaking these

- [GUI](https://learnopencv.com/category/gui/), [UI](https://learnopencv.com/category/ui/)

![CVUI : A GUI lib built on top of OpenCV drawing primitives](https://cdn.learnopencv.com/wp-content/uploads/2017/06/04100734/cvui-e1498048157149.png)

Often the development of a computer vision project involves tweaking parameters of a technique to achieve the desired outcome. These parameters could be the thresholds of an edge detection algorithm or the brightness of an image, for instance. If you don’t use any graphical user interface (GUI) for tweaking these parameters, you need to stop your application, adjust your code, run the application again, evaluate, and repeat until it is good. That is tedious and time-consuming.

There are plenty of great GUI libs, e.g. [Qt](https://www.qt.io/) and [imgui](https://github.com/ocornut/imgui), that can be used together with OpenCV to allow you to tweak parameters during runtime. For using [Qt with OpenCV on a Mac](https://learnopencv.com/configuring-qt-for-opencv-on-osx), check out [this post](https://learnopencv.com/configuring-qt-for-opencv-on-osx). There might be cases, however, where you don’t have (or don’t want) the dependencies of such libs, e.g. you have not compiled OpenCV with Qt support, or you can’t use OpenGL. In such situations, all you need is a quick and hassle-free way of creating a GUI to tweak your algorithms.

That is the purpose of [cvui](https://dovyski.github.io/cvui/). It is a C++, header-only and cross-platform (Windows, Linux and OSX) UI lib built on top of OpenCV drawing primitives. It has no dependencies other than OpenCV itself (which you are probably already using).

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2025/02/PyTorch_Bootcamp.jpg)\\
\\
10K+ Learners\\
\\
8 Hours of Learning\\
\\
**Join Free PyTorch Bootcamp**](https://opencv.org/university/free-pytorch-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/All-CV-Courses-Thumbnails-3.jpg)\\
\\
100K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free OpenCV Bootcamp**](https://opencv.org/university/free-opencv-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2025/02/PyTorch_Bootcamp.jpg)\\
\\
10K+ Learners\\
\\
8 Hours of Learning\\
\\
**Join Free PyTorch Bootcamp**](https://opencv.org/university/free-pytorch-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/All-CV-Courses-Thumbnails-3.jpg)\\
\\
100K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free OpenCV Bootcamp**](https://opencv.org/university/free-opencv-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[View all AI Free Courses](https://opencv.org/university/free-courses/?utm_source=lopcv&utm_medium=blog)

It follows the rule

> One line of code should produce one UI component on the screen.

As a result, the lib has a friendly and C-like API with no classes/objects and several components, e.g. trackbar, button, text, among others:

![CVUI : A GUI lib built on top of OpenCV drawing primitives](https://learnopencv.com/wp-content/uploads/2017/06/cvui.png)A few of the UI components available in cvui

## How to use cvui in your application

In order to use cvui, you just include `cvui.h` in your project, give it an image ( i.e. `cv::Mat` ) to render components and you are done!

## Basic “hello world” application

Let’s take a look at the capabilities of cvui by creating a simple hello-world application with some UI interactions. The application contains a button and a visual indicator showing how many times that button was clicked. Here is the code:

|     |     |
| --- | --- |
| 1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20<br>21<br>22<br>23<br>24<br>25<br>26<br>27<br>28<br>29<br>30<br>31<br>32<br>33<br>34<br>35<br>36<br>37<br>38<br>39<br>40<br>41 | `#include <opencv2/opencv.hpp>`<br>`#include "cvui.h"`<br>`#define WINDOW_NAME "CVUI Hello World!"`<br>`int``main(``void``)`<br>`{`<br>```cv::Mat frame = cv::Mat(200, 500, CV_8UC3);`<br>```int``count = 0;`<br>```// Init a OpenCV window and tell cvui to use it.`<br>```cv::namedWindow(WINDOW_NAME);`<br>```cvui::init(WINDOW_NAME);`<br>```while``(``true``) {`<br>```// Fill the frame with a nice color`<br>```frame = cv::Scalar(49, 52, 49);`<br>```// Show a button at position (110, 80)`<br>```if``(cvui::button(frame, 110, 80,``"Hello, world!"``)) {`<br>```// The button was clicked, so let's increment our counter.`<br>```count++;`<br>```}`<br>```// Show how many times the button has been clicked.`<br>```// Text at position (250, 90), sized 0.4, in red.`<br>```cvui::``printf``(frame, 250, 90, 0.4, 0xff0000,``"Button click count: %d"``, count);`<br>```// Update cvui internal stuff`<br>```cvui::update();`<br>```// Show everything on the screen`<br>```cv::imshow(WINDOW_NAME, frame);`<br>```// Check if ESC key was pressed`<br>```if``(cv::waitKey(20) == 27) {`<br>```break``;`<br>```}`<br>```}`<br>```return``0;`<br>`}` |

**Download Code**
To easily follow along this tutorial, please download code by clicking on the button below. It's FREE!

Download Code

![](https://cdn.learnopencv.com/wp-content/uploads/2021/10/04093413/cropped-favicon-512x512-1-150x150.png)

Click here to download the source code to this post

The result of the code above is the following:

![](https://learnopencv.com/wp-content/uploads/2017/06/cvui-hello-world.gif)Basic cvui application featuring a button and a text

To ensure cvui works properly with your project

1. Call the initialization function `cvui::init()` before rendering any components.
2. Call `cvui::update()` once after all components are rendered.

Regarding the components used in the code above, the `cvui::button()` function returns `true` everytime the button is clicked, so you can conveniently use it in if statements. The `cvui::printf()` function works similarly to the standard C `printf()` function, so you can easily render texts and numbers on the screen using notations as `%d` and `%s`. You can also choose the color of the text using hex values as `0xRRGGBB`, e.g. `0xFF0000` (red), `0x00FF00` (green) and `0x0000FF` (blue).

## A more advanced application

Now let’s build something a bit more sophisticated, but as easily as before. The application applies the [Canny Edge algorithm](https://en.wikipedia.org/wiki/Canny_edge_detector) to an image, allowing the user to enable/disable the technique and adjust its threshold values.

### Step 1: Foundation

We start by creating an application with no UI elements. The use of the Canny Edge algorithm is defined by a boolean variable (`use_canny`), while the algorithm thresholds are defined by two integers (`low_threshold` and `high_threshold`). Using that approach, we must recompile the code every time we want to enable/disable the technique or adjust its thresholds.

The code for that application is the following:

|     |     |
| --- | --- |
| 1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20<br>21<br>22<br>23<br>24<br>25<br>26<br>27<br>28<br>29<br>30<br>31<br>32<br>33<br>34 | `#include <opencv2/opencv.hpp>`<br>`#define WINDOW_NAME "CVUI Canny Edge"`<br>`int``main(``int``argc,``const``char``*argv[])`<br>`{`<br>```cv::Mat lena = cv::imread(``"lena.jpg"``);`<br>```cv::Mat frame = lena.clone();`<br>```int``low_threshold = 50, high_threshold = 150;`<br>```bool``use_canny =``false``;`<br>```cv::namedWindow(WINDOW_NAME);`<br>```while``(``true``) {`<br>```// Should we apply Canny edge?`<br>```if``(use_canny) {`<br>```// Yes, we should apply it.`<br>```cv::cvtColor(lena, frame, CV_BGR2GRAY);`<br>```cv::Canny(frame, frame, low_threshold, high_threshold, 3);`<br>```}``else``{`<br>```// No, so just copy the original image to the displaying frame.`<br>```lena.copyTo(frame);`<br>```}`<br>```// Show everything on the screen`<br>```cv::imshow(WINDOW_NAME, frame);`<br>```// Check if ESC was pressed`<br>```if``(cv::waitKey(30) == 27) {`<br>```break``;`<br>```}`<br>```}`<br>```return``0;`<br>`}` |

The result is an application that either shows the original image (`use_canny` is `false`) or shows the detected edges (`use_canny` is `true`):

![](https://learnopencv.com/wp-content/uploads/2017/06/cvui-canny-manual-small.png)The activation of the Canny edge algorithm requires changes to the code and a new compilation

### Step 2: Dynamically enable/disable the edge detection

Let’s improve the workflow by using cvui and adding a checkbox to control the value of `use_canny`. Using that approach, the user can enable/disable the use of Canny Edge while the application is still running. We add the required cvui code and use the `cvui::checkbox` function:

|     |     |
| --- | --- |
| 1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20<br>21<br>22<br>23<br>24<br>25<br>26<br>27<br>28<br>29<br>30<br>31<br>32<br>33<br>34<br>35<br>36<br>37<br>38<br>39<br>40<br>41<br>42<br>43 | `#include <opencv2/opencv.hpp>`<br>`#include "cvui.h"`<br>`#define WINDOW_NAME "CVUI Canny Edge"`<br>`int``main(``void``)`<br>`{`<br>```cv::Mat lena = cv::imread(``"lena.jpg"``);`<br>```cv::Mat frame = lena.clone();`<br>```int``low_threshold = 50, high_threshold = 150;`<br>```bool``use_canny =``false``;`<br>```// Init a OpenCV window and tell cvui to use it.`<br>```cv::namedWindow(WINDOW_NAME);`<br>```cvui::init(WINDOW_NAME);`<br>```while``(``true``) {`<br>```// Should we apply Canny edge?`<br>```if``(use_canny) {`<br>```// Yes, we should apply it.`<br>```cv::cvtColor(lena, frame, CV_BGR2GRAY);`<br>```cv::Canny(frame, frame, low_threshold, high_threshold, 3);`<br>```}``else``{`<br>```// No, so just copy the original image to the displaying frame.`<br>```lena.copyTo(frame);`<br>```}`<br>```// Checkbox to enable/disable the use of Canny edge`<br>```cvui::checkbox(frame, 15, 80,``"Use Canny Edge"``, &use_canny);`<br>```// Update cvui internal stuff`<br>```cvui::update();`<br>```// Show everything on the screen`<br>```cv::imshow(WINDOW_NAME, frame);`<br>```// Check if ESC was pressed`<br>```if``(cv::waitKey(30) == 27) {`<br>```break``;`<br>```}`<br>```}`<br>```return``0;`<br>`}` |

This small modification alone is already a time saver for testing the application without recompiling everything:

![](https://learnopencv.com/wp-content/uploads/2017/06/cvui-canny-basic-ui.gif)Basic UI to allow the use or not of Canny Edge

It might be difficult to see the rendered checkbox and its label depending on the image being used, e.g. image with a white background. We can prevent that problem by creating a window using `cvui::window()` to house the checkbox.

cvui renders each component at the moment the component function is called, so we must call `cvui::window()` before `cvui::checkbox()`, otherwise the window will be rendered in front of the checkbox:

|     |     |
| --- | --- |
| 1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20<br>21<br>22<br>23<br>24<br>25<br>26<br>27<br>28<br>29<br>30<br>31<br>32<br>33<br>34<br>35<br>36<br>37<br>38<br>39<br>40<br>41<br>42<br>43<br>44<br>45<br>46 | `#include <opencv2/opencv.hpp>`<br>`#include "cvui.h"`<br>`#define WINDOW_NAME "CVUI Canny Edge"`<br>`int``main(``void``)`<br>`{`<br>```cv::Mat lena = cv::imread(``"lena.jpg"``);`<br>```cv::Mat frame = lena.clone();`<br>```int``low_threshold = 50, high_threshold = 150;`<br>```bool``use_canny =``false``;`<br>```// Init a OpenCV window and tell cvui to use it.`<br>```cv::namedWindow(WINDOW_NAME);`<br>```cvui::init(WINDOW_NAME);`<br>```while``(``true``) {`<br>```// Should we apply Canny edge?`<br>```if``(use_canny) {`<br>```// Yes, we should apply it.`<br>```cv::cvtColor(lena, frame, CV_BGR2GRAY);`<br>```cv::Canny(frame, frame, low_threshold, high_threshold, 3);`<br>```}``else``{`<br>```// No, so just copy the original image to the displaying frame.`<br>```lena.copyTo(frame);`<br>```}`<br>```// Render the settings window to house the UI`<br>```cvui::window(frame, 10, 50, 180, 180,``"Settings"``);`<br>```// Checkbox to enable/disable the use of Canny edge`<br>```cvui::checkbox(frame, 15, 80,``"Use Canny Edge"``, &use_canny);`<br>```// Update cvui internal stuff`<br>```cvui::update();`<br>```// Show everything on the screen`<br>```cv::imshow(WINDOW_NAME, frame);`<br>```// Check if ESC was pressed`<br>```if``(cv::waitKey(30) == 27) {`<br>```break``;`<br>```}`<br>```}`<br>```return``0;`<br>`}` |

The result is a more pleasant UI:

![](https://learnopencv.com/wp-content/uploads/2017/06/cvui-canny-decent-ui.png)A more pleasant UI with the use of cvuis window component

### Step 3: Tweak threshold values

It is time to allow the user to select the values for `low_threashold` and `high_threashold` during runtime as well. Since those parameters can vary within an interval, we can use `cvui::trackbar()` to create a trackbar:

|     |     |
| --- | --- |
| 1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20<br>21<br>22<br>23<br>24<br>25<br>26<br>27<br>28<br>29<br>30<br>31<br>32<br>33<br>34<br>35<br>36<br>37<br>38<br>39<br>40<br>41<br>42<br>43<br>44<br>45<br>46<br>47<br>48<br>49<br>50<br>51 | `#include <opencv2/opencv.hpp>`<br>`#include "cvui.h"`<br>`#define WINDOW_NAME "CVUI Canny Edge"`<br>`int``main(``void``)`<br>`{`<br>```cv::Mat lena = cv::imread(``"lena.jpg"``);`<br>```cv::Mat frame = lena.clone();`<br>```int``low_threshold = 50, high_threshold = 150;`<br>```bool``use_canny =``false``;`<br>```// Init a OpenCV window and tell cvui to use it.`<br>```cv::namedWindow(WINDOW_NAME);`<br>```cvui::init(WINDOW_NAME);`<br>```while``(``true``) {`<br>```// Should we apply Canny edge?`<br>```if``(use_canny) {`<br>```// Yes, we should apply it.`<br>```cv::cvtColor(lena, frame, CV_BGR2GRAY);`<br>```cv::Canny(frame, frame, low_threshold, high_threshold, 3);`<br>```}``else``{`<br>```// No, so just copy the original image to the displaying frame.`<br>```lena.copyTo(frame);`<br>```}`<br>```// Render the settings window to house the UI`<br>```cvui::window(frame, 10, 50, 180, 180,``"Settings"``);`<br>```// Checkbox to enable/disable the use of Canny edge`<br>```cvui::checkbox(frame, 15, 80,``"Use Canny Edge"``, &use_canny);`<br>```// Two trackbars to control the low and high threshold values`<br>```// for the Canny edge algorithm.`<br>```cvui::trackbar(frame, 15, 110, 165, &low_threshold, 5, 150);`<br>```cvui::trackbar(frame, 15, 180, 165, &high_threshold, 80, 300);`<br>```// Update cvui internal stuff`<br>```cvui::update();`<br>```// Show everything on the screen`<br>```cv::imshow(WINDOW_NAME, frame);`<br>```// Check if ESC was pressed`<br>```if``(cv::waitKey(30) == 27) {`<br>```break``;`<br>```}`<br>```}`<br>```return``0;`<br>`}` |

The `cvui::trackbar()` function accepts parameters that specify the minimum and maximum values allowed for the trackbar. In the example above, they are \[5, 150\] for `low_threshold` and \[80, 300\] for `high_threshold`, respectively.

The result is a fully interactive application that allows users to quickly and easily explore the tweaking of Canny Edge parameters, as well as enable/disable its use:

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2025/02/PyTorch_Bootcamp.jpg)\\
\\
10K+ Learners\\
\\
8 Hours of Learning\\
\\
**Join Free PyTorch Bootcamp**](https://opencv.org/university/free-pytorch-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/All-CV-Courses-Thumbnails-3.jpg)\\
\\
100K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free OpenCV Bootcamp**](https://opencv.org/university/free-opencv-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2025/02/PyTorch_Bootcamp.jpg)\\
\\
10K+ Learners\\
\\
8 Hours of Learning\\
\\
**Join Free PyTorch Bootcamp**](https://opencv.org/university/free-pytorch-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/All-CV-Courses-Thumbnails-3.jpg)\\
\\
100K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free OpenCV Bootcamp**](https://opencv.org/university/free-opencv-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[![](https://opencv.org/university/wp-content/uploads/sites/4/2023/05/Free-TF-Bootcamp_4.jpg)\\
\\
15K+ Learners\\
\\
3 Hours of Learning\\
\\
**Join Free TensorFlow Bootcamp**](https://opencv.org/university/free-tensorflow-keras-course/?utm_source=locv&utm_medium=midblog&utm_campaign=cvui-a-gui-lib-built-on-top-of-opencv-drawing-primitives)

[View all AI Free Courses](https://opencv.org/university/free-courses/?utm_source=lopcv&utm_medium=blog)

![](https://learnopencv.com/wp-content/uploads/2017/06/cvui-canny-complete-ui.gif)Final result of using cvui to create a UI to adjust Canny Edge thresholds

Below is the complete code for this application, without the comments. It shows that you don’t need many lines of code to produce a minimal (and useful) UI for your application:

|     |     |
| --- | --- |
| 1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20<br>21<br>22<br>23<br>24<br>25<br>26<br>27<br>28<br>29<br>30<br>31<br>32<br>33<br>34<br>35<br>36<br>37 | `#include <opencv2/opencv.hpp>`<br>`#include "cvui.h"`<br>`#define WINDOW_NAME "CVUI Canny Edge"`<br>`int``main(``void``)`<br>`{`<br>```cv::Mat lena = cv::imread(``"lena.jpg"``);`<br>```cv::Mat frame = lena.clone();`<br>```int``low_threshold = 50, high_threshold = 150;`<br>```bool``use_canny =``false``;`<br>```cv::namedWindow(WINDOW_NAME);`<br>```cvui::init(WINDOW_NAME);`<br>```while``(``true``) {`<br>```if``(use_canny) {`<br>```cv::cvtColor(lena, frame, CV_BGR2GRAY);`<br>```cv::Canny(frame, frame, low_threshold, high_threshold, 3);`<br>```}``else``{`<br>```lena.copyTo(frame);`<br>```}`<br>```cvui::window(frame, 10, 50, 180, 180,``"Settings"``);`<br>```cvui::checkbox(frame, 15, 80,``"Use Canny Edge"``, &use_canny);`<br>```cvui::trackbar(frame, 15, 110, 165, &low_threshold, 5, 150);`<br>```cvui::trackbar(frame, 15, 180, 165, &high_threshold, 80, 300);`<br>```cvui::update();`<br>```cv::imshow(WINDOW_NAME, frame);`<br>```if``(cv::waitKey(30) == 27) {`<br>```break``;`<br>```}`<br>```}`<br>```return``0;`<br>`}` |

## Conclusion

The [cvui](https://dovyski.github.io/cvui/) lib was created out of a necessity. It was not designed to be a full-blown solution for the development of complex graphical applications. It is simple and limited in many ways. However, it is practical, easy to use and can save you several hours of frustration and tedious work.

If you like cvui, don’t forget to check out its [repository on Github](https://github.com/Dovyski/cvui), its [documentation](https://dovyski.github.io/cvui/) and all [example applications](https://github.com/Dovyski/cvui/tree/master/example/src) (buildable with [cmake](https://cmake.org/)).

Was This Article Helpful?

## Subscribe & Download Code

If you liked this article and would like to download code (C++ and Python) and example images used in this post, please click here. Alternately, sign up to receive a free Computer Vision Resource Guide. In our newsletter, we share OpenCV tutorials and examples written in C++/Python, and Computer Vision and Machine Learning algorithms and news.

Download Example Code

[PrevPreviousInstall Dlib on Windows](https://learnopencv.com/install-dlib-on-windows/)

[NextParallel Pixel Access in OpenCV using forEachNext](https://learnopencv.com/parallel-pixel-access-in-opencv-using-foreach/)

Load Comments

### [How to Master YOLOE: Real-Time Open-Vocabulary Detection Made Easy](https://learnopencv.com/yoloe-tutorial-real-time-open-vocabulary-detection/)

Learn YOLOE for real-time open-vocabulary object detection and instance segmentation in Python with Ultralytics —

### [Vision Banana: How Image Generators Are Becoming Powerful Vision Models](https://learnopencv.com/vision-banana-explained/)

Vision Banana turns Nano Banana Pro into a powerful vision model for segmentation, depth estimation,

### [YOLO26 Keypoint Estimation: Real-Time Pose Estimation with Ultralytics](https://learnopencv.com/yolo26-pose-estimation-tutorial/)

Learn how to use YOLO26-pose with Python for real-time keypoint estimation on images and videos,

#### Table of Contents

1. [How to use cvui in your application](https://learnopencv.com/cvui-gui-lib-built-on-top-of-opencv-drawing-primitives/#elementor-toc__heading-anchor-0)

2. [Basic “hello world” application](https://learnopencv.com/cvui-gui-lib-built-on-top-of-opencv-drawing-primitives/#elementor-toc__heading-anchor-1)

3. [A more advanced application](https://learnopencv.com/cvui-gui-lib-built-on-top-of-opencv-drawing-primitives/#elementor-toc__heading-anchor-2)

4. [Conclusion](https://learnopencv.com/cvui-gui-lib-built-on-top-of-opencv-drawing-primitives/#elementor-toc__heading-anchor-3)


Was This Article Helpful?

- [C++](https://learnopencv.com/tag/c/), [edge detector](https://learnopencv.com/tag/edge-detector/), [trackbar](https://learnopencv.com/tag/trackbar/)

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