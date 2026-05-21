Please switch to your local website for related news and events

[Change to local website](https://www.msi.com/blog/blog/msi-afterburner-on-screen-display)

[Back](https://www.msi.com/blogs)

[How To Guide](https://www.msi.com/blog/msi-afterburner-on-screen-display)

# MSI Afterburner Walkthrough Part 2: On Screen Display, Monitoring and Features

By Rick Novlesky\|September 27,2023

Graphics Cards


In our first MSI Afterburner article, we showed the interface for overclocking and undervolting. In this section we’ll cover more features like on screen display, hardware monitoring, custom fan profiles and video capture. Be careful to only download MSI Afterburner from msi.com or guru3d.com, as there are phishing download sites with modified installers that steal your data. To ensure you download the real version, verify and only [download MSI Afterburner from our website](https://www.msi.com/Landing/afterburner/graphics-cards) or Guru3D.

[This article:](https://www.msi.com/blog/msi-afterburner-on-screen-display)

- [Chapter 4a. ON SCREEN DISPLAY](https://www.msi.com/blog/msi-afterburner-on-screen-display#onscreendisplay)
- [Chapter 4b. ON SCREEN DISPLAY: RTSS](https://www.msi.com/blog/msi-afterburner-on-screen-display#onscreendisplayrtss)
- [Chapter 5. HARDWARE MONITORING](https://www.msi.com/blog/msi-afterburner-on-screen-display#hardwaremonitoring)
- [Chapter 6. CUSTOM FAN PROFILE](https://www.msi.com/blog/msi-afterburner-on-screen-display#customfanprofile)
- [Chapter 7. VIDEO CAPTURE](https://www.msi.com/blog/msi-afterburner-on-screen-display#videocapture)

[Previous article:](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide)

- [Chapter 1. BEFORE YOU START](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide/#beforeyoustart)
- [Chapter 2a. OC SCANNER: OVERCLOCKING](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide/#overclockingguide)
- [Chapter 2b. MANUAL OC, TROUBLESHOOTING](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide/#manualoverclocking)
- [Chapter 3a. UNDERVOLTING: CURVE EDITOR](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide/#undervoltingguide)
- [Chapter 3b. UNDERVOLTING TIPS, Q&A](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide/#undervoltingtips)

You’ll need the latest GPU drivers installed, and the latest version of MSI Afterburner, which comes bundled with RivaTuner Statistics Server which you will need to install to give monitoring stats and on screen display functionality. If you encounter installation issues because MSI Afterburner is already running, in the Settings menu you would disable the Start with Windows, hit OK to save that. Restart your PC and proceed with the installation.

Having a stability testing tool [such as FurMark](https://geeks3d.com/furmark/downloads/) can be useful for setup and testing. If you’ve just installed a new GPU, the first time you boot up, your PC will install the drivers for it, and you may need to restart your PC again to see the GPU recognized in MSI Afterburner. We'll address frequently asked questions, and we invite you to [subscribe to MSI Gaming](https://www.youtube.com/@MSIGamingOfficial?sub_confirmation=1) on YouTube for the latest updates, news, and tutorials.

## Chapter 4a. OSD / ON SCREEN DISPLAY

The on screen display can show all the GPU and CPU statistics on top of your gameplay, so let's set that up. First you need to select what items you want to see, then show those items in the OSD. Click on Settings or press Ctrl+S, then go to the Monitoring tab. Click the checkmark beside several stats to display them- GPU usage, GPU core clock, GPU temp, GPU Power, GPU memory usage, and GPU memory clock.

![MSI Afterburner guide 04a Selecting entries to monitor](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04a-Selecting-entries-to-monitor.jpg)

If you see GPU1 and GPU2, that's listing your integrated graphics on your CPU. Determine which is your GPU by closing this window, click the Info button or press Ctrl+I. Scroll down and at the bottom of the Information window, it will list what GPU1 and GPU2 are. Intel CPUs except those with "F" at the end all have integrated graphics; for AMD, Ryzen 7000 series do, but older AMD CPUs only with a "G" at the end have this.

![MSI Afterburner guide 04c Intel AMD CPU iGPU](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04c-Intel-AMD-CPU-iGPU.jpg)

Back in Settings/Monitoring tab, near the bottom you'll find the entries Framerate, Frametime, Framerate Avg and Framerate 1% Low. Drag to reposition these items, dragging a bit outside the top or bottom of the whole box will get the list to scroll. You can select a group and drag them all up, and you may get single items to jump to the top or bottom with the Home or End keys, with a little trial and effort.

![MSI Afterburner guide 04d Select Framerate entries](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04d-Select-Framerate-entries.jpg)

Now you have everything together, click on the top item and mouse wheel or drag the sidebar down. Holding Shift, click the bottom item to select them all. Enable monitoring for these statistics by clicking the checkmark icon on the left-hand side; click the box labeled Show in On Screen Display to for them to appear in the OSD. Click Apply to save your changes.

![MSI Afterburner guide 04f All entries monitored, shown in OSD](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04f-All-entries-monitored-%20shown-in-OSD.jpg)

At the top, click on the On Screen Display tab. Click in the Toggle On Screen Display box and press the Scroll Lock key, and click Apply so you can turn the OSD on and off. Note that global hotkeys like this one only function when the Settings menu is closed. You would need to click OK to close this window, have a game running, then press Scroll Lock to turn on the OSD.

![MSI Afterburner guide 04g OSD global hotkey Scroll Lock](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04g-OSD-global-hotkey-Scroll-Lock.jpg)

The OSD is currently likely missing or not updating the framerate average and 1% low- the benchmark mode must be recording in order to see them updating in real time. In Settings click the Benchmark tab at the top, and in the Global benchmark hotkey section, assign the F9 key and Shift+F9 to Begin and End recording. Click Apply, then the OK button. Again, global hotkeys will not work until you close the settings menu.

With a game or benchmark running, hit F9 and the Framerate Avg and 1% Low appear at the end of the D3D11 line. To refresh and clear previous data, hit F9 to start a new measurement. Shift+F9 will stop the benchmark, pausing the last values, and Shift+F9 again will turn off these stats. Press F9 to get them running again.

## Chapter 4b. ON SCREEN DISPLAY: RTSS

Returning to the Settings/On Screen Display tab, down this window on the right, click on the More button, which will bring up the RivaTuner Statistics Server (RTSS) window. You can also navigate here by right-clicking RivaTuner in the system tray and clicking Show.

![MSI Afterburner guide 04j More button opens RTSS](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04j-More-button-opens-RTSS.jpg)

The OSDs size can be adjusted with the Zoom slider, the number 60 in the box below reflects your adjustment. You can change the display corner, and drag the position around. If the OSD is hard to see on top of game text, click Display Fill on, and clicking on the right color dot will bring up a window where you can change the fill color and opacity.

In these RTSS settings, you can check to see if Benchmark mode is on by clicking Setup. In the new window, scrolling down to the bottom, Enable Benchmark Mode should be checked. The keys you assigned, F9 and Shift+F9, will toggle this on or off.

Clicking on the Plugins tab shows two entries, with the very useful OverlayEditor. Double-click to open, and the OSD shows without a game or benchmark running. But it blocks adjusting zoom and other settings in the main RTSS window. However, clicking on MSI Afterburner, and back in the Settings/Monitoring tab, you can configure everything the way you want in real-time.

![MSI Afterburner guide 04n OverlEdit open, configure Monitoring](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04n-OverlEdit-open-configure-Monitoring.jpg)

In Settings/Monitoring, to get a Framerate graph, select Framerate, and Show in OSD has a drop-down menu that says text. Change this to "text, graph." The graph speed scrolls according to your FPS, and faster with higher FPS values. If you're noticing stuttering or 1% lows, capping or limiting your framerate in the game may lessen these and get you a more consistent experience.

![MSI Afterburner guide 04q Adding two more graphs](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04q-Adding-two-more-graphs.jpg)

You can graph other stats, too. Select GPU usage and Core Clock together, and choose "text, graph" so you can monitor them. You can alter the graphs' size by clicking on the three dots, and scrolling to the bottom, double-click to change values and make them bigger or smaller. Let's add CPU usage, CPU clock, CPU temperature, RAM usage to your Monitored stats.

To modify text color groups, on the Show in OSD line, click on the three dots button next to text. Color 0 controls GPU entries, Color 1 does CPU entries, and System Color 0 handles GPU and CPU stats. For framerate stats, the entry names are Color 2, and Color 4 controls the stats themselves.

![MSI Afterburner guide 04t Changed OSD text result](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04t-Changed-OSD-text-result.jpg)

To change the text for OSD items, let's rename GPU usage by checking Override group name and typing your GPU in the box, "RTX 4070 Ti" for example, and hit Apply. GPU Usage gets its own line now. If you want Core Clock to appear on the same line then select it, click to check Override Group name and type in the exact same text, and click Apply.

![MSI Afterburner guide 04u Change GPU name in OSD](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-04u-Change-GPU-name-in-OSD.jpg)

Certain games like Forza Horizon 4 may not show the OSD, in this case you can try Windows Game Bar to view some statistics. With your game running and selected, press Win+G, and click the Widget Menu, then Performance. You'll need to Pin the window for it to stay up. In Options you can adjust transparency, and you can click on an entry in the left column to show the graph you want. HWInfo may also be worth checking out, to see if it's able to show stats in this instance.

## Chapter 5. HARDWARE MONITORING

Moving on to hardware monitoring, and clicking the heartbeat monitor icon or Ctrl+M will show a separate hardware monitor window with graphs. The graphs include everything you've selected by checking to monitor in the previous section for [the On Screen Display](https://www.msi.com/blog/msi-afterburner-on-screen-display#onscreendisplay). If you want to add entries, click the check mark on the left side of the entry. Try adding add Fan speed, and so you can see everything, change Graph Columns to 3 and click Apply.

![MSI Afterburner guide 05c GPU fan speed now top item](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-05c-GPU-fan-speed-now-top-item.jpg)

The graphs go left to right, and Fan speed appears at the bottom. It won't be in the OSD because you didn't check the Show in On Screen Display box. If you want it to display at the top, drag this all the way up, and click Apply, so you can see it as the top graph item.

Right-clicking on the monitor, you can Pause, Clear History, and more, including disabling Always on top, if you'd like this window to go behind others. You can attach graphs to each other, like Fan Speed and Cpu usage, and similar graph sizes work well together as they both have 0 to 100 ranges. If you right-click on a graph and click Properties, it will bring up the Settings/Monitoring tab with the same graph selected.

![MSI Afterburner guide 05e Combining graph start](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-05e-Combining-graph-start.jpg)

Enabling graphs while you're playing gives you real-time stats on temps, fan speed and more, but can block a part of your game screen. Having a second monitor for Afterburner configuration and graph monitoring is a great idea, even a regular 1080p 60Hz display will do just fine, and allow you to stream or have game guides and maps up while you play.

While a faster than 60 Hz 1080p panel may have DisplayPort, most 1080p displays only have HDMI. Connect your faster gaming monitor with DisplayPort, and use your second screen with DisplayPort or HDMI. Using DisplayPort to connect your gaming monitor always delivers the max FPS, getting around HDMI hardware limitations that some monitors have. Also note that stuttering can rarely occur in Windows with monitors running at different refresh rates.

## Chapter 6. CUSTOM FAN PROFILE

Let's see how to set a custom fan profile for our graphics card. Run a benchmark like Unigine Heaven or FurMark for a few minutes and keep it running in the background while you adjust fan speeds. Write down your framerates and GPU temperature- when your GPU is running at max load, making your fans slower and quieter will start to overheat your card, and you'll lose FPS as the card thermal throttles.

In Settings, click the Fan tab at the top, and click Enable user defined software automatic fan control. If you have a non-MSI card and don't see any changes, you can try Using firmware control mode, and if your fans start/stop all the time, the Override zero fan speed with hardware curve may help. Keep in mind your case fans need to move enough air through your case to feed cool air to your graphics card.

![MSI Afterburner guide 06c Faster, more aggressive fan speed cooling](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-06c-Faster-more-aggressive-fan-speed-cooling.jpg)

On the graph, you'll see points on a line, for faster or more aggressive cooling, you can select them and move them to the left, and for quieter fans, you can move them to the right. In testing, you'll see your GPU plateau as it reaches a heat load/cooling equilibrium, so higher fan speeds means it should take longer to climb up. You can add a point by clicking on the line, and pressing delete will get rid of it.

At idle, many GPUs have a zero fan speed mode, where they stop spinning. Clicking Override zero fan speed with hardware curve may allow you to have a zero fan speed mode for your fans and still use a custom fan curve. When gaming, your case fans should be speeding up to move more air through and keep your GPU cool. If you've adjusted your GPU fan speed to be quiet at idle and your PC is still noisy, adjust the CPU cooler and case fan curves in BIOS, or MSI center.

Seeing the current fan speed is easy as it shows the percent in the main Afterburner window, and in the hardware monitor graphs if you selected the entry to be monitored. Some FAQs: There's one custom fan profile, so the fan profile doesn't change when selecting different saved profiles. How about setting a custom fan profile for a laptop GPU? Unfortunately not, as it's set by the manufacturer in BIOS.

![MSI Afterburner guide 06g GPU fan speed in graph](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-06g-GPU-fan-speed-in-graph.jpg)

## Chapter 7. VIDEO CAPTURE

In the Settings/Video Capture tab, you can set your capture or pre-record hotkeys, unlocking more settings. Again, global hotkeys don't work with the Settings window open. Video format has options for compression in the drop down menu; if you choose external plugin, the small button with three dots to the right will become active. Note you'll need to use Matroska as the container format.

![MSI Afterburner guide 07a Video capture external plugin](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-07a-Video-capture-external-plugin.jpg)

![MSI Afterburner guide 07c VC configuring external plugin](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-07c-VC-configuring-external-plugin.jpg)

![MSI Afterburner guide 07d VC Typical encoder settings](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-07d-VC-Typical-encoder-settings.jpg)

Clicking the three dots button, the Encoder drop down has NVIDIA, Intel or AMD. Note there's no restriction on choice but the hardware needs to be in your system; if you're trying Intel Quick Sync on an AMD Ryzen based PC, that won't work. Configure will bring up the encoder settings, which are slightly different for each, though they all have Target Usage choosing between speed and quality, and target bitrate.

Back to the Video Capture tab there's quality, frame size, rate and limiter options, higher values look better but have larger file sizes, and the save location can be customized, along with compatibility options below. For audio capture, you can define different sources along with a Push-to-Talk hotkey, and you can downmix to stereo and after you select a second audio source, mixing multiple audio tracks becomes available. This allows you to combine audio sources into your recordings.

![MSI Afterburner guide 07e VC Choose audio source1](https://storage-asset.msi.com/global/picture/news/2023/vga/MSI-Afterburner-guide-07e-VC-Choose-audio-source1.jpg)

Finally, in the Screen Capture tab up at the top, there's hotkeys to snap in-game screen grabs, too. MSI Afterburner has a lot of customization for capturing available, and you may also want to check out Nvidia Share and AMD Radeon ReLive for game capture, they're highly optimized software for each brand of GPU with very little impact on FPS while gaming.

That's our guide to all the features in MSI Afterburner, the most used overclocking and versatile GPU tweaking solution around. If you're looking at new hardware or a graphics card, there's a wide selection on the MSI website with our latest products and their features and specs.

Click to go to our [MSI Afterburner Overclocking](https://www.msi.com/blog/msi-afterburner-overclocking-undervolting-guide) article which also covers Undervolting your GPU.

Watch MSI Afterburner Overclocking & Undervolting Guide:

All You Need To Know \| MSI AFTERBURNER Overclocking & Undervolting Full Walkthrough \| MSI - YouTube

Tap to unmute

[All You Need To Know \| MSI AFTERBURNER Overclocking & Undervolting Full Walkthrough \| MSI](https://www.youtube.com/watch?v=f2zQiMKdso8) [MSI Gaming](https://www.youtube.com/channel/UCezxSW8kVueIU0pr_rukbYA)

![thumbnail-image](https://yt3.ggpht.com/2DX4Z9HQJSASIe6r-LQUNlhaktVBwV1U-sMyl8lpJUdfHYNdvi-ZcXyYAKv-welrPvr-8qxnghE=s68-c-k-c0x00ffffff-no-rj)

MSI Gaming1.67M subscribers

[Watch on](https://www.youtube.com/watch?v=f2zQiMKdso8)

Watch MSI Afterburner Quick Walkthrough

All You Need To Know \| MSI AFTERBURNER How to Easily Overclock, Undervolt A GPU \| MSI - YouTube

Tap to unmute

[All You Need To Know \| MSI AFTERBURNER How to Easily Overclock, Undervolt A GPU \| MSI](https://www.youtube.com/watch?v=HZdTex-tsgM) [MSI Gaming](https://www.youtube.com/channel/UCezxSW8kVueIU0pr_rukbYA)

![thumbnail-image](https://yt3.ggpht.com/2DX4Z9HQJSASIe6r-LQUNlhaktVBwV1U-sMyl8lpJUdfHYNdvi-ZcXyYAKv-welrPvr-8qxnghE=s68-c-k-c0x00ffffff-no-rj)

MSI Gaming1.67M subscribers

[Watch on](https://www.youtube.com/watch?v=HZdTex-tsgM)

### Related Blogs

[![What Is a Fleet? More Businesses Qualify Than You Think](https://storage-asset.msi.com/global/picture/article/article_17787385717c922f654ffc787d9cbc0b0ba35a886e.jpeg)\\
How To Guide\\
\\
\\
May 29,2026\\
\\
What Is a Fleet? More Businesses Qualify Than You Think\\
\\
MSI EVSE Most business owners don't think of themselves as fleet owners or managers. However, a fleet is simply any \[...\]\\
\\
EV Charging Solutions](https://www.msi.com/blog/what-is-an-electric-fleet)

[![Building a Maintenance Revenue Stream from AC Charging Installations](https://storage-asset.msi.com/global/picture/article/article_177736046840857c377e25f45e82bbc16617c54654.jpeg)\\
How To Guide\\
\\
\\
May 15,2026\\
\\
Building a Maintenance Revenue Stream from AC Charging Installations\\
\\
MSI EVSE Most installers treat EV charger installation as a one-time job. The ones building recurring revenue offer annual inspection \[...\]\\
\\
EV Charging Solutions](https://www.msi.com/blog/ev-charger-maintenance-revenue-stream)

[![MSI Memory Extension Mode: Boost Your Intel Core Ultra 7 270K Plus Gaming Performance in One Click](https://storage-asset.msi.com/global/picture/article/article_1778550507467b8f599a41e8825b83d9b75cac5873.png)\\
How To Guide\\
\\
\\
May 13,2026\\
\\
MSI Memory Extension Mode: Boost Your Intel Core Ultra 7 270K Plus Gaming Performance in One Click\\
\\
Just picked up an Intel Core Ultra 7 Plus and want better gaming performance? Reducing memory latency is one of the \[...\]\\
\\
Motherboards](https://www.msi.com/blog/msi-memory-extension-mode-boost-your-intel-core-ultra-7-270k-plus-gaming-performance-in-one-click)

Subscribe to Our Blog

Stay up to date with the latest hardware, tips and news

Subscribe

Please check the email format

### Subscribe to MSI Newsletters

Please check the box if you would like to receive our latest news and updates.By clicking here, you consent to the processing of your personal data by \[Micro-Star International Co., LTD.\] to send you information about \[MSI’s products, services and upcoming events\]. Please note that you can unsubscribe from the MSI Newsletters [here](https://account.msi.com/unsubscribe) at any time.

Further details of our data processing activities are available in the [MSI Privacy Policy](https://account.msi.com/terms/privacy_policy)

[Share this page on Facebook](https://www.facebook.com/sharer/sharer.php?t=MSI+Afterburner+Walkthrough+Part+2%3A+On+Screen+Display%2C+Monitoring+and+Features&u=https%3A%2F%2Fwww.msi.com%2Fblog%2Fmsi-afterburner-on-screen-display "Share this page on Facebook")[Tweet about this page](https://twitter.com/intent/tweet?text=MSI+Afterburner+Walkthrough+Part+2%3A+On+Screen+Display%2C+Monitoring+and+Features&url=https%3A%2F%2Fwww.msi.com%2Fblog%2Fmsi-afterburner-on-screen-display "Tweet about this page")[Share this page by email](mailto:example@example.com?subject=MSI%20BLOG&body=https%3A%2F%2Fwww.msi.com%2Fblog%2Fmsi-afterburner-on-screen-display "Share this page by email")