# BattlEye

BattlEye

Anti-cheat software

| BattlEye |
| --- |
| [![](https://upload.wikimedia.org/wikipedia/en/5/58/BattlEye_Logo.png)](https://en.wikipedia.org/wiki/File:BattlEye_Logo.png) |
| [Original author](https://en.wikipedia.org/wiki/Programmer "Programmer") | Bastian Suter |
| [Developer](https://en.wikipedia.org/wiki/Programmer "Programmer") | BattlEye Innovations e.K. |
| Initial release | 2004 |
| [Operating system](https://en.wikipedia.org/wiki/Operating_system "Operating system") | [Windows](https://en.wikipedia.org/wiki/Microsoft_Windows "Microsoft Windows"), [macOS](https://en.wikipedia.org/wiki/MacOS "MacOS"), [Linux](https://en.wikipedia.org/wiki/Linux "Linux")[\[1\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-1)[\[2\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-2) |
| [Type](https://en.wikipedia.org/wiki/Software_categories#Categorization_approaches "Software categories") | [Anti-cheat software](https://en.wikipedia.org/wiki/Anti-cheat_software "Anti-cheat software") |
| [License](https://en.wikipedia.org/wiki/Software_license "Software license") | [Proprietary](https://en.wikipedia.org/wiki/Proprietary_software "Proprietary software") |
| Website | [Official website](https://www.battleye.com/) |

**BattlEye** is [proprietary](https://en.wikipedia.org/wiki/Proprietary_software "Proprietary software") [kernel–level](https://en.wikipedia.org/wiki/Kernel_(operating_system) "Kernel (operating system)") [anti-cheat software](https://en.wikipedia.org/wiki/Anti-cheat_software "Anti-cheat software") designed to detect players that hack or abusively use [exploits](https://en.wikipedia.org/wiki/Video_game_exploits "Video game exploits") in an [online game](https://en.wikipedia.org/wiki/Online_game "Online game"). It was initially released as a third-party anti-cheat for _[Battlefield Vietnam](https://en.wikipedia.org/wiki/Battlefield_Vietnam "Battlefield Vietnam")_ in 2004 and has since been officially implemented in numerous video games, primarily [shooter games](https://en.wikipedia.org/wiki/Shooter_game "Shooter game") such as _[PUBG: Battlegrounds](https://en.wikipedia.org/wiki/PUBG:_Battlegrounds "PUBG: Battlegrounds")_, _[Arma 3](https://en.wikipedia.org/wiki/Arma_3 "Arma 3")_, _[Destiny 2](https://en.wikipedia.org/wiki/Destiny_2 "Destiny 2")_, [War Thunder](https://en.wikipedia.org/wiki/War_Thunder "War Thunder"), and _[DayZ](https://en.wikipedia.org/wiki/DayZ_(video_game) "DayZ (video game)")_.[\[3\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-3)[\[4\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto1-4)

BattlEye is developed by German company BattlEye Innovations e. K., headquartered in [Reutlingen](https://en.wikipedia.org/wiki/Reutlingen "Reutlingen").

BattlEye supports [Valve Corporation](https://en.wikipedia.org/wiki/Valve_Corporation "Valve Corporation")'s [Proton](https://en.wikipedia.org/wiki/Proton_(software) "Proton (software)") compatibility layer and is usable on the [Steam Deck](https://en.wikipedia.org/wiki/Steam_Deck "Steam Deck") if the game developer opts in.[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)[\[6\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-6)

## History

BattlEye was created in 2004 by German developer Bastian Suter as an external third-party anti-cheat tool for _Battlefield Vietnam_. According to the official "About" page, support was quickly expanded to _Battlefield 1942_ and _Battlefield 2_, where the software was adopted by a number of online leagues and server communities.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)

In 2006 BattlEye was integrated directly into the freeware first-person shooter _Warsow_ and the indie multiplayer game _Soldat_, which were the first titles to ship with internal support rather than using BattlEye as a separate tool.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7) In the following years it was added to several commercial games, including _ArmA: Armed Assault_ / _Combat Operations_ and _S.T.A.L.K.E.R.: Shadow of Chernobyl_, and later to _Arma 2_, _Arma 2: Operation Arrowhead_, _Iron Front: Liberation 1944_, _Arma 3_ and the standalone version of _DayZ_.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)

Around 2014 BattlEye began moving away from a traditional detection-only model. The company introduced a more proactive system that focused on blocking cheats before they could affect other players. This update, described in the "A New Dawn" announcement, introduced a kernel-mode driver and a new launcher for games such as _Arma 2: Operation Arrowhead_ and _DayZ_.[\[8\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-benewdawn-8) The developer later stated that, after the proactive system went fully live in February 2015, cheating in _DayZ_ had been reduced to a very low level.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)

From the mid-2010s BattlEye expanded to more online games. Official news posts and the home page list support for titles including _H1Z1_, _ARK: Survival Evolved_, _PlanetSide 2_, _Unturned_, _Insurgency_, _Day of Infamy_, _Conan Exiles_ and several others.[\[9\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-bemoregames-9)[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10) BattlEye is also part of the standard dedicated server setup for [Bohemia Interactive](https://en.wikipedia.org/wiki/Bohemia_Interactive "Bohemia Interactive") titles such as the _Arma_ series and _DayZ_.[\[11\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-bicommunity-11)

In 2025 [Rockstar Games](https://en.wikipedia.org/wiki/Rockstar_Games "Rockstar Games") added BattlEye to the PC version of _Grand Theft Auto Online_ as part of a wider anti-cheat update for the game.[\[12\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-gtabe-12)

## Technology

BattlEye continuously updates in [background processes](https://en.wikipedia.org/wiki/Background_processes "Background processes") and has its own infrastructure which is connected to the game servers. It interacts with the game at the kernel level. BattlEye is said to support a "global" ban system for cheaters using unique fingerprints that stop players switching accounts to defeat bans.[\[13\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-13)

The developer describes BattlEye as a "fully proactive kernel-based protection system" that performs fast, dynamic and permanent scanning of the player's system using both specific and generic detection routines.[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10) The proactive update introduced in 2014–2015 added a kernel-mode driver to strengthen protection, but BattlEye has stated that the driver is not intended to behave as a rootkit on the user's computer.[\[8\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-benewdawn-8)

BattlEye can scan in both user mode and kernel mode, giving it access to low-level system activity that can be used to detect cheats that try to hide their presence or modify data at that level.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)[\[8\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-benewdawn-8) In addition to static checks on files, the system uses heuristic and behaviour-based methods to monitor how other programs interact with the game process and its memory.[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10)

BattlEye includes a global ban infrastructure. According to the company, cheaters can be banned based on account identifiers, and these bans can be enforced across all BattlEye-protected servers for a game.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10) Communication between the client and the backend uses encrypted packets sent over the game's normal network connection, and the software can update itself automatically from remote servers when new detection rules or features are deployed.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10)

## Design

The overall design of BattlEye focuses on both preventing cheats from running and detecting them once they are active. The system runs on both the game client and the [game server](https://en.wikipedia.org/wiki/Game_server "Game server") and communicates using the game's usual network traffic.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7) It is distributed together with the game files and is described as having no special hardware requirements, with the goal of using only a small amount of CPU, RAM, and bandwidth so that it remains unobtrusive during normal play.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)

On the client side, BattlEye adds a protective layer around the game process. This layer is intended to block common attack methods such as direct memory editing, DLL injection and attaching external tools as early as possible in the startup phase.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10) While the game is running, BattlEye performs dynamic checks that look for unusual behaviour, known cheat patterns and tools that try to hide themselves. Confirmed cheaters can be kicked from the current session or globally banned depending on the game developer's policy.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)[\[14\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-besupport-14)

On the server side, BattlEye provides an administration component and remote console (RCon) tools that allow server owners to monitor players, list connections, kick or ban accounts and manage ban lists.[\[15\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-bedocs-15)[\[11\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-bicommunity-11) Server documentation notes that the BattlEye server process keeps tight control over how quickly clients must respond and that non-responsive clients can be removed to prevent certain types of manipulation.[\[15\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-bedocs-15)

The company emphasises that BattlEye performs its detection work independently, without requiring constant manual input from game studios. Developers mainly integrate the interface, after which BattlEye's backend manages detection rules and global ban decisions.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)

## Additional restrictions and system behaviour

Because BattlEye operates at a low level in the operating system, it can conflict with some tools and security features. The official FAQ notes that Windows security options such as hardware-enforced stack protection can prevent the BattlEye driver from loading correctly and that systems running in kernel debugging or test-signing modes are not supported, as those modes make it easier for cheats to bypass protection.[\[16\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-befaq-16)

BattlEye can block or terminate applications that use vulnerable kernel drivers or that try to inject into the game process. This may affect some hardware-monitoring tools or graphics injectors, which can stop working while a BattlEye-protected game is running.[\[16\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-befaq-16) The FAQ explains that non-cheat overlays and visual enhancement tools are generally allowed unless a specific game developer chooses to block them, and that in some cases such programs may be kicked but not banned if they cause technical problems.[\[16\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-befaq-16)

On Windows, BattlEye installs itself as a system service. According to the FAQ, the service is only active while a BattlEye-protected game is running with anti-cheat enabled and can be completely removed using an uninstall script or by manually deleting the service entry.[\[16\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-befaq-16) Rockstar Games similarly notes that in _Grand Theft Auto Online_ the BattlEye component only needs to run during online sessions and can be disabled when the player uses Story Mode or certain community servers.[\[12\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-gtabe-12)

BattlEye's support page describes global bans as permanent and non-negotiable. Players are not banned for using non-hack programs alone, being on a server with a cheater or picking up hacked items, but cheating is forbidden on all BattlEye-enabled servers, including private ones, and ban appeals require precise ban IDs or account identifiers.[\[14\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-besupport-14)

## Social impact

BattlEye's stated goal is to protect the online experience of legitimate players by keeping cheaters out of multiplayer games. The official site describes cheating as a "massive issue" in modern online titles and presents BattlEye as a long-term service that aims to keep communities playable by actively hunting down hacks and banning cheaters.[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10) On its about page the company claims that the proactive system reduced cheating in _DayZ_ to a minimum after it was fully rolled out in early 2015.[\[7\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beabout-7)

Large publishers treat BattlEye as one part of a wider fair-play strategy. Ubisoft's anti-cheat developer blog for _Tom Clancy's Rainbow Six Siege_ credits improved detection, detailed reporting and data-sharing with BattlEye as important factors in a substantial increase in yearly ban numbers, and notes that BattlEye bans are linked to other platform bans to make it harder for cheaters to return on new accounts.[\[17\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-ubir6-17) Rockstar Games uses BattlEye in _Grand Theft Auto Online_ to monitor suspicious activity on PC and states that information collected by BattlEye can lead to suspensions or bans when cheat software is detected on an account.[\[12\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-gtabe-12)

Because BattlEye operates at kernel level, it has access to technical data from the player's system while a protected game is running. Rockstar's FAQ lists items such as IP address, game identifiers, hardware device information and details about the operating system and running processes, and states that BattlEye follows a data-minimisation policy where data is only stored when necessary, for example when potential cheat activity is detected.[\[12\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-gtabe-12) BattlEye's own privacy policy says that the company does not look at or transmit personal documents, passwords or payment details and that it normally only stores data when a detection is involved, such as IP addresses, account names and hardware serial information, which are kept on secure servers.[\[18\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-beprivacy-18)

## Games using BattlEye

- _[Battlefield Vietnam](https://en.wikipedia.org/wiki/Battlefield_Vietnam "Battlefield Vietnam")_ (2004)[\[19\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-19)
- _[Battlefield 1942](https://en.wikipedia.org/wiki/Battlefield_1942 "Battlefield 1942")_ (2005)
- _[Battlefield 2](https://en.wikipedia.org/wiki/Battlefield_2 "Battlefield 2")_ (2005)
- _[Warsow](https://en.wikipedia.org/wiki/War%C2%A7ow "War§ow")_ (2006)
- _[ArmA: Armed Assault](https://en.wikipedia.org/wiki/ArmA:_Armed_Assault "ArmA: Armed Assault")_ (2007)
- _[S.T.A.L.K.E.R.: Shadow of Chernobyl](https://en.wikipedia.org/wiki/S.T.A.L.K.E.R.:_Shadow_of_Chernobyl "S.T.A.L.K.E.R.: Shadow of Chernobyl")_ (2008)
- _[Arma 2](https://en.wikipedia.org/wiki/Arma_2 "Arma 2")_ (2009)[\[20\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-20)
- _[PlanetSide 2](https://en.wikipedia.org/wiki/PlanetSide_2 "PlanetSide 2")_ (2012)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)
- _[Arma 3](https://en.wikipedia.org/wiki/Arma_3 "Arma 3")_ (2013)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)
- _[Rainbow Six Siege](https://en.wikipedia.org/wiki/Rainbow_Six_Siege "Rainbow Six Siege")_ (2015)[\[21\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-21)
- _[Heroes & Generals](https://en.wikipedia.org/wiki/Heroes_&_Generals "Heroes & Generals")_ (2016)
- _[Escape from Tarkov](https://en.wikipedia.org/wiki/Escape_from_Tarkov "Escape from Tarkov")_ (2017)[\[22\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-22)
- _[Ark: Survival Evolved](https://en.wikipedia.org/wiki/Ark:_Survival_Evolved "Ark: Survival Evolved")_ (2017)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)
- _[Unturned](https://en.wikipedia.org/wiki/Unturned "Unturned")_ (2017)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)
- _[Destiny 2](https://en.wikipedia.org/wiki/Destiny_2 "Destiny 2")_ (2017)[\[4\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto1-4)
- _[PUBG: Battlegrounds](https://en.wikipedia.org/wiki/PUBG:_Battlegrounds "PUBG: Battlegrounds")_ (2017)[\[23\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-23)[\[24\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-24)
- _[Fortnite Battle Royale](https://en.wikipedia.org/wiki/Fortnite_Battle_Royale "Fortnite Battle Royale")_ (2017)[\[10\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-behome-10)[\[a\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-25)
- _[Ghost Recon: Wildlands](https://en.wikipedia.org/wiki/Tom_Clancy's_Ghost_Recon_Wildlands "Tom Clancy's Ghost Recon Wildlands")_ (2017)
- _[Atlas](https://en.wikipedia.org/wiki/Atlas_(video_game) "Atlas (video game)")_ (2018)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)
- _[Z1 Battle Royale](https://en.wikipedia.org/wiki/Z1_Battle_Royale "Z1 Battle Royale")_ (2018)
- _[DayZ](https://en.wikipedia.org/wiki/DayZ_(video_game) "DayZ (video game)")_ (2018)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)[\[25\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-26)
- _[PlanetSide Arena](https://en.wikipedia.org/wiki/PlanetSide_Arena "PlanetSide Arena")_ (2019)
- _[Ghost Recon: Breakpoint](https://en.wikipedia.org/wiki/Tom_Clancy's_Ghost_Recon_Breakpoint "Tom Clancy's Ghost Recon Breakpoint")_ (2019)
- _Path of Titans_ (2020)
- _[Watch Dogs: Legion](https://en.wikipedia.org/wiki/Watch_Dogs:_Legion "Watch Dogs: Legion")_ (2020)
- _[Arma Reforger](https://en.wikipedia.org/wiki/Arma_Reforger "Arma Reforger")_ (2022)
- _[The Crew 2](https://en.wikipedia.org/wiki/The_Crew_2 "The Crew 2")_ (2018)
- _[The Cycle: Frontier](https://en.wikipedia.org/wiki/The_Cycle:_Frontier "The Cycle: Frontier")_ (2022)[\[26\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-27)
- _[Mount & Blade II: Bannerlord](https://en.wikipedia.org/wiki/Mount_&_Blade_II:_Bannerlord "Mount & Blade II: Bannerlord")_ (2022)[\[5\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-auto-5)
- _[Tom Clancy's Rainbow Six Extraction](https://en.wikipedia.org/wiki/Tom_Clancy's_Rainbow_Six_Extraction "Tom Clancy's Rainbow Six Extraction")_ (2022)
- _[Tibia](https://en.wikipedia.org/wiki/Tibia_(video_game) "Tibia (video game)")_ (2017)
- _[War Rock](https://en.wikipedia.org/wiki/War_Rock "War Rock")_ (2023)
- _[Ark: Survival Ascended](https://en.wikipedia.org/wiki/Ark:_Survival_Ascended "Ark: Survival Ascended")_ (2023)
- _[Skull and Bones](https://en.wikipedia.org/wiki/Skull_and_Bones_(video_game) "Skull and Bones (video game)")_ (2024)
- _[XDefiant](https://en.wikipedia.org/wiki/XDefiant "XDefiant")_ (2024)
- _[Enlisted](https://en.wikipedia.org/wiki/Enlisted_(video_game) "Enlisted (video game)")_ (2024)
- _[Grand Theft Auto Online](https://en.wikipedia.org/wiki/Grand_Theft_Auto_Online "Grand Theft Auto Online")_ (2024)[\[27\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-trueman-28)
- _[War Thunder](https://en.wikipedia.org/wiki/War_Thunder "War Thunder")_ (2024)[\[28\]](https://en.wikipedia.org/wiki/BattlEye#cite_note-29)
- _[Dune: Awakening](https://en.wikipedia.org/wiki/Dune:_Awakening "Dune: Awakening")_ (2025)
- _[SCUM](https://en.wikipedia.org/wiki/Scum_(video_game) "Scum (video game)")_ (2025)
- _[Super People](https://en.wikipedia.org/wiki/Super_People "Super People")_ (2025)
- _[Marathon](https://en.wikipedia.org/wiki/Marathon_(2026_video_game) "Marathon (2026 video game)")_ (2026)

## See also

- [nProtect GameGuard](https://en.wikipedia.org/wiki/NProtect_GameGuard "NProtect GameGuard")
- [PunkBuster](https://en.wikipedia.org/wiki/PunkBuster "PunkBuster")
- [Valve Anti-Cheat](https://en.wikipedia.org/wiki/Valve_Anti-Cheat "Valve Anti-Cheat")

## Notes

1. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-25)Used alongside [Easy Anti-Cheat](https://en.wikipedia.org/wiki/Easy_Anti-Cheat "Easy Anti-Cheat")

## References

01. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-1)["BattlEye anti-cheat will support Steam Deck, but there's a catch"](https://www.pcgamesn.com/steam-deck/proton-battleye-anti-cheat-support). _PCGamesN_. 27 September 2021.
02. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-2)Chalk, Andy (September 24, 2021). ["BattlEye anti-cheat confirms Steam Deck support"](https://www.pcgamer.com/battleye-anti-cheat-confirms-steam-deck-support/). _PC Gamer_– via www.pcgamer.com.
03. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-3)["About"](https://www.battleye.com/about/). _BattlEye – The Anti-Cheat Gold Standard_.
04. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto1_4-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto1_4-1)Will Sawyer (August 26, 2021). ["Destiny 2 now has BattlEye anti-cheat – here's everything we know"](https://www.gamesradar.com/destiny-2-battleye-anti-cheat/). _gamesradar_.
05. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-1) [3](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-2) [4](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-3) [5](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-4) [6](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-5) [7](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-6) [8](https://en.wikipedia.org/wiki/BattlEye#cite_ref-auto_5-7)Hollister, Sean (December 3, 2021). ["Valve says DayZ and five other games are now anti-cheat ready for Linux (and Steam Deck)"](https://www.theverge.com/2021/12/3/22816279/valve-proton-battleye-anti-cheat-dayz-linux-steam-deck). _The Verge_.
06. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-6)["'Arma 3' and 'DayZ' add BattlEye anti-cheat support through Valve Proton"](https://www.engadget.com/arma-3-dayz-proton-battleye-support-224625719.html). _Engadget_.
07. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-1) [3](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-2) [4](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-3) [5](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-4) [6](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-5) [7](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-6) [8](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-7) [9](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-8) [10](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-9) [11](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-10) [12](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-11) [13](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beabout_7-12)["About"](https://www.battleye.com/about/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. Retrieved 18 November 2025.
08. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-benewdawn_8-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-benewdawn_8-1) [3](https://en.wikipedia.org/wiki/BattlEye#cite_ref-benewdawn_8-2)["A New Dawn"](https://www.battleye.com/2015/02/09/a-new-dawn/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. 9 February 2015. Retrieved 18 November 2025.
09. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-bemoregames_9-0)["More games!"](https://www.battleye.com/2017/04/04/more-games/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. 4 April 2017. Retrieved 18 November 2025.
10. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-1) [3](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-2) [4](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-3) [5](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-4) [6](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-5) [7](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-6) [8](https://en.wikipedia.org/wiki/BattlEye#cite_ref-behome_10-7)["BattlEye – The Anti-Cheat Gold Standard"](https://www.battleye.com/). _BattlEye_. BattlEye Innovations e.K. Retrieved 18 November 2025.
11. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-bicommunity_11-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-bicommunity_11-1)["BattlEye"](https://community.bistudio.com/wiki/BattlEye). _Bohemia Interactive Community_. Bohemia Interactive. Retrieved 18 November 2025.
12. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-gtabe_12-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-gtabe_12-1) [3](https://en.wikipedia.org/wiki/BattlEye#cite_ref-gtabe_12-2) [4](https://en.wikipedia.org/wiki/BattlEye#cite_ref-gtabe_12-3)["Grand Theft Auto Online BattlEye FAQ"](https://support.rockstargames.com/articles/1nenwhZlVrJY6CTFeSS2Fx/grand-theft-auto-online-battleye-faq). _Rockstar Games Customer Support_. Rockstar Games. 17 June 2025. Retrieved 18 November 2025.
13. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-13)["BattlEye – The Anti-Cheat Gold Standard» About"](https://www.battleye.com/about/). Retrieved 2024-05-22.
14. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-besupport_14-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-besupport_14-1)["Support"](https://www.battleye.com/support/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. Retrieved 18 November 2025.
15. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-bedocs_15-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-bedocs_15-1)["Documentation"](https://www.battleye.com/support/documentation/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. Retrieved 18 November 2025.
16. [1](https://en.wikipedia.org/wiki/BattlEye#cite_ref-befaq_16-0) [2](https://en.wikipedia.org/wiki/BattlEye#cite_ref-befaq_16-1) [3](https://en.wikipedia.org/wiki/BattlEye#cite_ref-befaq_16-2) [4](https://en.wikipedia.org/wiki/BattlEye#cite_ref-befaq_16-3)["FAQ"](https://www.battleye.com/support/faq/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. Retrieved 18 November 2025.
17. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-ubir6_17-0)["Dev Blog: Update on Anti-Cheat in Rainbow Six Siege"](https://www.ubisoft.com/en-us/game/rainbow-six/siege/news-updates/4CpkSOfyxgYhc5a4SbBTx/dev-blog-update-on-anticheat-in-rainbow-six-siege). _Tom Clancy's Rainbow Six Siege – News & Updates_. Ubisoft. 3 February 2021. Retrieved 18 November 2025.
18. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-beprivacy_18-0)["Privacy Policy"](https://www.battleye.com/privacy-policy/). _BattlEye – The Anti-Cheat Gold Standard_. BattlEye Innovations e.K. Retrieved 18 November 2025.
19. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-19)["BattlEye – The Anti-Cheat Gold Standard» About"](https://www.battleye.com/about/). Retrieved 2024-12-05.
20. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-20)Yin-Poole, Wesley (June 15, 2012). ["DayZ hackers slapped with global bans"](https://www.eurogamer.net/articles/2012-06-15-day-z-hackers-slapped-with-global-bans). _[Eurogamer](https://en.wikipedia.org/wiki/Eurogamer "Eurogamer")_.
21. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-21)["Rainbow Six Siege Cheaters Are About to Get Their Comeuppance"](https://www.gamespot.com/articles/rainbow-six-siege-cheaters-are-about-to-get-their-/1100-6442539/).
22. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-22)["Escape from Tarkov banned 3,000 players the day after the latest wipe"](https://www.pcgamesn.com/escape-from-tarkov/ban-wave). _PCGamesN_. 30 May 2020.
23. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-23)["BattleEye Banned Over One Million PUBG Cheaters In January"](https://www.shacknews.com/article/103190/battleeye-banned-over-one-million-pubg-cheaters-in-january). _Shacknews_. 5 February 2018.
24. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-24)Hall, Charlie (February 5, 2018). ["PUBG anti-cheat maker banned a million players in January alone"](https://www.polygon.com/2018/2/5/16973984/pubg-cheat-ban-1-million-january-china-battleeye). _[Polygon](https://en.wikipedia.org/wiki/Polygon_(website) "Polygon (website)")_.
25. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-26)[_How DayZ Deals With Cheaters_](https://www.youtube.com/watch?v=0M0xBMEuWdU), retrieved 2022-10-06
26. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-27)["The Cycle: Frontier works nicely on Steam Deck and Linux desktops"](https://www.gamingonlinux.com/2022/06/the-cycle-frontier-works-nicely-on-steam-deck-and-linux-desktops/). _GamingOnLinux_. 20 June 2022. Retrieved 9 August 2022.
27. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-trueman_28-0)Trueman, Aaron (2024-09-17). ["New GTA Online Update Adds Anti-Cheat For 11th Anniversary With BattlEye, Patch Notes"](https://rockstarintel.com/new-gta-online-update-adds-anti-cheat-for-11th-anniversary-with-battleye/). _RockstarINTEL_. Retrieved 2024-09-17.
28. [↑](https://en.wikipedia.org/wiki/BattlEye#cite_ref-29)["War Thunder Anti-Cheat System Update - News - War Thunder"](https://warthunder.com/en/news/9247-war-thunder-anti-cheat-system-update-en). _warthunder.com_. Retrieved 2024-12-05.

## External links

- [Official website](https://www.battleye.com/)

## Categories

- [Category:Linux software](https://en.wikipedia.org/wiki/Category:Linux_software)
- [Category:Proprietary software](https://en.wikipedia.org/wiki/Category:Proprietary_software)
- [Category:MacOS software](https://en.wikipedia.org/wiki/Category:MacOS_software)
- [Category:Windows software](https://en.wikipedia.org/wiki/Category:Windows_software)
- [Category:Articles with short description](https://en.wikipedia.org/wiki/Category:Articles_with_short_description)
- [Category:Short description is different from Wikidata](https://en.wikipedia.org/wiki/Category:Short_description_is_different_from_Wikidata)
- [Category:Anti-cheat software](https://en.wikipedia.org/wiki/Category:Anti-cheat_software)
- [Category:2004 software](https://en.wikipedia.org/wiki/Category:2004_software)