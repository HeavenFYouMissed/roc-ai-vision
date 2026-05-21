[The Gistre Blog](https://blog.gistre.epita.fr/)

## [A History of Anti-Cheat Techniques in Video Games, from Server-Side Code to Kernel Level](https://blog.gistre.epita.fr/posts/lucas.demenais-2025-06-24-history_of_anticheat/)

2025-11-19 3262 words 16 minutes[anti-cheat](https://blog.gistre.epita.fr/tags/anti-cheat) [gaming](https://blog.gistre.epita.fr/tags/gaming) [kernel](https://blog.gistre.epita.fr/tags/kernel) [english](https://blog.gistre.epita.fr/tags/english) Lucas Demenais

## 1\. Introduction

Cheating has been around in video games for as long as games have existed.
In single-player games, it’s often seen as pretty harmless.
But when it comes to online games, cheating quickly becomes a serious problem.
Whether it’s snapping to enemy heads, seeing through walls, or generating endless in-game currency, cheats can ruin the fairness and experience for everyone else.

In competitive environments where rankings, tournaments, and even real money are involved, the impact of cheating is even more significant.

To fight back, developers rely on anti-cheat systems.
These are built to detect and prevent a wide range of unfair behavior.
While they work in different ways (which we’ll get into), they generally fall into two main categories: **server-side** and **client-side**.
Server-side anti-cheat checks gameplay data on the game’s servers for anything suspicious, whereas client-side anti-cheat monitors the player’s computer for signs of cheating.

The history of anti-cheat software is basically a technological arms race.
As developers create better detection methods, cheat creators come up with new ways to get around them. Cheats have evolved from basic memory hacks to much more complex tools and so has anti-cheat.

This article will try to dive into that history, focusing on how anti-cheat technologies have progressed over time.
We’ll look at the shift from early server-side systems to today’s often controversial kernel-level drivers, and explore why platforms like Linux are frequently left out.
We’ll also touch on what the future might hold for these systems.

We won’t cover in detail manual or community-based anti-cheat efforts—such as player reporting, replay reviews, or volunteer moderators.
While these methods are important, our focus here is on the automated systems that operate behind the scenes.
However, it could be noted that such manual work is sometimes used to confirm suspicions before taking actions (e.g Counter-Strike Overwatch system).
We will also not cover built-in cheats (cheat codes) as they are not much interesting from a technical perspective.

## 2\. Early Days: Server-Side Enforcement

Before battle royales, before ranked matchmaking, and even before voice chat, there was online multiplayer.
The roots of online gaming go back further than most people realize, back to the 1970s, when early networked games like MUD1 ran on university mainframes.
It quickly became apparent that we needed some systems to keep players from being able to do everything.

### The Basic Client-Server Model

Let’s start with the fundamentals. Most multiplayer games use a client-server architecture. The game client (on your computer or console) sends data called packets to a central server that keeps track of everyone’s actions and the game state.
Think of a packet like a message in a bottle, containing information like “Player X moved forward 3 meters” or “Player Y shot at coordinates (120, 80).”

A common approach is for the client to send position updates every few milliseconds.
You might wonder: why not directly send every keypress or mouse movement?
The answer is bandwidth and efficiency.
The game doesn’t need to know that you pressed “W”, it just needs to know where you ended up.
So instead of raw inputs, you send movement summaries at regular intervals.

### The Teleport Hack Example

Now imagine a player intercepts one of their movement packets and changes the position from something like (X: 100, Y: 200) to (X: 9999, Y: 9999) instantly teleporting across the map.

Even worse, this tampering doesn’t even have to happen on the player’s machine.
Someone with access to the network (say, a compromised router or a “middlebox” with packet injection capabilities) could alter the data in transit.

Here is an overview of the issue.

![Procedure for a teleportation hack](https://blog.gistre.epita.fr/images/lucas.demenais/lucas.demenais-2025-06-24-history_of_anticheat/teleport-hack.png)

This highlights the problem: the server was blindly trusting whatever packet it received, without verifying whether it made sense.

### Authoritative Server Model

To combat this, developers began to adopt the Authoritative Server Model.
It is a now-standard practice in online games. In this setup, the server doesn’t just take the client’s word for it.
Instead, it checks if the movement or action is valid, given the game rules.

Let’s revisit our teleport hack.
With an authoritative server, when the server receives a movement packet, it compares the new position to the last known one and calculates the speed.
If the calculated speed exceeds the maximum allowed, the move is rejected or the player is snapped back to their last valid position.

For example, in Counter-Strike 1.6, trying to move too quickly or outside valid boundaries would cause your character to rubber-band back.
In Minecraft, if you don’t have flying enabled, trying to fly via a client mod will simply result in the server repositioning you back to the ground.

This model makes certain cheats, like speedhacks or basic teleportation, almost useless—unless the cheater can also trick the server-side checks (which is much harder).

### Statistical Detection and Obfuscation

But what about cheats that look valid to the server?
Like perfectly timed headshots or precise movement?

This is where other methods, other server-side techniques like statistical detection, come into play.
Rather than check if each individual packet is legal, the server monitors behavior over time.
If a player consistently lands headshots at 99% accuracy or reacts to enemies in 50 milliseconds flat, that may raise flags.
These systems don’t always lead to immediate bans but can trigger alarms for manual investigation.

Some developers also started experimenting with network obfuscation, adding randomization or encryption to data flows to make it harder for cheat tools to read and manipulate packets.
While not a perfect solution, it adds complexity to cheating.

### When Server Authority Goes Missing

Today, you would think all online games would use an authoritative model by default.
But surprisingly, that’s not always the case.

Take Fall Guys, a game that launched with notoriously poor anti-cheat enforcement.
Even today, you can see players teleporting directly to the finish line.
Why? Because the game relies too heavily on the client to report positions, likely a design decision made to keep server costs low for a massive player base.

Or look at GTA Online, a game where hackers can spawn money, vehicles, or even other players, thanks to a blend of weak server-side checks and overreliance on the client.
It’s not always “bad code”, sometimes it’s a trade-off between performance, server load, and complexity.

### Pros and Cons of Server-Side Enforcement.

While server-side enforcement is foundational to modern online games, it has clear limits.
On the plus side, it’s relatively safe and efficient and it doesn’t need to access the player’s system beyond basic networking. But it’s not foolproof.
It can’t see what the player sees, so visual cheats like wallhacks or overlays go completely unnoticed.
It also adds load to the server, which can be costly or lead to latency in certain cases.
And since it only validates what the client sends, it often reacts after the fact cheating actions can still occur, even if they’re later corrected or flagged.

This fundamental limitation is what pushed developers to dig deeper—into the players’ machines themselves.

## 3\. The Rise of Client-Side Anti-Cheat

Now after a server authority, players could no longer teleport, but they still can see through walls, auto-aim at enemies, or track player positions using hidden overlays.
These techniques didn’t require sending invalid data to the server—they operated entirely within the player’s machine. And that meant the server couldn’t see them.

### Fighting the Cheats from Inside

Client-side anti-cheat software runs on the player’s computer, alongside the game, and monitors everything from memory usage to running processes to file changes. The idea is to detect or block suspicious behavior before it can affect the game. Think of it like an antivirus, but tailored specifically for games.

One of the earliest and most well-known example of client-side anti-cheat was PunkBuster, released in 2000, which started appearing in games like Battlefield 1942.
Around the same time, Valve rolled out VAC (Valve Anti-Cheat) for Counter-Strike and later CS: Source.
These systems laid the groundwork for how client-side anti-cheat would function for years to come.

You might notice that from this point, due to the increasing complexity of anti-cheat, it became its own specialized field.
Companies like Even Balance (behind PunkBuster), Valve (with VAC), and later BattlEye and Easy Anti-Cheat started offering off-the-shelf anti-cheat solutions to game studios.
These systems were constantly updated, backed by dedicated engineering teams, and made specifically to detect new types of cheats.

At the same time, cheat developers also professionalized.
Some sold monthly subscriptions.
Others used encrypted delivery channels and DRM to protect their cheats from being reverse-engineered.
In a way, the cheat market became a shadow industry operating in parallel to the games themselves.

Client-side anti-cheat gave developers a much-needed window into the player’s environment.
But it also raised new questions—about privacy, security, and system stability.

Let’s break down some of the core techniques they used and still use.

### Game File Verification

One of the simplest but surprisingly effective cheat strategies is just… modifying the game. For example, a player could swap out a texture file so that enemy players are glowing red or completely transparent walls are now see-through.
That’s a basic wallhack, and it doesn’t even need to touch memory or inject code!

To counter this, anti-cheats introduced file verification systems. These work by calculating a hash, which is a sort of digital fingerprint for each game file.
If the hash doesn’t match what it’s supposed to be, that’s a red flag.
It might mean the player has added, modified or deleted game assets.

But of course, nothing’s ever that simple. A cheat can also modify the hashing system itself to return the “right” result even when the file has been changed.
So while file checks are a good start, they’re easy to bypass by more sophisticated cheats.

### Memory and Process Scanning

Where things really escalated was in memory and process scanning.
A running game stores tons of valuable information in your computer’s RAM—player coordinates, health values, timers, weapon states, etc.
All of this is technically accessible to any other program running on the same machine.

This is exactly how many cheats operate: by scanning the game’s memory for useful values and either reading them (to display extra info) or changing them (to give you infinite ammo or invincibility).

So anti-cheat software started scanning memory too.
If it found known cheat signatures—certain byte patterns or suspicious memory access patterns, it could trigger an alert or ban the player.
This can be as simple as checking for known process names (like `WallHackPro.exe`) or as complex as searching for unique code patterns or strings commonly used by cheat tools.

The approach is similar to how antivirus software works: keep a database of known threats and constantly check the system against it. Of course, this leads to a cat-and-mouse game, cheat developers disguise their tools under innocent-looking names and try to spoof their behavior to avoid detection.

While renaming a cheat to `word.exe` ( [real story](https://www.vice.com/en/article/professional-counter-strike-player-caught-cheating-in-tournament/)) might fool a naive scanner, the underlying behavior, like continuously scanning a game’s memory for coordinates can still be flagged.

This is where anti-cheat tools shift from surface-level scanning to behavioral monitoring, watching not just what a program is called, but how it behaves.

### Obfuscation

Developers started obfuscating their game’s code and memory layout, intentionally making it harder for external tools to understand what’s going on.
Location in RAM (addresses) for important data would shift every time the game is launched.
Code paths would be scrambled or randomized.

This didn’t make cheating impossible, but it raised the bar significantly.
Cheat developers now had to reverse-engineer **every version** of the game, figure out what changed, and update their tools constantly.
What used to be a weekend project now took full-time effort.

### Why Not Just Block Everything?

At some point, the logical question arises: why not just prevent any process from reading or modifying the game’s memory in the first place?

That’s the dream.
But in practice, it’s incredibly difficult due to how operating systems manage privileges.
On most platforms, processes run in what we call **user space**, and any program with the same privilege level can, in theory, interact with others.
To lock things down, the anti-cheat would need higher permissions than everything else, which comes with its own set of risks (more on that in the next chapter).

It turns out that securing a game client within a general-purpose OS like Windows is kind of like locking a safe in the middle of a public park.
You can build the best safe in the world, but if people are allowed to walk up and poke at it, you’ve still got a problem.

## The Kernel-Level Era

Several popular games use kernel-level anti-cheat solutions. Some well-known examples include:

- Vanguard (used in Valorant)
- Ricochet (used in Call of Duty)
- Easy Anti-Cheat (used in many online games)

### What Is the Kernel and What Does “Kernel-Level” Mean?

The kernel is the core part of an operating system like Windows.
You can think of it as the control center or even the police force of your computer.
It manages access to hardware and ensures that programs don’t do things they’re not supposed to. For example, if a program wants to read another program’s memory, it has to go through the kernel first.

When we say “kernel-level anti-cheat” we mean that the anti-cheat software runs at the same level as the operating system’s most trusted code.
On Windows, this is typically done through a driver, similar to how your computer uses drivers to control a mouse or a printer.

The level of access is implemented with something called **protection-rings**, while more than 2 exist, today’s OSes only use 2.
Programs you run normally, like games or browsers, operate in Ring 1, a more restricted zone.
These modes of operation are also enforced by the design of the CPU itself!

![Protection Rings in Windows](https://blog.gistre.epita.fr/images/lucas.demenais/lucas.demenais-2025-06-24-history_of_anticheat/protection_rings.png)

### Why Use Kernel-Level Anti-Cheat?

There are two main reasons why modern anti-cheats operate at the kernel level:

1. To block cheats from reading or changing the game’s memory. Without this protection, a cheat program could look inside the game, find out where enemies are, how much health they have, and more.

2. To detect cheats that run at the same low level. If a cheat is also running in the kernel, it could completely hide from a regular, user-level anti-cheat. By running at Ring 0, anti-cheat software gets a fighting chance to see or block these stealthy tools.


This approach makes cheating much harder. Unless there’s a bug or vulnerability in the anti-cheat itself, most traditional cheating methods won’t work.

### The Rise of DMA Cheats

Even with kernel-level protection, one kind of cheat stands out as especially hard to stop: DMA cheats.

DMA stands for Direct Memory Access, a feature that allows certain hardware devices to read your computer’s memory without bothering the CPU.
This means a cheat using DMA can peek into the game’s memory without any software—even the kernel—knowing it happened.

In theory, DMA access cannot be detected because it bypasses the operating system entirely.
However, some anti-cheat systems scan for unusual or unauthorized devices connected to the system. By looking at hardware IDs, they may identify suspicious devices.
Unfortunately, some DMA tools can spoof these IDs to look like a regular USB drive or graphics card, making detection even harder.

However these cheats usually require a second machine and specialized hardware like a DMA card, making them difficult and expensive to put together.

Here is an example of the architecture of a Wallhack using DMA:

![Wallhack Architecture using DMA](https://blog.gistre.epita.fr/images/lucas.demenais/lucas.demenais-2025-06-24-history_of_anticheat/dma.png)

Still, they are sometimes considered the “final boss” of cheating.

### Concerns About Kernel-Level Anti-Cheat

While effective, kernel-level anti-cheat raises serious concerns:

Trust issues are at the core of the debate. Giving a game developer’s software deep access to your computer means trusting them not to abuse it.
If something goes wrong, the damage can be severe.

There have already been cases that fuel this distrust.
One example is [when attackers used the anti-cheat driver for Genshin Impact to disable antivirus software, allowing ransomware to spread more easily](https://www.trendmicro.com/en_us/research/22/h/ransomware-actor-abuses-genshin-impact-anti-cheat-driver-to-kill-antivirus.html)
).

In another case, [the ESEA gaming platform was caught using its anti-cheat to secretly mine Bitcoin on users’ computers](https://www.wired.com/2013/11/e-sports/).

Most of these systems are also closed-source.
That means independent experts can’t review the code to ensure it’s safe.
If there’s a bug or backdoor, players simply have to hope it won’t be exploited.

Public opinion has been divided.
Some players support strong anti-cheat to protect fair play.
Others worry about privacy and system stability, especially with software running so deep in the system.

### Why Anti-Cheat Rarely Works on Linux

Many games that use strong anti-cheat simply don’t support Linux, and the reason is actually more technical than political.

Linux is an open operating system.
Unlike Windows, it has no central authority controlling what runs in the kernel.
This openness is a strength in many ways, but it also means that anti-cheat systems can’t rely on strict rules to enforce security.

If an anti-cheat tries to block memory access in Linux, a cheater could just recompile the kernel and remove those blocks.
If the anti-cheat uses kernel modules, those can be swapped out or tricked.
If it relies on user-level checks, a cheat can simply run with higher permissions in a hidden environment.

In short, there’s no reliable way to hide from or block a determined cheat on Linux.

You might say that some anti-cheats, like Easy Anti-Cheat (EAC), appear to support Linux on paper.
But in reality, this support is very basic. It usually just helps the game talk to its servers securely, not actually detect or prevent cheating.

One example is Apex Legends, which allows EAC to run under Linux using Proton.
However, cheats for this game can still run on Linux without needing to bypass any anti-cheat.
The protections simply aren’t doing much.

## Conclusion

Anti-cheat systems have come a long way over the years.
From basic server-side checks to full-blown kernel-level drivers, the tech has grown more advanced and more aggressive.
These improvements have made cheating harder, but the fight is far from over.

There’s still no perfect system.
Every method has its own set of weaknesses, and cheat developers continue to adapt.
It’s a constant back-and-forth, like a never-ending arms race.
We’ve only scratched the surface in this article—there are plenty of other techniques we didn’t dive into, like hardware bans, DLL injection detection and a lot more…

Looking ahead, the future of anti-cheat may lean more on AI-based behavior analysis.
Instead of just scanning your system, these tools aim to learn how players behave and flag anything suspicious.
[Call of Duty, for example, has started using AI in its Ricochet anti-cheat to spot cheaters based on how they play, not just what’s running on their PC](https://www.callofduty.com/blog/2025/05/call-of-duty-black-ops-6-warzone-ricochet-anti-cheat-season-three-recap).

Will AI finally tip the scales? We don’t know.
But as long as games stay competitive and high-stakes, cheaters will keep trying and anti-cheat systems will have to keep evolving right alongside them.

## Bibliography

1. **David El** [A brief history of game cheating](https://www.cyberark.com/resources/threat-research-blog/a-brief-history-of-game-cheating) _An article that explores various cheating methods in history and talks in more details of DMA cheating_

2. **Samuli Lehtonen** [2020 Comparative Study of Anti-cheat Methods in Video Games](https://helda.helsinki.fi/server/api/core/bitstreams/89d7c14b-58e0-441f-a0de-862254f95551/content).
_A study from 2020 that compares and explains different methods of anti-cheat_

3. **Samuel Tulach** [The Issue of Anti-cheat in Linux](https://tulach.cc/the-issue-of-anti-cheat-on-linux/).
_A really Interesting and technical blog by a security researcher, it actually goes a lot deeper than only the issue of anti-cheat in Linux_


### Various News Article Used

- [‘word.exe’ Cheat Found in Competition](https://www.vice.com/en/article/professional-counter-strike-player-caught-cheating-in-tournament/)
- [Genshin impact anti-cheat ransomware](https://www.trendmicro.com/en_us/research/22/h/ransomware-actor-abuses-genshin-impact-anti-cheat-driver-to-kill-antivirus.html)
- [ESEA Bitcoin Mining](https://www.wired.com/2013/11/e-sports/)
- [AI-based detection example](https://www.callofduty.com/blog/2025/05/call-of-duty-black-ops-6-warzone-ricochet-anti-cheat-season-three-recap)

### Github repositories of “Open Source Anti-cheat”

For those who want to dig a deeper:

- [Kernel Level Anti-cheat by a single person](https://github.com/donnaskiez/ac)
- [More Polished Anti-cheat by 4 contributors](https://github.com/AlSch092/UltimateAntiCheat)

### Pages

- [Home](https://blog.gistre.epita.fr/)
- [Archives](https://blog.gistre.epita.fr/archives/)
- [About](https://blog.gistre.epita.fr/about/)
- [Contact](https://blog.gistre.epita.fr/contact/)
- [Search](https://blog.gistre.epita.fr/search/)
- [RSS](https://blog.gistre.epita.fr/index.xml)

### Links

- [GitLab](https://gitlab.cri.epita.fr/ing/majeures/gistre/blog)

### Tags

[3D printing](https://blog.gistre.epita.fr/tags/3d-printing/)[3DS](https://blog.gistre.epita.fr/tags/3ds/)[aarch64](https://blog.gistre.epita.fr/tags/aarch64/)[actuators](https://blog.gistre.epita.fr/tags/actuators/)[additive-manufacturing](https://blog.gistre.epita.fr/tags/additive-manufacturing/)[anti-cheat](https://blog.gistre.epita.fr/tags/anti-cheat/)[architecture](https://blog.gistre.epita.fr/tags/architecture/)[arduino](https://blog.gistre.epita.fr/tags/arduino/)[ARM](https://blog.gistre.epita.fr/tags/arm/)[article](https://blog.gistre.epita.fr/tags/article/)[asan](https://blog.gistre.epita.fr/tags/asan/)[Atari 2600](https://blog.gistre.epita.fr/tags/atari-2600/)[atmega328p](https://blog.gistre.epita.fr/tags/atmega328p/)[automaton](https://blog.gistre.epita.fr/tags/automaton/)[automobile](https://blog.gistre.epita.fr/tags/automobile/)[autonomous vehicles](https://blog.gistre.epita.fr/tags/autonomous-vehicles/)[avion](https://blog.gistre.epita.fr/tags/avion/)[AVR](https://blog.gistre.epita.fr/tags/avr/)[bindings](https://blog.gistre.epita.fr/tags/bindings/)[binning](https://blog.gistre.epita.fr/tags/binning/)[BIOS](https://blog.gistre.epita.fr/tags/bios/)[BLE](https://blog.gistre.epita.fr/tags/ble/)[bluetooth](https://blog.gistre.epita.fr/tags/bluetooth/)[bluez](https://blog.gistre.epita.fr/tags/bluez/)[boost](https://blog.gistre.epita.fr/tags/boost/)[BOOT](https://blog.gistre.epita.fr/tags/boot/)[bootloader](https://blog.gistre.epita.fr/tags/bootloader/)[buffer overflow](https://blog.gistre.epita.fr/tags/buffer-overflow/)[c](https://blog.gistre.epita.fr/tags/c/)[c++](https://blog.gistre.epita.fr/tags/c++/)[Card Emulation](https://blog.gistre.epita.fr/tags/card-emulation/)[Centipede](https://blog.gistre.epita.fr/tags/centipede/)[chip](https://blog.gistre.epita.fr/tags/chip/)[code pattern](https://blog.gistre.epita.fr/tags/code-pattern/)[collision avoidance algorithms](https://blog.gistre.epita.fr/tags/collision-avoidance-algorithms/)[color](https://blog.gistre.epita.fr/tags/color/)[compiletime](https://blog.gistre.epita.fr/tags/compiletime/)[Compositor](https://blog.gistre.epita.fr/tags/compositor/)[computer architecture](https://blog.gistre.epita.fr/tags/computer-architecture/)[contactless](https://blog.gistre.epita.fr/tags/contactless/)[CoW filesystem](https://blog.gistre.epita.fr/tags/cow-filesystem/)[CPU](https://blog.gistre.epita.fr/tags/cpu/)[CrossOver](https://blog.gistre.epita.fr/tags/crossover/)[cryptanalysis](https://blog.gistre.epita.fr/tags/cryptanalysis/)[cryptography](https://blog.gistre.epita.fr/tags/cryptography/)[cybersecurity](https://blog.gistre.epita.fr/tags/cybersecurity/)[d-bus](https://blog.gistre.epita.fr/tags/d-bus/)[data privacy](https://blog.gistre.epita.fr/tags/data-privacy/)[data storage](https://blog.gistre.epita.fr/tags/data-storage/)[data-transfert](https://blog.gistre.epita.fr/tags/data-transfert/)[Debug](https://blog.gistre.epita.fr/tags/debug/)[desktop](https://blog.gistre.epita.fr/tags/desktop/)[dfa](https://blog.gistre.epita.fr/tags/dfa/)[disk wiping](https://blog.gistre.epita.fr/tags/disk-wiping/)[display](https://blog.gistre.epita.fr/tags/display/)[DIY](https://blog.gistre.epita.fr/tags/diy/)[dna](https://blog.gistre.epita.fr/tags/dna/)[DRAM](https://blog.gistre.epita.fr/tags/dram/)[DRM](https://blog.gistre.epita.fr/tags/drm/)[DWARF](https://blog.gistre.epita.fr/tags/dwarf/)[dynamixel](https://blog.gistre.epita.fr/tags/dynamixel/)[ECU](https://blog.gistre.epita.fr/tags/ecu/)[editors](https://blog.gistre.epita.fr/tags/editors/)[electromechanics](https://blog.gistre.epita.fr/tags/electromechanics/)[electronics](https://blog.gistre.epita.fr/tags/electronics/)[electronique](https://blog.gistre.epita.fr/tags/electronique/)[ELF](https://blog.gistre.epita.fr/tags/elf/)[embedded](https://blog.gistre.epita.fr/tags/embedded/)[Embedded C/C++](https://blog.gistre.epita.fr/tags/embedded-c/c++/)[Emulation](https://blog.gistre.epita.fr/tags/emulation/)[emulator](https://blog.gistre.epita.fr/tags/emulator/)[EMV](https://blog.gistre.epita.fr/tags/emv/)[english](https://blog.gistre.epita.fr/tags/english/)[Entropy](https://blog.gistre.epita.fr/tags/entropy/)[esp32](https://blog.gistre.epita.fr/tags/esp32/)[EXT4](https://blog.gistre.epita.fr/tags/ext4/)[F1](https://blog.gistre.epita.fr/tags/f1/)[FDM](https://blog.gistre.epita.fr/tags/fdm/)[file system](https://blog.gistre.epita.fr/tags/file-system/)[Filesystem](https://blog.gistre.epita.fr/tags/filesystem/)[firewall](https://blog.gistre.epita.fr/tags/firewall/)[Firmware](https://blog.gistre.epita.fr/tags/firmware/)[flipper zero](https://blog.gistre.epita.fr/tags/flipper-zero/)[francais](https://blog.gistre.epita.fr/tags/francais/)[français](https://blog.gistre.epita.fr/tags/fran%C3%A7ais/)[FreeRTOS](https://blog.gistre.epita.fr/tags/freertos/)[french](https://blog.gistre.epita.fr/tags/french/)[FSM](https://blog.gistre.epita.fr/tags/fsm/)[FUSE](https://blog.gistre.epita.fr/tags/fuse/)[Games](https://blog.gistre.epita.fr/tags/games/)[gaming](https://blog.gistre.epita.fr/tags/gaming/)[gazebo](https://blog.gistre.epita.fr/tags/gazebo/)[gcc](https://blog.gistre.epita.fr/tags/gcc/)[GNSS](https://blog.gistre.epita.fr/tags/gnss/)[gpu](https://blog.gistre.epita.fr/tags/gpu/)[Graphics API](https://blog.gistre.epita.fr/tags/graphics-api/)[graphics programming](https://blog.gistre.epita.fr/tags/graphics-programming/)[hal](https://blog.gistre.epita.fr/tags/hal/)[hardware](https://blog.gistre.epita.fr/tags/hardware/)[hare](https://blog.gistre.epita.fr/tags/hare/)[HID](https://blog.gistre.epita.fr/tags/hid/)[holographic displays](https://blog.gistre.epita.fr/tags/holographic-displays/)[HRNG](https://blog.gistre.epita.fr/tags/hrng/)[HUD](https://blog.gistre.epita.fr/tags/hud/)[ia](https://blog.gistre.epita.fr/tags/ia/)[IMUs](https://blog.gistre.epita.fr/tags/imus/)[internet](https://blog.gistre.epita.fr/tags/internet/)[interpreter](https://blog.gistre.epita.fr/tags/interpreter/)[introduction](https://blog.gistre.epita.fr/tags/introduction/)[IoT](https://blog.gistre.epita.fr/tags/iot/)[IPC](https://blog.gistre.epita.fr/tags/ipc/)[JIT](https://blog.gistre.epita.fr/tags/jit/)[Kalman Filters](https://blog.gistre.epita.fr/tags/kalman-filters/)[kernel](https://blog.gistre.epita.fr/tags/kernel/)[Kernel Driver](https://blog.gistre.epita.fr/tags/kernel-driver/)[language](https://blog.gistre.epita.fr/tags/language/)[linux](https://blog.gistre.epita.fr/tags/linux/)[linux security](https://blog.gistre.epita.fr/tags/linux-security/)[llvm](https://blog.gistre.epita.fr/tags/llvm/)[Localization](https://blog.gistre.epita.fr/tags/localization/)[logging](https://blog.gistre.epita.fr/tags/logging/)[logic gate](https://blog.gistre.epita.fr/tags/logic-gate/)[Low-energy](https://blog.gistre.epita.fr/tags/low-energy/)[lsp](https://blog.gistre.epita.fr/tags/lsp/)[macOS](https://blog.gistre.epita.fr/tags/macos/)[malloc](https://blog.gistre.epita.fr/tags/malloc/)[manufacturing](https://blog.gistre.epita.fr/tags/manufacturing/)[mcu](https://blog.gistre.epita.fr/tags/mcu/)[memory](https://blog.gistre.epita.fr/tags/memory/)[MEMS](https://blog.gistre.epita.fr/tags/mems/)[metaprogramming](https://blog.gistre.epita.fr/tags/metaprogramming/)[microarchitecture](https://blog.gistre.epita.fr/tags/microarchitecture/)[microcontroller](https://blog.gistre.epita.fr/tags/microcontroller/)[Minecraft](https://blog.gistre.epita.fr/tags/minecraft/)[miniaturisation](https://blog.gistre.epita.fr/tags/miniaturisation/)[moteur](https://blog.gistre.epita.fr/tags/moteur/)[motors](https://blog.gistre.epita.fr/tags/motors/)[msvc](https://blog.gistre.epita.fr/tags/msvc/)[musk](https://blog.gistre.epita.fr/tags/musk/)[mémoire](https://blog.gistre.epita.fr/tags/m%C3%A9moire/)[native execution](https://blog.gistre.epita.fr/tags/native-execution/)[networking](https://blog.gistre.epita.fr/tags/networking/)[news](https://blog.gistre.epita.fr/tags/news/)[nfa](https://blog.gistre.epita.fr/tags/nfa/)[NFC](https://blog.gistre.epita.fr/tags/nfc/)[Nintendo](https://blog.gistre.epita.fr/tags/nintendo/)[Nintendo DS](https://blog.gistre.epita.fr/tags/nintendo-ds/)[Nix](https://blog.gistre.epita.fr/tags/nix/)[non-planar 3D slicing](https://blog.gistre.epita.fr/tags/non-planar-3d-slicing/)[nuclear](https://blog.gistre.epita.fr/tags/nuclear/)[nucleo](https://blog.gistre.epita.fr/tags/nucleo/)[observability](https://blog.gistre.epita.fr/tags/observability/)[OpenStLinux](https://blog.gistre.epita.fr/tags/openstlinux/)[Operating System](https://blog.gistre.epita.fr/tags/operating-system/)[optimisation](https://blog.gistre.epita.fr/tags/optimisation/)[optimization](https://blog.gistre.epita.fr/tags/optimization/)[OS](https://blog.gistre.epita.fr/tags/os/)[oscillateur](https://blog.gistre.epita.fr/tags/oscillateur/)[packet](https://blog.gistre.epita.fr/tags/packet/)[parser-exploits](https://blog.gistre.epita.fr/tags/parser-exploits/)[Payment](https://blog.gistre.epita.fr/tags/payment/)[PCB](https://blog.gistre.epita.fr/tags/pcb/)[perf](https://blog.gistre.epita.fr/tags/perf/)[Physics Simulation](https://blog.gistre.epita.fr/tags/physics-simulation/)[PIC32](https://blog.gistre.epita.fr/tags/pic32/)[polyglot](https://blog.gistre.epita.fr/tags/polyglot/)[PRNG](https://blog.gistre.epita.fr/tags/prng/)[processor](https://blog.gistre.epita.fr/tags/processor/)[profiling](https://blog.gistre.epita.fr/tags/profiling/)[Project Management](https://blog.gistre.epita.fr/tags/project-management/)[protocol](https://blog.gistre.epita.fr/tags/protocol/)[protocols](https://blog.gistre.epita.fr/tags/protocols/)[PyO3](https://blog.gistre.epita.fr/tags/pyo3/)[pytest](https://blog.gistre.epita.fr/tags/pytest/)[Python](https://blog.gistre.epita.fr/tags/python/)[QEmu](https://blog.gistre.epita.fr/tags/qemu/)[Random generation](https://blog.gistre.epita.fr/tags/random-generation/)[raspberry pi](https://blog.gistre.epita.fr/tags/raspberry-pi/)[Real-time](https://blog.gistre.epita.fr/tags/real-time/)[real-time systems](https://blog.gistre.epita.fr/tags/real-time-systems/)[recovery mode](https://blog.gistre.epita.fr/tags/recovery-mode/)[regex](https://blog.gistre.epita.fr/tags/regex/)[regular expressions](https://blog.gistre.epita.fr/tags/regular-expressions/)[ReRAM](https://blog.gistre.epita.fr/tags/reram/)[reverse engineering](https://blog.gistre.epita.fr/tags/reverse-engineering/)[RISC](https://blog.gistre.epita.fr/tags/risc/)[RISC-V](https://blog.gistre.epita.fr/tags/risc-v/)[rm](https://blog.gistre.epita.fr/tags/rm/)[robot](https://blog.gistre.epita.fr/tags/robot/)[ros](https://blog.gistre.epita.fr/tags/ros/)[Rosetta](https://blog.gistre.epita.fr/tags/rosetta/)[RRAM](https://blog.gistre.epita.fr/tags/rram/)[RTK](https://blog.gistre.epita.fr/tags/rtk/)[RTKLib](https://blog.gistre.epita.fr/tags/rtklib/)[runtime](https://blog.gistre.epita.fr/tags/runtime/)[rust](https://blog.gistre.epita.fr/tags/rust/)[sanitizer](https://blog.gistre.epita.fr/tags/sanitizer/)[satellite](https://blog.gistre.epita.fr/tags/satellite/)[Screenshare](https://blog.gistre.epita.fr/tags/screenshare/)[Screenshot](https://blog.gistre.epita.fr/tags/screenshot/)[Secure Element](https://blog.gistre.epita.fr/tags/secure-element/)[security](https://blog.gistre.epita.fr/tags/security/)[semiconductor](https://blog.gistre.epita.fr/tags/semiconductor/)[sensor](https://blog.gistre.epita.fr/tags/sensor/)[sensors](https://blog.gistre.epita.fr/tags/sensors/)[shred](https://blog.gistre.epita.fr/tags/shred/)[side-channel](https://blog.gistre.epita.fr/tags/side-channel/)[simulation](https://blog.gistre.epita.fr/tags/simulation/)[slicer](https://blog.gistre.epita.fr/tags/slicer/)[SSHFS](https://blog.gistre.epita.fr/tags/sshfs/)[starlink](https://blog.gistre.epita.fr/tags/starlink/)[stm32](https://blog.gistre.epita.fr/tags/stm32/)[storage](https://blog.gistre.epita.fr/tags/storage/)[suckless](https://blog.gistre.epita.fr/tags/suckless/)[switch](https://blog.gistre.epita.fr/tags/switch/)[system](https://blog.gistre.epita.fr/tags/system/)[tegra](https://blog.gistre.epita.fr/tags/tegra/)[template](https://blog.gistre.epita.fr/tags/template/)[test](https://blog.gistre.epita.fr/tags/test/)[thread](https://blog.gistre.epita.fr/tags/thread/)[tuto](https://blog.gistre.epita.fr/tags/tuto/)[USB](https://blog.gistre.epita.fr/tags/usb/)[Verlet integration](https://blog.gistre.epita.fr/tags/verlet-integration/)[VHDL](https://blog.gistre.epita.fr/tags/vhdl/)[Video](https://blog.gistre.epita.fr/tags/video/)[video games](https://blog.gistre.epita.fr/tags/video-games/)[virus](https://blog.gistre.epita.fr/tags/virus/)[vulnerability](https://blog.gistre.epita.fr/tags/vulnerability/)[wayland](https://blog.gistre.epita.fr/tags/wayland/)[window](https://blog.gistre.epita.fr/tags/window/)[windows](https://blog.gistre.epita.fr/tags/windows/)[WINE](https://blog.gistre.epita.fr/tags/wine/)[x](https://blog.gistre.epita.fr/tags/x/)[x11](https://blog.gistre.epita.fr/tags/x11/)[x86](https://blog.gistre.epita.fr/tags/x86/)[xorg](https://blog.gistre.epita.fr/tags/xorg/)

### Pages

- [Home](https://blog.gistre.epita.fr/)
- [Archives](https://blog.gistre.epita.fr/archives/)
- [About](https://blog.gistre.epita.fr/about/)
- [Contact](https://blog.gistre.epita.fr/contact/)
- [Search](https://blog.gistre.epita.fr/search/)
- [RSS](https://blog.gistre.epita.fr/index.xml)

### Links

- [GitLab](https://gitlab.cri.epita.fr/ing/majeures/gistre/blog)

### Tags

[3D printing](https://blog.gistre.epita.fr/tags/3d-printing/)[3DS](https://blog.gistre.epita.fr/tags/3ds/)[aarch64](https://blog.gistre.epita.fr/tags/aarch64/)[actuators](https://blog.gistre.epita.fr/tags/actuators/)[additive-manufacturing](https://blog.gistre.epita.fr/tags/additive-manufacturing/)[anti-cheat](https://blog.gistre.epita.fr/tags/anti-cheat/)[architecture](https://blog.gistre.epita.fr/tags/architecture/)[arduino](https://blog.gistre.epita.fr/tags/arduino/)[ARM](https://blog.gistre.epita.fr/tags/arm/)[article](https://blog.gistre.epita.fr/tags/article/)[asan](https://blog.gistre.epita.fr/tags/asan/)[Atari 2600](https://blog.gistre.epita.fr/tags/atari-2600/)[atmega328p](https://blog.gistre.epita.fr/tags/atmega328p/)[automaton](https://blog.gistre.epita.fr/tags/automaton/)[automobile](https://blog.gistre.epita.fr/tags/automobile/)[autonomous vehicles](https://blog.gistre.epita.fr/tags/autonomous-vehicles/)[avion](https://blog.gistre.epita.fr/tags/avion/)[AVR](https://blog.gistre.epita.fr/tags/avr/)[bindings](https://blog.gistre.epita.fr/tags/bindings/)[binning](https://blog.gistre.epita.fr/tags/binning/)[BIOS](https://blog.gistre.epita.fr/tags/bios/)[BLE](https://blog.gistre.epita.fr/tags/ble/)[bluetooth](https://blog.gistre.epita.fr/tags/bluetooth/)[bluez](https://blog.gistre.epita.fr/tags/bluez/)[boost](https://blog.gistre.epita.fr/tags/boost/)[BOOT](https://blog.gistre.epita.fr/tags/boot/)[bootloader](https://blog.gistre.epita.fr/tags/bootloader/)[buffer overflow](https://blog.gistre.epita.fr/tags/buffer-overflow/)[c](https://blog.gistre.epita.fr/tags/c/)[c++](https://blog.gistre.epita.fr/tags/c++/)[Card Emulation](https://blog.gistre.epita.fr/tags/card-emulation/)[Centipede](https://blog.gistre.epita.fr/tags/centipede/)[chip](https://blog.gistre.epita.fr/tags/chip/)[code pattern](https://blog.gistre.epita.fr/tags/code-pattern/)[collision avoidance algorithms](https://blog.gistre.epita.fr/tags/collision-avoidance-algorithms/)[color](https://blog.gistre.epita.fr/tags/color/)[compiletime](https://blog.gistre.epita.fr/tags/compiletime/)[Compositor](https://blog.gistre.epita.fr/tags/compositor/)[computer architecture](https://blog.gistre.epita.fr/tags/computer-architecture/)[contactless](https://blog.gistre.epita.fr/tags/contactless/)[CoW filesystem](https://blog.gistre.epita.fr/tags/cow-filesystem/)[CPU](https://blog.gistre.epita.fr/tags/cpu/)[CrossOver](https://blog.gistre.epita.fr/tags/crossover/)[cryptanalysis](https://blog.gistre.epita.fr/tags/cryptanalysis/)[cryptography](https://blog.gistre.epita.fr/tags/cryptography/)[cybersecurity](https://blog.gistre.epita.fr/tags/cybersecurity/)[d-bus](https://blog.gistre.epita.fr/tags/d-bus/)[data privacy](https://blog.gistre.epita.fr/tags/data-privacy/)[data storage](https://blog.gistre.epita.fr/tags/data-storage/)[data-transfert](https://blog.gistre.epita.fr/tags/data-transfert/)[Debug](https://blog.gistre.epita.fr/tags/debug/)[desktop](https://blog.gistre.epita.fr/tags/desktop/)[dfa](https://blog.gistre.epita.fr/tags/dfa/)[disk wiping](https://blog.gistre.epita.fr/tags/disk-wiping/)[display](https://blog.gistre.epita.fr/tags/display/)[DIY](https://blog.gistre.epita.fr/tags/diy/)[dna](https://blog.gistre.epita.fr/tags/dna/)[DRAM](https://blog.gistre.epita.fr/tags/dram/)[DRM](https://blog.gistre.epita.fr/tags/drm/)[DWARF](https://blog.gistre.epita.fr/tags/dwarf/)[dynamixel](https://blog.gistre.epita.fr/tags/dynamixel/)[ECU](https://blog.gistre.epita.fr/tags/ecu/)[editors](https://blog.gistre.epita.fr/tags/editors/)[electromechanics](https://blog.gistre.epita.fr/tags/electromechanics/)[electronics](https://blog.gistre.epita.fr/tags/electronics/)[electronique](https://blog.gistre.epita.fr/tags/electronique/)[ELF](https://blog.gistre.epita.fr/tags/elf/)[embedded](https://blog.gistre.epita.fr/tags/embedded/)[Embedded C/C++](https://blog.gistre.epita.fr/tags/embedded-c/c++/)[Emulation](https://blog.gistre.epita.fr/tags/emulation/)[emulator](https://blog.gistre.epita.fr/tags/emulator/)[EMV](https://blog.gistre.epita.fr/tags/emv/)[english](https://blog.gistre.epita.fr/tags/english/)[Entropy](https://blog.gistre.epita.fr/tags/entropy/)[esp32](https://blog.gistre.epita.fr/tags/esp32/)[EXT4](https://blog.gistre.epita.fr/tags/ext4/)[F1](https://blog.gistre.epita.fr/tags/f1/)[FDM](https://blog.gistre.epita.fr/tags/fdm/)[file system](https://blog.gistre.epita.fr/tags/file-system/)[Filesystem](https://blog.gistre.epita.fr/tags/filesystem/)[firewall](https://blog.gistre.epita.fr/tags/firewall/)[Firmware](https://blog.gistre.epita.fr/tags/firmware/)[flipper zero](https://blog.gistre.epita.fr/tags/flipper-zero/)[francais](https://blog.gistre.epita.fr/tags/francais/)[français](https://blog.gistre.epita.fr/tags/fran%C3%A7ais/)[FreeRTOS](https://blog.gistre.epita.fr/tags/freertos/)[french](https://blog.gistre.epita.fr/tags/french/)[FSM](https://blog.gistre.epita.fr/tags/fsm/)[FUSE](https://blog.gistre.epita.fr/tags/fuse/)[Games](https://blog.gistre.epita.fr/tags/games/)[gaming](https://blog.gistre.epita.fr/tags/gaming/)[gazebo](https://blog.gistre.epita.fr/tags/gazebo/)[gcc](https://blog.gistre.epita.fr/tags/gcc/)[GNSS](https://blog.gistre.epita.fr/tags/gnss/)[gpu](https://blog.gistre.epita.fr/tags/gpu/)[Graphics API](https://blog.gistre.epita.fr/tags/graphics-api/)[graphics programming](https://blog.gistre.epita.fr/tags/graphics-programming/)[hal](https://blog.gistre.epita.fr/tags/hal/)[hardware](https://blog.gistre.epita.fr/tags/hardware/)[hare](https://blog.gistre.epita.fr/tags/hare/)[HID](https://blog.gistre.epita.fr/tags/hid/)[holographic displays](https://blog.gistre.epita.fr/tags/holographic-displays/)[HRNG](https://blog.gistre.epita.fr/tags/hrng/)[HUD](https://blog.gistre.epita.fr/tags/hud/)[ia](https://blog.gistre.epita.fr/tags/ia/)[IMUs](https://blog.gistre.epita.fr/tags/imus/)[internet](https://blog.gistre.epita.fr/tags/internet/)[interpreter](https://blog.gistre.epita.fr/tags/interpreter/)[introduction](https://blog.gistre.epita.fr/tags/introduction/)[IoT](https://blog.gistre.epita.fr/tags/iot/)[IPC](https://blog.gistre.epita.fr/tags/ipc/)[JIT](https://blog.gistre.epita.fr/tags/jit/)[Kalman Filters](https://blog.gistre.epita.fr/tags/kalman-filters/)[kernel](https://blog.gistre.epita.fr/tags/kernel/)[Kernel Driver](https://blog.gistre.epita.fr/tags/kernel-driver/)[language](https://blog.gistre.epita.fr/tags/language/)[linux](https://blog.gistre.epita.fr/tags/linux/)[linux security](https://blog.gistre.epita.fr/tags/linux-security/)[llvm](https://blog.gistre.epita.fr/tags/llvm/)[Localization](https://blog.gistre.epita.fr/tags/localization/)[logging](https://blog.gistre.epita.fr/tags/logging/)[logic gate](https://blog.gistre.epita.fr/tags/logic-gate/)[Low-energy](https://blog.gistre.epita.fr/tags/low-energy/)[lsp](https://blog.gistre.epita.fr/tags/lsp/)[macOS](https://blog.gistre.epita.fr/tags/macos/)[malloc](https://blog.gistre.epita.fr/tags/malloc/)[manufacturing](https://blog.gistre.epita.fr/tags/manufacturing/)[mcu](https://blog.gistre.epita.fr/tags/mcu/)[memory](https://blog.gistre.epita.fr/tags/memory/)[MEMS](https://blog.gistre.epita.fr/tags/mems/)[metaprogramming](https://blog.gistre.epita.fr/tags/metaprogramming/)[microarchitecture](https://blog.gistre.epita.fr/tags/microarchitecture/)[microcontroller](https://blog.gistre.epita.fr/tags/microcontroller/)[Minecraft](https://blog.gistre.epita.fr/tags/minecraft/)[miniaturisation](https://blog.gistre.epita.fr/tags/miniaturisation/)[moteur](https://blog.gistre.epita.fr/tags/moteur/)[motors](https://blog.gistre.epita.fr/tags/motors/)[msvc](https://blog.gistre.epita.fr/tags/msvc/)[musk](https://blog.gistre.epita.fr/tags/musk/)[mémoire](https://blog.gistre.epita.fr/tags/m%C3%A9moire/)[native execution](https://blog.gistre.epita.fr/tags/native-execution/)[networking](https://blog.gistre.epita.fr/tags/networking/)[news](https://blog.gistre.epita.fr/tags/news/)[nfa](https://blog.gistre.epita.fr/tags/nfa/)[NFC](https://blog.gistre.epita.fr/tags/nfc/)[Nintendo](https://blog.gistre.epita.fr/tags/nintendo/)[Nintendo DS](https://blog.gistre.epita.fr/tags/nintendo-ds/)[Nix](https://blog.gistre.epita.fr/tags/nix/)[non-planar 3D slicing](https://blog.gistre.epita.fr/tags/non-planar-3d-slicing/)[nuclear](https://blog.gistre.epita.fr/tags/nuclear/)[nucleo](https://blog.gistre.epita.fr/tags/nucleo/)[observability](https://blog.gistre.epita.fr/tags/observability/)[OpenStLinux](https://blog.gistre.epita.fr/tags/openstlinux/)[Operating System](https://blog.gistre.epita.fr/tags/operating-system/)[optimisation](https://blog.gistre.epita.fr/tags/optimisation/)[optimization](https://blog.gistre.epita.fr/tags/optimization/)[OS](https://blog.gistre.epita.fr/tags/os/)[oscillateur](https://blog.gistre.epita.fr/tags/oscillateur/)[packet](https://blog.gistre.epita.fr/tags/packet/)[parser-exploits](https://blog.gistre.epita.fr/tags/parser-exploits/)[Payment](https://blog.gistre.epita.fr/tags/payment/)[PCB](https://blog.gistre.epita.fr/tags/pcb/)[perf](https://blog.gistre.epita.fr/tags/perf/)[Physics Simulation](https://blog.gistre.epita.fr/tags/physics-simulation/)[PIC32](https://blog.gistre.epita.fr/tags/pic32/)[polyglot](https://blog.gistre.epita.fr/tags/polyglot/)[PRNG](https://blog.gistre.epita.fr/tags/prng/)[processor](https://blog.gistre.epita.fr/tags/processor/)[profiling](https://blog.gistre.epita.fr/tags/profiling/)[Project Management](https://blog.gistre.epita.fr/tags/project-management/)[protocol](https://blog.gistre.epita.fr/tags/protocol/)[protocols](https://blog.gistre.epita.fr/tags/protocols/)[PyO3](https://blog.gistre.epita.fr/tags/pyo3/)[pytest](https://blog.gistre.epita.fr/tags/pytest/)[Python](https://blog.gistre.epita.fr/tags/python/)[QEmu](https://blog.gistre.epita.fr/tags/qemu/)[Random generation](https://blog.gistre.epita.fr/tags/random-generation/)[raspberry pi](https://blog.gistre.epita.fr/tags/raspberry-pi/)[Real-time](https://blog.gistre.epita.fr/tags/real-time/)[real-time systems](https://blog.gistre.epita.fr/tags/real-time-systems/)[recovery mode](https://blog.gistre.epita.fr/tags/recovery-mode/)[regex](https://blog.gistre.epita.fr/tags/regex/)[regular expressions](https://blog.gistre.epita.fr/tags/regular-expressions/)[ReRAM](https://blog.gistre.epita.fr/tags/reram/)[reverse engineering](https://blog.gistre.epita.fr/tags/reverse-engineering/)[RISC](https://blog.gistre.epita.fr/tags/risc/)[RISC-V](https://blog.gistre.epita.fr/tags/risc-v/)[rm](https://blog.gistre.epita.fr/tags/rm/)[robot](https://blog.gistre.epita.fr/tags/robot/)[ros](https://blog.gistre.epita.fr/tags/ros/)[Rosetta](https://blog.gistre.epita.fr/tags/rosetta/)[RRAM](https://blog.gistre.epita.fr/tags/rram/)[RTK](https://blog.gistre.epita.fr/tags/rtk/)[RTKLib](https://blog.gistre.epita.fr/tags/rtklib/)[runtime](https://blog.gistre.epita.fr/tags/runtime/)[rust](https://blog.gistre.epita.fr/tags/rust/)[sanitizer](https://blog.gistre.epita.fr/tags/sanitizer/)[satellite](https://blog.gistre.epita.fr/tags/satellite/)[Screenshare](https://blog.gistre.epita.fr/tags/screenshare/)[Screenshot](https://blog.gistre.epita.fr/tags/screenshot/)[Secure Element](https://blog.gistre.epita.fr/tags/secure-element/)[security](https://blog.gistre.epita.fr/tags/security/)[semiconductor](https://blog.gistre.epita.fr/tags/semiconductor/)[sensor](https://blog.gistre.epita.fr/tags/sensor/)[sensors](https://blog.gistre.epita.fr/tags/sensors/)[shred](https://blog.gistre.epita.fr/tags/shred/)[side-channel](https://blog.gistre.epita.fr/tags/side-channel/)[simulation](https://blog.gistre.epita.fr/tags/simulation/)[slicer](https://blog.gistre.epita.fr/tags/slicer/)[SSHFS](https://blog.gistre.epita.fr/tags/sshfs/)[starlink](https://blog.gistre.epita.fr/tags/starlink/)[stm32](https://blog.gistre.epita.fr/tags/stm32/)[storage](https://blog.gistre.epita.fr/tags/storage/)[suckless](https://blog.gistre.epita.fr/tags/suckless/)[switch](https://blog.gistre.epita.fr/tags/switch/)[system](https://blog.gistre.epita.fr/tags/system/)[tegra](https://blog.gistre.epita.fr/tags/tegra/)[template](https://blog.gistre.epita.fr/tags/template/)[test](https://blog.gistre.epita.fr/tags/test/)[thread](https://blog.gistre.epita.fr/tags/thread/)[tuto](https://blog.gistre.epita.fr/tags/tuto/)[USB](https://blog.gistre.epita.fr/tags/usb/)[Verlet integration](https://blog.gistre.epita.fr/tags/verlet-integration/)[VHDL](https://blog.gistre.epita.fr/tags/vhdl/)[Video](https://blog.gistre.epita.fr/tags/video/)[video games](https://blog.gistre.epita.fr/tags/video-games/)[virus](https://blog.gistre.epita.fr/tags/virus/)[vulnerability](https://blog.gistre.epita.fr/tags/vulnerability/)[wayland](https://blog.gistre.epita.fr/tags/wayland/)[window](https://blog.gistre.epita.fr/tags/window/)[windows](https://blog.gistre.epita.fr/tags/windows/)[WINE](https://blog.gistre.epita.fr/tags/wine/)[x](https://blog.gistre.epita.fr/tags/x/)[x11](https://blog.gistre.epita.fr/tags/x11/)[x86](https://blog.gistre.epita.fr/tags/x86/)[xorg](https://blog.gistre.epita.fr/tags/xorg/)

Unless otherwise noted, the content of this site is licensed under [Creative Commons BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).


© 2022-2026

\| [Source code](https://gitlab.cri.epita.fr/ing/majeures/gistre/blog)
\| Powered by [Fuji-v2](https://github.com/dsrkafuu/hugo-theme-fuji/) (modified) & [Hugo](https://gohugo.io/)