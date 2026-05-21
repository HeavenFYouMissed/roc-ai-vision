# On the Validity of Using First-Person Shooters for Fitts’ Law Studies

Julian Looser and Andy Cockburn Human-Computer Interaction Lab Department of Computer Science and Software Enginee ring University of Canterbury Christchurch, New Zealand _[jcl55@student.canterbury.ac.nz](mailto:jcl55@student.canterbury.ac.nz), [andy@cosc.canterbur](mailto:andy@cosc.canterbur) y.ac.nz_

**3D first-person shooter games provide highly polish ed, compelling and entertaining environments that** **far out-strip the refinement of most research syste ms. Many also provide extensive support for tailori ng** **the environment, allowing researchers to adapt them to specific research agendas. This paper examines** **using 3D first-person shooter game environments for motivating participants in Fitts’ Law studies. In** **particular, we examine whether the first-person met aphor for target acquisition, which involves rotati ng** **the world to pan the target to the screen-centre, i s accurately modelled by Fitts’ Law, and whether th e** **resultant Fitts’ metrics are representative of trad itional cursor-based acquisition. Results show** **excellent Fitts’ modelling, with metrics that are similar to traditional pointing.**

_Fitts’ Law, 3D Games, empirical studies, target acquisition._

## 1\. INTRODUCTION

| Almost all contemporary computer-based user interfa | ces are designed around direct manipulation of grap |  |  | hical |
| --- | --- | --- | --- | --- |
| components, with a pointing device being used to mo | ve the cursor to items, and to click, drag or manip |  | ulate them. |  |
| The accuracy and efficiency of pointing is therefor use. Many researchers and hardware/software vendors | e extremely important to the overall effectiveness | have developed technologies to improve pointing, | of computer |  |
| including isometric input devices such as the IBM T | rackpoint, configurable control-display gain to acc |  |  | elerate |
| mouse movement \[7\], and interface components that e |  | xpand \[8\] or are ‘sticky’ \[2\]. Although heavily res | earched, |  |
| moving the cursor to the target is not only method | of target acquisition. It is also possible to move |  | the target to the |  |
| pointer by panning the information space. This styl | e of acquisition is common in first-person ‘shooter |  | ’ computer |  |
| games (see Figure 1), in which the user aims by bri | nging the target to a cross-hair at the screen-cent |  | re—dragging |  |
| the mouse rightwards rotates the user’s view rightw | ards as through they had turned their head in that |  | direction. |  |
| Fitts’ Law \[3\] is the standard empirical tool for a | ssessing and comparing the efficiency of pointing t |  | echniques. |  |
| Fitts’ robust model shows that the time to select a divided by the target size. During a Fitts’ Law | n item is proportional to the logarithm of the dist |  | ance to the target |  |
| study, participants will typically sit in front of | a |  |  |  |

computer screen, looking at a circle of blobs, moving the cursor as quickly as possible to the next illuminated blob. It is intensely tedious. While participants can be recruited for single-session studies through small incentives, it can be hard to find willing participants for longitudinal studies that demand multiple sessions in order to study skill development. This paper examines pan-based target acquisition in 3D first-person computer games. The aim is two-fold: first, to verify that Fitts’ Law accurately models this type of target selection; and second, to compare user performance with traditional pointing and pan-based pointing. If there is little difference between traditional and pan- based pointing, there is an argument supporting the use of 3D game environments for motivating participants in longitudinal Fitts’ Law studies. **FIGURE 1:** A typical first-person shooter game.

* * *

**(a)** Traditional targeting. **(b)** 3D pan-based targeting.
**FIGURE 2:** The experimental interfaces.

## 2\. BACKGROUND

## 2.1 Fitts’ Law

Fitts’ Law \[3\] models the time taken to acquire targets in graphical user interfaces. Linear regression is used to determine the line of best fit between movement tim e _MT_ and the Index of Difficulty (IoD): _MT_ = _a_ \+ _b_· _IoD_, where _IoD_ = log2( A / W +)1 . _A_ is the movement ‘amplitude’ or distance to the tar get, and _W_ is the target width. The reciprocal of the slope constant _b_ provides a useful estimate of hand-eye coordination using the targeting method, called the “Index of Performance” ( _IoP_) and measured in bits per second. Although almost exclusively used to examine cursor movement to on-screen targets, recen t work has shown that Fitts’ Law also models scroll-based target acquisition where the target lies outside the initial view \[5\]. Fitts’ Law also remains a robust model for off- screen target acquisition when zooming is used \[9\]\[ 4\]. Guiard et al \[9\] distinguish between two types of pointing involved in multi-scale (zoomable) off-screen target acquisition: _view-pointing_ in which the user moves their view until the target is visible; and _cursor-pointing_ in which the user moves the cursor over the final target. Our study essentially examines _view-pointing_ for the entire acquisition process.

## 2.2 3D Game Environments

Gaming is one of the main uses of computer systems, with over 60% of all Americans playing video games \[1\]. “First-person shooters” are a game-genre in which t he player is immersed in a virtual-reality environment viewed from the first-person perspective. To reduce the co st and complexity of game development many games ar e typically built from the same core code component, called the ‘game engine’. The game engine supports fundamental game features such as graphics renderin g, physics modeling, and lighting. Many game engines also provide facilities that allow users to tailor and edit the game content. For example, the game ‘Unreal Tournament’ 1 supports an integrated scripting language called UnrealScript. We used UnrealScript to support our investigation of pan-based target acquisition. First-person shooter games are extremely violent, r aising obvious ethical questions about their use in HCI research. Most game engines, however, allow some or all of the violent content to be removed.

## 3 EXPERIMENT

The purpose of the experiment is to determine wheth er Fitts’ Law accurately models pan-based target ac quisition in 3D game environments and, assuming it does, to c ompare the resultant models of traditional and pan-based target acquisition. If the Fitts’ Law models are similar and accurate, then 3D game environments may b e usable as substitutes for traditional pointing in Fitts’ Law analyses. The traditional and 3D-game interfaces were designe d to make equitable targeting demands, except for t he essential differences between targeting through cur sor movement (traditional) and through panning (3D game). Performance data was recorded in log files generate d automatically by the programs. Participants briefly practiced with each interface prior to completing 100 logged tasks with each interface. The traditional system, shown in Figure 2a, consist ed of one 20x200pixel green target displayed within a 1000x700 window. The users’ tasks involved selectin g the green bar as quickly as possible; with each s election causing the target to move to a new x-coordinate (the bar was displayed at a constant y-coordinate, half way down

## EpicGames, 1998. [www.unrealtournament.com](http://www.unrealtournament.com/)

* * *

the window). The target positions were selected ran domly from a set of seven positions. Mouse accelera tion was disabled, with a constant control-display gain of 1:1 \[7\]. The 3D game-based interface, shown in Figure 2b, wa s implemented using UnrealScript, and was displayed in full-screen mode on a 1024x768 monitor. The targets were ‘aliens’ and target acquisition involved shooting the aliens with a ‘blaster’. Like the traditional interface, only one target was displayed at a time, and successfully ‘blasting’ an alien (by clicking on it) caused another one to appear at one of seven randomly selected locations on the x-axis. The target aliens had a cylindrical collision volume, meaning that although they appeared to be alien- shaped, they actually presented a rectangular targe t to the user. The aliens did not move, and the use r’s movement within the 3D world was restricted to rota tion (panning) around a fixed point. The game’s sou nd and explosion effects were maintained. Mouse accelerati on was disabled, with a 1:1 control-display gain, and a low in- game mouse sensitivity setting was used. All eleven participants (9 males and 2 females) studied or worked in a university Computer Science dep artment, and while eight of them regularly played computer g ames, three did not play games at all. After comple ting their tasks with each interface type the participants responded to several questions assessing their enjoyme nt of the environment and about their perception of the violent content in the 3D environment.

## 3.1 Amplitude and Width Measurements

Measurements of amplitude (distance to target) and target-width are required for Fitts’ Law calculations. Direct pixel values are used with the traditional interface, but pixel measurements are complicated in 3D env ironments, requiring angular measures to be used instead. The amplitude is the angle through which the user must rotate to bring the target to the screen-centre, and the width is the portion of the user’s field of view that the target occupies. Bearing in mind that targets are presented along a single dimension, the player’s rotation refers only to the player’s yaw. A field of view of 90 ° was used throughout the experiment.

## 4 RESULTS

The tasks were completed quickly, with high levels of concentration, and with low error rates, as is normal for Fitts’ Law studies. It was clear that the participants enjoyed using the 3D-game environment much more, with several participants smiling and making verbal utterance such as “kapow!” and “gotcha” throughout their 3D tasks. The overall mean selection time was faster with traditional targeting (mean 0.86secs, standard deviation 0.24) than with the 3D game environment (mean 1.09secs, s.d. 0.24), but this comparison is unimportant due t o differences between the task Index of Difficulty values with the two interfaces. We inspected whether the non-gamers produced outlie r slow values in game-based selections, but found n o evidence of slow performance; indeed, one of the no n-game players produced the lowest mean selection t ime with both interfaces. Linear regression between movement time and Index o f Difficulty showed excellent Fitts’ models for both traditional and pan-based targeting. Figure 3 shows the data-points and linear lines of best fit for the two interface types. For traditional pointing, the Fitts’ Law model is, as expected, highly accurate, with _MT=0.18xIoD+0.19,_ _R_ _2_ _=0.94_, p<0.01, giving an _IoP_ value of

5.5 bits/second (in-line with MacKenzie’s 1.8 \[6\] studies showing mouse-based IoPTraditional Pointing
MT= 0.19IoD + 0.46 values of 5.6). The Fitts’ model of 3D

1.6
R 2 = 0.93 pan-based pointing is also accurate, with

1.4 3D Game Pan-pointing
_MT=0.19xIoD+0.46, R_ _2_ _=0.93_, p<0.01, giving a similar _IoP_ value of 5.3 1.2 bits/second\*.\* The two important IoPMT= 0.18IoD + 0.19 measures are therefore within 4% of one 1 R 2 = 0.94 another, indicating that user performance

0.8
with the two pointing mechanisms degrades at a similar rate as the distance 0.6 to the target increases. Questionnaire responses supported \*\*Mean movement time (seconds)\*\*0.4

our casual observations of higher

0.2
enjoyment with the 3D interface. Mean responses to a semantic-differential scale 0 of enjoyment (1=“not enjoyable”, 5=“very 0 1 2 3 4 5 6 7 enjoyable”) were a low 1.9 (s.d. 0.5) with **Index of Difficulty (IoD)** traditional targeting, but a high 4.3 (s.d.

0.5) with the 3D game environment **FIGURE 3:** Mean movement times plotted against Index of Difficulty, with Fitts’ (p<0.01, Wilcoxon). All eleven Models determined through linear regression.

* * *

participants stated that they preferred using the game environment.

We also asked questions regarding the violent conte nt of the game (the aliens exploded when successful

We also asked questions regarding the violent conte nt of the game (the aliens exploded when successful ly
acquired), with participants checking boxes when th ey agreed that the violence was “disturbing”, “irre levant”,
“entertaining” and “motivating”. Worryingly, 18% of the participants agreed that the violence was “disturbing”; yet
they all would prefer to use the game. 55% rated th e violence as “irrelevant”, probably because they have been
inured to it in their everyday game-play. 72% checked the box to state that the violence was entertaining, and 46%
agreed that it was motivating.

5 DISCUSSION

The results show that Fitts’ Law accurately models 3D pan-based target acquisition, and that the important Index of
Performance measures resulting from Fitts’ Law anal ysis are very similar for traditional and pan-based pointing.
The main difference between the two Fitts’ models is in the value of the constant a, or the intercept of the linear

The main difference between the two Fitts’ models is in the value of the constant a, or the intercept of the linear
model at IoD=0. With traditional pointing a=0.19secs, but with pan-based game pointing a=0.46secs. This constant
is often characterised as the cognitive and motor p reparation time that precedes movement towards the target.
Without further research it is risky to hypothesise about the cause of the difference. It could be that the different
lighting effects in the 3D environment (the target is more subtly illuminated within the 3D scene) slowed the initial
perception of the target, or it could be that the final stages of acquisition are slower by a constant amount
(independent of Index of Difficulty) in pan-based acquisition. Further research is needed to explain this constant
cost effect in pan-based 3D acquisition.

6 CONCLUSIONS

3D games are graphically and aurally rich environme nts that are built to entertain people. They are highly refined,
and many offer sophisticated tailoring capabilities that allow researchers to adapt them to their needs. The popular
first-person-shooter game genre uses an unusual met aphor for target acquisition in which the user pans the
environment (through a strong metaphor of bodily rotation) to bring the target to the centre of their view, rather than
the traditional approach of placing the cursor over the target.

This paper has shown that Fitts’ Law accurately mod els pan-based target acquisition in 3D gaming

the traditional approach of placing the cursor over the target.

This paper has shown that Fitts’ Law accurately mod els pan-based target acquisition in 3D gaming
environments, that target acquisition in these environments is dramatically more entertaining than traditional target
acquisition, and that there is little difference between the important Fitts’ Law Index of Performance measure when
using the same mouse input device for traditional a nd 3D pan-based acquisition. 3D gaming environments
therefore appear to be a viable tool for researcher s wishing to compare different input devices, parti cularly in
longitudinal studies when participant recruitment for ‘tedious’ tasks is difficult.

REFERENCES

01. State of the Industry Report, 2001-2002, Interactive Digital Software Association, 2002.

02. Cockburn, A. and Firth, A., Improving the Acquis ition of Small Targets. in People and Computers XVI I
    (Proceedings of the 2003 British Computer Society C onference on Human-Computer Interaction.), (Bath, E ngland,
    2003), 181-196.

03. Fitts, P. The Information Capacity of the Human Motor System in Controlling the Amplitude of Moveme nt.
    Journal of Experimental Psychology, 47. 1954, 381-3 91.

04. Guiard, Y., Beaudouin-Lafon, M., Bastin, J., Pasveer, D. and Zhai, S., View Size and Pointing Difficulty in Multi-
    Scale Navigation. in Proceedings of Advanced Visual Interfaces, AVI04, (Gallipoli, Italy, 2004), ACM Press.

05. Hinckley, K., Cutrell, E., Bathiche, S. and Muss, T., Quantitative Analysis of Scrolling Techniques. in
    Proceedings of CHI'2002 Conference on Human Factors in Computing Systems, (Minneapolis, Minnesota, 200 2),
    65-72.

06. MacKenzie, I. Fitts' Law as a Performance Model in Human-Computer Interaction. 1991.

07. MacKenzie, I. and Riddersma, S. Effects of Outpu t Display and Control-Display Gain on Human Perform ance in
    Interactive Systems. Behaviour and Information Tech nology, 13 (5). 1994, 328--337.

08. McGuffin, M. and Balakrishnan, R., Acquisition o f Expanding Targets. in Proceedings of CHI'2002 Con ference
    on Human Factors in Computing Systems, (Minneapolis, Minnesota, 2002), 57--64.

09. State of the Industry Report, 2001-2002, Interactive Digital Software Association, 2002.

10. Cockburn, A. and Firth, A., Improving the Acquis ition of Small Targets. in People and Computers XVI

11. McGuffin, M. and Balakrishnan, R., Acquisition o f Expanding Targets. in Proceedings of CHI'2002 Con ference
    on Human Factors in Computing Systems, (Minneapolis, Minnesota, 2002), 57--64.

12. Savage, J. The Calibration and Evaluation of Spe ed-Dependent Automatic Zooming Interfaces Computer

13. Savage, J. The Calibration and Evaluation of Spe ed-Dependent Automatic Zooming Interfaces Computer
    Science, University of Canterbury, Christchurch, 2004.