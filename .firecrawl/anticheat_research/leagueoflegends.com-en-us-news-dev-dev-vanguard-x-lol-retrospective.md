[League of Legends](https://na.leagueoflegends.com/en-us/)

[Game Overview](https://www.leagueoflegends.com/en-us/how-to-play/)

[Champions](https://www.leagueoflegends.com/en-us/champions/)

News

- [ALL](https://www.leagueoflegends.com/en-us/news)
- [GAME UPDATES](https://www.leagueoflegends.com/en-us/news/game-updates)
- [ESPORTS](https://www.leagueoflegends.com/en-us/news/esports)
- [DEV](https://www.leagueoflegends.com/en-us/news/dev)
- [LORE](https://www.leagueoflegends.com/en-us/news/lore)
- [MEDIA](https://www.leagueoflegends.com/en-us/news/media)
- [MERCH](https://www.leagueoflegends.com/en-us/news/merch)
- [COMMUNITY](https://www.leagueoflegends.com/en-us/news/community/)
- [RIOT GAMES](https://www.leagueoflegends.com/en-us/news/riot-games)

[Patch Notes](https://www.leagueoflegends.com/en-us/news/tags/patch-notes)

Discover

- [COMMUNITY](https://www.leagueoflegends.com/en-us/news/community/join-the-community/)
- [FIND MY CHAMP](https://findmychamp.leagueoflegends.com/?rg_link=sitebar)
- [HALL OF LEGENDS](https://halloflegends.leagueoflegends.com/)
- [LEAGUE DISPLAYS](https://na.leagueoflegends.com/en/featured/league-displays)
- [RIOT MOBILE](https://www.leagueoflegends.com/en-us/news/riot-games/hello-riot-mobile-farewell-league/)

[Esports](https://lolesports.com/)

[Universe](https://universe.leagueoflegends.com/en_US/)

[Shop](https://merch.riotgames.com/en-us/)

[Support](https://support.riotgames.com/hc/en-us/)

[PBE](https://www.leagueoflegends.com/pbe)

[League of Legends](https://na.leagueoflegends.com/en-us/)

[Game Overview](https://www.leagueoflegends.com/en-us/how-to-play/) [Champions](https://www.leagueoflegends.com/en-us/champions/) [News](https://www.leagueoflegends.com/en-us/news/dev/dev-vanguard-x-lol-retrospective/#) [Patch Notes](https://www.leagueoflegends.com/en-us/news/tags/patch-notes) [Discover](https://www.leagueoflegends.com/en-us/news/dev/dev-vanguard-x-lol-retrospective/#) [Esports](https://lolesports.com/) [Universe](https://universe.leagueoflegends.com/en_US/) [Shop](https://merch.riotgames.com/en-us/) [Support](https://support.riotgames.com/hc/en-us/) [PBE](https://www.leagueoflegends.com/pbe)

PLAY NOW

PLAY NOW

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/bc6914f7d2cfdd2de7a506ece8042431d7d9fafd-1920x1080.jpg?accountingTag=LoL&auto=format&fit=fill&q=80&w=1905)

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/bc6914f7d2cfdd2de7a506ece8042431d7d9fafd-1920x1080.jpg?accountingTag=LoL&auto=format&fit=fill&q=80&w=1184)

# /dev: Vanguard x LoL Retrospective

How the anti-cheat is anti-cheating so far.

[Dev](https://www.leagueoflegends.com/en-us/news/dev/)

mirageofpenguins

8/22/2024

Greetings Travelers,

It’s now been almost eight megaseconds since we released Vanguard for League of Legends, and in that unfathomable amount of time, there have been several highlights, a few lowlights, and at least one Vanguard cosplayer (haute couture). We’re gonna share the first two things with you now, in a continuation of our steadfast attempts to become both the loudest and the most transparent anti-cheat team in PC gaming, but before we do that, please consider also reading the article we published prior to Vanguard’s release as a palette-cleansing, [14-course appetizer](https://www.leagueoflegends.com/en-us/news/dev/dev-vanguard-x-lol/).

One philosophical amuse-bouche that we should get out of the way early is that, while we’ll keep working on bettering the Vanguard x LoL integration (and cheaters will keep working on their cheats), we’re not going to relitigate the decision to utilize this technology. All anti-cheat methods trade friction for effectiveness, and where we position ourselves on that curve demonstrates how seriously Riot takes its competitive experiences. The F2P model encounters too many unwinnable problems when the bans can’t stick to the bad actors, including for behaviors like feeding, toxicity, and boosting. It’s okay if kernel anti-cheat just isn’t your bag, but we’re not going to wait around for the PC platform to offer security features sufficient to support the games we want to make. Until that day, we genuinely believe that an effective anti-cheat is the best way to keep League thriving.

Alright, onto the rest of the meal. Once again, my name is “mirageofpenguins,” and I’ll be your maitre d’ this evening. My culinary credentials thus far include being fired from a Subway franchise after nine months of employment, but I’ve been cooking oven-fresh anti-cheat at Riot for over a decade since then.

# How It’s Going

Like any decent sandwich, here at Vanguard’s we put the bread on top. In that regard, updating LoL’s anti-cheat has achieved a great many of our initial goals, some to the degree that several people have even considered releasing Vanguard 2. Okay, no one thought that (and anti-cheat updates are about as fun as putting gloves on a cat), but there have been several immediate wins from bringing LoL’s anti-cheat system into this century that probably aren’t too large a surprise.

## Fewer Scripters

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/ae20936ffc7bffd2b3d96c5472495bf6d67169ab-1664x829.png)

_The percentage of Ranked games with at least one scripter in them, and bans issued explicitly for “scripting” in competitive play. Legends has it that I love this graph so much I’ve had it tattooed onto both my forearms, and I go back to the parlor every three weeks to have them both excruciatingly updated. Sure hope we run out of scripters before I run out of flesh._

The first and most obvious victory is that there are far fewer cheaters in the game now. Above is LoL’s Ranked scripting rate, graphed alongside daily anti-cheat ban volume and bucketed based on the system in which the ban originated (“Packman” is the old anti-cheat, and “Vanguard” is the new one). A significant portion of the immediate reduction in scripters is due to Vanguard’s preventative features, but as might be evident from the manifestation of the “hardware ban” category, further reductions have been accomplished through detecting cheaters that _heroically_ still tried to cheat post-Vanguard, bravely sacrificing all of their accounts to see if the anti-cheat had a ban limit (it didn’t).

Since the release of Vanguard, we have banned over 175,000 accounts for cheating, but more importantly, **our Ranked scripting rate fell below 1% for the first time in nearly four years**. As of the time of this writing, only 1 in every 200 Ranked games is played with a scripter, and I figure it’s finally safe for me to complete my placement games. I’ll for sure still land in Iron II with a crisp 10% win rate, but at least now, I’ll only have Zed’s entire existence as a champion to blame.

The most recent spike in ban throughput (on July 8th) was the direct result of a little something Riot likes to call “ [summer break](https://www.riotgames.com/en/news/riot-games-mid-year-summer-break),” and our returning from it was accompanied by a sudden burst in raw anti-cheat energy, nuking 35,000 scripters in just under 48 hours. While many were stunned to find that anti-cheaters even needed to sleep, please be assured that most of us are nearly 100% detectably human. We too love simple pleasures like looking at trees and ingesting nutrients, so a good week of respite only served to strengthen our resolve.

## Fewer Bots

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/c2c179a0095643cd61e3aeb6a8f1d86551d2342c-1668x830.png)

_The raw number of botting hours, broken down by queue in which the game occurred. You’d be correct to assume that I used “hours” here so that executives could more immediately feel the impact in the deepest fold of their wallet (server costs)._

Our second graph is a look at how many game hours bots have wasted this year. Botting and scripting software is similar, so we deconflict the two offenses primarily through the performance of the player and the game client. i.e. Bots die to towers and play at a high-powered maximum of 9 FPS. In the native data tongue, it would sound a little something like this:

select date, game\_mode, sum(minutes\_in\_game) \* 60

from anticheat.detections

where client.resolution\_x + client.resolution\_y < 1000

and client.avg\_fps < 15 fps

group by 1, 2

Anyways, Vanguard’s anti-virtual machine techniques put a pretty hefty dent in the average programmable coffeemaker’s ability to brew a League of Legends session, **dropping total hours spent botting from north of 1 million a day to less than 5 thousand**. Many botting farms were struck with lasting emotional turmoil, and if you played co-op vs AI, you may’ve actually seen them standing in your fountain having deep, introspective thoughts. This rapid onset of bot-paralysis was the simple result of their not running Vanguard—no Vanguard session, no connection to the server. Some bots have tried to trickle back in on old OSX VMs, but that’s something we’re saving for dessert (keep reading).

Shortly after releasing Vanguard, we also cleaned up 3.5 million bot accounts that hadn’t yet been sold, and the idea there is to slowly dehydrate the market for secondary accounts. Bots are a big part of the engine that powers competitive LoL abuse, both for supplying boosters with new smurfs on which to queue alongside their customers and also for supplying scripters with new accounts on which to “play.” I’m actually not sure you can call either of these things “playing” League of Legends, so we’ll want to stay extremely vigilant on smart toasters to maximize the misery per second of our other punishments.

## Faster Bans

You may’ve experienced a fleeting moment of relief at the prospect that I’d abandoned the insufferable sandwich metaphor, but like all struggling writers, I wrestle with my artistic commitment to the literary and nutritional mediums. So, these next graphs can be considered the mayonnaise, and a lot like actual mayonnaise, they’re the entire point of the sandwich.

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/4edfa02f23bf9533f2f7bb3da952c12d02450142-1658x809.png)

_“Time to Action” (left) is measured in games, and “Time to Detection” (right) is measured in days. The former is more a measure of how swiftly we’re removing bad actors, and the latter examines the speed at which we’re able to ship detections for known cheats. It’s important to note that detecting something does not immediately indicate we are banning for it._

Above on the left is our primary anti-cheat KPI, called “Time-to-Action,” and it can be interpreted simply as the number of games a cheater is able to play before their account is merged with a reality in which it never existed, a spacetime-compression algorithm that most three-dimensional beings perceive as a ban. Due in part to our no longer needing to rely on LoL’s update cadence, Vanguard has significantly expedited the process of removing scripters from League of Legends. **Our time-to-action has fallen from 45+ games to less than 10**, and even that small delay is mostly deliberate to slow “realizations” from developers that we have them surrounded.

To that effect, the graph on the right is a glance at the other side of this coin.“Time-to-Detection” is a measurement of how long a cheat (or an update to one) is able to hide in the LoL ecosystem before a detection for it is written on a scroll and burned in tribute to the sentient Vanguard Cloud. We can estimate this by looking at the oldest age (in days) of all the account + hardware combinations identified for the first time when a new detection is committed (when it likely “first appeared”). We’re extremely fast right now, but as cheats move underground or become more advanced, it will take us longer to find and create detections for them. This is the delicate balancing act of anti-cheat: we cannot over-prioritize “actions” without also expediting cheater updates and slowing our “detections.”

A cheater willing to cheat in a post-vanguard universe is usually not committed to the idea of account ownership or to the idea of a fair game. Their community is often one of other cheaters, the way they interface with the game is by cheating, and nothing but time or puberty can change that. Until then, all we can do is iterate with them, and the speed at which they are re-identified is a reflection of how effective we are at forcing them to start over.

## Other “Interesting” Changes

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/2696db77707107c49fb0734658c392997f9071e0-1660x826.png)

_That Zeri delta might look juicy, but as a reminder, cheating is a one way ticket to Ban City, where the tax rate is 100% of your yearly accounts._

The above dashboard is monitoring the 9 most popular scripting champions for any changes that may’ve resulted from Vanguard’s deployment, and because a veritable buffet of mouse clicks is something scripts can serve faster than a human, the majority of them are ADCs. The timeline on the left is how much more a champion wins when they’re cheating than when they’re not (as the delta in ranked win rate between scripters and normal players), and the graph on the right is that same champion’s total win rate, measured 60 days before and 60 days after Vanguard. To ensure a competitive sample, all games included here are ones in which the player on the identified champion was then-rated Platinum or above.

A joyous thing can be observed from allowing photons to bounce off the graphs and into your retinas: **Cheaters are getting slightly _worse_**. There’s a wide variety of factors at play here, but the MVP is Vanguard making it extremely annoying to use “internal” cheats without opting directly into a detectable pattern, resulting in many cheaters pivoting to either, (1) playing manually or (2) settling for cheats of the “external” variety. As that title implies, these cheats don’t have the luxury of reading game memory, so they get all their data by reading the screen and attempting to submit input on the cheater’s behalf. To make a long story short, they’re just.. not very good.

Additionally, the reduction in scripters (and the reduction in scripting efficacy) appear to be affecting even overall win rates for the scripting poster-children. It’s hard to control for things like balance changes, seasonal resets, and counterpicks falling into favor, but some percentage of these decreases are due to cheaters having difficulty keeping accounts in Diamond. Puts a real spring in my step when I think about it.

## Minimal False Positives

All new anti-cheat can run the risk of flagging software assets that “look” like cheats (usually malware or cheats for _other_ games), but luckily Vanguard isn’t all that “new”—it turned four this year. As the final piece of bread on this sandwich (that I now realize is comprised only of two wet ciabatta slices dipped in condiment), we will examine Vanguard’s false positive rates on LoL.

![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news/b220100cba75e46f544ebc6b5380bf0a187f45e9-1663x828.png)

_It’s worth mentioning that “my brother’s pet iguana installed scripts on my computer” is not currently accepted as a valid excuse for cheating, but based on the frequency that we receive it, we’re beginning to have concerns that the “Godzilla” franchise could be prophetic._

On the left axis above is the percentage of all Vanguard suspensions reversed (bars), bucketed based on the underlying reason for the unban, and on the right axis is that same breakdown examining the average amount of time those accounts spent suspended (lines). There are three types of ban reversals here, in order of appearance:

1. An account that was stolen (not deliberately shared) at the time they were detected cheating.

2. An account that was locked due to borrowing or buying previously banned hardware.

3. An account that was banned for an asset or behavior that is not explicitly for cheating in League of Legends.


We consider a “real” false positive to be the last condition, and so far, this rate in total is sub 0.01%, or **less than 1 in every 10,000 bans**. Better still, the average duration any of these innocent accounts were suspended was less than 72 hours. We had to tune a few rules at launch to better accommodate LoL’s atypical patterns of playing other games simultaneously, but it’s been relatively smooth sailing since. We remain extremely committed to the accuracy of our punitive actions, and we continually re-review Vanguard’s rules to minimize collateral damage.

All that said, cheaters still need more accounts to cheat on, so the “stolen account” case remains far-and-away the most common. While player support makes occasional one-time-exceptions for accounts that were _obviously_ compromised, sometimes it’s impossible to tell “who” originally owned an account, especially when it’s deliberately shared over long periods of time. Putting Vanguard on League has alerted many account co-owners to a comrade’s cheating proclivities, but there’s not much we can actually do when two or more players have claim to an account.

**Don’t share your account, don’t reuse passwords, and** [**please enable MFA**](https://support-leagueoflegends.riotgames.com/hc/en-us/articles/4415714343187-Two-Factor-Authentication-How-to-FAQ).

# The Tougher Topics

Approximately 0.0% of people are thrilled at the idea of installing mandatory anti-cheat, so it probably wouldn’t surprise anyone to discover that the Vanguard team didn’t exactly anticipate a red carpet for LoL. Vanguard is a fairly complex product that operates in near-complete opacity. A lot of this is necessary to be effective on cheaters that would love only to understand a little more of it, but that same obscurity makes Vanguard an extremely visible target that doesn’t always offer an explanation. Some of these next sections are going to get a little technically intense, but stay with me and we’ll plow through it together.

## Vulnerable Driver Blocking

Vanguard’s goal is not to become some sort of continually-surveilling police state, but instead to function as a badge of pre-existing security for the system on which it’s being run. By creating a perimeter around the Windows kernel, Vanguard allows us to require less information from systems that haven’t had Windows’ native protections breached and are still in known-secure states.

Our anti-cheat accomplishes this perimeter **without network connectivity** by having its driver component start when the operating system does, blocking other drivers from being used in a “race” to the kernel—where they could then hide themselves indefinitely from anything that loads after. Often called the “Who Loads First?” problem, Vanguard attests to this not having happened since boot simply by still being there when the game launches.

The things Vanguard blocks are:

1. Vulnerable drivers with privilege escalation exploits that can be used to get code into the kernel.

2. Relatively old drivers with certificates in which one of the signatures does not possess a timestamp.

3. Drivers that are outright used for cheats, signed by cheat developers masquerading as legitimate software companies.


The second case is the most common collision, but the problem with allowing old certificates is that many of them have been stolen by cheaters. In most cases, this can be resolved by simply downloading a newer version of the affected driver, but sometimes, the developers have long since moved on. Even if they could, revoking their old signatures now would prevent every legitimate user from ever running software they’ve signed, so instead, Vanguard blocks drivers with these certificates when it’s active. You can always kill Vanguard to load them anyway, but to play a Vanguarded game, we still need to know nothing had a chance to compromise Windows since boot.

## BootLooping

A recent [mass-bootlooping event](https://en.wikipedia.org/wiki/2024_CrowdStrike_incident) generated world-wide concern over the potential hazards of operating kernel drivers, but while it was certainly spooky, Vanguard is largely not at-risk for this worst-case scenario. Several differentiating factors and direct mitigations are at play here.

#### Boot Differentiation

Microsoft-certified anti-malware components have the [ELAM](https://learn.microsoft.com/en-us/windows-hardware/drivers/install/early-launch-antimalware) entitlement, and with it, the privilege of loading their driver on “boot” start, earlier than Vanguard’s “system” start (studious readers might recognize this as a natural progression of the “Who Loads First” arms race). But more importantly, many anti-malware drivers also _dynamically_ pull configuration blobs from a remote server at runtime, without the need to rebuild and recertify the driver. Such designs substantially accelerate threat response, but they also persist data locally for use on each initialization, exposing a vector for irrevocable updates if any config should result in a race condition where new blobs cannot be downloaded before the operating system crashes. A dynamic, driver entry would’ve been a significant risk surface increase, and it’s one the Vanguard team figured we’d sleep better without.

#### Static Code

Instead, Vanguard’s driver (VGK.sys) does nothing dynamically at launch— **it is all static code**. We leverage Vanguard’s client component (VGC.exe) to activate features within the driver remotely only when actively playing a game. No configurations are stored, changed, or persisted to the next launch of the driver, and if there ever was to be a critical bug, we would just stop sending the affected config, reverting the driver back to its statically passive state on next reboot. Vanguard’s driver component has no network connectivity itself, and the client must establish a connection with the platform before it ever actively “does” anything beyond blocking vulnerable drivers that are loaded after it.

#### A Simple Failsafe

Some _enterprising_ young engineers have already detailed this process, but Vanguard’s driver entry has a deadman’s switch mechanism in the form of vgkbootstatus.dat file. When Vanguard first starts, it checks the status of this file, and if it doesn’t say “launched,” the driver exits safely. Otherwise, it sets the status of said file to a “launching” state, and once the preamble successfully completes, it again sets that same status to “launched.” Basically, should a launch of VGK.sys fail to complete successfully, that file would still read “launching,” and the driver would not run again until it was updated (by launching a Riot title or deliberately reinstalling Vanguard), mitigating bootloops in the unlikely event of a launch conflict.

## “Vanguard Event” Near You

Vanguard’s integrations with LoL are a little unique, chief of which is that the anti-cheat session is created when the light desktop client starts, instead of when the game client does (like VALORANT). This is quirky from an anti-cheat perspective, and it comes with a deceivingly simple challenge: **players often leave their desktop client running.** This means that (1) the computer can go to sleep while a LoL session is active and (2) one session usually kicks another (e.g. between a home and work computer).

Unfortunately, LoL’s integration with Vanguard did not handle these events, and because each account can only have one Vanguard session at a time, the end result is players ending up in a Vanguard-less state. So, if you were in-game and a second computer re-authenticated to Vanguard, you’d be booted from the server for no longer having an anti-cheat session. Similarly, if you lost your session while searching for a game, you might not get a notification about it until you got through the loading screen and were kicked, potentially subjecting you to the brutal experience of a match remake plus the resulting LP loss.

Riot royally biffed this one, but the quick fixes have been patches to re-auth logistics. LoL is following up with pre-game Vanguard session checks to be 300% sure it can’t happen again. If you’ve been following my infallible dinner metaphor, this dish would best be described as the spaghetti, and while it’s Riot’s signature platter, we’d like to prevent circumstances where anyone has to eat it.

## And another thing…

Finally, I’m gonna steal eyeballs for one moment more to highlight three other issues that we’ve seen making the rounds, on the off chance that google indexes this page sufficiently for anyone to find it helpful. As always, the best way to get assistance is to [submit a ticket](https://support-leagueoflegends.riotgames.com/hc/en-us/requests/new).

#### Click Lag or FPS Drops

There are several 3rd party applications (mods, overlays, or passive benchmarking tools) that can sometimes flippantly try to open read handles or set hooks for notifications on certain events within the LoL client, and now that the game is protected by Vanguard, these operations will inevitably fail. We outright don’t want things interfering with the game, so the blocking behavior is 100% intended. However, the way certain apps wrap a Windows operation failing can range from silently ignoring the problem to _repeatedly trying to do it again without any delay whatsoever_, something that is almost impossible to address on our side. So, if you know what application is to blame, and it doesn’t already have allowlisting mechanisms for adding LoL.exe exceptions, you can prevent its attempts to manipulate LoL.exe yourself by using [the following cheat code](https://support-leagueoflegends.riotgames.com/hc/en-us/articles/201752684-Low-Frame-Rate-FPS-Troubleshooting).

#### Enabling TPM 2.0

We found that our requiring TPM 2.0 on Windows 11 could generate confusion for some players when they went into BIOS to enable it. BIOS settings can vary wildly based on the manufacturer, and in exactly two known cases, players were also prompted to switch to UEFI mode to enable TPM, even though their existing Windows installation was under a Master Boot Record partition table style (MBR). Unfortunately, to support UEFI mode, Windows would need to be installed on a disk using the GUID partition table format (GPT), otherwise it could become unbootable. While this should’ve been handled when originally installing Windows 11 (as [Microsoft also requires](https://support.microsoft.com/en-us/windows/enable-tpm-2-0-on-your-pc-1fd5a332-360d-4f46-a1e7-ae6b0c90645c)), Vanguard forced the issue for a few players that had bypassed Microsoft’s original TPM 2.0 checks.

If you’re trying to enable TPM 2.0 for any of the anti-cheats that now require it, and you know yourself to be in the MBR scenario with data you don’t want to lose in a straight reformat, [microsoft has a tool](https://learn.microsoft.com/en-us/windows/deployment/mbr-to-gpt) that can potentially allow you to convert a disk to GPT without deleting any data.

#### Specific Hardware Interactions

Driver development can be especially tricky when OEMs or vendors accidentally ship or push faulty firmware onto a percentage of devices. We try and stay ahead of this in the compatibility lab, but there are some things that are genuinely out of our control. If you’re still plagued with random blue screens _,_ and you’ve got an intel 13th or 14th generation CPU, it could very likely be the case that you’ve got a device [with out-of-date firmware](https://support-leagueoflegends.riotgames.com/hc/en-us/articles/30677122946195-vgk-sys-Error-Troubleshooting-13900k-14900k-processors-only). Intel is working on addressing these wider-reaching issues.

# Next for Anti-Cheat on LoL

Anti-cheat is never finished, and while Vanguard has reduced our surface area and increased their barrier to entry, cheaters are always on the hunt for new ways to gain an unfair advantage. Here’s a look at a few things we’re cooking to make sure they stay famished.

## Powering UP

While many cheat developers have thrown in the towel, we’re thrilled to report that several did not get the message, and we can hardly wait to turn on more of Vanguard for them to play with. Bans thus far have been only an aperitif, and we are extremely confident in our kitchen’s preparedness for this service. Many cheaters can often get stuck in the “denial” stage of the grief cycle, so we think of each ban as extinguishing a flame on the candelabra of hope. By gradually allowing every cheater to be swallowed by complete darkness, they can finally achieve true enlightenment.

## “On Demand” Vanguard

As was foretold, a future will eventually arrive where we can rely on the security features of Windows to protect its own kernel, instead of protecting it from boot with a driver. This will allow us the opportunity to start our anti-cheat services when the game client runs, provided the end-user has opted into all of these features. We’ll have more communication on this topic early next year, but if you’re on Windows 11 and on relatively recent hardware, we wanted to let you know that you won’t have to tolerate the taskbar icon forever (even though we worked very hard on Vanguard’s logo).

## Boosting Detection

Boosting refers to the behavior of deliberately playing with (or on) lower rated accounts for the purposes of increasing their Ranked standing. Detection is something the anti-cheat team has not touched since 2018, and we’re extremely excited about now having the fingerprinting technology sufficient to tackle it once more. We plan to focus the majority of our efforts here on detecting boosters queuing with customers on freshly de-ranked smurfs, and we’ll reward those accounts with bans lasting the rest of the season (Layer 1). Players that repeatedly queue with boosters affected by these punishments will also receive a similar vacation (Layer 2). Should any be brave enough to fully share their account with a boosting service, it will similarly be detected as a smurf (under Layer 1), effectively closing the loop.

There’s still a lot of work left on this one, but we plan to have it banging on 8 out of 8 cylinders next summer.

## Mac x Vanguard (aka Vanguard 2)

As we were getting at in the “botting” section, some cheaters have begun to pivot to macOS VMs, in an effort to free themselves from the Vanguard requirement. This move was about as unexpected as syrup on pancakes, so we’re pleased to announce that Vanguard’s companion product, Embedded Vanguard (mVG) is coming soon to a Mac build near you. The unique security of the macOS environment allows us to be a little less stringent on defending its kernel, so as the name implies, this won’t require any extra installs—the security is “embedded” right into the game client. Further still, we’re actually already using mVG to great effect on VALORANT console and on Wild Rift.

Once it ships at the end of this year, we hope it will be the final blow to bots and to the two public scripting suite developers that are now grappling with the realization they wasted three months on porting cheats to OSX. Don’t worry too much though, Swift looks great on a resume.

# Exit

And now my friends, we must return to the dark anti-cheat kitchen to prepare our next meal. But never fret, we'll always be fighting for your right to a competition free from cheaters that refuse to get good. It’s not every decade you get to work on a game through four different iterations of anti-cheat systems, but I’m unfathomably glad LoL’s now ranks amongst the best. It’s been an absolute pleasure playing with, writing for, and banning alongside you. Until next time.

P.S. I’ve since re-read this piece, and I think I may’ve been hungry while I was writing it?

## Related Articles

1. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/b428c14c41ee524cc92c30594ba4c48c8eceda39-1920x1080.png?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
4/21/2026\\
\\
/dev: Apex Tier Ranked Reset\\
\\
Why we’re doing a Ranked reset for Master+ in NA, EUW, EUNE, BR, LAN, and TR in 26.9.](https://www.leagueoflegends.com/en-us/news/dev/dev-apex-tier-ranked-reset)
2. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/364936d73003424efaa9b122cf9f4002f7d7a88b-1920x1080.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
4/21/2026\\
\\
New Events and Augments Head to the Arena\\
\\
Prepare for a new season of glorious Arena combat that kicks off on Patch 26.09.](https://www.youtube.com/watch?v=rVYts-6Jdas)
3. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/5f9732d597be41a92d6400e619dec7b8b3d9d94b-1920x1080.png?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
4/14/2026\\
\\
/dev: WASD’s Ranked Release\\
\\
After months of testing and feedback, WASD is finally ready for primetime.](https://www.leagueoflegends.com/en-us/news/dev/dev-wasds-ranked-release)
4. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/3e4e70fab04442caf14f90256604516550d30464-1920x1080.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
4/14/2026\\
\\
TL;DW: Pandemonium, Arena & More Dev Update\\
\\
A short rundown of changes from today’s Dev Update.](https://www.leagueoflegends.com/en-us/news/dev/tldw-pandemonium-arena-more-dev-update)
5. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/700a26e353717c03d458c0facb94c0fafe6fea2b-1280x720.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
4/14/2026\\
\\
Dev Update: Arena Updates & More\\
\\
Pabro and Meddler talk about Season Two Pandemonium, gameplay changes, Arena updates, and more.](https://www.youtube.com/watch?v=K1TLR8HQgiU)
6. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/f919c87b96b0f72de811498af458079c8c488af7-1920x1080.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
4/14/2026\\
\\
/dev: Leveling Up Arena\\
\\
A rundown of all the changes coming to Arena in Season 2.](https://www.leagueoflegends.com/en-us/news/dev/dev-leveling-up-arena)
7. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/4ed6deb7fdb3d5e4d99ea364dfc70dddc63ae2c9-1920x1080.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
3/28/2026\\
\\
TFT Dev Drop: SPACE GODS\\
\\
The Space Gods have arrived! Earn their boon and take to the stars with their godly power.](https://www.youtube.com/watch?v=IOnfAHUDM10)
8. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/d018d0eb6c574a0a0e84a0bd7f182e7304056d0f-1920x1080.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
3/26/2026\\
\\
TFT Dev Drop Teaser\\
\\
Who needs a Carousel when you can beseech the stars?](https://www.youtube.com/watch?v=V18pKOq73Ts)
9. [![](https://cmsassets.rgpub.io/sanity/images/dsfx7636/news_live/7aeb1f299392e2b0641502ea183200af7d97349a-1920x1080.jpg?accountingTag=LoL&auto=format&fit=crop&q=80&h=270&w=480&crop=center)\\
\\
Dev\\
\\
3/2/2026\\
\\
TL;DW: Ranked, Shyvana & More Dev Update\\
\\
A short rundown of changes from today’s Dev Update.](https://www.leagueoflegends.com/en-us/news/dev/tldw-ranked-shyvana-more-dev-update)

- [ABOUT LEAGUE OF LEGENDS](https://na.leagueoflegends.com/en/game-info/get-started/what-is-lol/)
- [Server Status](https://status.riotgames.com/?locale=en_US&region=na)
- [Support](https://support.riotgames.com/hc/en-us)
- [Esports Pro Site](https://www.lolesports.com/en-US/)
- [Download Riot Mobile Companion App](https://riotgames.sng.link/Dnyzj/csgh)

- [Youtube](https://www.youtube.com/user/riotgamesinc)
- [Twitter](https://twitter.com/leagueoflegends)
- [Facebook](https://www.facebook.com/leagueoflegends)
- [Instagram](https://www.instagram.com/leagueoflegends/)
- [Reddit](https://www.reddit.com/r/leagueoflegends/)

[Riot Games](http://www.riotgames.com/)

™ & © 2026 Riot Games, Inc. League of Legends and all related logos, characters, names and distinctive likenesses thereof are exclusive property of Riot Games, Inc. All Rights Reserved.

- [Privacy Notice](https://www.riotgames.com/en/privacy-notice)
- [Terms of Service](https://www.riotgames.com/en/terms-of-service)

[![ESRB](https://cmsassets.rgpub.io/sanity/images/dsfx7636/riotbar/7e684cc4765a7d059f9018e16717472d7082dc37-65x97.png?&h=100&fit=max)](https://www.esrb.org/ratings/ratings_guide.jsp)

Blood

Fantasy Violence

Mild Suggestive Themes

Use of Alcohol and Tobacco

Online Interactions Not Rated by the ESRB

Check Session IFrame

OIDC OP Iframe

### Auth Error

|     |     |
| --- | --- |
| iss | https://auth.riotgames.com |
| state | 8d8603590ed2190335cae54c20 |
| error | interaction\_required |
| error\_description | login\_required |