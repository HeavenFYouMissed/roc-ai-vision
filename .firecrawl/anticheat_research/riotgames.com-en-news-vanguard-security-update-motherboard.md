```
(function() {
    'use strict';
})();
```

[Jump to Main Content](https://www.riotgames.com/en/news/vanguard-security-update-motherboard#content) [Jump to Primary Navigation](https://www.riotgames.com/en/news/vanguard-security-update-motherboard#riotbar-bar)

[Home](https://www.riotgames.com/en)

[Who We Are](https://www.riotgames.com/en/who-we-are)

[Work with us](https://www.riotgames.com/en/work-with-us)

[News](https://www.riotgames.com/en/news)

[Home](https://www.riotgames.com/en)

[Who We Are](https://www.riotgames.com/en/who-we-are) [Work with us](https://www.riotgames.com/en/work-with-us) [News](https://www.riotgames.com/en/news)

SIGN IN

SIGN IN

News

# Vanguard Security Update: Closing the Pre-Boot Gap

- **Dec 18, 2025**

- [Share this on Reddit](https://reddit.com/submit?url=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard)
- [Share this on Twitter](https://twitter.com/share?url=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard&text=Vanguard%20Security%20Update%3A%20Closing%20the%20Pre-Boot%20Gap)
- [Share this on Facebook](https://www.facebook.com/sharer/sharer.php?&u=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard)
- Toggle additional sharing options


  - [Share this on LinkedIn](https://www.linkedin.com/shareArticle?mini=true&url=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard)
  - [Share this via Email](mailto:?&subject=Riot%20Games%3A%20Vanguard%20Security%20Update%3A%20Closing%20the%20Pre-Boot%20Gap&body=Vanguard%20Security%20Update%3A%20Closing%20the%20Pre-Boot%20Gap%20%0A%0Ahttps%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard)
  - [Share this on Reddit](https://reddit.com/submit?url=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard)
  - [Share this on Twitter](https://twitter.com/share?url=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard&text=Vanguard%20Security%20Update%3A%20Closing%20the%20Pre-Boot%20Gap)
  - [Share this on Facebook](https://www.facebook.com/sharer/sharer.php?&u=https%3A%2F%2Fwww.riotgames.com%2Fen%2Fnews%2Fvanguard-security-update-motherboard)

![](https://www.riotgames.com/darkroom/1440/308cce32cbf185d76a71f92274453486:bd6e9de929ae3ebc5a5fee1f3b8af4f1/rg-com-vanguard-security-update-article-1.png)

_TL;DR: We discovered a critical flaw in a variety of motherboards that can be exploited for injecting code unnoticed. If your system is affected, Vanguard will prompt you with our VAN:Restriction service to update your motherboard’s firmware._

Greetings, ladder demons.

My name is Mohamed, and some of you may know me as **ItsGamerDoc** on X. This is my first official article at Riot. The Anti-Cheat Kitchen has been hard at work serving ladle after ladle of hearty ban chowder to cheaters, but today I don't want to focus on the delicious bans being served. Instead, I want to focus on what's been cooking in the back: a recipe for security enforcement that leaves DMA cheats… well… cooked.

Alright, enough with the cooking metaphors. **In the very near future, Vanguard will begin enforcing stricter checks on system boot security for _certain_ players.**

Why, you ask? Well, earlier this year we discovered a critical flaw affecting a variety of modern motherboards. This issue allowed hardware cheats to potentially inject code unnoticed, even when security settings on the host appeared to be enabled.

Here's the lowdown on why this is happening and, more importantly, what to do if you're affected.

## The "Who Loads First?" Problem

To understand this enforcement change, we need to talk about how your computer normally boots. At the very moment your PC powers on, it’s in its most privileged state: it has full, unrestricted access to the entire system and all connected hardware. The system begins by loading and executing its initial firmware (usually UEFI), which then starts a chain of hardware, software, and security initialization. Only after all of this does control finally pass to the operating system, which is what we typically think of as “the computer.”

The problem this creates is that components that load _earlier_ in this chain are usually _more_ privileged, and can manipulate components that load later. Cheat developers understand this extremely well. Unfortunately, the operating system your game runs on loads near the end of the process. This means cheats can load early, gain higher privilege, and hide effectively within the system before the OS or anything running on it has a chance to defend itself.

Thankfully, security features like Secure Boot, VBS, and IOMMU already exist to help provide security. And for the most part, they are effective at preventing this attack vector, assuming they are on and working correctly.

Vanguard’s strategy is simple: create a perimeter around the Windows kernel to ensure the system hasn't been compromised as early as we can. If a cheat loads before we do, it has a better chance of hiding where we can’t find it. This creates an opportunity for cheats to try and remain undetected, wreaking havoc in your games for longer than we are ok with.

## The Discovery: A Sleeping Bouncer

Remember moments ago when we talked about existing security features that do a fine job, assuming they work? Let’s talk about one of them, **IOMMU**.

For years, the most effective (and expensive) form of cheating has involved the use of **DMA (Direct Memory Access) devices**.

DMA cards are hardware devices that plug into your PC and touch memory directly, bypassing the CPU and Windows. To stop them, we rely on a hardware feature called **IOMMU** (Input-Output Memory Management Unit). Think of the IOMMU as a bouncer for your RAM; it checks the ID of every device trying to access memory and kicks out the ones not on the guest list.

**"Pre-Boot DMA Protection"** is a security feature present in the bios/firmware of many devices that leverages the systems **IOMMU** to prevent rogue DMA access to a systems memory early on in the boot sequence. This gives the loaded operating system some confidence that the system it’s on was initialized with some predictable integrity. Which is awesome, in theory... But that also means the operating system has to trust the security feature itself.

And that is precisely what the Vanguard team’s research uncovered. In some cases, hardware manufacturer firmwares incorrectly signaled to the operating system that this feature was fully active, when it was actually failing to initialize the IOMMU correctly during early boot.

This meant that while **"Pre-Boot DMA Protection"** settings appeared to be enabled in the BIOS, the underlying hardware implementation wasn't fully initializing the **IOMMU** during the earliest seconds of the boot process. In essence, the system's “bouncer” appeared to be on duty, but was actually asleep in the chair. So by the time the system is fully loaded, it can’t be 100% confident that 0 integrity breaking code was injected via DMA.

This brief window is all a sophisticated hardware cheat needs to sneak in, inject code, and hide itself before Vanguard wakes up.

## The Solution: A Unified Update

Earlier this year, we shared our findings with our hardware partners, and they have been excellent collaborators in this process, validating the issue and producing comprehensive BIOS updates to close this gap.

Don't take our word for it - here's a look at security advisories from major hardware manufacturers:

- [Case VU#382314](https://www.kb.cert.org/vuls/id/382314)

- [Asus Security Advisory](https://www.asus.com/security-advisory/) (CVE-2025-11901)

- [Gigabyte Security Advisory](https://www.gigabyte.com/Support/Security?type=1) (CVE‑2025‑14302)

- [MSI Security Advisory](https://csr.msi.com/global/product-security-advisories) (CVE-2025-14303)

- [Asrock Security Advisory](https://www.asrock.com/support/Security.asp) (CVE-2025-14304)


These updates will ensure that when you enable security features, they're active from the very first millisecond of power-on.

## What to do if I Receive a Restriction?

Our VAN:Restriction system is Vanguard’s way of telling you we cannot guarantee system integrity due to the outlined disabled security features. This will prevent you from launching VALORANT, and you’ll see a pop up box stating what is required to enable so you continue playing. These restrictions are applied on the account or HWID level when we detect suspicious hardware behavior or statistical anomalies. These anomalies can occur due to various factors including disabled security features, or in this case, the newly discovered pre-boot loophole that invalidates IOMMU. Getting one of these warnings doesn't necessarily mean we suspect you of cheating–it means that your current system configuration is too similar to cheaters who get around security features in order to become undetectable to Vanguard.

This minimum security baseline is essential for countering cheaters. If restricted, you are prompted to either enable said features or update your motherboard firmware by following your manufacturer's official guidance before you can play. [You can learn more about restrictions in this support page.](https://support-valorant.riotgames.com/hc/en-us/articles/22291331362067-Vanguard-Restrictions)

Additionally, we're investigating rolling this requirement out to all players at the highest level of play (Ascendant and above) to ensure a trusted baseline of security for the top of the ladder.

## The Goal

BIOS updates aren't exactly as exciting as looking at ban numbers, but this is a necessary step in our arms race against hardware cheats. By closing this pre-boot loophole, we are neutralizing an entire class of previously untouchable cheats and significantly raising the cost of unfair play.

This entire ordeal is a significant achievement, not just for our anti-cheat but for the entire gaming industry, extending beyond Riot Games. Had this issue gone unnoticed, it would have completely nullified all existing DMA detection and prevention tech currently on the market–including that of other gaming companies–due to the nature of this class of cheats running in a privileged area that anti-cheats typically do not run.

If you want to get ahead of this and have an uninterrupted grind, you can proactively update your motherboard to the latest firmware by visiting the manufacturers' websites. Raising the gaming industry's overall security posture has been a core focus for us since 2021. The adoption of these stringent measures is now evident across the industry, as highlighted by articles like Xbox's " [Building a Trusted Gaming Future: How Security Powers Fair Play](https://news.xbox.com/en-us/2025/11/14/building-a-trusted-gaming-future-how-security-powers-fair-play/)." These security fundamentals are non-negotiable in the fight against cheating. We're proud to have pioneered these security advances and are committed to maintaining a fair and trusted competitive environment for everyone.

We appreciate the effort it takes to keep your system up to date. Thanks for helping keep games fair and secure for everyone. Feel free to reach out to [Riot Support](https://support.riotgames.com/) if you have any issues, and we can provide general guidance or point you to official manufacturer resources.

## Latest News

[Ranked, Modes, Agent Balance, and more // Dev Updates - VALORANT](https://www.riotgames.com/en/news/ranked-modes-agent-balance-and-more-dev-updates-valorant)

![](https://www.riotgames.com/darkroom/1000/249b36fc7e08df091d9915351fb9821f:c03e71cd64120c5567fd8c747ae7c082/v26a3-dev-diary-thumbnail-16x9-textless.jpg)

News

May 12, 2026

### Ranked, Modes, Agent Balance, and more // Dev Updates - VALORANT

The Dev team drops by for a mid-year check-in on a bunch of hot topics.

May 12, 2026

[How to Teamfight \| New Fuse - 2XKO Dev Update](https://www.riotgames.com/en/news/new-fuse-teamfight)

![](https://www.riotgames.com/darkroom/1000/3eafd246c778d3ca3fda226931e0a3b2:121c84898ec6e112ef76eda8fe1a8287/newfuse0temfight-thumnbail-textless.jpg)

News

May 7, 2026

### How to Teamfight \| New Fuse - 2XKO Dev Update

Here’s what to know about our new Fuse: Teamfight. Grab a friend and 2v2 like never before, May 12.

May 7, 2026

[League, VALORANT, and TFT Partner Programs Open Applications Coming...](https://www.riotgames.com/en/news/riot-partner-programs-open)

![](https://www.riotgames.com/darkroom/1000/081e8d7bdc457a0b5aaf85455e5bdc2d:a3c89c3d291a386cdade4266b95a7f98/rg-com-rpp-article-1920x1080.png)

News

May 1, 2026

### League, VALORANT, and TFT Partner Programs Open Applications Coming...

Creators in these three games will be able to apply starting in August.

May 1, 2026

[You Are the Race \| Wild Rift x Porsche](https://www.riotgames.com/en/news/you-are-the-race-wild-rift-x-porsche)

![](https://www.riotgames.com/darkroom/1000/6af32638fb17613b0e76236f73f471f1:9c59326b9543846153d2fb9b7025b3f5/thumbnail-2-1920x1080-textles.jpg)

News

Apr 30, 2026

### You Are the Race \| Wild Rift x Porsche

Not your opponents. Not the clock. You set your limits–and your speed. Prestige Select Neon Daredevil Kai’Sa and her custom Porsche hit the Rift April 30th UTC.

Apr 30, 2026

[Know Before You Go: 2XKO @ Evo Japan 2026](https://www.riotgames.com/en/news/know-before-you-go-2xko-evo-japan-2026)

![](https://www.riotgames.com/darkroom/1000/5fb8dabc368cb92bc27bacf5cb891484:33d0d9b33a7dbdda7d69474724175c51/evo-japan-kbyg-article-banner.jpg)

News

Apr 28, 2026

### Know Before You Go: 2XKO @ Evo Japan 2026

Evo Japan 2026 starts May 1. Here’s everything to know about 2XKO at the event.

Apr 28, 2026

## Latest News

[2XKO @ Evo Japan 2026 Trailer](https://www.riotgames.com/en/news/2xko-evo-japan-2026-trailer)

![](https://www.riotgames.com/darkroom/1000/058d6a54ed36ed66e4c5c5c56152c5da:222c8a7738d76aedf84f9fe41c7909b2/2xko-evojp-thumb-client-website.jpg)

News

Apr 28, 2026

### 2XKO @ Evo Japan 2026 Trailer

2WINz, ikoan, and poka defend their home turf at Evo Japan. Watch 2XKO’s next Major starting May 1.

Apr 28, 2026

[TFT Space Gods Set Trailer](https://www.riotgames.com/en/news/tft-space-gods-set-trailer)

![](https://www.riotgames.com/darkroom/1000/540a95692e8a62bd821d2d98789bf354:0afff1aee0fa9ceb5ccebbf33fbf2cbf/tft-s1726-thumbnail-client-setlaunchtrailer-t2-1920x1080-v001-summoner-optimized.jpg)

News

Apr 15, 2026

### TFT Space Gods Set Trailer

Ready for some divine intervention? The very power of the cosmos can be yours – worship wisely. TFT’s latest set, Space Gods, has arrived. Play now!

Apr 15, 2026

[Dev Update: Arena Updates & More](https://www.riotgames.com/en/news/dev-update-arena-updates-more)

![](https://www.riotgames.com/darkroom/1000/e7dcfbdc254a3d4b7851d427bca91221:4bae55701ff30cdc065304b6957c38c5/lol26-s2-act-i-dev-update-thumbnail-16x9-textless.jpg)

News

Apr 14, 2026

### Dev Update: Arena Updates & More

Pabro and Meddler talk about Season Two Pandemonium, gameplay changes, Arena updates, and more.

Apr 14, 2026

[The VCT in 2027 where EVERYTHING IS A TOURNAMENT?!](https://www.riotgames.com/en/news/the-vct-in-2027-where-everything-is-a-tournament)

![](https://www.riotgames.com/darkroom/1000/a7df3a875d5349946e7345f528c7a301:20012d1858df969abaa2a8c74666242a/v27-atlas-thumbnail-v02-textless.jpg)

News

Apr 8, 2026

### The VCT in 2027 where EVERYTHING IS A TOURNAMENT?!

Open Qualifiers, Cups, and Masters + Champions. It’s simple, right?

Apr 8, 2026

[Riot Named to Fortune’s 100 Best Companies to Work For list](https://www.riotgames.com/en/news/Riot-Fortune-100)

![](https://www.riotgames.com/darkroom/1000/43c0b666431a05378360f9cb7f4b33bf:9647ab784642d3bcea4e79ec0b35e7ae/fortune-100-redirect-header.png)

News

Apr 2, 2026

### Riot Named to Fortune’s 100 Best Companies to Work For list

This recognition reflects our focus on ensuring Riot the best place to make games and is thanks to the passion, creativity, and dedication Rioters bring to serve players every day.

Apr 2, 2026

[Have a Nice Deity! \| SPACE GODS Set Cinematic – Teamfight Tactics](https://www.riotgames.com/en/news/have-a-nice-deity-space-gods-set-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/85cf8baf9e324dbea3e092855a0692be:6f8150abbc14d68d0f1f16a82f6bf323/tft-s1726-thumbnail-client-setcinematic-t2-1920x1080-v001-summoner-03.jpg)

News

Mar 27, 2026

### Have a Nice Deity! \| SPACE GODS Set Cinematic – Teamfight Tactics

Enter the celestial realm of the SPACE GODS! Come forth, pay homage, and be blessed with cosmic power.

Mar 27, 2026

[NEVER FIGHT ALONE // Miks Agent Trailer - VALORANT](https://www.riotgames.com/en/news/never-fight-alone-miks-agent-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/779e37965d9a955adf8cc623ddbc7366:e512ea887749b5d9ae7853f978ab44f7/v26-act2-reveal-thumb-16x9-vb-no-text.jpg)

News

Mar 15, 2026

### NEVER FIGHT ALONE // Miks Agent Trailer - VALORANT

Available in Season 26 // ACT II

Mar 15, 2026

[First Stand 2026](https://www.riotgames.com/en/news/first-stand-2026)

![](https://www.riotgames.com/darkroom/1000/0fe30eecb35c90b66bc163c453ff8278:e265c10fa46807c52707a1c265a21884/2026-fst-hypefilm-banner-textless-v2.jpg)

News

Mar 10, 2026

### First Stand 2026

Make your moves. Change the game. Lead the charge. First Stand 2026, coming at you.

Mar 10, 2026

[March Update 2026 \| Dev Snapshot](https://www.riotgames.com/en/news/march-update-2026-dev-snapshot)

![](https://www.riotgames.com/darkroom/1000/d17903e1e207a6f5cd281b5d21def89e:2ab019b0df07d8f1edae092de9ee33d8/final-16x9-lor-7-3-devvideothumbbanner-stripped-optimized-1.jpg)

News

Mar 9, 2026

### March Update 2026 \| Dev Snapshot

Dev Snapshot dives into the latest release, With Friends Like These. The topics? A new champion, new challenges, new mechanics, and more!

Mar 9, 2026

[FST 2026 Primer](https://www.riotgames.com/en/news/fst-2026-primer)

![](https://www.riotgames.com/darkroom/1000/aa8ed3206aa3e4d4562d6ac0fe3c4995:831affe843aef0281a286612da0a23af/lol-fst26-primerarticle-header-1600x900.jpg)

News

Mar 2, 2026

### FST 2026 Primer

Everything you need to know about FST in São Paulo!

Mar 2, 2026

[Shyvana, the Half-Dragon](https://www.riotgames.com/en/news/shyvana-the-half-dragon)

![](https://www.riotgames.com/darkroom/1000/4ae2b4af9603ea6d9f4f3b3b4d91d51f:d88b1a024e866d196afd5a4160a6b121/lol-champup-shyv-trailer-thumbnail-textless-jdiaz-banner-optimized.jpg)

News

Feb 27, 2026

### Shyvana, the Half-Dragon

Do not fear the fire within. Master it.

Feb 27, 2026

[Making a Zeus-inspired VALORANT Skin](https://www.riotgames.com/en/news/making-a-zeus-inspired-valorant-skin)

![](https://www.riotgames.com/darkroom/1000/6d4f982bd55830be159afc3ac7d5cfba:e4edba69cab6aefbca6a4f524728f717/powerhour-ep12-thumbnail-rg-com-textless.png)

Inside Riot

Feb 27, 2026

### Making a Zeus-inspired VALORANT Skin

From pulling inspiration from cars to the unique white space on a rifle, Denis Lakhanov shows the process of designing a new VALORANT skin from beginning to end.

Feb 27, 2026

[¡GRITA CON EL ALMA! - VALORANT 2026 Masters Santiago Cinematic](https://www.riotgames.com/en/news/grita-con-el-alma-valorant-2026-masters-santiago-cinematic)

![](https://www.riotgames.com/darkroom/1000/17bc00f57bbf2d0806d7186cb4036e1a:602d47dd58714b4ff1cd7774d600c55b/vct26-m1-santiago-promofilm-thumbnail-16x9-v2-tl.jpg)

News

Feb 23, 2026

### ¡GRITA CON EL ALMA! - VALORANT 2026 Masters Santiago Cinematic

Tune in, turn up, and scream with the soul during VCT Masters Santiago. ¡GRITA CON EL ALMA!

Feb 23, 2026

[Masters Santiago : Everything You Need To Know](https://www.riotgames.com/en/news/masters-santiago-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/8bfc50a1385b6f7265afd60a6da14025:56f7e0c29e46946c98290fda99932f48/vct-masterssantiago-eyntk-header-1920x1080-1.jpg)

News

Feb 20, 2026

### Masters Santiago : Everything You Need To Know

Get the full rundown on the first global event of the 2026 VCT season!

Feb 20, 2026

[Masters Santiago: Know Before You Go](https://www.riotgames.com/en/news/masters-santiago-know-before-you-go)

![](https://www.riotgames.com/darkroom/1000/7f181f13cd21558d8507d159002ef69b:53d67b9b378fff4231452a01f2f7d6e0/vct-masterssantiago-knowbeforeyougo-header-1920x1080.jpg)

News

Feb 17, 2026

### Masters Santiago: Know Before You Go

Find out about all the updates, activities, and activations we’ll have at Masters Santiago!

Feb 17, 2026

[Old Friends, New Bloom \| Lunar Revel 2026 Cinematic](https://www.riotgames.com/en/news/old-friends-new-bloom-lunar-revel-2026-cinematic)

![](https://www.riotgames.com/darkroom/1000/c3112062f622cb86e3f9cff851b31dd3:ac71618833d41d2545a18324c3adac78/lol-lny26-marquee-thumbnail-textless-banner-v2.jpg)

News

Feb 4, 2026

### Old Friends, New Bloom \| Lunar Revel 2026 Cinematic

A friendship rekindled with the first bloom of spring. The Lunar Revel begins in Patch 26.03.

Feb 4, 2026

[An Update on 2XKO](https://www.riotgames.com/en/news/an-update-on-2xko)

![](https://www.riotgames.com/darkroom/1000/739ecf0aaf8d6324a3c243fe3bbf8138:6f0232f63834af8b6d04669a2f084520/small-logo-black.png)

News

Feb 9, 2026

### An Update on 2XKO

What’s Changing and Why

Feb 9, 2026

[EP3: Fortune’s Flight \| Lunar Revel 2026](https://www.riotgames.com/en/news/ep3-fortunes-flight-lunar-revel-2026)

![](https://www.riotgames.com/darkroom/1000/e07ddbf4edbe1875e44f4653b000839d:dcb629b0109e2c3c8052e925319fe3e1/tft-lny26-thumbnails-client-minicinematic-fortunesflight-t2-1920x1080-v001-summoner-1.jpg)

News

Feb 4, 2026

### EP3: Fortune’s Flight \| Lunar Revel 2026

A wild ride with the Little Legends gets Prestige Chibi Valiant Sword Riven fired up for the festival.

Feb 4, 2026

[Find Our Way \| Petals of Spring 2026 Cinematic Trailer - LoL: WR](https://www.riotgames.com/en/news/find-our-way-petals-of-spring-2026-cinematic-trailer-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/ad5648a9370911482ed842cf02480c78:82e7090241b4e4edc67f9dc284d211b3/thumbnail-wrln26-1920x1080-textles.jpg)

News

Feb 4, 2026

### Find Our Way \| Petals of Spring 2026 Cinematic Trailer - LoL: WR

Home is what you make it. And with whom.

Feb 4, 2026

[EP2: Fortune’s Blessing \| Lunar Revel 2026](https://www.riotgames.com/en/news/ep2-fortunes-blessing-lunar-revel-2026-mini-cinematic)

![](https://www.riotgames.com/darkroom/1000/0bd80e85c1909ffc487c27b20bf284ad:65125bd502955d8ea2aa412c1f91b85d/tft-lny26-thumbnails-client-minicinematic-fortunesblessing-t2-1920x1080-v001-summoner.jpg)

News

Jan 28, 2026

### EP2: Fortune’s Blessing \| Lunar Revel 2026

A surprise visitor drops by to recharge God Fist Lee Sin Unbound for the new year.

Jan 28, 2026

[2XKO Season 1 2026 Launch Trailer](https://www.riotgames.com/en/news/2xko-season-1-2026-launch-trailer)

![](https://www.riotgames.com/darkroom/1000/8dbed5af7783fb82e505abefd4117ddd:4446faf7de8416018e67a9e60e45e542/2xko-s1-launchtrailer-thumbnail-16x9-textless.jpg)

News

Jan 19, 2026

### 2XKO Season 1 2026 Launch Trailer

2XKO Season 1 hits January 20-21 with a new champion, Battle Pass, and more.

Jan 19, 2026

[EP1: Fortune’s Dance \| Lunar Revel 2026](https://www.riotgames.com/en/news/ep1-fortunes-dance-lunar-revel-2026)

![](https://www.riotgames.com/darkroom/1000/b9fc42fc0d9f511f1d13b58f20371c22:34b81cd9c789caca37b982132bc88b63/tft-lny26-thumbnails-client-minicinematic-t2-1920x1080-v001-summoner.jpg)

News

Jan 16, 2026

### EP1: Fortune’s Dance \| Lunar Revel 2026

Mythmaker Irelia Unbound launches the celebrations with a sharp performance.

Jan 16, 2026

[Patch 7.0 Preview - League of Legends: Wild Rift](https://www.riotgames.com/en/news/patch-7-0-preview)

![](https://www.riotgames.com/darkroom/1000/7db9e04a90121e64363375b072b1dbc9:155111772be492d9d22e0937fc8beaca/6appthumbnail1920x1080-textles.jpg)

News

Jan 8, 2026

### Patch 7.0 Preview - League of Legends: Wild Rift

Join Ke, John, and David in ringing in blossoming new beginnings. Be ready for a rearing Year of the Horse party with new champs on a new Rift January 22nd UTC when patch 7.0: Ionia Celebrations goes live.

Jan 8, 2026

[Dev Update: For Demacia](https://www.riotgames.com/en/news/dev-update-for-demacia)

![](https://www.riotgames.com/darkroom/1000/b8e69d4c10831b3bad51ba0c414e6b86:3cefb562d9caf9656884b14f8b2895ca/lol-s1-26-youtube-thumbnail-season-launch-dev-update-textless.jpg)

News

Jan 8, 2026

### Dev Update: For Demacia

Pabro and Meddler talk about Season One 2026: Shyvana, lobby hostage intervention, and more.

Jan 8, 2026

[Salvation (ft. Forts) \| For Demacia 2026 Season 1 Cinematic](https://www.riotgames.com/en/news/salvation-ft-forts-for-demacia-2026-season-1-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/33bf447abb7cd7f5a13c18b6a27cc598:81553d017397df2784c7a2a1fd9f79a8/lol-s1-26-banner-textless.jpg)

News

Jan 7, 2026

### Salvation (ft. Forts) \| For Demacia 2026 Season 1 Cinematic

In the darkest hours, there is always a beacon of hope. Watch the 2026 Season One Cinematic now.

Jan 7, 2026

[Riot Games Community Pact](https://www.riotgames.com/en/news/riot-games-community-pact)

![](https://www.riotgames.com/darkroom/1000/6a443f32fe585217b4976b19a486450c:8eb526e8e05c1103034821cb4e4eba20/communitypact-redirect-header.png)

News

Jan 6, 2026

### Riot Games Community Pact

TL;DR: These are our community standards across every Riot game \[League of Legends, VALORANT, Teamfight Tactics, League of Legends: Wild Rift, 2XKO, and Legends of Runeterra\]

Jan 6, 2026

[Welcome to Season 2026 // Dev Updates - VALORANT](https://www.riotgames.com/en/news/welcome-to-season-2026-dev-updates)

![](https://www.riotgames.com/darkroom/1000/32816c9efb2425025666cd43733067ce:51b38b9a88f962d78c8ed9188fbc22e3/v26-a1-soy-devdiary-welcome-to-season-2026-16x9-b-textless.jpg)

News

Jan 6, 2026

### Welcome to Season 2026 // Dev Updates - VALORANT

The stage is set for a new year of VALORANT.

Jan 6, 2026

[WHY WE FIGHT BACK // Season 2026 Cinematic Kickoff Trailer - VALORANT](https://www.riotgames.com/en/news/why-we-fight-back-season-2026-cinematic-kickoff-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/821be91bd1b6ec2136355c037a2fa3a7:33178982e5d184edf6309a05082ef3cf/v26-act1-wwfb-a-textless-1.jpg)

News

Jan 5, 2026

### WHY WE FIGHT BACK // Season 2026 Cinematic Kickoff Trailer - VALORANT

For vengeance. For survival. For everyone who can’t do it themselves. This is why we fight back.

Jan 5, 2026

[Heading Out For Winter Break 2025](https://www.riotgames.com/en/news/winter-break-2025)

![](https://www.riotgames.com/darkroom/1000/b2f15a81b66ce4c6714454eb3bc3cb20:4dc12726babbad3ba9f62c229b1a1c53/riot-eoy-break-illo-2025-16x9.png)

News

Dec 19, 2025

### Heading Out For Winter Break 2025

Don’t worry, anti-cheat, security, and live operations teams are staggering their breaks to keep everything running smoothly.

Dec 19, 2025

[TFT Paris Open Anthem](https://www.riotgames.com/en/news/danse-de-la-lumiere-tft-paris-open-2025-anthem)

![](https://www.riotgames.com/darkroom/1000/4473d86d233acde37bb5c999cf36987d:5f8138f24b17a1eac58560ad03a59a1e/tft-25-tpo-anthem-t2-thumbnail-v001-1920x1080-gisljami-optimized.jpg)

News

Dec 8, 2025

### TFT Paris Open Anthem

Let’s shine together! Join the party and tune in to the TFT Paris Open Dec 12-14.

Dec 8, 2025

[TFT Dev Drop: Lore & Legends](https://www.riotgames.com/en/news/tft-dev-drop-lore-legends)

![](https://www.riotgames.com/darkroom/1000/fe9c5eab3c37230fcefdd1e8e8efa1e0:adc46aba5951bb9b428cab978c0957e9/copy-of-tft-s1625-thumbnails-client-devdrop-t2-1920x1080-v001-summoner.jpg)

News

Nov 14, 2025

### TFT Dev Drop: Lore & Legends

Relive Runeterran history, play every region as a trait, and Unlock units with an all-new mechanic.

Nov 14, 2025

[2025 Charity Voting Campaign Winners](https://www.riotgames.com/en/news/2025-charity-voting-campaign)

![](https://www.riotgames.com/darkroom/1000/54ed69b4576c7ec529b6a63ddf7dfe6d:6fec9a1590dedd67a484c3b9fb7fffb8/2025-charityvoting-1920x1080-rg-comarticle.png)

News

Nov 13, 2025

### 2025 Charity Voting Campaign Winners

These are the 29 nonprofits who received the most votes from their region in the 2025 campaign

Nov 13, 2025

[GO OFF // 2025 VALORANT Game Changers Championship Hype Film](https://www.riotgames.com/en/news/go-off-2025-valorant-game-changers-championship-hype-film-ft-m-i-a-by-katseye)

![](https://www.riotgames.com/darkroom/1000/49faa7f039c6e5911f3174e5bb2b8e4c:506ab9df831b1b928590ea0f69f72d73/textless-under-450updated.jpg)

News

Nov 11, 2025

### GO OFF // 2025 VALORANT Game Changers Championship Hype Film

Tune in live to the VCT Game Changers Championship 2025 Nov 20-30 in Seoul, South Korea.

Nov 11, 2025

[Still Legendary I Lore & Legends Set Cinematic (ft. Mako) - TFT](https://www.riotgames.com/en/news/still-legendary-i-lore-legends-set-cinematic-mako-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/4ead3b22e9fe5de1bba05bfabb0be3f3:ae900a1bc297b3fb8e257da004c73e9d/tft-st1625-thumbnail-set-cinematic-t2-set-cinematic-l-l-1920x1080-v001-optimized.jpg)

News

Nov 9, 2025

### Still Legendary I Lore & Legends Set Cinematic (ft. Mako) - TFT

Big, little, somewhere in-between… all legends echo across the Convergence.

Nov 9, 2025

[VALORANT Game Changers Championship 2025: Everything You Need to Know](https://www.riotgames.com/en/news/valorant-game-changers-championship-2025-EYNTK)

![](https://www.riotgames.com/darkroom/1000/2a89e5cce92716d0d9af14ad18bb7b54:7fee0d932540f30078f3cb69f77ba6a6/vct25-gcc-eyntk-article-header.jpg)

News

Nov 8, 2025

### VALORANT Game Changers Championship 2025: Everything You Need to Know

Find out all the details about the 2025 Game Changers Championship! Competitors, schedules, brackets and more!

Nov 8, 2025

[Season 3 Cinematic: Twilight’s End](https://www.riotgames.com/en/news/season-3-cinematic-twilights-end-cinematic)

![](https://www.riotgames.com/darkroom/1000/a7b7f314bb8ef2082bc2bfa178cf875e:5621c4973cb066cdeb13167e8aa49e88/tot-s3-twilights-end-riotgames-homepage-background-3840x1600.png)

News

Nov 7, 2025

### Season 3 Cinematic: Twilight’s End

Centuries ago, Zaahen made a choice. Now, Xin Zhao must make his own.

Nov 7, 2025

[Find a Worlds 2025 Watch Party Near You](https://www.riotgames.com/en/news/worlds-watch-parties-2025)

![](https://www.riotgames.com/darkroom/1000/1b95a9a8323987fba31b58280e6d2b03:6b63a2a66b385b27df38df0673108784/worlds2025-find-a-worlds-watch-party-near-you.png)

News

Nov 6, 2025

### Find a Worlds 2025 Watch Party Near You

Looking for a League of Legends Worlds watch party? You’ve come to the right place.

Nov 6, 2025

[Bringing League Champions to 2XKO’s Art - Super Art Power Hour Ep. 11](https://www.riotgames.com/en/news/bringin-league-champions-to-2xko-art-super-art-power-hour-ep11)

![](https://www.riotgames.com/darkroom/1000/352f24e670adf1e6fa8f11aee9dc72d6:926d01f29b7a90f7b4f7d5c424846ebc/powerhour-ep11-textless.png)

Inside Riot

Oct 29, 2025

### Bringing League Champions to 2XKO’s Art - Super Art Power Hour Ep. 11

What would it take to bring the Noxian assassin into 2XKO’s current art style? Concept Lead Knight Zhang tackles that challenge in the latest episode of Super Art Power Hour.

Oct 29, 2025

[Celebrating Ten Years of Global Service Month at Riot Games](https://www.riotgames.com/en/news/celebrating-ten-years-of-global-service-month-at-riot-games)

![](https://www.riotgames.com/darkroom/1000/f20794e7c7ba6ee6f5145c3180b02978:e40e40e486ead43d33819b90a411b1d5/2025-gsm-thumbnail-rg-com.png)

Inside Riot

Oct 28, 2025

### Celebrating Ten Years of Global Service Month at Riot Games

10 years and counting, Global Service Month is our annual time for Rioters to make an impact in their local communities

Oct 28, 2025

[Happy 5th Anniversary](https://www.riotgames.com/en/news/happy-5th-anniversary)

![](https://www.riotgames.com/darkroom/1000/df960cf2e4dad081f4c09f85678c2032:b787cdd130ce603cbe14a6e67b81c340/thumbnail-1920x1080-textles.jpg)

News

Oct 23, 2025

### Happy 5th Anniversary

You are Wild Rift. And Wild Rift is you. Happy 5th anniversary to us.

Oct 23, 2025

[How 2XKO Handles Online Play](https://www.riotgames.com/en/news/how-2xko-handles-online-play)

![](https://www.riotgames.com/darkroom/1000/85a902385bef9d0adc0b1e9927a6520b:2086225d2fa1aa188e507e532e26361f/5c72ebce584c63e615f677b2702460dff76c7356-1920x1080.png)

Tech Blog

Oct 16, 2025

### How 2XKO Handles Online Play

Here’s all the tech we’re using to make online play just as fair, fast, and fun as offline.

Oct 16, 2025

[Dev Doodles: ARAM](https://www.riotgames.com/en/news/dev-doodles-aram)

![](https://www.riotgames.com/darkroom/1000/969b79830fb10102f7e695200d2f1213:7a558b739e07c8f854c45eeb43e03b37/dd-aram-textless-banner.jpg)

News

Oct 15, 2025

### Dev Doodles: ARAM

From player-made custom games to League’s first permanent mode, this is ARAM’s story.

Oct 15, 2025

[Worlds 2025 Anthem Music Video](https://www.riotgames.com/en/news/worlds-2025-anthem-music-video)

![](https://www.riotgames.com/darkroom/1000/bb46cc1dcb3092c580062b4ac04bd34e:9b038b7d18215e3b814351a6b3a62cdc/wmv25-textless-image-riotclient-251010-min.jpg)

News

Oct 13, 2025

### Worlds 2025 Anthem Music Video

Watch the music video for “Sacrifice” ft. G.E.M. (鄧紫棋) now!

Oct 13, 2025

[Patch 6.3 Preview - League of Legends: Wild Rift](https://www.riotgames.com/en/news/patch-6-3-preview-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/2f0304667212842bb5b8b9ebd0da039f:f8a7f6385d450740cf3c38358a1db083/1920x1080-textles.jpg)

News

Oct 10, 2025

### Patch 6.3 Preview - League of Legends: Wild Rift

Join Ke, John, and some special friends for the biggest party of the year! We’re turning 5 and celebrating with WORLDS, Wild Rounds: SMASH, and MUCH MORE. Admire your Gilded Glory with patch 6.3 on October 23rd UTC.

Oct 10, 2025

[Everything you need to know about Wild Rounds: SMASH](https://www.riotgames.com/en/news/everything-you-need-to-know-about-wild-rounds-smash)

![](https://www.riotgames.com/darkroom/1000/ef9edbd7957d5cdcf85f3bb10a83ddd5:e48117b4e4d87f93013dcd92cc9ef5c7/smash-wild-rounds-kv-1920x1080-logo.jpg)

News

Oct 10, 2025

### Everything you need to know about Wild Rounds: SMASH

The community event of the year is right around the corner! Learn all about the event, where and how you can watch, and more.

Oct 10, 2025

[Ties That Bind (2XKO Official Cinematic)](https://www.riotgames.com/en/news/ties-that-bind-2xko-official-cinematic)

![](https://www.riotgames.com/darkroom/1000/777d7ed3470b903abe079121e8e659e2:bf15511231c99bb645c5b358f7e8bd5b/2xko-cg-trailer-yt-thumbnail-16x9-v2-c-textless.jpg)

News

Oct 6, 2025

### Ties That Bind (2XKO Official Cinematic)

Some battles can’t be fought alone.

Oct 6, 2025

[EARN YOUR LEGACY \| Worlds 2025](https://www.riotgames.com/en/news/earn-your-legacy-worlds-2025)

![](https://www.riotgames.com/darkroom/1000/c6bc044f0ae74166175b7707fefad0c8:c97a5a4f58164749bb27015de18fbd0f/w25-hype-thumb-01a-info-hub-version-v2.jpg)

News

Oct 6, 2025

### EARN YOUR LEGACY \| Worlds 2025

This is your legacy – if you can earn it. Welcome to Worlds 2025.

Oct 6, 2025

[GIVE THEM NOTHING // Veto Agent Trailer - VALORANT](https://www.riotgames.com/en/news/give-them-nothing-veto-agent-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/b5b21a3bc84aba51f44eee137c237c3d:9e2c8f37d3df0f2bca60f9ee01fc6210/v26-a6-veto-thumbs-cg-textless.jpg)

News

Oct 5, 2025

### GIVE THEM NOTHING // Veto Agent Trailer - VALORANT

Your fight. His terms. This is VALORANT’s new Agent: Veto.

Oct 5, 2025

[Final Contact - The Path to Paris](https://www.riotgames.com/en/news/final-contact-the-path-to-paris)

![](https://www.riotgames.com/darkroom/1000/4746e8bc9f13b7217ba587e3f26fd6c2:16f8028bf91575866505e8422d680ff3/vct25-ch-paris-heist-cg-textlessvct25-ch-paris-heist-cg-v2.png)

News

Oct 4, 2025

### Final Contact - The Path to Paris

All eyes on Paris - where the stage is set, and the stakes are personal.

Oct 4, 2025

[Worlds 2025 Primer](https://www.riotgames.com/en/news/worlds-2025-primer)

![](https://www.riotgames.com/darkroom/1000/794fa1cba5aeaf457e663a69cff90385:08fd31d6b865f6b8679dabc31bc6ad5f/lol-w25-header-primerarticle-1600x900.jpg)

News

Sep 30, 2025

### Worlds 2025 Primer

Learn more about the 2025 League of Legends World Championship in China

Sep 30, 2025

[Find a VALORANT Champions Watch Party](https://www.riotgames.com/en/news/valorant-champions-watch-parties-2025)

![](https://www.riotgames.com/darkroom/1000/99cba306cf885d60c74a3887c45a85c1:019776b5fa352a9d056838f204d583c8/riotgames-valorant-2025-champions-watch-parties.png)

News

Sep 29, 2025

### Find a VALORANT Champions Watch Party

Catch the Champs grand finals live with other VALORANT fans IRL

Sep 29, 2025

[The First Step Towards New Partner Programs for Creators](https://www.riotgames.com/en/news/partner-program-closed-beta)

![](https://www.riotgames.com/darkroom/1000/7c4f062b3b0abb1d1107b6f0df8fc49b:2339cf65712cb8018c46faef0a71d4af/web-rb-header-final-v04-kv.png)

News

Sep 22, 2025

### The First Step Towards New Partner Programs for Creators

Partner programs are in testing with closed betas for League of Legends, Teamfight Tactics, and VALORANT

Sep 22, 2025

[Celebrating 15 Years of LoL Esports](https://www.riotgames.com/en/news/celebrating-15-years-of-lol-esports)

![](https://www.riotgames.com/darkroom/1000/7b15a59cac58254c72ae1990ad1421b0:1d6b8b6f720896be2c50a1a711133540/loe-w25-celebrating15years-thumbnail-1280x720-textless.jpg)

News

Sep 19, 2025

### Celebrating 15 Years of LoL Esports

Celebrating 15 Years of LoL Esports with new Summoner’s Cup and Worlds Anthem featuring G.E.M.

Sep 19, 2025

[Rioters Run the Set at Evo 2025](https://www.riotgames.com/en/news/rioters-run-the-set-at-evo-2025)

![](https://www.riotgames.com/darkroom/1000/3697e14aeac5273b2814655b9264ece3:3e424b159cdcf914eec022c9fbdb205f/runtheset-evo25-no-text-thumbnail-4-1.jpg)

Inside Riot

Sep 16, 2025

### Rioters Run the Set at Evo 2025

Rioters have been attending Evo for years – to compete in the massive open-bracket tournaments, cheer on their friends, or just take part in the annual celebration of the FGC.

Sep 16, 2025

[September Update 2025 \| Dev Snack Shop - LoR](https://www.riotgames.com/en/news/september-update-2025-dev-snack-shop-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/6fcab24dab8f0c5f81775a3a51bfdf45:136a75a0544d08f885175769973dee8f/final-16x9-lor-6-9-devsnackshop-textless.jpg)

News

Sep 12, 2025

### September Update 2025 \| Dev Snack Shop - LoR

Dev Snack Shop gets cheesy as LoR Devs reveal the secret sauce behind Spirit Blossom and talk about the future over some pizza.

Sep 12, 2025

[Illustrating Riftbound Card Art - Super Art Power Hour Ep. 10](https://www.riotgames.com/en/news/illustrating-riftbound-card-art-super-art-power-hour-ep-10)

![](https://www.riotgames.com/darkroom/1000/abaf9334ed69e2dcf328ad9c511dc81d:be94dc6ee1a218da78b28bd57bdc6346/powerhour-ep10-textless.jpg)

Inside Riot

Sep 8, 2025

### Illustrating Riftbound Card Art - Super Art Power Hour Ep. 10

Designing a decidedly non-canon (and purely hypothetical) 1 of 1 Yasuo card for Riftbound in the latest Super Art Hour Power Hour with Jason Chan.

Sep 8, 2025

[Roo's Lantern \| Spirit Blossom: Afterglow \| Full Cinematic - LoR](https://www.riotgames.com/en/news/roos-lantern-spirit-blossom-afterglow-full-cinematic-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/1b790e0be88d5c289647814e5c467282:59f54a6c734a1743a4e2491cc1f8ae47/final-16x9-lor-update-6-09-cinematic-1-thumbnail-textless-noborder-450kbs.png)

News

Sep 9, 2025

### Roo's Lantern \| Spirit Blossom: Afterglow \| Full Cinematic - LoR

And the legend of Roo continues ✨

Sep 9, 2025

[Last Shot ft. templuv & 347aidan (Official Music Video) // VALORANT](https://www.riotgames.com/en/news/last-shot-ft-templuv-347aidan-official-music-video-valorant-champions-2025)

![](https://www.riotgames.com/darkroom/1000/483fda802cca096cef08588cc3a8dad4:b7f2b2823cbb15b9483eccdcc0061d16/vct25-ch-mv-last-shot-a-textless.png)

News

Sep 9, 2025

### Last Shot ft. templuv & 347aidan (Official Music Video) // VALORANT

Watch the official anthem and music video for Champions 2025: “Last Shot” by templuv and 347aidan.

Sep 9, 2025

[Everything You Need to Know: Champions Paris](https://www.riotgames.com/en/news/everything-you-need-to-know-champions-paris)

![](https://www.riotgames.com/darkroom/1000/4b543df8c5a730648c94b5f94f47b23f:2cd36edfdf451d3d1d5955b6572b7355/vct25-ch-eyntk-header.jpg)

News

Sep 4, 2025

### Everything You Need to Know: Champions Paris

Tournament match-ups, format, schedule, and more!

Sep 4, 2025

[2025 Season 3 Act 1 Trailer - League of Legends](https://www.riotgames.com/en/news/LOL-2025-Season-3-Act-1-Launch-Trailer)

![](https://www.riotgames.com/darkroom/1000/1ed6232b71136b28583540a58ff38e28:c3535bc53a4ddc081c7f8c5050c2117f/tot-s3-thumbnail-acti-launch-trailer-4k-textless-banner-v2-1.png)

News

Aug 27, 2025

### 2025 Season 3 Act 1 Trailer - League of Legends

The blossoms fall… and the Black Rose rises. The Trials of Twilight begin now.

Aug 27, 2025

[Beginning of the End \| Trials of Twilight 2025 S3 Cinematic - LoL](https://www.riotgames.com/en/news/beginning-of-the-end-trials-of-twilight-2025-s3-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/7e1a2ce17a1db7541cc94e6a35767e8e:083c0093d859987513c4c4e03dd47751/tot-s3-thumbnail-cinematic-4k-textless-alt.png)

News

Aug 25, 2025

### Beginning of the End \| Trials of Twilight 2025 S3 Cinematic - LoL

A terrible nightmare… or a vision of what’s to come?

Aug 25, 2025

[TFT Pro Circuit: Everything You Need to Know](https://www.riotgames.com/en/news/tft-pro-circuit-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/1fb2640c9d952d7d6bea0aa8e6b360a8:fdcc98d7fe6c968177a00cf00af280ca/pro-circuit-16x9.png)

News

Aug 22, 2025

### TFT Pro Circuit: Everything You Need to Know

Details on event dates, format, prize pool, and more.

Aug 22, 2025

[Roo's Lantern (Pt 1) \| Spirit Blossom: First Bloom \| Cinematic Trailer](https://www.riotgames.com/en/news/roos-lantern-pt-1-spirit-blossom-first-bloom-cinematic-trailer)

![](https://www.riotgames.com/darkroom/1000/6e328e5e67d43813c86fa9f0ad78f392:b41ec8793bb84c3e2c311ee091a1c346/lor-sb-cinematic-textless-thumb-optimized.png)

News

Aug 12, 2025

### Roo's Lantern (Pt 1) \| Spirit Blossom: First Bloom \| Cinematic Trailer

A canvas of great beauty starts with a single brushstroke.

Aug 12, 2025

[How to buy Riftbound](https://www.riotgames.com/en/news/how-to-buy-riftbound)

![](https://www.riotgames.com/darkroom/1000/529ddcc851e336257281ce2527424b85:f9a3f3d42834284c76d586848ef7b02d/how-to-buy-riftbound.png)

News

Jul 30, 2025

### How to buy Riftbound

Riftbound will be released in English on October 31, 2025. Players can buy Riftbound: Origins through local game stores or the Riot Games Store.

Jul 30, 2025

[Minions, Servers, and Progression – The Tech Behind Swarm](https://www.riotgames.com/en/news/the-tech-behind-swarm)

![](https://www.riotgames.com/darkroom/1000/5e75f661fc3cbf4f7744c26d05439e8f:fb2b818527a832662d147cb2ec9c655a/techblog-the-tech-behind-swarm.png)

Tech Blog

Jul 25, 2025

### Minions, Servers, and Progression – The Tech Behind Swarm

Hundreds of minions, millions of games, plenty of technical challenges. A triple feature Riot Games Tech Blog on Swarm.

Jul 25, 2025

[Know Before You Go - 2XKO at Evo 2025](https://www.riotgames.com/en/news/know-before-you-go-2xko-at-evo-2025)

![](https://www.riotgames.com/darkroom/1000/5dc117af36c8feeab7a7a094b97157ea:530c99f4d981305aec9a191e76af2942/2xko-evo2025-article-kbyg-1920x1080.png)

News

Jul 24, 2025

### Know Before You Go - 2XKO at Evo 2025

Come find 2XKO at Evo 2025, August 1–3. Until then, get the details on the demo, events, and all the ways to experience 2XKO at Evo.

Jul 24, 2025

[2XKO Closed Beta Announcement](https://www.riotgames.com/en/news/2xko-closed-beta-announcement)

![](https://www.riotgames.com/darkroom/1000/db4ff30e17cdfe3e29c333d001b6593c:0b9743feaf8fd65aaba3525ec36f897f/2xko-article-launchannouncement-1920x1080.png)

News

Jul 22, 2025

### 2XKO Closed Beta Announcement

The 2XKO Closed Beta starts September 9, 2025. Read on for all the details.

Jul 22, 2025

[Map Design in VALORANT - Super Art Power Hour Ep. 9](https://www.riotgames.com/en/news/map-design-in-valorant-super-art-power-hour-ep-9)

![](https://www.riotgames.com/darkroom/1000/3d1bd29cc7fd632b07763a12354914dd:45c62b32f27ae321a9e164bc996f2e63/powerhour-ep9-textless.png)

Inside Riot

Jul 19, 2025

### Map Design in VALORANT - Super Art Power Hour Ep. 9

For this episode, we wrangled Diego, a Game Designer, and Michelle, a 3D Environment Artist, who both work on the VALORANT maps team. The challenge? Add a fiery twist to Abyss. To be specific, we’re talkin’ about the age-old video game staple: lava pits.

Jul 19, 2025

[Riotling Day: A Moment for the Families of Riot](https://www.riotgames.com/en/news/riotling-day-2025)

![](https://www.riotgames.com/darkroom/1000/8873fc9487c64343361fd7bc81372fba:519efd3597ca0b870a251013b9f5f4ef/riotling-day-2025-239.png)

Inside Riot

Jul 17, 2025

### Riotling Day: A Moment for the Families of Riot

We went all out for our annual bring your kid to work day at our Los Angeles HQ.

Jul 17, 2025

[TFT Dev Drop: K.O. Coliseum I Dev Video - Teamfight Tactics](https://www.riotgames.com/en/news/tft-dev-drop-k-o-coliseum-i-dev-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/9e32833360b25f3dab5a61e760aa44d5:8684fcc96660cd083955439b7d123616/tft-st1525-thumbnails-client-t2-devdrop-1920x1080-v001-summoner.png)

News

Jul 13, 2025

### TFT Dev Drop: K.O. Coliseum I Dev Video - Teamfight Tactics

Rally your squad, power up, and battle your way to Top 4 glory—starting on July 30.

Jul 13, 2025

[Fighter!! (ft. ASCA) \| K.O. Coliseum OP \| Set Cinematic - TFT](https://www.riotgames.com/en/news/fighter-ft-asca-k-o-coliseum-op-set-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/ba6b0c7ed8da605c041796e5c647964c:adb6cd4c7c112439f4a9b66eb8500289/tft-14-8-infohub-patch-15-14-s1525-marquee-1920x1080.png)

News

Jul 11, 2025

### Fighter!! (ft. ASCA) \| K.O. Coliseum OP \| Set Cinematic - TFT

It’s showtime! Assemble your dream fighting team and unleash your superpowers on July 30.

Jul 11, 2025

[Heading Out on Summer Break](https://www.riotgames.com/en/news/riot-games-summer-break-2025)

![](https://www.riotgames.com/darkroom/1000/b49648a7b223e8c201352208df75844a:6a0f7f0949e9ddd62f523212a339c395/riot-summer-break-illo-2025.png)

News

Jun 27, 2025

### Heading Out on Summer Break

Because sunburns beat burnout anyday

Jun 27, 2025

[June Update 2025 \| Dev Snack Shop - Legends of Runeterra](https://www.riotgames.com/en/news/june-update-2025-dev-snack-shop-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/3017d1a5be30a978b89a1e03a5fa6458:633288b7e603137c97e7c55c0551f196/66-dev-thumb-textless-optimized.png)

News

Jun 27, 2025

### June Update 2025 \| Dev Snack Shop - Legends of Runeterra

Ready to dine in style? Devs preview what’s to come over high tea in the newest Dev Snack Shop.

Jun 27, 2025

[Why We're Opening Betting Sponsorships in Esports & How We're Doing...](https://www.riotgames.com/en/news/esports-betting-sponsorships)

![](https://www.riotgames.com/darkroom/1000/5394de79fb23544be1835b27ef525b9d:f1d9f7bd4e10488da5e74ddab8a73d4a/riotgames-esports-sportsbetting.png)

News

Jun 26, 2025

### Why We're Opening Betting Sponsorships in Esports & How We're Doing...

The guardrails we’re putting in place to ensure competitive integrity as we open this category for esports teams

Jun 26, 2025

[Corrode // Official Map Trailer - VALORANT](https://www.riotgames.com/en/news/corrode-official-map-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/e1b90c4b1e3d654727fb61a1af4f216f:22d060756a40a212489607506c252e56/val-mp1225-010-rookrevealtrailer-1920x1080-textless-master-v001.jpg)

News

Jun 22, 2025

### Corrode // Official Map Trailer - VALORANT

New ground, classic energy. This is Corrode.

Jun 22, 2025

[MSI 2025 Primer](https://www.riotgames.com/en/news/msi-2025-primer)

![](https://www.riotgames.com/darkroom/1000/0a9eff85422d4834b2946e0738e10d07:a735d7c77074be30844deb0b79c46b51/lol-msi25-primer-header-1600x900.png)

News

Jun 17, 2025

### MSI 2025 Primer

Everything you need to know about MSI!

Jun 17, 2025

[Reflecting on Reflection - A Mel Medarda Tech Blog](https://www.riotgames.com/en/news/mel-reflect-tech-blog)

![](https://www.riotgames.com/darkroom/1000/54d151c5ee56baf4fd9c17d517e6da96:21306529965d5e216ceaffde5fa7d8d7/base-mel-final.png)

Tech Blog

Jun 16, 2025

### Reflecting on Reflection - A Mel Medarda Tech Blog

Missile hijacking, balance levers, and the tech behind Mel’s W

Jun 16, 2025

[Hall of Legends: Uzi](https://www.riotgames.com/en/news/hall-of-legends-uzi)

![](https://www.riotgames.com/darkroom/1000/478c7ab0c98d259e56a011a59a49cc66:07a1a8cd229d1f97eeda7831cbb45af3/lol-hol-uzifulldoc-thumbnail-textless-1920x1080.png)

News

Jun 11, 2025

### Hall of Legends: Uzi

Introducing the latest inductee. Watch the full film presented by Mercedes-Benz.

Jun 11, 2025

[ARAM, Inting & More \| Dev Update - League of Legends](https://www.riotgames.com/en/news/aram-inting-more-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/2d452149e33df974bb30c5b5c903b68c:dc9e28975cc3d4449af602d314258812/spiritblossom-devupdate-act2-yt-textless-final.png)

News

Jun 9, 2025

### ARAM, Inting & More \| Dev Update - League of Legends

Members of the League dev team talk about updates to ARAM, the Wyldbloom, inting, and more.

Jun 9, 2025

[Reflection // Redemption - VALORANT Masters Toronto Cinematic](https://www.riotgames.com/en/news/reflection-redemption-valorant-masters-toronto-cinematic)

![](https://www.riotgames.com/darkroom/1000/4ad7e9edce335cc168f1731963475613:21d12790f46f48cb988603f7db07fded/vct25-masters-toronto-promofilm-thumbnail-v1-tl.png)

News

Jun 5, 2025

### Reflection // Redemption - VALORANT Masters Toronto Cinematic

Tune in live to the Masters Toronto, June 7th - 22nd.

Jun 5, 2025

[Celebrate Pride with your VALORANT community.](https://www.riotgames.com/en/news/celebrate-pride-with-your-valorant-community)

![](https://www.riotgames.com/darkroom/1000/4059a98f1d2a56a2d48e29b05bd5989c:671e630da32e8d184f8b83796208464f/val-pride2025-article.png)

News

Jun 2, 2025

### Celebrate Pride with your VALORANT community.

We’re kicking off Pride month by celebrating our players and giving away in-game items.

Jun 2, 2025

[WHY WE FIGHT // EGO ft. Qing Madi // Year 5 Music Video - VALORANT](https://www.riotgames.com/en/news/why-we-fight-ego-ft-qing-madi-year-5-music-video-valorant)

![](https://www.riotgames.com/darkroom/1000/130fd0cf8d5b4325e891ba1c31e5b7a9:a6aad7e4e613ed8c1acf749a8ec4dde2/v25-ego-mv-thumbnail-a.png)

News

Jun 2, 2025

### WHY WE FIGHT // EGO ft. Qing Madi // Year 5 Music Video - VALORANT

For love. For family. For redemption. For remembrance. For life. This is why we fight.

Jun 2, 2025

[Abilities, Smurfing, Replays, and More // Dev Updates - VALORANT](https://www.riotgames.com/en/news/abilities-smurfing-replays-and-more-dev-updates-valorant)

![](https://www.riotgames.com/darkroom/1000/5ecc4ae56a49c6bc921512aa104dd5d7:43b5b9cf23a52c8a72ba32e3064145fa/v25-v5-dev-diaries-textless.jpg)

News

May 30, 2025

### Abilities, Smurfing, Replays, and More // Dev Updates - VALORANT

Learn more about how the team is approaching core topics like ability overload, smurfing, and more.

May 30, 2025

[Celebrating Pride In Game and Out](https://www.riotgames.com/en/news/celebrating-pride-2025)

![](https://www.riotgames.com/darkroom/1000/f7bb98ec59615ae708efce22603777db:f6070ccbe6e2b0c970285610406833bc/2025-pride-article-header-2up.png)

News

May 30, 2025

### Celebrating Pride In Game and Out

From emotes in our games to donations for the future of LGBTQ+ representation in tech, here’s how we’re celebrating Pride month.

May 30, 2025

[Making Splash Art in TFT - Super Art Power Hour Ep. 8](https://www.riotgames.com/en/news/making-splash-art-in-tft-super-art-power-hour-ep-8)

![](https://www.riotgames.com/darkroom/1000/67c1a757449f05a12607a68d3f6074fb:78b7bd831a4ab7be2994a1a769d99915/saph-ep8-thumbnail-textless.png)

Inside Riot

May 30, 2025

### Making Splash Art in TFT - Super Art Power Hour Ep. 8

In this episode of Super Art Power Hour we gave Valentine, a concept artist on Teamfight Tactics, a challenge. Her task: pick a Champ that doesn’t currently exist in TFT’s Set 14 Cyber City, then recreate them in the style of one of the set’s existing traits, all in under an hour.

May 30, 2025

[Masters Toronto: Everything You Need To Know](https://www.riotgames.com/en/news/masters-toronto-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/59a1205dd98ab9da19d0ed6d8fb431ef:89716291a595d200709b9c5902a5bde4/vct25-masters-toronto-eyntk-header.png)

News

May 28, 2025

### Masters Toronto: Everything You Need To Know

Get the full rundown on the second global event of the 2025 VCT season!

May 28, 2025

[TFT 6-Year Bash](https://www.riotgames.com/en/news/tft-6yr-bash)

![](https://www.riotgames.com/darkroom/1000/ecf949458af347f0352fd725ee78db35:ccf78ab239263622e5f9505fe73d208f/pngu25-gameplay-article-banner-1920x1080-gentz.png)

News

May 26, 2025

### TFT 6-Year Bash

It’s Pengu’s Party and you’re invited.

May 26, 2025

[Hall of Legends: Uzi \| Trailer](https://www.riotgames.com/en/news/hall-of-legends-uzi-trailer)

![](https://www.riotgames.com/darkroom/1000/8b2c2323c6478c8615108744418a9e38:3d6e68e0c59e0b240368b0492ef82399/riot-client-lol-hol25-uzitrailer-thumbnail-textless-1920x1080.png)

News

May 20, 2025

### Hall of Legends: Uzi \| Trailer

Introducing the latest inductee. Watch the full film presented by Mercedes-Benz on June 11.

May 20, 2025

[Celebrate Pride 2025: League and TFT](https://www.riotgames.com/en/news/celebrate-pride-2025-league-and-tft)

![](https://www.riotgames.com/darkroom/1000/89817517ab5dfa2ffd806a58334b6819:55666718baa4b284b72c847d699e22a2/pride-splash-and-banner-asset.png)

News

May 13, 2025

### Celebrate Pride 2025: League and TFT

Celebrate Pride and the community with us!

May 13, 2025

[LoL and VALORANT Esports Partnering with Coinbase](https://www.riotgames.com/en/news/lol-valorant-esports-coinbase-partnership)

![](https://www.riotgames.com/darkroom/1000/44678b8e003f8188b360c74864eaef8c:053548ed42a3e078597539e006ff1531/val-lol-coinbase-articleheader.png)

News

May 6, 2025

### LoL and VALORANT Esports Partnering with Coinbase

This multi-year deal will kick off during VCT Masters Toronto

May 6, 2025

[5 Years of VALORANT // Act 3 Kickoff Trailer](https://www.riotgames.com/en/news/5-years-of-valorant-act-3-kickoff-trailer)

![](https://www.riotgames.com/darkroom/1000/7f60af46f41b03e9184674b433246825:50c786a9eaa88fd1ddef041bcc54927d/val-act3launchtrailer-thumbnail-16x9-textless.jpg)

News

Apr 30, 2025

### 5 Years of VALORANT // Act 3 Kickoff Trailer

Kicking off VALORANT’s Five Year Anniversary celebration with the launch of Act 3.

Apr 30, 2025

[Celebrating 5 Years of VALORANT](https://www.riotgames.com/en/news/celebrating-5-years-of-valorant)

![](https://www.riotgames.com/darkroom/1000/7468810a2efebfe35d57a1029b485e61:2fa6acfcd3ac132ec29cf713eca19d50/anniversary-comms-v1-tl.jpg)

News

Apr 29, 2025

### Celebrating 5 Years of VALORANT

Anna Donlon reflects on VALORANT’s journey to 5 years and invites you to join us for many more.

Apr 29, 2025

[Now Available – Multi-Factor Authentication on Riot Mobile](https://www.riotgames.com/en/news/now-available-multi-factor-authentication-on-riot-mobile)

![](https://www.riotgames.com/darkroom/1000/c6b7ce1dc5ed70231ee55a998121fe50:4fdd0431779dc305259ef921f1be1a7c/mfa.png)

News

Apr 29, 2025

### Now Available – Multi-Factor Authentication on Riot Mobile

Streamline your ability to login while keeping your account secure with Riot Mobile

Apr 29, 2025

[Get Rewarded for Enabling Multi-Factor Authentication (MFA)](https://www.riotgames.com/en/news/get-rewarded-for-enabling-2fa)

![](https://www.riotgames.com/darkroom/1000/e33ccab40a757ae55451c648f1b1c2c5:3b1e78523c3bc43f77dba5ffc7b4f6b4/all-riot-games-reward-graphic-1920x1080-tftupdate2025.png)

News

Apr 29, 2025

### Get Rewarded for Enabling Multi-Factor Authentication (MFA)

Keep your account secure and receive unique rewards in the process.

Apr 29, 2025

[Hoby Darling joins Riot Games as its next President](https://www.riotgames.com/en/news/hoby-darling-joins-riot-games-as-its-next-president)

![](https://www.riotgames.com/darkroom/1000/7ff5c785cc53c84d45e39cdf57f27399:94ffcaf4a45343695959c5133e104d88/riotgames-logo-header-1.png)

News

Apr 21, 2025

### Hoby Darling joins Riot Games as its next President

As President Hoby will oversee teams at Riot working to make it the best place to make Games.

Apr 21, 2025

[Look Up! \| Ryze x Faker Trailer - League of Legends: Wild Rift](https://www.riotgames.com/en/news/look-up-ryze-x-faker-trailer-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/8aa55fb15f36ab6b8ce13d53e1b3f351:81cc6e70d4b664d0ba54284a4651c2c2/copy-of-1920x1080-textles.png)

News

Apr 17, 2025

### Look Up! \| Ryze x Faker Trailer - League of Legends: Wild Rift

Everybody’s star looks different. Join #Ryze and #Faker on their journey and fight for your own!

Apr 17, 2025

[Spirit Blossom, Brawl & More \| Dev Update - League of Legends](https://www.riotgames.com/en/news/spirit-blossom-brawl-more-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/29239ce33fc5fcea58b5d05453546ea1:00ac43c22607960756b1e06659d1a59f/spiritblossom-devupdate-yt-textless-final-banner-v2.png)

News

Apr 15, 2025

### Spirit Blossom, Brawl & More \| Dev Update - League of Legends

A look at what we have planned for Season 2: new mode, gameplay changes, inting detection, and more.

Apr 15, 2025

[Here, Tomorrow (ft Lilas, Kevin Penkin) - 2025 Season 2 Cinematic...](https://www.riotgames.com/en/news/here-tomorrow-ft-lilas-kevin-penkin-2025-season-2-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/97aa601e55273e350cf5265008b093b8:69677691b3b4ccef29d9f1c338beb1e2/2025-season-2-marquee-thumbnail-textless-v1.png)

News

Apr 14, 2025

### Here, Tomorrow (ft Lilas, Kevin Penkin) - 2025 Season 2 Cinematic...

Around every light, a shadow. Beyond every ending, a beginning.

Apr 14, 2025

[A Dark Gambit \| 2025 Season 1 Cinematic - League of Legends](https://www.riotgames.com/en/news/a-dark-gambit-2025-season-1-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/4c686362226d71871676ed631097bb8a:a875f5f04d600fe87212820dfa484947/lol-darkgambit-thumbnail.png)

News

Apr 10, 2025

### A Dark Gambit \| 2025 Season 1 Cinematic - League of Legends

A single whisper can cut deeper than a thousand blades.

Apr 10, 2025

[Building Great Experiences for Players: A Never Ending Quest](https://www.riotgames.com/en/news/building-great-experiences-for-players-a-never-ending-quest-riot-games-marc-merrill-dice-2025)

![](https://www.riotgames.com/darkroom/1000/cd8edcdc4e9db9759671a27cc1d28450:9cd0858fafae6fa35742bcfed367b4a3/dice-thumbnail-2.png)

Inside Riot

Apr 11, 2025

### Building Great Experiences for Players: A Never Ending Quest

In this #DICE2025 talk, Marc Merrill (Co-founder and Chief Product Officer, Riot Games) shares how Riot prioritizes taking the time necessary to create games and experiences right for players, how this player-first philosophy shapes every decision Riot makes, and why this simple–yet ultimately complex–approach makes game development a meaningful career path.

Apr 11, 2025

[Introducing Riftbound - The League of Legends Trading Card Game](https://www.riotgames.com/en/news/introducing-riftbound-the-league-of-legends-trading-card-game)

![](https://www.riotgames.com/darkroom/1000/8f805df8cd9109bea5f33d425debca29:cad23cccefab21bc85c93035a86fbaa4/riftbound-header-logo.png)

News

Mar 31, 2025

### Introducing Riftbound - The League of Legends Trading Card Game

Bring your favorite characters off the screen and into your hands with Riftbound, a TCG inspired by League of Legends.

Mar 31, 2025

[TFT Dev Drop: Cyber City \| Dev Video - Teamfight Tactics](https://www.riotgames.com/en/news/tft-dev-drop-cyber-city-dev-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/016cdf055dab043d9e1b21dbbd5f05f1:7078ca48947d77f2f445a47094f09a8a/tft-s1425-thumbnails-client-t2-devdrop-1920x1080-v002-summoner.png)

News

Mar 17, 2025

### TFT Dev Drop: Cyber City \| Dev Video - Teamfight Tactics

Build your crew, expand your turf, and hack your way to the top of Cyber City. Are you willing to do whatever it takes?

Mar 17, 2025

[/Dev TFT: Into the Arcane Learnings](https://www.riotgames.com/en/news/dev-tft-into-the-arcane-learnings)

![](https://www.riotgames.com/darkroom/1000/470841427e47e0206bdf9db83a1cc1e2:d8956ae049fc9a080b8dcf6031b04b9a/tft-st1324-art-kv-phase2-v002-1920x1080.png)

Inside Riot

Mar 17, 2025

### /Dev TFT: Into the Arcane Learnings

Reflecting on Into the Arcane (and more) with Riot Mort, Riot Xtna, and a few other teammates.

Mar 17, 2025

[March Update 2025 \| Dev Snack Shop - Legends of Runeterra](https://www.riotgames.com/en/news/march-update-2025-dev-snack-shop-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/8e6f0575c221fac40ddcd0130a0b7f5f:435a6d2ccdb1f01b40f6798e2ad8c24a/6-3devvid-thumb-textless.png)

News

Mar 14, 2025

### March Update 2025 \| Dev Snack Shop - Legends of Runeterra

Feeling peckish? Dev Snack Shop returns as LoR devs preview 2025 & more.

Mar 14, 2025

[High Speed Heist \| Cyber City Set Cinematic - Teamfight Tactics](https://www.riotgames.com/en/news/high-speed-heist-cyber-city-set-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/370ae98f702f6d516cb036b1158c0c61:bb0c1d2a72947b529b7812415702345a/tft-st1425-marquee-thumb-select-notext.png)

News

Mar 14, 2025

### High Speed Heist \| Cyber City Set Cinematic - Teamfight Tactics

Catch the train to Cyber City on April 2 – and be sure to bring your crew with you.

Mar 14, 2025

[DAWN BREAKS // Waylay Agent Trailer - VALORANT](https://www.riotgames.com/en/news/dawn-breaks-waylay-agent-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/fe4c1554fd2b10fcda7a34255b88f40e:8282438a18b2ef650d52c36fe6a2ab32/v25-waylay-thumbnail-b-16x9-v6.png)

News

Mar 2, 2025

### DAWN BREAKS // Waylay Agent Trailer - VALORANT

Bend light to break your foes with Waylay – VALORANT’s new Thai Duelist Agent. Available in S25: ACT II.

Mar 2, 2025

[FULL FEARLESS DRAFT UNLEASHED! First Stand Tournament 2025](https://www.riotgames.com/en/news/full-fearless-draft-unleashed-first-stand-tournament-2025)

![](https://www.riotgames.com/darkroom/1000/bbf2e756b37778eebb31d30a74682e89:24d64373d110dc3e3d33962c49a52366/fst25-hypefilm-thumb.png)

News

Feb 28, 2025

### FULL FEARLESS DRAFT UNLEASHED! First Stand Tournament 2025

With League of Legends Esports’ first split complete, the best teams from the LCK, LPL, LTA, LCP, and LEC will converge in Seoul, South Korea, on March 10th to battle it out at the First Stand Tournament.

Feb 28, 2025

[Riot and UVS to bring Project K to global players](https://www.riotgames.com/en/news/project-k-global-release)

![](https://www.riotgames.com/darkroom/1000/38fa15cf26730a31b1cb9eeade6b0ec3:f21dc5e864b5794433826eef22cf9d97/projectk-uvs-logo-lock-up-articleheader.png)

News

Feb 24, 2025

### Riot and UVS to bring Project K to global players

We are working with UVS Games to bring Project K, our League of Legends physical trading card came, to players around the world

Feb 24, 2025

[FST 2025 Primer](https://www.riotgames.com/en/news/fst-2025-primer)

![](https://www.riotgames.com/darkroom/1000/551b68e6f9ccc2a05212bb9d31012653:a76cb9d623bbe2b7bbadbf20beb706d0/fst25-primer-header-1600x900.jpg)

News

Feb 24, 2025

### FST 2025 Primer

Everything you need to know about the newest LoL Esports international event.

Feb 24, 2025

[Dawn of the Duelist // VALORANT Masters Bangkok Cinematic](https://www.riotgames.com/en/news/dawn-of-the-duelist-valorant-masters-bangkok-cinematic)

![](https://www.riotgames.com/darkroom/1000/1aa382c51adf18ac506b986ac28fd6ab:e18968893f6176d8080dcca8c5ccb3ca/v25-m1-thumbnail-16x9-textless.png)

News

Feb 13, 2025

### Dawn of the Duelist // VALORANT Masters Bangkok Cinematic

In the heart of Thailand, where pros turn into masters, this is the Dawn of the Duelist. The top 8 teams from around the world will be stepping up to the international stage and battle it out to prove they have what it takes to become number one.

Feb 13, 2025

[Masters Bangkok: EYNTK](https://www.riotgames.com/en/news/masters-bangkok-eyntk)

![](https://www.riotgames.com/darkroom/1000/c243efee63e8cc1c4c2d6fe1bb3e1259:5c89e09c5018043ceee69e8d491a39ce/vct25-m1-eyntk-header.png)

News

Feb 11, 2025

### Masters Bangkok: EYNTK

Find out the details of the first global event of the 2025 VCT Season!

Feb 11, 2025

[Why We’re Returning to the Esports World Cup with League, TFT, and VAL](https://www.riotgames.com/en/news/ewc-2025)

![](https://www.riotgames.com/darkroom/1000/3e950a5d879b887192da613ff6c2ad6e:5a3982831f15a60727566670e85ff2fa/ewc-header.png)

News

Feb 10, 2025

### Why We’re Returning to the Esports World Cup with League, TFT, and VAL

League, TFT, and VALORANT are heading to EWC. As part of this agreement, you’ll also see limited ads during global broadcasts.

Feb 10, 2025

[Celebrating the Year of the Snake](https://www.riotgames.com/en/news/year-snake-league-valorant)

![](https://www.riotgames.com/darkroom/1000/d497d632c535436f69a9963e3c2ebfdd:13bdd9006802f8e67ca97f5e28aef685/cb-mythmaker-no-vignette.png)

News

Jan 29, 2025

### Celebrating the Year of the Snake

League, Wild Rift, TFT, and VALORANT are all celebrating the new year in their own way

Jan 29, 2025

[Dev Double Up: Making Unbound and Chibi Tacticians](https://www.riotgames.com/en/news/dev-double-up-making-unbound-chibi-tacticians)

![](https://www.riotgames.com/darkroom/1000/8dd7481bac12fb1688f6539994e92c97:0efa200784faf4f11f8edbc32f7e8a5f/tft-lny25-thumb-devdoubleup-1920x1080-textless.png)

News

Jan 29, 2025

### Dev Double Up: Making Unbound and Chibi Tacticians

TFT devs in Singapore team up to show how they make Tacticians and celebrate the Year of the Snake.

Jan 29, 2025

[The Bridge Between \| Lunar Revel 2025 Cinematic - League of Legends](https://www.riotgames.com/en/news/the-bridge-between-lunar-revel-2025-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/f8e39a0468851a837c08d4ce7392974a:eeb35ed1d11075740497c8ed2659f740/16-9-option-2.png)

News

Jan 21, 2025

### The Bridge Between \| Lunar Revel 2025 Cinematic - League of Legends

The year of the snake begins with a bang. Mythmaker Cassiopeia, Jarvan IV, and Nami guide their spirit companions home—but Mythmaker Jhin has his own plan for the festivities.

Jan 21, 2025

[Update on the Southern California Wildfires and Our Fundraiser for...](https://www.riotgames.com/en/news/la-wildfire-fundraiser)

![](https://www.riotgames.com/darkroom/1000/b1636c932b6a2962c02a5d1a4f854f14:0c328238a76de11d760dda899284db2c/2025-la-wildfire-relief-article.png)

News

Jan 16, 2025

### Update on the Southern California Wildfires and Our Fundraiser for...

We’re launching in-game fundraisers in League of Legends PC and VALORANT

Jan 16, 2025

[Riot Tech Blog: Improving Performance by Streamlining League’s...](https://www.riotgames.com/en/news/tech-blog-cpu-usage)

![](https://www.riotgames.com/darkroom/1000/d912f44d4d68459ae1ff6993ace21252:85578e09c488e665e1674786a273f112/anima-squad-yuumi-final-1.png)

Tech Blog

Jan 16, 2025

### Riot Tech Blog: Improving Performance by Streamlining League’s...

A technical deep dive into changes we made to new game server selection to maximize performance for players

Jan 16, 2025

[Fortune Favors the Hungry \| Mini Cinematic - Teamfight Tactics](https://www.riotgames.com/en/news/fortune-favors-the-hungry-mini-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/cd536c7868cc2efde8efd59f99f78251:b2dce6bbb479beeb30618a11da7b81c7/tft-lny25-mini-cine-thumbnail-1920x1080-t2-textless-generic-optimized.png)

News

Jan 9, 2025

### Fortune Favors the Hungry \| Mini Cinematic - Teamfight Tactics

Chibi Mythmaker Zoe sends Chibi Prestige Porcelain Ezreal on a delicious quest during the new year.

Jan 9, 2025

[Welcome to Noxus \| Dev Update - League of Legends](https://www.riotgames.com/en/news/welcome-to-noxus-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/cf0dd6c00e48cf810615ea250d9df7cb:853778a7cf7869efaacaa2bb4d659dfa/ih-ml4-dev-update-textless.png)

News

Jan 8, 2025

### Welcome to Noxus \| Dev Update - League of Legends

Pabro and Meddler are joined by other members of the League dev team to talk about everything in Season One 2025: a new champion, a champion update, modes, skins, esports, and more. Welcome to Season One. Welcome to Noxus.

Jan 8, 2025

[Welcome to Noxus - Bite Marks (ft. TEYA) \| 2025 Season 1 Cinematic...](https://www.riotgames.com/en/news/welcome-to-noxus-bite-marks-ft-teya-2025-season-1-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/b298931a7d65aa869910215f57b4b937:22dc25f6d4930d2bd76d2f99edef5da1/thumbnail-wtn-01-16-9-textless.png)

News

Jan 7, 2025

### Welcome to Noxus - Bite Marks (ft. TEYA) \| 2025 Season 1 Cinematic...

There is only one question to answer. Are you strong enough?

Jan 7, 2025

[What’s New for Season 2025 // Dev Updates - VALORANT](https://www.riotgames.com/en/news/whats-new-for-season-2025-dev-updates-valorant)

![](https://www.riotgames.com/darkroom/1000/84f7f3eb2d0ff9f7c4f7b6e84ab973f2:1477363ab6eb94878c1a2f883ada78e8/hot-topics-16x9.png)

News

Jan 7, 2025

### What’s New for Season 2025 // Dev Updates - VALORANT

Andy and the dev team are here to welcome you to a new year of VALORANT—VAL’s 5th year! Watch to find out what the devs are cooking up in their areas of the game for the start of 2025 and beyond.

Jan 7, 2025

[Welcome to Season 2025 - VALORANT](https://www.riotgames.com/en/news/welcome-to-season-2025-valorant)

![](https://www.riotgames.com/darkroom/1000/bbf6272aaf465aa4fd9b77f3d81c2091:bcd30ed6ee6ed92a738c14d362a08333/soty-16x9.png)

News

Jan 7, 2025

### Welcome to Season 2025 - VALORANT

Studio Head, Anna Donlon, stops by to share some feels as VALORANT heads into its 5th year—a time for reflection, evolution, and dedication to all of you. Welcome to Season 2025. GL HF.

Jan 7, 2025

[INITIATE.V25 // Season 2025 Kickoff Trailer - VALORANT](https://www.riotgames.com/en/news/initiate-v25-season-2025-kickoff-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/fa3169b66c0b2876dc233b97c88cff01:17925bf9c826a577aa9fc26949cf1323/v25-tejo-cinematic-thumbnail-textless-16x9-v1.png)

News

Jan 6, 2025

### INITIATE.V25 // Season 2025 Kickoff Trailer - VALORANT

Ready the squad as we kick off Season 2025: a year of new Agents, new locations, new stories, and plenty of new ways to flex.

Jan 6, 2025

[It’s Time for our End of Year Break](https://www.riotgames.com/en/news/end-of-year-break-2024)

![](https://www.riotgames.com/darkroom/1000/bb05769ea980ce361c477e4c80c7b3c1:41992d843c507e4e2d2ef53571c94e22/riot-eoy-break-illo-2024.png)

News

Dec 19, 2024

### It’s Time for our End of Year Break

We’re taking two weeks off for the holidays, see you in January!

Dec 19, 2024

[Temporary Changes to English Skin VO During the SAG-AFTRA Strike](https://www.riotgames.com/en/news/temporary-changes-to-english-skin-vo-during-the-sag-aftra-strike)

![](https://www.riotgames.com/darkroom/1000/f4a2bb9a88698dbb2dc20e69ddad3216:049c8971b1d51859b896412e3ba4d98e/lol-wr-sagaftra.png)

News

Dec 16, 2024

### Temporary Changes to English Skin VO During the SAG-AFTRA Strike

For English-language League PC and some Wild Rift skins, we’ll be temporarily using existing (base) VO.

Dec 16, 2024

[December Update 2024 \| Dev Snack Shop - Legends of Runeterra](https://www.riotgames.com/en/news/december-update-2024-dev-snack-shop-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/ec494ae0ef4f2c24b56a5eca62bf0c44:8d5acd9401c69c8016d81bf40739ffa8/textless.jpeg)

News

Dec 13, 2024

### December Update 2024 \| Dev Snack Shop - Legends of Runeterra

Snack, crackle, shop! LoR Devs reflect on 2024 & more.

Dec 13, 2024

[TFT Macao Open: Everything You Need to Know](https://www.riotgames.com/en/news/tft-macao-open-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/70998652116e4cf49f1fb4a002502c66:b64e8d26a63bd725f13ca468389376e2/tft-tmo-headerimage-png.png)

News

Dec 6, 2024

### TFT Macao Open: Everything You Need to Know

Broadcast Schedule, Drops, Event Format, and more!

Dec 6, 2024

[Introducing Project K: The League of Legends Trading Card Game](https://www.riotgames.com/en/news/introducing-project-k-the-league-of-legends-trading-card-game)

![](https://www.riotgames.com/darkroom/1000/7079ba205b65796c4421d672a18bea14:3c0d429370ce4b90aaa9e993af1af88c/lol-kylinannouncement-thumbnail.png)

News

Dec 6, 2024

### Introducing Project K: The League of Legends Trading Card Game

Meet Project K: the immersive League of Legends TCG we’re designing for everyone who loves card games, collecting, and connecting with friends across the table.

Dec 6, 2024

[Season 1 Reveal, Gameplay Preview & Ranked Resets \| Dev Update - LoL](https://www.riotgames.com/en/news/season-1-reveal-gameplay-preview-ranked-resets-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/68f0e86442cc390db832ceefdfed5ac4:4b85edd57f0354c0a3c9e6c4baca356c/20241125-lol-devupdate-banner-textless.png)

News

Nov 25, 2024

### Season 1 Reveal, Gameplay Preview & Ranked Resets \| Dev Update - LoL

Pabro, Meddler, and other devs share the Season One theme, gameplay changes for 2025, Ranked resets, and more.

Nov 25, 2024

[TFT Dev Drop: 6-Costs Enter Into the Arcane I Dev Video - TFT](https://www.riotgames.com/en/news/tft-dev-drop-6-costs-enter-into-the-arcane-i-dev-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/22fa6fe87a93af26ec1b48793b2a0ad9:d1b9edde768328548812702574d2b076/tft-dev-drop-6-costs-1920x1080.png)

News

Nov 25, 2024

### TFT Dev Drop: 6-Costs Enter Into the Arcane I Dev Video - TFT

If you felt like someone was missing from Into the Arcane, that’s because they were. Three ultra-powerful 6-costs are about to enter the Convergence. And just like they did in Arcane, they’re completely changing the game.

Nov 25, 2024

[Nightmare on Reroll Street: Part 2 \| Into the Arcane Launch Cinematic](https://www.riotgames.com/en/news/nightmare-on-reroll-street-part-2-into-the-arcane-launch-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/49fe28da9ad0ea849dbc10eed45a310b:6ec4d2ff94033ee29da180434451a127/league-hub-tft-st1324-stills-cinematicpt2-1920x1080-v001-summoner.png)

News

Nov 25, 2024

### Nightmare on Reroll Street: Part 2 \| Into the Arcane Launch Cinematic

With family by your side, even the impossible can seem possible. Play TFT’s latest set, Into the Arcane, and reimagine the fates of your favorite characters from the show. Patch 14.24 brings new champions and more on Dec 11!

Nov 25, 2024

[Arcane Season 2 Across Our Games](https://www.riotgames.com/en/news/arcane-season-2-in-game-activations)

![](https://www.riotgames.com/darkroom/1000/8e00f1bfd6fa471c99ec6cd08c145a72:55b1797adbef7075f0438db960e163c6/base-domina-final.png)

News

Nov 22, 2024

### Arcane Season 2 Across Our Games

What League, TFT, VALORANT, Wild Rift, and Legends of Runeterra are doing to celebrate Arcane’s final season.

Nov 22, 2024

[Why We Rumble: Inside our Annual Riot vs. Riot Tournaments](https://www.riotgames.com/en/news/why-we-rumble-riot-tournaments)

![](https://www.riotgames.com/darkroom/1000/4d853275006bf27e4144618bae4d28b0:935aedf0dded74f7d11921789f8294e8/riot-rumble-2023-winners-websized-70.png)

Inside Riot

Nov 19, 2024

### Why We Rumble: Inside our Annual Riot vs. Riot Tournaments

We’ve been competing against each other for over a decade. Here’s why we do it.

Nov 19, 2024

[Nightmare on Reroll Street: Part 1 \| Into the Arcane Launch Cinematic](https://www.riotgames.com/en/news/nightmare-on-reroll-street-part-1-into-the-arcane-launch-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/ff62c7b2522ac04250bb204ba9dd856b:19ed503252078509233d6fb43e0dcb8e/tft-st1324-thumb-marquee-pt-1-1920x1080-textless-optimized.png)

News

Nov 18, 2024

### Nightmare on Reroll Street: Part 1 \| Into the Arcane Launch Cinematic

TFT’s latest set goes Into the Arcane on Nov 20 with exclusive playable characters from the hit Netflix show. Stay tuned to see what happens in Part 2, coming soon!

Nov 18, 2024

[Becoming Unbound \| Mini Cinematic - Teamfight Tactics](https://www.riotgames.com/en/news/becoming-unbound-mini-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/2c25df1fda250823305f4ae2432e1f8d:419ead08d1763e065377bca237c0cc19/tft-unboundjinx-minicine-thumb.png)

News

Nov 12, 2024

### Becoming Unbound \| Mini Cinematic - Teamfight Tactics

Meet our newest Tactician type in patch 14.23: Unbound Champions!

Nov 12, 2024

[Jinx Gameplay Sneak Peek \| 2XKO](https://www.riotgames.com/en/news/jinx-gameplay-sneak-peek-2xko)

![](https://www.riotgames.com/darkroom/1000/7a1b6e0363675534a77dfb90e484b92b:1bc9eb526bf43291aee569d4e4e0f6eb/2xko-arc2-jinxtrailer-yt-thumbnail16x9.png)

News

Nov 11, 2024

### Jinx Gameplay Sneak Peek \| 2XKO

Jinx joins 2XKO with an updated arsenal of deadly weapons, traps, and explosions.

Nov 11, 2024

[TFT Dev Drop: Into the Arcane](https://www.riotgames.com/en/news/tft-dev-drop-into-the-arcane)

![](https://www.riotgames.com/darkroom/1000/dae4443d23e7e5842f72c961042b218f:ac2702d78e137fc8ed53bfe1cddd832f/tft-st1324-cc-devdrop-1-t2-notext-1920x1080-v001-optimized.png)

Inside Riot

Nov 10, 2024

### TFT Dev Drop: Into the Arcane

Arcane is in full force in the Convergence.

Nov 10, 2024

[TFT Goes Into the Arcane](https://www.riotgames.com/en/news/tft-goes-into-the-arcane)

![](https://www.riotgames.com/darkroom/1000/d760dd6b2af471d0c2e3a343330c1cc0:7a62e021d197531da01a16a35787fdff/tft-goes-into-arcane-banner.png)

News

Nov 10, 2024

### TFT Goes Into the Arcane

Check out how Teamfight Tactics lets you relive, rewrite, and replay the events of Arcane.

Nov 10, 2024

[League of Legends x Arcane Season 2](https://www.riotgames.com/en/news/league-of-legends-x-arcane-season-2)

![](https://www.riotgames.com/darkroom/1000/3fe502769f4ddb50eb2a757a7e83491e:8e8a9aa6d3996ecc43edfbb03e5d3bda/header-web.png)

News

Nov 9, 2024

### League of Legends x Arcane Season 2

How we’re celebrating Arcane Season 2 in League.

Nov 9, 2024

[Champion Insights: Ambessa](https://www.riotgames.com/en/news/champion-insights-ambessa)

![](https://www.riotgames.com/darkroom/1000/4cd9e5e44dd6c046fc3c23f8eddd0431:3e4742e3b77815ce9c4b755e26395b21/lol-thumbnails-championinsights-ambessa-16x9-textless.jpg)

Inside Riot

Nov 8, 2024

### Champion Insights: Ambessa

How our favorite mother came to dominate the RIft.

Nov 8, 2024

[VALORANT x Arcane: The Season 2 Arcane Collector’s Set is coming](https://www.riotgames.com/en/news/valorant-x-arcane-the-season-2-arcane-collectors-set-is-coming)

![](https://www.riotgames.com/darkroom/1000/9eb17a928a04b16d9cf87bda1eb8f923:48e4991312432910c9ddf84b1478f838/header-image.jpg)

News

Nov 4, 2024

### VALORANT x Arcane: The Season 2 Arcane Collector’s Set is coming

Everything you need to know about the Limited Edition bundle.

Nov 4, 2024

[/Dev TFT: Magic n' Mayhem Learnings](https://www.riotgames.com/en/news/dev-tft-magic-n-mayhem-learnings)

![](https://www.riotgames.com/darkroom/1000/723575c503fa48c1bcead19d7da781ee:e7f9c311120b35019babf762d48e95f4/mnm-t2.jpg)

Inside Riot

Nov 4, 2024

### /Dev TFT: Magic n' Mayhem Learnings

A reflection on Magic n' Mayhem with our Director of Game Design, Riot Mort.

Nov 4, 2024

[Paint The Town Blue ft. Ashnikko (from the series Arcane: League Of...](https://www.riotgames.com/en/news/paint-the-town-blue-ft-ashnikko-from-the-series-arcane-league-of-legends-season-2-official-music-video)

![](https://www.riotgames.com/darkroom/1000/78f3e5f20f6a6cf3270c7ce6ca88ba3b:bde250cd5c1daf0e7c7fc643f207cacd/copy-of-pttb-thumb-15.png)

News

Oct 30, 2024

### Paint The Town Blue ft. Ashnikko (from the series Arcane: League Of...

The official music video for Ashnikko's "Paint The Town Blue" from the Arcane Season 2 Original Soundtrack is here!

Oct 30, 2024

[VALORANT Game Changers Championship 2024: Everything You Need to Know](https://www.riotgames.com/en/news/valorant-game-changers-championship-2024-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/e7c5fcc787ee2053eeced1e912385f2c:753595a5126468cbf1efdd58df965154/gcc24-header-16x9-textless-v1.png)

News

Oct 29, 2024

### VALORANT Game Changers Championship 2024: Everything You Need to Know

Find out all the details about the 2024 Game Changers Championship! Competitors, schedules, brackets and more!

Oct 29, 2024

[THE DRIVE // 2024 VALORANT Game Changers Championship Hype Film](https://www.riotgames.com/en/news/the-drive-2024-valorant-game-changers-championship-hype-film)

![](https://www.riotgames.com/darkroom/1000/4886b966b878cd4ec41d84a082305f39:ac1813de4b39593fa21e21f9514b0dd0/gcc-hype-thumb-textless-16x9-v5.png)

News

Oct 29, 2024

### THE DRIVE // 2024 VALORANT Game Changers Championship Hype Film

Tune in live to the VCT Game Changers Championship Nov 8-17 in Berlin, Germany.

Oct 29, 2024

[Global Community Premiere of Arcane Season 2: Co-Streaming...](https://www.riotgames.com/en/news/global-community-premiere-of-arcane-season-2-co-streaming-guidelines-and-rewards)

![](https://www.riotgames.com/darkroom/1000/b0461e6c5c2cd4257f92c5596b8fc3e4:32bc2074a56a7a1d3cdd021df7281506/act-1-key-art-nologo.png)

News

Oct 28, 2024

### Global Community Premiere of Arcane Season 2: Co-Streaming...

Learn how to co-stream the first episode of Arcane Season 2 on Twitch and earn exclusive in-game rewards.

Oct 28, 2024

[Blood Sweat & Tears \| Official Music Video - League of Legends](https://www.riotgames.com/en/news/blood-sweat-tears-official-music-video-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/02a9051ddc3320afb4e89cbad2e3c3a8:c4de207735dc2c72ea92934291338a7e/lol-thumbnail-bloodsweattears-mv-16x9-notext.png)

News

Oct 26, 2024

### Blood Sweat & Tears \| Official Music Video - League of Legends

Wounded and near death, Ambessa sees a vision of what awaits her as a follower of the Wolf.

Oct 26, 2024

[JUST. ONE. MORE // Episode 9: Act lll Trailer - VALORANT](https://www.riotgames.com/en/news/just-one-more-episode-9-act-lll-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/64212b2a8b3098f88a9bfe4b7e15543f:631c50d503ebda7bc9193207218b02f6/val24-ep9act3-thumbnail-16x9-textless.png)

News

Oct 21, 2024

### JUST. ONE. MORE // Episode 9: Act lll Trailer - VALORANT

New skinlines, the Year End Patches and so much more in the last act of COLLISION.

Oct 21, 2024

[Find a Worlds 2024 Watch Party Near You](https://www.riotgames.com/en/news/worlds-watch-parties-list-2024)

![](https://www.riotgames.com/darkroom/1000/773b7704606501a148fd450c6319c282:7310e5f5a3026648d0942268d24d568b/54059618374-38677da016-o.png)

News

Oct 18, 2024

### Find a Worlds 2024 Watch Party Near You

Worlds is better with friends. Here’s how you can find a place to watch Worlds in-person with other players

Oct 18, 2024

[Need to live \| Heimerdinger Champion Trailer - League of Legends: WR](https://www.riotgames.com/en/news/need-to-live-heimerdinger-champion-trailer-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/b2f3a2ce75125a8f53bde44f8c65064c:7826302fd4c498e3268e585001f60f35/wr-heimerdinger-champion-trailer-thumbnail-2x.jpg)

News

Oct 17, 2024

### Need to live \| Heimerdinger Champion Trailer - League of Legends: WR

You can take the Yordle out of Piltover, but you can’t take the progress out of the Yordle!

Oct 17, 2024

[Arcane: "Come Play" Series Trailer](https://www.riotgames.com/en/news/arcane-come-play-series-trailer)

![](https://www.riotgames.com/darkroom/1000/643d2c778b8a9043c573f0c0a87eab78:744ea1c6f50124a61b034c52906ca4db/official-clip-thumbnail-1920x1080.png)

News

Oct 16, 2024

### Arcane: "Come Play" Series Trailer

Progress has a price.

Oct 16, 2024

[Thunderdome: Inside Riot Games’ 48-Hour Hackathon](https://www.riotgames.com/en/news/thunderdome-inside-riot-games-48-hour-hackathon)

![](https://www.riotgames.com/darkroom/1000/356cd0d2d49ba5d175588e5e06582572:23ceceae5e0f344673c3a1fba6e982e1/thdrm-thumb-a-hor.jpg)

Inside Riot

Oct 15, 2024

### Thunderdome: Inside Riot Games’ 48-Hour Hackathon

Since the first in 2012, Thunderdome has come a long way. 1,500+ Rioters from 20+ offices participated this year. But it still begs the question… Why do we do it?

Oct 15, 2024

[Watch, Play, Experience: Here’s How We’re Celebrating Arcane...](https://www.riotgames.com/en/news/watch-play-experience-heres-how-were-celebrating-arcane-season-2-across-the-globe)

![](https://www.riotgames.com/darkroom/1000/5a70b5a571789e0eb8cc69f13adf7897:d70cd14647d6e2cf56043bd4d6fa7b67/event-article-thumb.png)

News

Oct 10, 2024

### Watch, Play, Experience: Here’s How We’re Celebrating Arcane...

We’re excited to share the first batch of many upcoming Arcane events in celebration of season two!

Oct 10, 2024

[Unstoppable (burnboy ft. NEONI) \| Welcome to Runeterror Cinematic...](https://www.riotgames.com/en/news/unstoppable-burnboy-ft-neoni-welcome-to-runeterror-cinematic-trailer-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/ae9b1966a471313dc0f5171ac4507486:6a1fb46b757c5e7ef82b6d58f0245b50/cinematicthumbnail-textless.jpeg)

News

Oct 8, 2024

### Unstoppable (burnboy ft. NEONI) \| Welcome to Runeterror Cinematic...

A new music track and two unforgettable faces. You won’t want to miss our next patch, Welcome to Runeterror. Live October 9.

Oct 8, 2024

[Riot Tech Interns 2024 Part 2: Aurora, VAL on Console, and skin...](https://www.riotgames.com/en/news/2024-intern-tech-blog-part-two)

![](https://www.riotgames.com/darkroom/1000/e08fdf8f12543d5e263fb84398fcd765:491fdb6f11199889f107ce62c796dede/2024-la-intern-week-104.png)

Tech Blog

Oct 2, 2024

### Riot Tech Interns 2024 Part 2: Aurora, VAL on Console, and skin...

Three Riot tech interns share their experiences working on an improvement to channel abilities, agent gear objectives for VALORANT on console, and an internal tool for animation picking on skins.

Oct 2, 2024

[Heavy Is The Crown ft. Linkin Park (Official Music Video)...](https://www.riotgames.com/en/news/heavy-is-the-crown-ft-linkin-park-official-music-video-league-of-legends-worlds-2024-anthem)

![](https://www.riotgames.com/darkroom/1000/bf1b8ff25d0ec43c2ba1f39a97f66ef1:e52f5d4b151b3a1964b0b59b7c524cdb/thumb2-clean.png)

News

Sep 24, 2024

### Heavy Is The Crown ft. Linkin Park (Official Music Video)...

The official anthem and music video for Worlds 2024.

Sep 24, 2024

[LoL Player Days](https://www.riotgames.com/en/news/lol-player-days)

![](https://www.riotgames.com/darkroom/1000/31bba59751d6b77666b7d71f6f9cd4c7:14c16754409a41f68feef7ea45e37c8a/article-header-eventkv-lol-play24-003-eventarticle-worldsclient-t2thumbnail-1920x1080.png)

News

Sep 23, 2024

### LoL Player Days

Everything you need to know about the upcoming community celebration.

Sep 23, 2024

[Vanguard x VALORANT](https://www.riotgames.com/en/news/vanguard-x-valorant)

![](https://www.riotgames.com/darkroom/1000/246dc277304b168dad18ffb4f3272c4b:dbf036b2d85f1311ead81e57e4da5fc2/copy-of-vanguard-x-valorant-dev-blog.png)

Inside Riot

Sep 20, 2024

### Vanguard x VALORANT

How the anti-cheat team continues to evolve as cheaters do.

Sep 20, 2024

[Player Behavior, Balance, and Anti-Cheat // Dev Updates - VALORANT](https://www.riotgames.com/en/news/player-behavior-balance-and-anti-cheat-dev-updates-valorant)

![](https://www.riotgames.com/darkroom/1000/78266658938fb3d66f8a93fc953af21d:85a44154c2e2ee095091fc0e168937d6/year-end-dev-update-thumbnail-16x9-textless-v4.png)

News

Sep 20, 2024

### Player Behavior, Balance, and Anti-Cheat // Dev Updates - VALORANT

Our final Dev Update before we roll into 2025.

Sep 20, 2024

[The Return of Thunderdome](https://www.riotgames.com/en/news/riot-thunderdome-hackathon-2024)

![](https://www.riotgames.com/darkroom/1000/bfdf19737d3d04df8eeb841d25effd6c:65d1d96ac02fdb5dabaa56423df0162e/cyw-thunderdomethurs-199.png)

Inside Riot

Sep 19, 2024

### The Return of Thunderdome

After a brief hiatus, it’s Thunderdome time once again. Here’s why we carve out time for our take on a 48-hour hackathon.

Sep 19, 2024

[Worlds 2024 \| Make Them Believe](https://www.riotgames.com/en/news/worlds-2024-make-them-believe)

![](https://www.riotgames.com/darkroom/1000/22483beb843d8c4b978b5404c5605d81:d806cc83b46a8fec9e35c8be20f29d85/w24-hypefilm-textless-thumb02.png)

News

Sep 17, 2024

### Worlds 2024 \| Make Them Believe

Get ready to watch Worlds 2024

Sep 17, 2024

[September Update 2024 \| Dev Snapshot - LoR](https://www.riotgames.com/en/news/september-update-2024-dev-snapshot-lor)

![](https://www.riotgames.com/darkroom/1000/8e9118afda0d7fe7bf0602fa14a096a7:b4e9b4700f2617fbed9ea9b68a651577/lor-update5-9-devsnapshot-webredirect.png)

News

Sep 12, 2024

### September Update 2024 \| Dev Snapshot - LoR

Balancing fright with delight, Eric and Brian cover upcoming 2024 content.

Sep 12, 2024

[Worlds 2024 Primer](https://www.riotgames.com/en/news/worlds-2024-primer)

![](https://www.riotgames.com/darkroom/1000/1538b165cac3473d6f7e5fb06c7741ba:4737981bdf9b984f7b5377eb29b8d5b2/w24-makethembelieve-header-v2.png)

News

Sep 9, 2024

### Worlds 2024 Primer

Learn more about the 2024 League of Legends World Championship in Europe

Sep 9, 2024

[Arcane: Season 2 \| Official Trailer](https://www.riotgames.com/en/news/arcane-season-2-official-trailer)

![](https://www.riotgames.com/darkroom/1000/4fccf878fa5ed54b41c442ec078d4a57:0e3f4687c3383648e9eaa0de2294b194/riot-arc2-social-yt-maintrailer-thumbnail-1920x1080-textless.png)

News

Sep 5, 2024

### Arcane: Season 2 \| Official Trailer

Time to turn things upside down.

Sep 5, 2024

[Upcoming Regional Price Updates](https://www.riotgames.com/en/news/2024-regional-price-updates)

![](https://www.riotgames.com/darkroom/1000/6791cedc43cb912108a6086e4c4f9220:52921cbc8a6136edab9f150dd3238126/pricing-article-header-image.png)

News

Sep 3, 2024

### Upcoming Regional Price Updates

On September 18, 2024 we’re adjusting prices in most regions.

Sep 3, 2024

[Technical Assessment Tips: Riot Games Internship Study Guide Vol. 4](https://www.riotgames.com/en/news/technical-assessment-internship-study-guide-vol-4)

![](https://www.riotgames.com/darkroom/1000/8cfa3401a47255d3c78b9de77d4013d9:cac30aa38c5495e2c936db2966d3726f/up-studyguide-vol-4-header-image.png)

Disciplines

Aug 28, 2024

### Technical Assessment Tips: Riot Games Internship Study Guide Vol. 4

The technical assessment–or coding test–is crucial for prospective interns. Here’s how to prepare.

Aug 28, 2024

[Riot Tech Interns 2024 Part 1: League of Legends, VALORANT, and TFT](https://www.riotgames.com/en/news/2024-intern-tech-blog-part-one)

![](https://www.riotgames.com/darkroom/1000/1d7681edc6e8db68a1b2a761b5ad90f0:131c34c6beb0439b7807f77a7ca7e382/summer-intern-tech-blog-article-header.png)

Tech Blog

Aug 27, 2024

### Riot Tech Interns 2024 Part 1: League of Legends, VALORANT, and TFT

Three Riot tech interns share their experiences working on Champion silhouettes, gameplay capture on VAL Console, and a TFT asset browser.

Aug 27, 2024

[ALL PATHS END HERE // Vyse Agent Trailer - VALORANT](https://www.riotgames.com/en/news/all-paths-end-here-vyse-agent-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/c659f2d3c2d7b24de092167c82abeaf1:39c99136f54d2d4bfadf8c4ed52df1a5/vyse-cinematic-thumb-a-textless-16x9-v1-primary.png)

News

Aug 23, 2024

### ALL PATHS END HERE // Vyse Agent Trailer - VALORANT

All paths end with Vyse–lethal tactician, metal manipulator, and the newest Sentinel Agent in VALORANT.

Aug 23, 2024

[Honor, Skins & More \| Dev Update - League of Legends](https://www.riotgames.com/en/news/honor-skins-more-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/809c9b3c409ea8964cce47db14cec41e:109ed41f416de929cb157a513f633dd0/lol-devupdate-thumbnail-honorskinsandmore-16x9.png)

Inside Riot

Aug 22, 2024

### Honor, Skins & More \| Dev Update - League of Legends

Meddler and Pabro answer questions on modes, skins, honor, Vanguard, matchmaking, Yuumi, and more.

Aug 22, 2024

[Bringing VALORANT to Console through Global Collaboration](https://www.riotgames.com/en/news/valorant-console-global-collaboration)

![](https://www.riotgames.com/darkroom/1000/8d5c497da1c2eeec8cffa99b01abc64b:9cebd331afd4d14cda873819e644f8e2/ps-f2p-val-console-launch-16x9.jpg)

Inside Riot

Aug 21, 2024

### Bringing VALORANT to Console through Global Collaboration

From Sydney to Seattle, Rioters around the world worked together to make VAL Console a reality.

Aug 21, 2024

[We're Updating our Internal Structure to Better Connect Games...](https://www.riotgames.com/en/news/riot-games-connecting-games-with-esports-music-publishing)

![](https://www.riotgames.com/darkroom/1000/739ecf0aaf8d6324a3c243fe3bbf8138:6f0232f63834af8b6d04669a2f084520/small-logo-black.png)

News

Aug 15, 2024

### We're Updating our Internal Structure to Better Connect Games...

We’re also establishing dedicated animation and live-action studios to lead future TV & film projects.

Aug 15, 2024

[How’s 2XKO Alpha Lab Going? - Dev Chat](https://www.riotgames.com/en/news/hows-2xko-alpha-lab-going-dev-chat)

![](https://www.riotgames.com/darkroom/1000/87c5ed2859f5b053bad290cd661fe736:d3cf99c09241079bced9692e5862aec0/screenshot-2024-08-14-at-10-51-24-am.png)

Inside Riot

Aug 13, 2024

### How’s 2XKO Alpha Lab Going? - Dev Chat

We’re halfway through 2XKO Alpha Lab, our first playtest. Here’s what we’ve heard so far.

Aug 13, 2024

[What’s in 2XKO Alpha Lab?](https://www.riotgames.com/en/news/whats-in-2xko-alpha-lab)

![](https://www.riotgames.com/darkroom/1000/3cbabf3e6143b33f8d95704900e8d20d:7daef55a05d739699afe977d0d6a5b84/2xko-whats-in-alphalab-16x9-1920x1080-final.png)

News

Aug 7, 2024

### What’s in 2XKO Alpha Lab?

2XKO is starting its first at-home playtest: Alpha Lab. Read on to learn more.

Aug 7, 2024

[INCURSIONS // Official Console Cinematic Trailer - VALORANT](https://www.riotgames.com/en/news/incursions-official-console-cinematic-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/62ea3214977f12e151a7dc6c8bbac30f:63c2457bf4ea5e437e305fa7835738e1/val-cnsl-incursion-thumb-a-v2.png)

News

Aug 2, 2024

### INCURSIONS // Official Console Cinematic Trailer - VALORANT

Break through into a brand new way to play with VALORANT on PlayStation 5 and Xbox Series X \| S.

Aug 2, 2024

[Magic Is Brewing \| Magic n’ Mayhem Launch Cinematic - TFT](https://www.riotgames.com/en/news/magic-is-brewing-magic-n-mayhem-launch-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/f2ab1fb3344ce13e001829a58134c964:0022d890784b6d918c08aa000ca242b8/tft-st1224-video-thumb-launchcinematic.png)

News

Jul 31, 2024

### Magic Is Brewing \| Magic n’ Mayhem Launch Cinematic - TFT

The real magic was the friends we summoned along the way…and a perfect cup of tea!

Jul 31, 2024

[Devs Answer Questions While Swarmed by Kittens \| League of Legends](https://www.riotgames.com/en/news/devs-answer-questions-while-swarmed-by-kittens-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/9ca369530aeea4afbe0814ce1628de09:a93b6a29b090f3c6bbfb286a530ed7e4/lol-20243107-ansq24-devs-kittens-banner.png)

Inside Riot

Jul 31, 2024

### Devs Answer Questions While Swarmed by Kittens \| League of Legends

Devs answer questions about Swarm with some aspiring—and adoptable—Battle Cats.

Jul 31, 2024

[Everything You Need to Know: Champions Seoul](https://www.riotgames.com/en/news/everything-you-need-to-know-champions-seoul)

![](https://www.riotgames.com/darkroom/1000/ca85d939a7ec5325f4896ad2c24231d8:a012979acdc5772f21d827d7af7e80ae/ch24-eyntk-cover-image.png)

News

Jul 31, 2024

### Everything You Need to Know: Champions Seoul

Tournament match-ups, format, schedule, and more!

Jul 31, 2024

[Swarm, Arena, and the Value of Game Modes](https://www.riotgames.com/en/news/riot-game-modes-2024)

![](https://www.riotgames.com/darkroom/1000/845bf7f72d28e1fbdb96572de318a247:9cba3c870b6eac4463b044fed3b9f77e/ansq24-strawberry-kv-final.png)

Inside Riot

Jul 24, 2024

### Swarm, Arena, and the Value of Game Modes

Between Swarm, Arena, Pengu’s Party, and Ruination, it’s been a busy time for game modes in League, TFT, and Wild Rift. Let’s talk about em.

Jul 24, 2024

[SUPERPOWER ft. KISS OF LIFE and Mark Tuan (Official Music Video)...](https://www.riotgames.com/en/news/superpower-ft-kiss-of-life-and-mark-tuan-official-music-video-valorant-champions-2024-anthem)

![](https://www.riotgames.com/darkroom/1000/680e70ab99cfdfd520ed84b541da13fa:3be5bc8a2d0569c20dfdbc05100606a2/champs-mv-thumb-16x9-v02.jpg)

News

Jul 24, 2024

### SUPERPOWER ft. KISS OF LIFE and Mark Tuan (Official Music Video)...

The official anthem and music video for Champions 2024 “SUPERPOWER” by VALORANT, featuring Julie & Natty of KISS OF LIFE and Mark Tuan.

Jul 24, 2024

[Key Learnings from Beyond Ruination](https://www.riotgames.com/en/news/key-learnings-from-beyond-ruination)

![](https://www.riotgames.com/darkroom/1000/233f196ada85b246b82996cdb8d317fb:a3e285c07df3ec476c163870757cb793/4b-kv-1920x1080.jpg)

Inside Riot

Jul 23, 2024

### Key Learnings from Beyond Ruination

Sit back and reflect with us on patch 5.1

Jul 23, 2024

[Dev Update: Heightened Hexperience](https://www.riotgames.com/en/news/dev-update-heightened-hexperience)

![](https://www.riotgames.com/darkroom/1000/9beb4eb2a676252bbc11a1463ca65767:91730bb17f2e6bf8c41b94973237c6cf/wr-1920x1080-textles.jpg)

News

Jul 22, 2024

### Dev Update: Heightened Hexperience

Join us for an outlook on what more hexciting things to expect and celebrate this year!

Jul 22, 2024

[The Hexolution is coming to Wild Rift!](https://www.riotgames.com/en/news/the-hexolution-is-coming-to-wild-rift)

![](https://www.riotgames.com/darkroom/1000/75e3c0fd5e823017e9a99f0e21b5c8c4:f21a00426ed483f61770b916224e56b7/1920x1080-textless-2x.png)

News

Jul 18, 2024

### The Hexolution is coming to Wild Rift!

Get even wilder with the new Rift. Coming to you on July 18th UTC!

Jul 18, 2024

[Fight for Final City \| Anima Squad 2024 Cinematic - League of Legends](https://www.riotgames.com/en/news/fight-for-final-city-anima-squad-2024-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/0a17e33fd52d9b2d8b0ba2e34cba0d81:2075b05a349fc03dc0c1beac082bd979/lol-ansq24-marquee-thumbnail-textless-1920x1080-v04-r01.png)

News

Jul 17, 2024

### Fight for Final City \| Anima Squad 2024 Cinematic - League of Legends

When Anima Squad stands together, the Primordians don’t stand a chance.

Jul 17, 2024

[Anima Squad 2024: Everything You Need To Know](https://www.riotgames.com/en/news/anima-squad-2024-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/1660b37e80e473252b71cbd44bfab020:a70236706dc193641c2c4627edf8b099/animasquad-kv-forarticle.png)

News

Jul 17, 2024

### Anima Squad 2024: Everything You Need To Know

Discover the new game mode, skins, and more!

Jul 17, 2024

[Know Before You Go - 2XKO at Evo 2024](https://www.riotgames.com/en/news/know-before-you-go-2xko-at-evo-2024)

![](https://www.riotgames.com/darkroom/1000/df9d55de773eda819e7a2cd906468e55:a4a90a48c3aabde0712ea7a7b57e6032/2xko-evo-2024-before-you-go-final-1920x1080.png)

News

Jul 16, 2024

### Know Before You Go - 2XKO at Evo 2024

2XKO is coming to Evo 2024, July 19–21. Get details on the demo, our booth schedule, Alpha Lab priority access, and more.

Jul 16, 2024

[TFT Dev Drop: Magic n’ Mayhem I Dev Video - Teamfight Tactics](https://www.riotgames.com/en/news/tft-dev-drop-magic-n-mayhem-i-dev-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/4032ac4c70a911fb2790341430f390b0:2118b6b4257e2f7d5ddced190ba8ac56/tft-st1224-video-thumb-devdrop-official.png)

Inside Riot

Jul 14, 2024

### TFT Dev Drop: Magic n’ Mayhem I Dev Video - Teamfight Tactics

From witches, to sugary treats, to bees—in Magic n’ Mayhem, all forms of magic are welcome.

Jul 14, 2024

[The Next TFT Open Heads to Macao!](https://www.riotgames.com/en/news/the-next-tft-open-heads-to-macao)

![](https://www.riotgames.com/darkroom/1000/55401865efe92d8d30c621b95138937d:e3dfe99afa9f12c27a4aa95a9b0ffce8/tft-20241407-tmo-announcearticle-01-banner.png)

News

Jul 14, 2024

### The Next TFT Open Heads to Macao!

The global open-bracket event will take place Dec 13 - 15.

Jul 14, 2024

[Braum Gameplay Reveal Trailer \| 2XKO](https://www.riotgames.com/en/news/braum-gameplay-reveal-trailer-2xko)

![](https://www.riotgames.com/darkroom/1000/00b6e3b0ce8c8bddcd1fa2e57c4423a2:a9a1751d675510c1bfe8bc4699c21948/2xko-vevo24-yt-thumbnail-new-champ-braum.png)

News

Jul 9, 2024

### Braum Gameplay Reveal Trailer \| 2XKO

He's got your back. Braum joins the 2XKO roster.

Jul 9, 2024

[We’re Taking a Break, BRB](https://www.riotgames.com/en/news/riot-games-mid-year-summer-break)

![](https://www.riotgames.com/darkroom/1000/f6cc0cedbe06b2a2e3b613b6114725a1:b25d4620304f8d0095267328f4f6848f/riot-summer-break-illo-2024.png)

News

Jun 28, 2024

### We’re Taking a Break, BRB

It’s time for our annual summer break. Because sunburns beat burnout any day.

Jun 28, 2024

[What is 2XKO Alpha Lab? - Dev Q&A w/ Sajam](https://www.riotgames.com/en/news/what-is-2xko-alpha-lab-dev-q-a-w-sajam)

![](https://www.riotgames.com/darkroom/1000/f8e768c0178a0811259d1d6d4b2e05c5:29960ecb04e898a4151fffb69a9c6bd7/2xko-alphalab-yt-thumbnail-16x9.png)

Inside Riot

Jun 26, 2024

### What is 2XKO Alpha Lab? - Dev Q&A w/ Sajam

Devs sit down with Sajam to discuss the upcoming Alpha Lab playtest.

Jun 26, 2024

[/Dev TFT: Inkborn Fables Learnings](https://www.riotgames.com/en/news/dev-tft-inkborn-fables-learnings)

![](https://www.riotgames.com/darkroom/1000/e30be59803b8ff5da3bc18d36c56a8a9:07ce5b5daf71e3d9ef3f8507cf666f59/030524-st1124-gameplay-overview-banner.jpg)

Inside Riot

Jun 24, 2024

### /Dev TFT: Inkborn Fables Learnings

A reflection on Inkborn Fables with our Director of Game Design, Riot Mort.

Jun 24, 2024

[THE FUTURE’S READY // Episode 9: Act l Trailer - VALORANT](https://www.riotgames.com/en/news/the-futures-ready-episode-9-act-l-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/497c9a21defadf9e455c3df04a8145d9:efe97a1dfdc942a16fb94fa8c7da5de8/val24-ep9act1-trailer-thumbnail-16x9-textless.jpg)

News

Jun 24, 2024

### THE FUTURE’S READY // Episode 9: Act l Trailer - VALORANT

Brave the chasm of the newest map Abyss, explore the Evori Dreamwings bundle, and witness VALORANT’s best compete to become world champion.

Jun 24, 2024

[Swarm \| Operation: Anima Squad - Event Trailer \| League of Legends](https://www.riotgames.com/en/news/swarm-operation-anima-squad-event-trailer-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/fdc187a026dacbb60c3c4298dec023ca:13530ad1ca24d4d0778ec067f493061b/lol-ansq24-event-trailer-thumbnail-textless-1920x1080-v04-r01.png)

News

Jun 21, 2024

### Swarm \| Operation: Anima Squad - Event Trailer \| League of Legends

It’s up to you, rookie. Join the squad and leap into an explosive new game mode.

Jun 21, 2024

[Champion Insights: Aurora](https://www.riotgames.com/en/news/champion-insights-aurora)

![](https://www.riotgames.com/darkroom/1000/9cf04f98a107bd3ebe46273d3e50efcd:a604e61834f96eaa006e3a89d5196302/article-header-champion-insights-web.png)

Inside Riot

Jun 20, 2024

### Champion Insights: Aurora

The Whimsical Witchy Wabbit

Jun 20, 2024

[Game Changers 2024: Championship Expansion & New Mobility Policies](https://www.riotgames.com/en/news/game-changers-2024-championship-expansion-new-mobility-policies)

![](https://www.riotgames.com/darkroom/1000/10d28957c8c7f2473862cbf9e5461d3f:231be56576a8ba8583f06416c1a32911/vct24-gc-cover-imagerev.png)

News

Jun 18, 2024

### Game Changers 2024: Championship Expansion & New Mobility Policies

FInd out more about this year’s Championship and our new policies for GC competitors!

Jun 18, 2024

[Mid-Year Update 2024 \| Dev Snapshot - LoR](https://www.riotgames.com/en/news/mid-year-update-2024-dev-snapshot-lor)

![](https://www.riotgames.com/darkroom/1000/5d1f7df8f74e053516809d96189709c3:51bb7e1183e181489e02fc9114747dd1/lor-5-6-devvideo-websiteredirectthumbnail.png)

News

Jun 18, 2024

### Mid-Year Update 2024 \| Dev Snapshot - LoR

Eric and Tyler provide a Dev Update encompassing past, present and future content drops to come.

Jun 18, 2024

[From VALORANT to LGBTQIA+ Game Shows, Euphoria Ng Builds Community...](https://www.riotgames.com/en/news/APAC-rainbow-rioters-euphoria)

![](https://www.riotgames.com/darkroom/1000/74c8779aaae3b8a29cd5587e8f685cc2:0286888bdec43d5128b2e3683e3a8494/rg-rainbowrioters-euphoria-ng.png)

Inside Riot

Jun 18, 2024

### From VALORANT to LGBTQIA+ Game Shows, Euphoria Ng Builds Community...

Be patient, be kind, be brave

Jun 18, 2024

[Hall of Legends: Faker](https://www.riotgames.com/en/news/hall-of-legends-faker)

![](https://www.riotgames.com/darkroom/1000/d9af39ef8404594cf58b875a81bbf6a0:ebe56c0ab963f50fe5f0b73d7dda23dc/lol-hol-fakerwinner-thumbnail-textless.png)

News

Jun 14, 2024

### Hall of Legends: Faker

Watch the full documentary on Faker, celebrating his unbelievable and unfinished career.

Jun 14, 2024

[We Asked TFT Devs Your Burning Questions \| 5 Year Bash - TFT](https://www.riotgames.com/en/news/we-asked-tft-devs-your-burning-questions-5-year-bash-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/a314d1d316df6ed0a37f0682e0943b6e:9021e0f3b0782da4f5f743bf4b63b8d3/20240612-tft-5y2024-video-ask-tft-banner.png)

Inside Riot

Jun 13, 2024

### We Asked TFT Devs Your Burning Questions \| 5 Year Bash - TFT

To celebrate five whole years of Teamfight Tactics, we’re taking a moment to ask longtime TFT devs your questions about Choncc lore, memorable patches, go-to tacticians, and more.

Jun 13, 2024

[Arcane Season 2 \| Official Teaser Trailer](https://www.riotgames.com/en/news/arcane-season-2-official-teaser-trailer)

![](https://www.riotgames.com/darkroom/1000/0f5693116d369910fb49b56ee0df3783:31e22baf53ca56b85d01e36913f332c8/arc2-thumb-teaser-trailer-textless.jpg)

News

Jun 11, 2024

### Arcane Season 2 \| Official Teaser Trailer

The hunt is on. Arcane, the final season coming this November to Netflix.

Jun 11, 2024

[Gameplay, Esports & Arcane \| Dev Update - League of Legends](https://www.riotgames.com/en/news/gameplay-esports-arcane-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/3c5e9be65725f045af2053c879e812f7:8c8765e7b49142e1e5536a1747ffef86/lol-devupdate-thumbnail-gameplay-esports-arcane-textlessv2.jpg)

News

Jun 11, 2024

### Gameplay, Esports & Arcane \| Dev Update - League of Legends

Updates on champions, mastery, Vanguard, esports, Arcane, and a teaser for season two.

Jun 11, 2024

[SANCTUM OF SECRETS // Abyss Official Map Trailer - VALORANT](https://www.riotgames.com/en/news/sanctum-of-secrets-abyss-official-map-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/2fe8391350306c6a8d2b89ddb97ed6df:6b9f1f2e66d272fa939865d25294bb9c/8-3-abyss-thumbnail-16x9-v2-textless.png)

News

Jun 9, 2024

### SANCTUM OF SECRETS // Abyss Official Map Trailer - VALORANT

Gaze into the Abyss: a clandestine base buried deeper than its secrets.

Jun 9, 2024

[Register now for the VALORANT Console Limited Beta](https://www.riotgames.com/en/news/register-now-for-the-valorant-console-limited-beta)

![](https://www.riotgames.com/darkroom/1000/b0b0e00176707b22bfdc5fcb8f324fc1:7c7cce95e1f6632c5246afdd99dbdb50/val-lb-riot-bar-product-card.png)

News

Jun 8, 2024

### Register now for the VALORANT Console Limited Beta

Limited Beta begins June 14. Register now up at beta.playvalorant.com

Jun 8, 2024

[VALORANT CONSOLE ANNOUNCE // Cinematic Gameplay Trailer - VALORANT](https://www.riotgames.com/en/news/valorant-console-announce-cinematic-gameplay-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/5851218d8ec1a13c4de4ce22f4edad15:4848c08676f769f52df6ab05dfeb4523/val-csln24-012-announcetrailer-thumb-textless.png)

News

Jun 8, 2024

### VALORANT CONSOLE ANNOUNCE // Cinematic Gameplay Trailer - VALORANT

VALORANT is headed to PlayStation 5 and Xbox Series X \| S.

Jun 8, 2024

[Bringing VALORANT to Console // Dev Diaries - VALORANT](https://www.riotgames.com/en/news/bringing-valorant-to-console-dev-diaries-valorant)

![](https://www.riotgames.com/darkroom/1000/efe9166a5db4ad656d4457a96bb24ce3:e641dd052e419824fb32c865d24471e7/dev-diary-16x9-textless-v3.png)

Inside Riot

Jun 8, 2024

### Bringing VALORANT to Console // Dev Diaries - VALORANT

Join us on our journey of taking VALORANT from the keyboard to the sticks in our newest Dev Diary.

Jun 8, 2024

[Happy Pride!](https://www.riotgames.com/en/news/riot-pride-2024)

![](https://www.riotgames.com/darkroom/1000/834c1b25fc52ddeb08d8b80f2b132cba:28af3876ac942754457c78fbb19c0c91/pride-2024-articleheader-16x9.png)

News

May 30, 2024

### Happy Pride!

We’re celebrating this year with new in-game content, an updated Pride playlist, and a community hub where you all can show your pride.

May 30, 2024

[Keeping Our Community Healthy // Dev Updates](https://www.riotgames.com/en/news/keeping-our-community-healthy-dev-updates)

![](https://www.riotgames.com/darkroom/1000/e3f226f68815b1a2d0e317b1eaaacf0e:edf85263242921962e0f9f9d0019826b/tl-dev-diary-player-behavior-16x9.png)

News

May 30, 2024

### Keeping Our Community Healthy // Dev Updates

VALORANT Studio Head Anna Donlon shares her thoughts on the current state of player behavior in VALORANT and outlines some of the upcoming changes aimed at making our community a better place for all.

May 30, 2024

[Crafting Visual Effects in Teamfight Tactics - Super Art Power Hour...](https://www.riotgames.com/en/news/crafting-visual-effects-in-teamfight-tactics-super-art-power-hour-ep-6)

![](https://www.riotgames.com/darkroom/1000/3a00356c5e4aeb9e1bb40cecd1c5d9c6:045785dfbccb0bbd4f20346fc0b07421/saph-ep6-thumbnail-1.jpg)

Inside Riot

May 30, 2024

### Crafting Visual Effects in Teamfight Tactics - Super Art Power Hour...

Visual effects artists can do anything they want! Well, almost anything. We linked up with visual effects artist Jonathan Shishido for a speed run in building a celebration effect for TFT to experience the entire process from start to finish. He was also kind enough to share some tips and advice for anyone interested in learning more about visual effects work or working in games.

May 30, 2024

[Legacy of the Demon King \| Hall of Legends Event Trailer - LoL](https://www.riotgames.com/en/news/legacy-of-the-demon-king-hall-of-legends-event-trailer-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/a5911dc95c29f4442d64fb895f730c83:a32804025c1931d62a1d0d7f2a27a218/lol-hol-eventtrailer-thumbnail-textless-16x9.jpg)

News

May 28, 2024

### Legacy of the Demon King \| Hall of Legends Event Trailer - LoL

Celebrate Faker’s legacy with 100 levels of rewards, exclusive Ahri skins, and more during the Hall of Legends event.

May 28, 2024

[Constellations \| Launch Video - Legends of Runeterra](https://www.riotgames.com/en/news/constellations-comes-to-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/2208db6254f8cff2ec0bb5f1902d3ba8:ef393ced2ae240020d16a2e9f2c12e60/copy-of-lor-constellations-ytthumb-1920x1080-v1.jpg)

News

May 22, 2024

### Constellations \| Launch Video - Legends of Runeterra

Power is written in the stars. Constellations has come to Legends of Runeterra, and with it comes Path of Champions biggest update yet! Watch to learn who the stars have blessed with Level 6 Star Power.

May 22, 2024

[TFT’s 5-Year Bash!](https://www.riotgames.com/en/news/tfts-5-year-bash)

![](https://www.riotgames.com/darkroom/1000/8ddf9cab9d3906e01396dc0ac72e0b29:357d7c55d7165f802cd31dab98c21901/tft-5yr-birthday-bash.webp)

News

May 21, 2024

### TFT’s 5-Year Bash!

TFT’s 5th Birthday is just around the corner, and everyone is invited to celebrate, including all of our past sets!

May 21, 2024

[Map Rotations, Replays, and More // Dev Updates - VALORANT](https://www.riotgames.com/en/news/map-rotations-replays-and-more-dev-updates-valorant)

![](https://www.riotgames.com/darkroom/1000/a8300cec0fa6232f8b7e79b2261facdf:b47584c2694d115cfd97584a7604e92b/dev-updates-16x9-tl-v2.jpg)

News

May 21, 2024

### Map Rotations, Replays, and More // Dev Updates - VALORANT

Anna and Andy stop by with some VALORANT leadership news, and then hand things over to some members of the dev team for updates on map rotations, balance, skins, and REPLAYS?! We’ll be back with more Dev Updates later this year.

May 21, 2024

[Button Day \| Launch Video - Legends of Runeterra](https://www.riotgames.com/en/news/button-day-launch-video-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/e1ac56770e2ae6faa9dc14748eba5a24:296e4a9719264a523f174d7c765ae42e/lor-2024-buttondayvideo-full-articlebanner-option1-1920x1080.jpg)

News

May 15, 2024

### Button Day \| Launch Video - Legends of Runeterra

Players can now play, or install LoR from the League Client.

May 15, 2024

[Celebrate Pride 2024: League and TFT](https://www.riotgames.com/en/news/celebrate-pride-2024-league-and-tft)

![](https://www.riotgames.com/darkroom/1000/3d3e6e53f283f47a6b22e3df8d706523:6073f99680400dace04b37d72fc18365/pride-t1-template-cluttered-image.jpg)

News

May 13, 2024

### Celebrate Pride 2024: League and TFT

Celebrate Pride with us and our LGBTQIA+ community

May 13, 2024

[Everything You Need to Know: Masters Shanghai](https://www.riotgames.com/en/news/everything-you-need-to-know-masters-shanghai)

![](https://www.riotgames.com/darkroom/1000/d5b59609d3301c5cb64a3742b42d5ae3:7606776d3a37339662b476bf54922ab0/vct24-m2-shanghai-header.jpg)

News

May 13, 2024

### Everything You Need to Know: Masters Shanghai

Tournament match-ups, format, schedule, and more!

May 13, 2024

[Update on 2021 Lawsuit Against Riot & Former CEO](https://www.riotgames.com/en/news/update-on-2021-lawsuit)

![](https://www.riotgames.com/darkroom/1000/739ecf0aaf8d6324a3c243fe3bbf8138:6f0232f63834af8b6d04669a2f084520/small-logo-black.png)

News

May 7, 2024

### Update on 2021 Lawsuit Against Riot & Former CEO

Arbitrator overseeing Sharon O'Donnell Vs Riot Games Inc., Et Al. finds all claims in the January 2021 lawsuit against Riot Games and our former CEO, Nicolo Laurent, were unfounded.

May 7, 2024

[Making the 2023 Worlds Opening Ceremony Presented by Mastercard](https://www.riotgames.com/en/news/making-the-2023-worlds-opening-ceremony-presented-by-mastercard)

![](https://www.riotgames.com/darkroom/1000/ed9ea4352525e90cf0e13175bfaf5b4c:a42d84c28cfe65e6412da950d9606104/lol-w23-oc-bts-thumbnail.jpg)

News

May 13, 2024

### Making the 2023 Worlds Opening Ceremony Presented by Mastercard

Riot returned to Korea for Worlds 2023 with live performances by NewJeans, HEARTSTEEL, and much more! Watch to see how it all came together.

May 13, 2024

[Dive into patch 5.1: Beyond Ruination \| League of Legends: Wild Rift](https://www.riotgames.com/en/news/dive-into-patch-5-1-beyond-ruination-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/3b6ac200aa5249af8838dc7938bdd0c0:4d9bb758f250f9c7da254ba678767470/lol-wr-patch-5-1.png)

Inside Riot

May 1, 2024

### Dive into patch 5.1: Beyond Ruination \| League of Legends: Wild Rift

Patch 5.1 takes you #ThroughTheMist, in and out of game! Laura, Paul, and Anthony are here to guide you and lift the mist on this immersive patch!

May 1, 2024

[Know Before You Go - 2XKO at EVO Japan](https://www.riotgames.com/en/news/2xko-evo-japan-know-before-you-go)

![](https://www.riotgames.com/darkroom/1000/04ff10b412a5b212df52e154a986175d:3519f82ebc08e0198a441418b8de0fcb/2xko-2024-articles-jevo-lockup-1920x1080.png)

News

Apr 23, 2024

### Know Before You Go - 2XKO at EVO Japan

2XKO is coming to EVO Japan 2024, April 27–29. Get details on how to play the demo, our booth schedule, and more.

Apr 23, 2024

[How to Play 2XKO - EVO Japan 2024](https://www.riotgames.com/en/news/play-2xko-evo-japan-2024)

![](https://www.riotgames.com/darkroom/1000/df403b27ec96c76b514c88604f349fad:d5f09fdc6c9cae8b85e34e60bad1e047/2xko-2024-jevo-howtoplay-thumbnail.png)

News

Apr 23, 2024

### How to Play 2XKO - EVO Japan 2024

We’re bringing a new demo to EVO Japan 2024, April 27–29. If you’re attending, brush up on basic controls, combos, and tag mechanics with the beginner’s guide below.

Apr 23, 2024

[MSI 2024 Primer](https://www.riotgames.com/en/news/msi-2024-primer)

![](https://www.riotgames.com/darkroom/1000/6747c7e350b640e47b46ed7bdf877eff:d97e62b0ab872448f0e81bd75a71a5d0/lol-msie24-009-primerarticle-header-1600x900.png)

News

Apr 22, 2024

### MSI 2024 Primer

Learn more about the 2024 League of Legends Mid-Season Invitational in Chengdu.

Apr 22, 2024

[Three Ways We’re Making Our Offices More Sustainable](https://www.riotgames.com/en/news/sustainability-riot-offices-earth-week-2024)

![](https://www.riotgames.com/darkroom/1000/add5f292dac71b0ab76c6b71f9c34bcc:8d09a112ae5b28e290092b7255c200be/ivern.png)

Inside Riot

Apr 22, 2024

### Three Ways We’re Making Our Offices More Sustainable

Taking a look at some of the key areas where we’re making choices with sustainability in mind.

Apr 22, 2024

[Beyond the Mist \| Kalista Champion Trailer - League of Legends: WR](https://www.riotgames.com/en/news/beyond-the-mist-kalista-champion-trailer-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/b1b9ff60584153d4c20aa7fef0c378e9:46c3b6a1e67555c7427b44b2f1bf6ec2/1920x1080-textles-2x.jpg)

News

Apr 11, 2024

### Beyond the Mist \| Kalista Champion Trailer - League of Legends: WR

Moments before reaching the Blessed Isles, Kalista’s fate is written in the mist. Be there on April 11th when it lifts!

Apr 11, 2024

[Arena, Lee Sin & PvE Mode First-Look \| Dev Update - League of Legends](https://www.riotgames.com/en/news/arena-lee-sin-pve-mode-first-look-dev-update-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/1b2aec2afb29eaf8d7f3c71b22bce7af:7f9c07998f5e26773ffc8773995251ab/andreijeremy-3-web.jpg)

News

Apr 8, 2024

### Arena, Lee Sin & PvE Mode First-Look \| Dev Update - League of Legends

Riot Brightmoon, Meddler, and the Modes team discuss Arena, Lee Sin, Ranked Rewards, and a PvE mode.

Apr 8, 2024

[Advancing the Esports and Player Experience in MENA](https://www.riotgames.com/en/news/riot-games-mena-players)

![](https://www.riotgames.com/darkroom/1000/3d95dff2218458d50df362c0d51ff1fa:a8c27259bce243f9a0f2f4fff21d1af5/burj-khalifa.png)

News

Mar 30, 2024

### Advancing the Esports and Player Experience in MENA

We’ll see League and TFT played at the Esports World Cup, localize League of Legends in Arabic, and launch servers in the Middle East this year

Mar 30, 2024

[Devs Spill the Beans: Skarner’s New Lore \| Dev Video - LoL](https://www.riotgames.com/en/news/devs-spill-the-beans-skarners-new-lore-dev-video-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/3f2273896b4c9c1c9bdfe6f321545258:5dac6deffb0fd7c078eff74399542a4a/lol-skrn24-thumbnail-v02-textless.png)

Inside Riot

Mar 29, 2024

### Devs Spill the Beans: Skarner’s New Lore \| Dev Video - LoL

Senior narrative writer Elyse “Riot apothecarie” Lemoine talks about elevating Skarner to new heights, his role within Ixtal, and how he injects a new kind of horror in Runeterra.

Mar 29, 2024

[2 WORLDS // Clove Agent Trailer - VALORANT](https://www.riotgames.com/en/news/2-worlds-clove-agent-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/1365ff18418164a983e00d234300802c:4f75944c537bd3f82c2668f10191541e/8-2-clove-cinematic-thumb-16x9-v5.png)

News

Mar 24, 2024

### 2 WORLDS // Clove Agent Trailer - VALORANT

Make some mischief from beyond the grave with Scottish troublemaker Clove.

Mar 24, 2024

[A Brush With Fate \| Inkborn Fables Launch Cinematic - TFT](https://www.riotgames.com/en/news/a-brush-with-fate-inkborn-fables-launch-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/c8ed99478ce3a1466353b5e50341b984:78d47e93a0307629f6898fce0c4d6a0c/riot-client-t2-notext.png)

News

Mar 19, 2024

### A Brush With Fate \| Inkborn Fables Launch Cinematic - TFT

Once upon a time, a featherknight found a magic ink brush… and the courage to try again.

Mar 19, 2024

[Adjusting our LoL Esports Strategy](https://www.riotgames.com/en/news/lol-esports-strategy-adjustments-2024)

![](https://www.riotgames.com/darkroom/1000/d86a28c080173fc0f98acd2bcf923c67:247071e3a79915ea1a0e51b7a8aa5fd1/rg-esports-johnneedham-one-shot-03-2024.png)

News

Mar 14, 2024

### Adjusting our LoL Esports Strategy

Sharing our new business model for teams in the LCS, LEC, and LCK designed to create more predictable revenue for teams and a path to long-term sustainability.

Mar 14, 2024

[Celebrating Ramadan in Wild Rift Together Under the Crescent](https://www.riotgames.com/en/news/ramadan-wild-rift-event)

![](https://www.riotgames.com/darkroom/1000/0f1bba8f6c7e3e59c4073ea10b2caf43:ee157611ff5363c355e8b4291bd6aebd/celebrating-ramadan-article-header.png)

Inside Riot

Mar 11, 2024

### Celebrating Ramadan in Wild Rift Together Under the Crescent

Rioters from around the world collaborated to represent a variety of cultural traditions.

Mar 11, 2024

[Together Under the Crescent \| Event Trailer - League of Legends: WR](https://www.riotgames.com/en/news/together-under-the-crescent-event-trailer-league-of-legends-wild-rift)

![](https://www.riotgames.com/darkroom/1000/e3ce29da2906c771f2fe1dbdfa702364:27d29962dc524aed06dbe064fe69c9b2/together-under-the-crescent-event-trailer.png)

News

Mar 6, 2024

### Together Under the Crescent \| Event Trailer - League of Legends: WR

The Poros are setting the mood! Don’t miss out and celebrate with them under the crescent moon starting March 8th!

Mar 6, 2024

[Everything You Need to Know: Masters Madrid](https://www.riotgames.com/en/news/everything-you-need-to-know-masters-madrid)

![](https://www.riotgames.com/darkroom/1000/29948c240d5bb464f36a7312970b4980:23cf3bda5af01bb0b60aa3116df8becb/vct24-m1-etynk-header.jpg)

News

Mar 4, 2024

### Everything You Need to Know: Masters Madrid

Tournament match-ups, format, schedule, and more!

Mar 4, 2024

[TFT Dev Drop: Inkborn Fables I Dev Video - Teamfight Tactics](https://www.riotgames.com/en/news/tft-dev-drop-inkborn-fables-i-dev-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/3e40e817d29034f56204ad4493ee53be:deddce5640312d0d50309e6c00ab27bf/030324-tft-dev-drop-set-11-banner.jpg)

Inside Riot

Mar 3, 2024

### TFT Dev Drop: Inkborn Fables I Dev Video - Teamfight Tactics

Join us for conversations about our next set, the Rotating Shop, Chibi Morgana and more!

Mar 3, 2024

[Dev Update: Gameplay, Vanguard & More](https://www.riotgames.com/en/news/dev-update-gameplay-vanguard-more)

![](https://www.riotgames.com/darkroom/1000/b15aec28d1e1d095fe428c5e288ef552:e5bc3c2a74232075af83efe70b651964/thumbnail-textless.png)

News

Feb 29, 2024

### Dev Update: Gameplay, Vanguard & More

Riot Brightmoon, Pupulasers, and Phroxzon discuss the gameplay changes, Vanguard, bots, and skins.

Feb 29, 2024

[Project L Is Now 2XKO: Year in Preview 2024](https://www.riotgames.com/en/news/2xko-project-l-2024-preview)

![](https://www.riotgames.com/darkroom/1000/cc76dcb8df98c2c47d61ed72c27c0d09:13b2ef2dde11f74147e32ab06c908e6e/2xko-brvl24-social-3840x2160-desktopwallpaper-duo-yasuo-darius-v001a.png)

News

Feb 22, 2024

### Project L Is Now 2XKO: Year in Preview 2024

First things first: we have a name! Project L is now officially 2XKO.

Feb 22, 2024

[2XKO Social Media Wallpaper Kit](https://www.riotgames.com/en/news/2xko-wallpapers-social-kit)

![](https://www.riotgames.com/darkroom/1000/f89dcd503e2bba01d44f436544c794be:e26f912743198e7848e51836cffdda41/2xko-brvl24-social-3840x2160-desktopwallpaper-duo-ekko-ahri-v001a-1.png)

News

Feb 22, 2024

### 2XKO Social Media Wallpaper Kit

Can’t wait to play? Get a little bit closer with 2XKO wallpapers for your desktop, phone, and social media profile.

Feb 22, 2024

[Bandle Tale - Available Now!](https://www.riotgames.com/en/news/bandle-tale-available-now)

![](https://www.riotgames.com/darkroom/1000/6062e6aac1ef4c12896bfc831d70f15e:8d4e529ebfa492f23fcc19ad89ab84f3/bandletale-launch-redirect.png)

News

Feb 21, 2024

### Bandle Tale - Available Now!

With your unique knitting magic and unflappable spirit, enlist the help of new friends, restore the portals, and reunite Bandle City once more!

Feb 21, 2024

[Introducing the 2024 VCT Team Capsules // Skin Reveal Trailer - VAL](https://www.riotgames.com/en/news/2024-vct-team-capsules-skin-reveal-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/f8da1378729a8097d3ae590cfc970e1f:fa20d6a8a160b14cd76caeed3b9966f4/vct24-team-capsules-textless-1920x1080.jpg)

News

Feb 21, 2024

### Introducing the 2024 VCT Team Capsules // Skin Reveal Trailer - VAL

Make Game Night, Team Night. Rep your favorite VALORANT esports team with the all-new VCT team capsules, featuring a unique gun buddy, spray, player card, and Classic skin, designed in partnership with the VCT 2024 teams.

Feb 21, 2024

[Celebrating the Year of the Dragon in Runeterra and Beyond](https://www.riotgames.com/en/news/lunar-festival-riot-games-2024)

![](https://www.riotgames.com/darkroom/1000/0d9ad3b3c416f6e83edce947395728fc:11c062f20600dbb92aaa4042daf3a10d/lny24-banner.png)

News

Feb 9, 2024

### Celebrating the Year of the Dragon in Runeterra and Beyond

Heavenscale, Scales of Fortune, Dragon Lantern, and a TFT Set Revival. Welcome to Lunar Festival 2024.

Feb 9, 2024

[A Dragon’s Spirit \| Mini Cinematic - Teamfight Tactics](https://www.riotgames.com/en/news/a-dragons-spirit-mini-cinematic-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/e784f1b735354f4fdcaf98c077bc7fae:0c8fa39d2e86fd34174b86d1d1f17d5f/tft-14-02-mini-cine-thumbnail-generic.png)

News

Feb 8, 2024

### A Dragon’s Spirit \| Mini Cinematic - Teamfight Tactics

At the Lunar Festival, only the finest of gifts will do!

Feb 8, 2024

[Happy Year of the Dragon!](https://www.riotgames.com/en/news/happy-year-of-the-dragon)

![](https://www.riotgames.com/darkroom/1000/0d2c598ab43970646c15746d7f76be80:364d235935a1beaac8932c44d3b35124/wr24-pp5-0-happyyearofthedragon-devdiary-thumbnails-2x.png)

News

Feb 8, 2024

### Happy Year of the Dragon!

The arrival of the Dragon is a cause for big celebrations all over the world. We don’t want to miss the chance to mark this occasion with plenty of goodies on the Rift!

Feb 8, 2024

[A Prince’s Wish \| Lunar Revel 2024 Cinematic](https://www.riotgames.com/en/news/a-princes-wish-lunar-revel-2024-cinematic)

![](https://www.riotgames.com/darkroom/1000/4dbdbca3950cbfed3f083ea52f1087c2:2a2b56723a91b0a74f78d1b83988f345/011824-hs-youtube-cinematic-sm-textless.png)

News

Feb 7, 2024

### A Prince’s Wish \| Lunar Revel 2024 Cinematic

From the heavens to the earth, real magic ignites when we celebrate together.

Feb 7, 2024

[Dragon Lantern - Skin Feature](https://www.riotgames.com/en/news/dragon-lantern-skin-feature)

![](https://www.riotgames.com/darkroom/1000/f2c2a1e807cb4884d7098d907b7e4508:f07d342c47b2fe3443e0bce8a23d32c4/1920x1080-textles-2x-1.jpg)

Inside Riot

Feb 7, 2024

### Dragon Lantern - Skin Feature

The Year of the Dragon gave us the perfect occasion to marry cultural significance with some awesome designs for our Dragon Lantern Skin. Here’s a little sneak peek at what that process looked like!

Feb 7, 2024

[State of the Game 2024](https://www.riotgames.com/en/news/state-of-the-game-2024-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/f1f2bca28f5b97101e1112e644963ee3:3429d36fe260c223ccddafca64541ff6/lor-2024-stateofthegame-notext-1920x1080.png)

News

Feb 1, 2024

### State of the Game 2024

A sit-down with Riot leaders, touching on key points from LoR's history and giving players a peek at its future.

Feb 1, 2024

[Dev Update: Patch Cadence, Themes & Exciting Additions](https://www.riotgames.com/en/news/dev-update-patch-cadence-themes-exciting-additions)

![](https://www.riotgames.com/darkroom/1000/ae070bde57bac3065ae3d9732db1e86d:b03934e6ccaa6b9745b274a695514866/1920x1080-textles.png)

News

Feb 1, 2024

### Dev Update: Patch Cadence, Themes & Exciting Additions

As the new year’s kicking off we want to share some of the exciting things coming your way. Sit down with David Xu, Wild Rift Product Lead, to look back on 2023 and get excited about what the Rift will have to offer in 2024!

Feb 1, 2024

[Changes at Riot and the Road Ahead](https://www.riotgames.com/en/news/2024-player-update)

![](https://www.riotgames.com/darkroom/1000/739ecf0aaf8d6324a3c243fe3bbf8138:6f0232f63834af8b6d04669a2f084520/small-logo-black.png)

News

Jan 22, 2024

### Changes at Riot and the Road Ahead

Here’s what these changes mean for our games and what players can expect from us going forward.

Jan 22, 2024

[An Important Update about Riot’s Future](https://www.riotgames.com/en/news/2024-rioter-update)

![](https://www.riotgames.com/darkroom/1000/8f9e1eb219ffe4626e56c2f899a8cbd2:f3a80b1cd0433d2b8575739f7372c803/small-logo-red-16x9.png)

News

Jan 22, 2024

### An Important Update about Riot’s Future

The following is a copy of the internal message from our CEO.

Jan 22, 2024

[Riot’s Korea Office and the Cultural Heritage Protection Project](https://www.riotgames.com/en/news/korea-cultural-heritage-protection-project)

![](https://www.riotgames.com/darkroom/1000/f51a36a3e417033c6d9bd9f1357ba2f0:d348278a3b29eca8a8e7b10842f0fb71/20190403-04.png)

Inside Riot

Jan 16, 2024

### Riot’s Korea Office and the Cultural Heritage Protection Project

Since Riot’s Korea office opened in 2011, Rioters have worked alongside the Cultural Heritage Administration to support, protect, and restore Korea’s cultural heritage

Jan 16, 2024

[Dev Update: One Runeterra & Arcane Season 2](https://www.riotgames.com/en/news/dev-update-one-runeterra-arcane-season-2)

![](https://www.riotgames.com/darkroom/1000/e689dea3ad80f34e4806eea6c318fe6a:37caa28ae2b921122be5f431b193e4da/1920x1080-textles-0-5x.png)

News

Jan 16, 2024

### Dev Update: One Runeterra & Arcane Season 2

As the new year’s kicking off we want to share some of the exciting things coming your way. Join Andrei “Meddler” van Roon, Head of League Studio, for our Wild Rift 2024 outlook on the League ecosystem, Arcane Season 2, and other news!

Jan 16, 2024

[Introducing the Hall of Legends](https://www.riotgames.com/en/news/introducing-the-hall-of-legends)

![](https://www.riotgames.com/darkroom/1000/0f4464def389ecf6ecf5149646a45d15:ff6e555cf53b42ac3d05c91577cb2ca1/hol-textless-thumbnail.png)

News

Jan 16, 2024

### Introducing the Hall of Legends

Throughout history, we saw players ascend to the highest levels. But now, a new tier is born.

Jan 16, 2024

[Still Here \| Season 2024 Cinematic - League of Legends (ft. Forts,...](https://www.riotgames.com/en/news/still-here-season-2024-cinematic-league-of-legends-ft-forts-tiffany-aris-and-2wei)

![](https://www.riotgames.com/darkroom/1000/28063f8720f5d2499e22a277b4db1ffd:a65f3959882ca82926475804ae53ae48/sn24-cinematic-textless-thumbnail-1920x1080-aveiger-r01.png)

News

Jan 10, 2024

### Still Here \| Season 2024 Cinematic - League of Legends (ft. Forts,...

Fight for tomorrow.

Jan 10, 2024

[Riot and HP Team Up on New Global Partnership](https://www.riotgames.com/en/news/riot-HP-partnership-2024)

![](https://www.riotgames.com/darkroom/1000/3a6ee90efa59d6008fb57e7da940f7aa:661e28f6e673074dd56a343f402e348c/riot-social-1920x1080-1.png)

News

Jan 10, 2024

### Riot and HP Team Up on New Global Partnership

HP’s industry-leading brands OMEN and HyperX are joining forces with Riot to power esports events and collaborate on future products

Jan 10, 2024

[RECKONING // Episode 8 Cinematic - VALORANT](https://www.riotgames.com/en/news/reckoning-episode-8-cinematic-valorant)

![](https://www.riotgames.com/darkroom/1000/9fcb7c815cbd90d315a0d5b8119c4c2f:3b1e38fb6466e86f1e3f90ab71a61a5b/8-1-thumbnail-a-no-copy.png)

News

Jan 8, 2024

### RECKONING // Episode 8 Cinematic - VALORANT

Bring the fight and make them know your name in Episode 8: DEFIANCE.

Jan 8, 2024

[Champion Insights: Smolder](https://www.riotgames.com/en/news/champion-insights-smolder)

![](https://www.riotgames.com/darkroom/1000/cca075a8f938da7782e3c6fbf28c170e:e115de655d146f639007895986f0e053/00header-smolder.png)

Inside Riot

Jan 8, 2024

### Champion Insights: Smolder

ADC: Audacious Dragon Cutie.

Jan 8, 2024

[Talking Tactics: TFT's First Set Revival](https://www.riotgames.com/en/news/talking-tactics-tft-first-set-revival)

![](https://www.riotgames.com/darkroom/1000/70a1f0c003f4124e0ee093e8d84472f8:edb8b4611457ade1723612d2995b8672/010723-tft-lny2024-article-banner.png)

Inside Riot

Jan 7, 2024

### Talking Tactics: TFT's First Set Revival

The devs behind the revival of Return to the Stars talk about why it’s back and what’s updated!

Jan 7, 2024

[Arcane: Season 2 First Look](https://www.riotgames.com/en/news/arcane-season-2-first-look)

![](https://www.riotgames.com/darkroom/1000/d57fd65cf18d5ef656ce475d1823dc07:c49dd4b9ff5d600eb99d9996cc0e2c22/arc2-first-look-thumb-with-text.png)

News

Jan 5, 2024

### Arcane: Season 2 First Look

A nightmarish augmentation. Brace yourself for Season 2 of Arcane, coming November 2024.

Jan 5, 2024

[Season 2024 Look Ahead: Champions, Modes, Arcane & More \| Dev Video](https://www.riotgames.com/en/news/season-2024-look-ahead-champions-modes-arcane-more-dev-video-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/95ff963e21c21d9d9472e31d2f52bdca:51fb45e03cb606c6c80811d638c1c754/sn24-lookahead-textless-thumbnail-1920x1080-aveiger-r01.png)

News

Jan 5, 2024

### Season 2024 Look Ahead: Champions, Modes, Arcane & More \| Dev Video

Riot Brightmoon and Meddler are joined by members of the League team to discuss Ranked, champions, Arcane, champion mastery, modes, anti cheat, skins, and esports in 2024. Including a sneak peak at Arcane season 2, a tease about the new game mode, and a preview of the Season Start 2024 cinematic. Welcome to Season 2024.

Jan 5, 2024

[The State of the Game: LoL Esports in 2024](https://www.riotgames.com/en/news/the-state-of-the-game-lol-esports-in-2024)

![](https://www.riotgames.com/darkroom/1000/90b55c61b92728c43f900c0e74d19d46:e6a6406e6c0b11cf43c56e3ed30531ed/ss24-articleheader.png)

News

Jan 4, 2024

### The State of the Game: LoL Esports in 2024

LoL Esports kicks off the new year with competitive changes, event updates, and more!

Jan 4, 2024

[The Winners of the 2023 Charity Voting Campaign](https://www.riotgames.com/en/news/charity-voting-riot-winners-2023)

![](https://www.riotgames.com/darkroom/1000/155f74f38ada6deb9ee55a8b17b3cf7d:a90b7121fe92616ce14d273ef3fa02a4/2023-charityvotingcampaign-winners-articlebanner.png)

News

Dec 13, 2023

### The Winners of the 2023 Charity Voting Campaign

84 charities, 28 regions, $4.5M, and 589,000 player votes. Here are the charities players voted to support during the 2023 campaign

Dec 13, 2023

[A View to Worlds](https://www.riotgames.com/en/news/a-view-to-worlds)

![](https://www.riotgames.com/darkroom/1000/c51580f92b7806b0ee16d1b01b49f6f6:bb9ce99d3ed42bfe414a3b8061e7da4d/worlds-viewer-guide-banner.png)

News

Dec 11, 2023

### A View to Worlds

A viewer's guide for LoR Worlds 2023 to assist new viewers navigating the event

Dec 11, 2023

[The Vision for Teamfight Tactics: Vegas Open](https://www.riotgames.com/en/news/tft-vegas-open-2023-vision)

![](https://www.riotgames.com/darkroom/1000/d888b43fd7b5208c68ffac964c6ffd97:41644748f4b3cd46bbdde37f97da0289/alt-kv-tft-character-rgb-16x9-10-09-23.png)

Inside Riot

Dec 8, 2023

### The Vision for Teamfight Tactics: Vegas Open

512 competitors, 60 Headliners, and 1 champion. Welcome to the Teamfight Tactics Vegas Open.

Dec 8, 2023

[TFT Vegas Open: Everything You Need to Know](https://www.riotgames.com/en/news/tft-vegas-open-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/dd015e79016909271277a316dd64adee:98ad7c150ea83a9abc18d5bef8a1e9a3/tft-tvo-primerarticle-header.png)

News

Dec 4, 2023

### TFT Vegas Open: Everything You Need to Know

How to Watch, Format Explainer, Drops, Trophy Reveal and more!

Dec 4, 2023

[Marc Merrill Takes on New Role as Chief Product Officer at Riot Games](https://www.riotgames.com/en/news/marc-merrill-chief-product-officer-riot-games)

![](https://www.riotgames.com/darkroom/1000/fbb3ed5a895fe314a036218e2a56fc0d:f6e0a81d3d75c1f7f0ed856688e13fa9/marc-cpo-announcement.png)

News

Nov 29, 2023

### Marc Merrill Takes on New Role as Chief Product Officer at Riot Games

Overseeing Riot’s vision across the ecosystem, including Game Titles, Game Experiences, and Research & Development

Nov 29, 2023

[Riot Games Social Impact Fund Crosses $50M Raised](https://www.riotgames.com/en/news/50M-raised-riot-games-social-impact)

![](https://www.riotgames.com/darkroom/1000/db402ff3e82fd87271978d66a1b9fb31:35bbbfa8387ebcc92a226b0b9406d2da/rgsif-50m-articleheader-1920x1080.png)

Inside Riot

Nov 28, 2023

### Riot Games Social Impact Fund Crosses $50M Raised

Looking back on the best moments from five years of the Riot Games Social Impact Fund

Nov 28, 2023

[VALORANT, LOUDER // 2023 GAME CHANGERS CHAMPIONSHIP HYPE FILM](https://www.riotgames.com/en/news/valorant-louder-2023-game-changers-championship-hype-film)

![](https://www.riotgames.com/darkroom/1000/e4492bc7303fa10fdbf9bc00cb4ea2fe:366dd95b84fdb47040e909dab4f76c64/gcc23-promo-thumb-textless.png)

News

Nov 22, 2023

### VALORANT, LOUDER // 2023 GAME CHANGERS CHAMPIONSHIP HYPE FILM

Experience VALORANT at a whole new decibel. VCT Game Changers: The VALORANT you love, Louder.

Nov 22, 2023

[Celebrating Filipino American History Month 2023 at Riot](https://www.riotgames.com/en/news/filipino-american-history-month-2023)

![](https://www.riotgames.com/darkroom/1000/c9d56f252d9fc14613e95c9679ca8e19:550feac93f9c4f219fe727769a250aaf/fahm-article-header-2.png)

Inside Riot

Nov 21, 2023

### Celebrating Filipino American History Month 2023 at Riot

From Pork Adobo to cultural artwork, here’s how Filipinos at Riot celebrated FAHM this year

Nov 21, 2023

[REMIX RUMBLE ft. Steve Aoki (Official Music Video) \| Teamfight Tactics](https://www.riotgames.com/en/news/remix-rumble-steve-aoki-official-music-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/756a0bd3e7441d663f283edc667031a9:394f4ffde910dcdc0a14bd77b628f1fd/111923-rg-remix-rumble-launch-marquee-youtube-thumbnail-v2.png)

News

Nov 19, 2023

### REMIX RUMBLE ft. Steve Aoki (Official Music Video) \| Teamfight Tactics

Choncc’s in it for the fun. Pengu’s in it for the fame. Neither is backing down. Yasuo, Yorick, Jhin, and Bard take center stage at the wildest music festival in the Convergence. Meanwhile Gnar, Twitch, Amumu, and more put on a show of their own! This is Remix Rumble.

Nov 19, 2023

[Champion Insights: Hwei](https://www.riotgames.com/en/news/champion-insights-hwei)

![](https://www.riotgames.com/darkroom/1000/c0f04c209853c21dccf64b180347482a:e410b5eb5b50c7ba2ae957d8a7b5a282/00header.png)

Inside Riot

Nov 19, 2023

### Champion Insights: Hwei

Every match can be a masterpiece.

Nov 19, 2023

[There’s No Place Like League](https://www.riotgames.com/en/news/theres-no-place-like-league)

![](https://www.riotgames.com/darkroom/1000/9c35c7e9037ea8fc8d112f044a31ec2c:c34b634f3da1727eb643ba883c080f27/53341668581-9d44a639dc-k.png)

News

Nov 19, 2023

### There’s No Place Like League

Looking back at the highlights of the 2023 season from Season Start through the Worlds Final

Nov 19, 2023

[Reworking the Riot ID Transition Plan](https://www.riotgames.com/en/news/reworking-the-riot-id-transition-plan)

![](https://www.riotgames.com/darkroom/1000/4b74918c5e0e37aa6b645ac681a7835b:d7b95c9a8a298c9ce71fc79d7423c89f/riotid-day-article.png)

News

Nov 17, 2023

### Reworking the Riot ID Transition Plan

We’re moving from Summoner Names to Riot IDs but making some changes to the rollout

Nov 17, 2023

[How Esports Rioters in Korea are Making Worlds 2023 One for the Books](https://www.riotgames.com/en/news/worlds-2023-events-korea)

![](https://www.riotgames.com/darkroom/1000/363466607682ff8d70c83ebb09d35c8d:d032a3845d9052bec577312ed3f990b0/league-of-legends-worlds-2023-korea-drone-show.png)

News

Nov 10, 2023

### How Esports Rioters in Korea are Making Worlds 2023 One for the Books

Now, Riot Games are working.

Nov 10, 2023

[Announcing the Winners of the Riot Games VFX Creative Contest!](https://www.riotgames.com/en/news/vfx-contest-league-valorant-2023)

![](https://www.riotgames.com/darkroom/1000/f8829fbf90543b5ea01a72e994dd840e:6b9ce28521d790a37ef011d9e57e3bb3/cc-winnersannouncement-intro.jpg)

News

Nov 9, 2023

### Announcing the Winners of the Riot Games VFX Creative Contest!

Now's your chance to showcase your VFX skills to a panel of Riot judges for a chance to win unique prizes! Check out the top three

Nov 9, 2023

[TFT Dev Drop: Remix Rumble I Dev Video - Teamfight Tactics](https://www.riotgames.com/en/news/tft-dev-drop-remix-rumble-i-dev-video-teamfight-tactics)

![](https://www.riotgames.com/darkroom/1000/17d7e99d72e42b2c331a38a3ff1f45b7:4f847aebceab413263b0c469ff7be140/tft-st1023-video-clientcard-devdrop-t2-notext.jpg)

Inside Riot

Nov 5, 2023

### TFT Dev Drop: Remix Rumble I Dev Video - Teamfight Tactics

In TFT’s newest set, you’re the DJ. Field traits like K/DA, HEARTSTEEL, and Pentakill, make powerful Headliners the star of your board, and create your own custom remix.

Nov 5, 2023

[Dev For a Day: Rusticles](https://www.riotgames.com/en/news/dev-for-a-day-rusticles)

![](https://www.riotgames.com/darkroom/1000/0cf4d0d9dda47a03d369cfada6c999c1:686276ba06877a96314b5a8fcde18ffb/dev-for-a-day-banner.png)

News

Nov 3, 2023

### Dev For a Day: Rusticles

Sit down with Rusticles who collaborated with the whole LoR team to bring to life his design and tell his story through the card game we all love.

Nov 3, 2023

[Portfolio Tips & Advice - Riot Games Internship Study Guide Vol. 3](https://www.riotgames.com/en/news/art-portfolio-tips-internships-study-guide-riot-vol-3)

![](https://www.riotgames.com/darkroom/1000/9560f518896421a3085307dcd5cac29d:caf1c9905f813b5d325a763d275ee560/up-studyguide-vol3-portfolio-tips-advice.png)

Disciplines

Nov 1, 2023

### Portfolio Tips & Advice - Riot Games Internship Study Guide Vol. 3

From UX Design to Concept Art, here’s what you need to know about submitting a portfolio for a Riot Games internship

Nov 1, 2023

[Celebrating Latin American Heritage Month with Riot Unidos](https://www.riotgames.com/en/news/riot-unidos-latin-american-heritage-2023)

![](https://www.riotgames.com/darkroom/1000/03eb1a7964e89723e5a9bb955dc8ba28:2a9d523da8ae98417b9216a3868487e6/lahm-2023.png)

Inside Riot

Oct 27, 2023

### Celebrating Latin American Heritage Month with Riot Unidos

From Jocón de Pollo to educational firesides, here’s everything Riot did during Latin American Heritage Month.

Oct 27, 2023

[HEARTSTEEL - PARANOIA ft. BAEKHYUN, tobi lou, ØZI, and Cal Scruby...](https://www.riotgames.com/en/news/heartsteel-paranoia-ft-baekhyun-tobi-lou-ozi-and-cal-scruby-official-music-video)

![](https://www.riotgames.com/darkroom/1000/63d4808b045f307f6b9a74546acf9113:a0c55f730399e617ce01f86b850be61f/hs-youtube-mv-thumb-smccrumb-v8.jpg)

News

Oct 23, 2023

### HEARTSTEEL - PARANOIA ft. BAEKHYUN, tobi lou, ØZI, and Cal Scruby...

Watch the music video for “PARANOIA” by HEARTSTEEL Kayn, Ezreal, Sett, Yone, K’Sante, and Aphelios.

Oct 23, 2023

[Planting Trees and Taking Drakes at Worlds 2023](https://www.riotgames.com/en/news/worlds-2023-sustainability-aws)

![](https://www.riotgames.com/darkroom/1000/f3642f5022b146edd7c2cb14939351d2:d3a5e39e138bdf0197623e774cab1134/worlds-take-the-drake-article-header.png)

News

Oct 19, 2023

### Planting Trees and Taking Drakes at Worlds 2023

We’re putting the Worlds Event Pass to work for reforestation and working with AWS to expand safe water access during Worlds 2023.

Oct 19, 2023

[Transitioning from Summoner Names to Riot IDs](https://www.riotgames.com/en/news/summoner-name-riot-ID)

![](https://www.riotgames.com/darkroom/1000/beab12824b8b6db9a39ec4827d1fe5a0:e46db67300f320cf01349ebc7fccbbdd/riotid-day-article-banner-1920x1080-updated.png)

News

Oct 16, 2023

### Transitioning from Summoner Names to Riot IDs

This updates player name terminology to better reflect the current world of League of Legends, creates more fluidity between Riot games, and streamlines how names work from a tech perspective.

Oct 16, 2023

[Dev Update: Champs, Lore & More](https://www.riotgames.com/en/news/dev-update-champs-lore-more)

![](https://www.riotgames.com/darkroom/1000/d4f02d5bc2568ccba83c97a22f6505bb:404c4fe54fe4ee6916320174392b197b/lol-dev-update-thumbnail-champs-lore-more.jpg)

Inside Riot

Oct 13, 2023

### Dev Update: Champs, Lore & More

Riot Brightmoon, Meddler, and guests are back with an update on champs, lore, Quickplay, and more.

Oct 13, 2023

[The Making of Riot’s New Home in Seattle](https://www.riotgames.com/en/news/riot-seattle-office-design-mercer)

![](https://www.riotgames.com/darkroom/1000/f786e98cf13fcdbdedb08ef57b65463b:be094215f8c342b1b923ef42234f3582/mercer-island-construction-37.png)

Offices

Oct 12, 2023

### The Making of Riot’s New Home in Seattle

An expert team of designers who create Riot’s offices have been working on the new office for years. Here’s some insight into their process and how it will impact the Rioters who work there.

Oct 12, 2023

[Riot Client Game Hubs](https://www.riotgames.com/en/news/riot-client-game-hubs)

![](https://www.riotgames.com/darkroom/1000/3842e6d651ce3662ac121876e11a4f75:a470271dce9a31c569422b5fdd5074ce/rg-article-header-riot-client-game-hubs.png)

News

Oct 9, 2023

### Riot Client Game Hubs

Stay connected with relevant content and experiences with new Game Hubs.

Oct 9, 2023

[Wild Rift Third Year Global Anniversary Celebrations](https://www.riotgames.com/en/news/wild-rift-third-year-global-anniversary-celebrations)

![](https://www.riotgames.com/darkroom/1000/587413e7db0a29cc5e0d4a3d0f2d7aff:5a833459b9c465bfd5fc35ad481083f4/wr-news-website-article-bannner-news-wry3-ekko-450kb.jpg)

News

Oct 5, 2023

### Wild Rift Third Year Global Anniversary Celebrations

We’re gearing up for our Global Anniversary and can’t wait to celebrate you! To kick off the festivities, our Power Spike Gallery is now live! Submit your creations to unlock anniversary rewards! Also learn about some of the other festivities and goodies we have in store!

Oct 5, 2023

[GODS ft. NewJeans (뉴진스) \| Worlds 2023 Anthem - League of Legends](https://www.riotgames.com/en/news/gods-ft-newjeans-official-music-video-worlds-2023-anthem)

![](https://www.riotgames.com/darkroom/1000/646f957748c17ee2f60653b945f37781:b2bf55a3cc868bf5b7f0328e1947cf58/gods-coverphoto.jpg)

News

Oct 4, 2023

### GODS ft. NewJeans (뉴진스) \| Worlds 2023 Anthem - League of Legends

This is the story of an unbreakable spirit and the unbending heart of a champion.

Oct 4, 2023

[“THE GRIND. THE GLORY.” Cinematic Trailer \| Worlds 2023](https://www.riotgames.com/en/news/the-grind-the-glory-cinematic-trailer-worlds-2023)

![](https://www.riotgames.com/darkroom/1000/a3ad2591b5a78c8727a64dc8669a4852:0bf6ad5015a40063a6da63d4738de4c8/lol-w23-thumbnail-cinematictrailer-v2.jpg)

News

Sep 28, 2023

### “THE GRIND. THE GLORY.” Cinematic Trailer \| Worlds 2023

There’s no glory without the grind. Worlds 2023 begins October 10th.

Sep 28, 2023

[Update on Our CEO Transition](https://www.riotgames.com/en/news/update-on-our-ceo-transition)

![](https://www.riotgames.com/darkroom/1000/b77d93b2d720324ab07fc3a13aa13c4f:cc49e7d5c27749114fe5c765b0bb90b6/rg-nicoloannouncement-2023q4.png)

News

Sep 22, 2023

### Update on Our CEO Transition

Following our CEO announcement in May, Nicolo Laurent and Dylan Jadeja have worked closely together over the past four months to ensure a smooth leadership transition for Riot.

Sep 22, 2023

[Worlds 2023 Primer](https://www.riotgames.com/en/news/worlds-2023-primer)

![](https://www.riotgames.com/darkroom/1000/d9155c234f3d750d5960e3a32625ff92:11fd71456886ee78bbaf21f23ab3e377/w23-primer-header-img-v1-0.jpg)

News

Sep 14, 2023

### Worlds 2023 Primer

Learn more about the 2023 League of Legends World Championship in South Korea.

Sep 14, 2023

[So You Want to Attend the TFT Vegas Open?](https://www.riotgames.com/en/news/so-you-want-to-attend-the-tft-vegas-open)

![](https://www.riotgames.com/darkroom/1000/f1526f95455fb9075a79b8f003f6b37e:7801f8dc0253f3684028a5afec9820ba/tvo-reg-announcement-article-01.jpg)

News

Aug 31, 2023

### So You Want to Attend the TFT Vegas Open?

Event info, registration, format, prizing, and more!

Aug 31, 2023

[Champion Insights: Briar](https://www.riotgames.com/en/news/champion-insights-briar)

![](https://www.riotgames.com/darkroom/1000/eb6c88f868b3eb6c311def44881b0e55:a57ced2e2c673c0bf9947e48c52d88bd/header-image.png)

Inside Riot

Aug 30, 2023

### Champion Insights: Briar

Thirsty for blood, hungry for life.

Aug 30, 2023

[SUNSET // Official Map Trailer - VALORANT](https://www.riotgames.com/en/news/sunset-official-map-trailer-valorant)

![](https://www.riotgames.com/darkroom/1000/1d74efc67b51756abd2bc8aa788cf2b3:4da45b75236dbd972c2475338bf78d02/082623-7-1-sunset-cg-banner.jpg)

News

Aug 26, 2023

### SUNSET // Official Map Trailer - VALORANT

The long road to Los Angeles ends here. See VALORANT’s first foray into this vibrant new map set in Gekko’s stomping grounds.

Aug 26, 2023

[Champions Weekend: Map Reveal, Fan Fest, and Grand Finals](https://www.riotgames.com/en/news/champions-weekend-map-reveal-fan-fest-and-grand-finals)

![](https://www.riotgames.com/darkroom/1000/00063c3a42518a7b81a3f6cca2ee4539:f69068fd2ebbc020b7e95aeb8f28469a/vct-r2la-header.png)

News

Aug 23, 2023

### Champions Weekend: Map Reveal, Fan Fest, and Grand Finals

A world champion will be crowned at the Kia Forum when VALORANT stars converge this weekend in Los Angeles.

Aug 23, 2023

[Teaming up with DonorsChoose to Support Gaming and Esports Programs...](https://www.riotgames.com/en/news/donors-choose-partnership-valorant-champions)

![](https://www.riotgames.com/darkroom/1000/d46962b6a37453848c50d81af28475ce:4b611b67d321ea8262dcdcb6e46af7ae/riotgames-donorschoose-announcement.png)

News

Aug 23, 2023

### Teaming up with DonorsChoose to Support Gaming and Esports Programs...

This partnership is aimed at supporting middle and high school teachers to give students access to careers in esports and gaming.

Aug 23, 2023

[How We Celebrated Pride 2023 at Riot](https://www.riotgames.com/en/news/celebrating-pride-2023-riot)

![](https://www.riotgames.com/darkroom/1000/d31de6dda5c1ef632c013398eec1780c:dc1de7eb19838883a9d46aef3039ca1a/riot-games-how-we-celebrate-pride-2023.png)

Inside Riot

Aug 22, 2023

### How We Celebrated Pride 2023 at Riot

From working with community artists to pie-inspired donations, here’s how we celebrated Pride Month in June.

Aug 22, 2023

[How a Global Esport Comes Together at VALORANT Champs 2023](https://www.riotgames.com/en/news/valorant-champs-2023-global-esport)

![](https://www.riotgames.com/darkroom/1000/079912352d14904fc4ab9030d42fdcab:70546f530ae7fd7e2887588c5739251a/53098657981-5372ccd676-o.png)

Inside Riot

Aug 18, 2023

### How a Global Esport Comes Together at VALORANT Champs 2023

From Brazil to the UK to Singapore, it feels like any team has a chance of winning Champs 2023. That’s by design.

Aug 18, 2023

[Pre-Register Now: TFT Mobile is Coming to the Asia-Pacific Region!](https://www.riotgames.com/en/news/pre-register-now-tft-mobile-is-coming-to-the-asia-pacific-region)

![](https://www.riotgames.com/darkroom/1000/dbf82678643cc81ba7e0b7bc9dfe5e83:63d22fe26670a0fb3d5a5012ef6bfcdd/tft-set923-apac-kv-1920x1080-v001-asteme.png)

News

Aug 18, 2023

### Pre-Register Now: TFT Mobile is Coming to the Asia-Pacific Region!

Pre-register for the Asia-Pacific arrival of TFT Mobile today and get rewarded.

Aug 18, 2023

[Project L @ Evo 2023 Recap](https://www.riotgames.com/en/news/project-l-demo-evo-2023-recap)

![](https://www.riotgames.com/darkroom/1000/1b1855161cb4f9d4e4313b5ee66a1d51:22a2f1969a060157c7412ba69917fdcf/projectl-evo1.png)

News

Aug 9, 2023

### Project L @ Evo 2023 Recap

Why go to Evo? It’s pretty simple: your feedback really is important to us.

Aug 9, 2023

[Soul Fighter Cosplay Guide: Round 2](https://www.riotgames.com/en/news/soul-fighter-cosplay-guide-round-two)

![](https://www.riotgames.com/darkroom/1000/927ba52fef017bb6d7325a9e721e743b:34156e77285a28890c9787a78f771b63/soul-fighter-viego-final.png)

News

Aug 9, 2023

### Soul Fighter Cosplay Guide: Round 2

Embody the Soul of these Fighters with your cosplays!

Aug 9, 2023

[Mid-Year Update 2023 \| Dev Snapshot - Legends of Runeterra](https://www.riotgames.com/en/news/mid-year-update-2023-dev-snapshot-legends-of-runeterra)

![](https://www.riotgames.com/darkroom/1000/f4d756e1f58f21e66638e4ac6563ea8e:878fe76ea18ff8995a90478a2173293f/73123-lor-2023-devsnapshot-mid-year-banner.png)

News

Jul 31, 2023

### Mid-Year Update 2023 \| Dev Snapshot - Legends of Runeterra

Game Director Dave Guskin is back with another Dev Snapshot. Watch to learn more about what we've seen and heard from you all so far in 2023, and for a look at where LoR is headed.

Jul 31, 2023

[Arena: Making a New Mode](https://www.riotgames.com/en/news/arena-making-a-new-mode)

![](https://www.riotgames.com/darkroom/1000/ba399fb71d008505d58e185fe74a80d3:dbd75febf41b563ef23eb4751750688f/soul23-arena-bts-textless-1920x1080-v01-aveiger.jpg)

Inside Riot

Jul 27, 2023

### Arena: Making a New Mode

We've got your ringside seats for a behind-the-scenes look at how we made League's new 2v2v2v2 mode, Arena.

Jul 27, 2023

[CHAMPIONS LOS ANGELES: EVERYTHING YOU NEED TO KNOW](https://www.riotgames.com/en/news/champions-los-angeles-everything-you-need-to-know)

![](https://www.riotgames.com/darkroom/1000/717c0c8a09008eef02ef49bb4c958b12:4c2e908ddc3c4dc1c27effdbb2b9c729/ch23-coverimage.jpg)

News

Jul 27, 2023

### CHAMPIONS LOS ANGELES: EVERYTHING YOU NEED TO KNOW

Get the official tournament information for Champions Los Angeles! Match ups, formats, schedules, and more!

Jul 27, 2023

[Soul Fighter Cosplay Guide: Round 1](https://www.riotgames.com/en/news/soul-fighter-cosplay-guide-round-one)

![](https://www.riotgames.com/darkroom/1000/b30b3d7bda3b0f4f4a1ee0a372c0c880:818dace12154086f8ea1f965d6afae2c/leagueoflegends-soul-fighter-cosplay-guide-part1.png)

News

Jul 27, 2023

### Soul Fighter Cosplay Guide: Round 1

Embody the Soul of these Fighters with your cosplays!

Jul 27, 2023

[Introducing Duo Play](https://www.riotgames.com/en/news/project-l-introducing-duo-play)

![](https://www.riotgames.com/darkroom/1000/6a46e6621a69c3d6656465fe0c50527c:614d5808473589a43d63b7f60c4131c0/lion-evo23-duoplay-thumbnail-1920x1080.jpg)

News

Jul 26, 2023

### Introducing Duo Play

Tom and Shaun show off duo play and the evolution of Project L, Riot’s upcoming 2D tag fighter.

Jul 26, 2023

[Tournament of Souls - Coming Alive \| Cinematic](https://www.riotgames.com/en/news/tournament-of-souls-coming-alive-cinematic)

![](https://www.riotgames.com/darkroom/1000/769be4684faf96d5e28521235f0c4973:89efc27483ef9a6c769b14652466f079/soul23-marquee-1920x1080-textless-v01-aveiger.jpg)

News

Jul 20, 2023

### Tournament of Souls - Coming Alive \| Cinematic

In the fight of a lifetime, you may get knocked down. But never knocked out.

Jul 20, 2023

[Behind the Mode: Soul Brawl](https://www.riotgames.com/en/news/behind-the-mode-soul-brawl)

![](https://www.riotgames.com/darkroom/1000/91fcd435706b148492d2672f0c2a7c1b:a8a3f66ee9689f7c23f95a9e9bb5cca8/00headerchoncc.jpg)

News

Jul 20, 2023

### Behind the Mode: Soul Brawl

It’s time for your training arc.

Jul 20, 2023

[How Underdogs Made League of Legends History \| DRX The Rise](https://www.riotgames.com/en/news/how-underdogs-made-league-of-legends-history-drx-the-rise)

![](https://www.riotgames.com/darkroom/1000/1f2fd6f895c78c52e23b3601522354e5:1cdb49fda99d6e08a1e076f832fb72f7/from-last-to-glory-v3.png)

News

Jul 12, 2023

### How Underdogs Made League of Legends History \| DRX The Rise

Presented by Red Bull Media House in Partnership with Riot Games. Entering Worlds 2022, nobody expected Deft and DRX to make it past the Play-In stage. However, their past record would not keep the determined LCK team from hoping for victory and enjoying themselves along the way. Join us as we retrace DRX’s journey from underdogs to defeating the game’s greatest teams to become the World Champions of League of Legends Esports.

Jul 12, 2023

[UNITED TOGETHER // China Launch Official Cinematic - VALORANT](https://www.riotgames.com/en/news/united-together-china-launch-official-cinematic-valorant)

![](https://www.riotgames.com/darkroom/1000/c1d8d0f02f8d69413ac2c6cfd167acad:29629ae075fbf565c92629787eee0876/val-ep7-china-cg-thumb-final.png)

News

Jul 11, 2023

### UNITED TOGETHER // China Launch Official Cinematic - VALORANT

Unite together. Save the world. Celebrate our China launch by taking a look back at VALORANT Protocol’s first steps into Haven.

Jul 11, 2023

[Shipping Xbox Game Pass](https://www.riotgames.com/en/news/shipping-xbox-game-pass)

![](https://www.riotgames.com/darkroom/1000/609775bdc63596dd4a88805c8623aa0d:88882b4dca35e7891bfa271b1b2eeeb6/110322-xbox-pass-1920x1080.png)

Tech Blog

Jun 29, 2023

### Shipping Xbox Game Pass

A technical look at how Riot’s Player Platform team helped to ship our Xbox Game Pass partnership.

Jun 29, 2023

[API at Riot Takes Center Stage for AAPI Heritage Month](https://www.riotgames.com/en/news/aapi-recap-night-market-riot)

![](https://www.riotgames.com/darkroom/1000/129d485df4f95fa22d874084ba9a256d:de115abb9fe4810a72ee86773308478e/aapi-heritage-month-2023.png)

Inside Riot

Jun 28, 2023

### API at Riot Takes Center Stage for AAPI Heritage Month

AAPI Heritage Month began with a Night Market and ended with conversations about API communities at Riot and the impact of representation

Jun 28, 2023

[DEFY DEFINITION: EPISODE 7 ACT 1](https://www.riotgames.com/en/news/defy-definition-episode-7-act-1)

![](https://www.riotgames.com/darkroom/1000/867d7420de4cb92cd431101ec0a0db29:08c590fe2300e9d7add32cd858029b09/banner-val23-ep7a1-modal-1920x1080-no-text-article.jpeg)

News

Jun 27, 2023

### DEFY DEFINITION: EPISODE 7 ACT 1

Check out how VALORANT’s evolving the game in this new Episode.

Jun 27, 2023

[The Tournament Begins: Soul Fighter 2023](https://www.riotgames.com/en/news/soul-fighter-event-league-tft)

![](https://www.riotgames.com/darkroom/1000/5b5052681c01a9d5ad5262f0d5e1f0cd:c81e1e84a43b0996fa530e79db3d84d4/soulfighter-franchise-kv-12june2023.png)

News

Jun 26, 2023

### The Tournament Begins: Soul Fighter 2023

Soul Fighter is bringing new modes, champions, stories, and skins to League of Legends, Teamfight Tactics, Wild Rift, and Legends of Runeterra.

Jun 26, 2023

[Champion Insights: Naafiri](https://www.riotgames.com/en/news/champion-insights-naafiri)

![](https://www.riotgames.com/darkroom/1000/f67c4616b1a7be895adbc425e28ceac8:3196c6fdb623d6d09971309807b3814e/naafiri-header-resized.png)

Inside Riot

Jun 22, 2023

### Champion Insights: Naafiri

There is no alpha. There is only the pack.

Jun 22, 2023

[Mastercard Expands Global Partnership to VALORANT Esports](https://www.riotgames.com/en/news/mastercard-riot-games-global-valorant)

![](https://www.riotgames.com/darkroom/1000/222235c429fd404c53ab6c17f4c2c4b3:baf759efbf77af7e78548de2db47ddb0/52726333596-253f647818-o-1.png)

News

Jun 20, 2023

### Mastercard Expands Global Partnership to VALORANT Esports

Riot Games and Mastercard have extended their long-standing partnership to now cover global events for the VALORANT Champions Tour

Jun 20, 2023

[The Making of a League of Legends Champion Theme](https://www.riotgames.com/en/news/creating-champion-themes-riot-games-music)

![](https://www.riotgames.com/darkroom/1000/9a143184990403eabe0d8cb5fbf7e50f:4914b4eeeced965459928353482edcce/making-of-a-champ-theme-16x9.png)

Inside Riot

Jun 13, 2023

### The Making of a League of Legends Champion Theme

Three Riot Games composers share how a champion theme comes to life.

Jun 13, 2023

[League of Legends x Coca-Cola Ultimate Zero Sugar: Now Available](https://www.riotgames.com/en/news/league-coca-cola-ultimate-zero-sugar-collaboration)

![](https://www.riotgames.com/darkroom/1000/e195a727aade29dd1a19cf3648fee6f8:6bf2da2ea829a432124ec5b5f1484fb4/221012-ccc-gaming2-0-kv-landscape-cmyk-preview.png)

News

Jun 8, 2023

### League of Legends x Coca-Cola Ultimate Zero Sugar: Now Available

League and Coca-Cola have teamed up to create a new flavor of Coke. Celebrate the launch with some Ultimate Emotes and more!

Jun 8, 2023

[Natalie Parker: Over a Decade of Work at Riot Games](https://www.riotgames.com/en/news/legal-riot-employee-natalie-swanson-parker)

![](https://www.riotgames.com/darkroom/1000/17ed58e1bdf65463e0443c8a2be3e7e6:8555930a90db380eaece6e9867331d0d/natalie-swanson-articleheader-16x9.png)

Inside Riot

Jun 5, 2023

### Natalie Parker: Over a Decade of Work at Riot Games

Natalie started as a receptionist and now leads a team of lawyers focused on labor and employment.

Jun 5, 2023

[Celebrating Pride with the Community](https://www.riotgames.com/en/news/celebrating-pride-community-artists)

![](https://www.riotgames.com/darkroom/1000/272ab03513cd1ba64bc3f0188d1eaa52:9f7e8088ccef96987b77b41ad0b417ef/pride2023-ksante-lolpc-ace-flag-edit.png)

News

Jun 1, 2023

### Celebrating Pride with the Community

See what’s available in game and hear from seven community artists who created unique art around our IP for Pride Month.

Jun 1, 2023

[CONV/RGENCE - Available Now!](https://www.riotgames.com/en/news/convrgence-available-now)

![](https://www.riotgames.com/darkroom/1000/e32c0b4dc92ad5d2424e56946942083b:a30a88a6bf3428bd0e44ba788f23a5b2/convrgence-en.jpg)

News

May 23, 2023

### CONV/RGENCE - Available Now!

Run, leap, and slide your way through the streets of Zaun. Encounter champions, face down enemies, and locate never-before-seen sides of the Undercity.

May 23, 2023

[Background Patching is coming to Riot Client](https://www.riotgames.com/en/news/riot-client-background-patching-valorant)

![](https://www.riotgames.com/darkroom/1000/d84e1b3a842da43f45df9ccc1ff1e5e0:169abadd989455174c68fdbd58128986/rc-background-patching-final-optomized-under5kb.png)

News

May 19, 2023

### Background Patching is coming to Riot Client

Keep your games up to date and ready to play when you are, starting with VALORANT and expanding to other games in the future.

May 19, 2023

[New Emotes for a Discord Birthday Present](https://www.riotgames.com/en/news/discord-riot-games-emotes)

![](https://www.riotgames.com/darkroom/1000/9d97c77fd87f8167e622dbac73686cdd:4b85075269a607e832aa25ccf7be3386/riot-discord-illu-1920x1080-a.jpg)

News

May 15, 2023

### New Emotes for a Discord Birthday Present

As thanks to the moderators who help to make Discord the platform of choice for gamers, Riot has created a new pack of emotes specifically for mods.

May 15, 2023

[Announcing Our Next CEO](https://www.riotgames.com/en/news/announcing-dylan-jadeja-riot-games-next-ceo)

![](https://www.riotgames.com/darkroom/1000/659ba4a277a73ea84918b1759bec6b2b:2605fe26f517f87b6f50cfb37e34bd89/arcane-s01e04-happy-progress-day02-17.jpg)

News

May 11, 2023

### Announcing Our Next CEO

We’re proud to announce that our current global president, Dylan Jadeja, will become our next CEO later this year.

May 11, 2023

[2022 Annual Impact Report](https://www.riotgames.com/en/news/2022-annual-impact-report)

![](https://www.riotgames.com/darkroom/1000/912e9461050da6f4f2d0d284f04e7a04:56eac5d583f686bfe721b88be10bf79c/riot-impactreport-2022-en-cover.png)

News

Apr 26, 2023

### 2022 Annual Impact Report

This report shows our progress as we climb the mountain toward a future for Riot, the gaming industry, and players.

Apr 26, 2023

[Building the Future of Sport at Riot Games](https://www.riotgames.com/en/news/building-the-future-of-sport-at-riot-games)

![](https://www.riotgames.com/darkroom/1000/592505fae56639af284572f290abbb6c:6cb1af3fcc39f7b7b837a024a6e441f0/rg-esports-johnneedham-one-shot-04-2023.png)

One-Shot

Apr 19, 2023

### Building the Future of Sport at Riot Games

John Needham, President of Esports at Riot Games, lays out his game plan toward building a sustainable future for the esports industry

Apr 19, 2023

[The Mageseeker - Available Now!](https://www.riotgames.com/en/news/the-mageseeker-available-now)

![](https://www.riotgames.com/darkroom/1000/df1faf9cdd27e4ed5ea712b75cbae202:ea46c5d770aafdbf6680e84f46a95ded/mageseeker-en.png)

News

Apr 18, 2023

### The Mageseeker - Available Now!

The Mageseeker is an action RPG set in the League of Legends universe. Play as Sylas and wield the chains that once bound you to liberate Demacia from tyranny.

Apr 18, 2023

[Looking Back on Women’s History Month 2023](https://www.riotgames.com/en/news/womens-history-month-riot-rad-2023)

![](https://www.riotgames.com/darkroom/1000/0246c326644245311cf3b613c9d359b2:42ef0bf6eaa87379154e7d4725d2608a/2023-whm-recaparticle.png)

Inside Riot

Apr 12, 2023

### Looking Back on Women’s History Month 2023

We celebrated Women’s History Month by hosting panels with women leaders, holding events to bring people together, and sharing women’s stories across Riot.

Apr 12, 2023

[Prime Gaming and Riot Games Run it Back](https://www.riotgames.com/en/news/prime-gaming-and-riot-games-in-game-content-esports-sponsorship)

![](https://www.riotgames.com/darkroom/1000/3b21bc61fe30b901349787ba324e2c87:9bdbaf591220f12dc56bcc2395f3dd02/riotpr-mar2023-social-twitch-1920x1080-03-17-2023.png)

News

Mar 20, 2023

### Prime Gaming and Riot Games Run it Back

In-Game Content for Riot's Biggest Titles, Esports Sponsorship, and More.

Mar 20, 2023

[From Duck Hunt to Riot Games, Maye Mac-Swiney’s Gaming Journey...](https://www.riotgames.com/en/news/maye-mac-swiney-riot-barcelona)

![](https://www.riotgames.com/darkroom/1000/d1eeab9c53f342509c0fb13a983b7079:de121b62417994e7b77129a2769b2680/2023-whm-maye-macswiney-16x9.png)

Inside Riot

Mar 15, 2023

### From Duck Hunt to Riot Games, Maye Mac-Swiney’s Gaming Journey...

With roots in Mexico and a career in Spain, games, and the people who play them, have been a throughline in Maye Mac-Swiney’s life

Mar 15, 2023

[Champion Insights: Milio](https://www.riotgames.com/en/news/champion-insights-milio)

![](https://www.riotgames.com/darkroom/1000/6ff3c57990a83611195ed08154feb388:fb25c6d22b60513e174f7963f441d93f/champion-insights-milioheader.jpeg)

Inside Riot

Mar 7, 2023

### Champion Insights: Milio

“Awww, so cute!” - You, until the late-game teamfight

Mar 7, 2023

[VCT Americas 2023 Kicks Off April 1](https://www.riotgames.com/en/news/vct-americas-2023-kicks-off-april-1)

![](https://www.riotgames.com/darkroom/1000/75fc06dd058c6d86eadddd13f6757d61:3adcc467bbfd7cfb95ef4a397e569612/vct-amer-primer-assets-header-image-2.png)

News

Mar 7, 2023

### VCT Americas 2023 Kicks Off April 1

Everything you need to know about VCT Americas

Mar 7, 2023

[Amplifying Black Voices for Black History Month](https://www.riotgames.com/en/news/black-history-month-riot-games)

![](https://www.riotgames.com/darkroom/1000/231feb0372550ecf40a01173020aaa1c:4d795630bdc377020e2cdb393f64a6ef/bhm-key-art.png)

Inside Riot

Mar 3, 2023

### Amplifying Black Voices for Black History Month

Riot Noir, our RIG for Black Rioters, spent the month highlighting Black culture, inspiring the next generation, and amplifying Black stories in gaming.

Mar 3, 2023

[How Exec. Pastry Chef Nicole Redd-McIntosh creates one-of-a-kind...](https://www.riotgames.com/en/news/nicole-redd-mcintosh-riot-games-pastry-chef)

![](https://www.riotgames.com/darkroom/1000/a52fb1de36c24b3ba6714e78996884e8:6d7879b76b82dd245b4b2e6ab8043d39/2023-bhm-noms-nicole-profile-15.png)

Inside Riot

Feb 23, 2023

### How Exec. Pastry Chef Nicole Redd-McIntosh creates one-of-a-kind...

For Black History Month, Nicole helped create dishes that hold a special place in her heart to share with Rioters on our LAX campus.

Feb 23, 2023

[How Kat Wong Found Her Home in Gaming With Riot’s Localization Team](https://www.riotgames.com/en/news/kat-riot-lgbtq-localization)

![](https://www.riotgames.com/darkroom/1000/00f4c6294e54414d04fcb27e18b61085:f49ba411afca10a78d04b3a36d4e7f92/rg-rainbowrioters-kat-wong.png)

Inside Riot

Feb 16, 2023

### How Kat Wong Found Her Home in Gaming With Riot’s Localization Team

Kat Wong and how her adventure of a career, with her wife by her side, led her to Riot.

Feb 16, 2023

[Celebrating Lunar New Year In-Game and Out](https://www.riotgames.com/en/news/lunar-revel-riot-games-lny-2023)

![](https://www.riotgames.com/darkroom/1000/34053b0c1be5e22b9a5671ff90bb8bdb:2ad3aae08d570948ee004d0e954ea694/celebrating-lny-2023.png)

Inside Riot

Feb 1, 2023

### Celebrating Lunar New Year In-Game and Out

As Runeterra Celebrates Lunar Revel, Rioters Around the World Come Together for Lunar New Year.

Feb 1, 2023

[Players Voted and These Charities Won the Lion’s Share of $6M](https://www.riotgames.com/en/news/social-impact-charity-voting-winners-2022)

![](https://www.riotgames.com/darkroom/1000/be0cdad4f93868282ee3f16b6b379d11:7ea6775bab6fc94547afa1353a9194a0/karma-charity-articlebanner.png)

News

Jan 26, 2023

### Players Voted and These Charities Won the Lion’s Share of $6M

Every nonprofit is guaranteed to receive at least $15,000 for each of the 83 charities from the Riot Games Social Impact Fund.

Jan 26, 2023

[LoL Esports: Season Kickoff & State of the Game](https://www.riotgames.com/en/news/lol-esports-season-kickoff-state-of-the-game)

![](https://www.riotgames.com/darkroom/1000/904b8aed999f3a9dc7860b1f4ea6e60b:cf74acebc81e4660888e92ad6f617fbb/sn23-state-of-the-game-ger.png)

News

Jan 10, 2023

### LoL Esports: Season Kickoff & State of the Game

Check out the latest announcements and what is in store for LoL Esports in 2023 by watching the Season Kickoff & State of the Game video.

Jan 10, 2023

[The Brink of Infinity \| Season 2023 Cinematic - League of Legends](https://www.riotgames.com/en/news/the-brink-of-infinity-season-2023-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/97cb1eecc076a09f343a8023379e0a16:58544ab553cdb80643bebc7bf656db4c/the-brink-of-infinity-thumbnail-no-text.jpg)

News

Jan 10, 2023

### The Brink of Infinity \| Season 2023 Cinematic - League of Legends

Year after year, we stand together at the edge of a new beginning. Inspired by the words and deeds of the champions who came before. It's time to make your mark—Season 2023 has arrived.

Jan 10, 2023

[/dev chat: Player Behavior \| Dev Video - League of Legends](https://www.riotgames.com/en/news/dev-chat-player-behavior-dev-video-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/1207a4e16e0b3030ed34473963602d51:23c4613dc36e494f8e1b733dc5708bc5/sn23-thumbnail-playerbehavior-textless-1920x1080-rkim-v01.jpg)

News

Jan 9, 2023

### /dev chat: Player Behavior \| Dev Video - League of Legends

Timtammonster and Zero Impact discuss player behavioral systems in League. They talk about text chat moderation/muting and other changes shipped in 2022, goals for 2023, and their dreams for League’s community.

Jan 9, 2023

[/dev chat: Midscope Updates \| Dev Video - League of Legends](https://www.riotgames.com/en/news/dev-chat-midscope-updates-dev-video-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/e5d888f1b760933d20f272f7d46016d7:1f9d384a33dd62a7240f7310845e32b1/sn23-thumbnail-midscopeupdates-textless-1920x1080-rkim-v01.jpg)

News

Jan 9, 2023

### /dev chat: Midscope Updates \| Dev Video - League of Legends

Game designers Riot Truexy, Riot Phlox, and August talk about midscope updates: Why do they exist? What is their purpose? And what’s coming next?

Jan 9, 2023

[/dev chat: State of the Game \| Dev Video - League of Legends](https://www.riotgames.com/en/news/dev-chat-state-of-the-game-dev-video-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/2f2a6c2b0a80e695fa42c7d1a00b5c4f:d3d4597863411551024051d3934eb1a8/sn23-thumbnail-sotg-textless-1920x1080-rkim-v01.jpg)

News

Jan 9, 2023

### /dev chat: State of the Game \| Dev Video - League of Legends

Summoner’s Rift Team leads Riot Phroxzon and Riot Petrie talk about the state of League of Legends, plans for positions, and goals for Season 2023.

Jan 9, 2023

[Season 2023 \| LoL Pls - League of Legends](https://www.riotgames.com/en/news/season-2023-lol-pls-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/d6c8621b65dca34d3d9cbda88ff0fcc1:24889ab71b544988a06f427032e51bb9/sn23-thumbnail-lolpls-textless-1920x1080-rkim-v01.jpg)

News

Jan 9, 2023

### Season 2023 \| LoL Pls - League of Legends

Welcome to Season 2023. Riot Brightmoon shares an update on skins and thematics, Riot Auberaun talks about changes to Ranked, and Riot Lexical gives a sneak peak at some upcoming champions.

Jan 9, 2023

[REVELATION // Episode 6 Cinematic - VALORANT](https://www.riotgames.com/en/news/revelation-episode-6-cinematic-valorant)

![](https://www.riotgames.com/darkroom/1000/18071d863d26882e7ee79f8dd23664c9:9e1b7ba995a88324fccfd0564235bb51/120522-valorant-2022-ep6-1-cinematic-banner.jpg)

News

Jan 5, 2023

### REVELATION // Episode 6 Cinematic - VALORANT

From darkness, revelation. Uncover the wonders of a forgotten world in Episode 6: REVELATION. Kicking off Jan 10, 2023.

Jan 5, 2023

[Season 2023 Kickoff \| The Madness is the Hype](https://www.riotgames.com/en/news/season-2023-kickoff-the-madness-is-the-hype)

![](https://www.riotgames.com/darkroom/1000/4ff10e1371412d5f42fd884043b94e24:7c9ac71d2992d3edce7faae1d9ae7258/sn23-hype-film-banner-textless.jpg)

News

Jan 5, 2023

### Season 2023 Kickoff \| The Madness is the Hype

Get ready for an all new adrenaline packed event featuring 9 regions over 2 days. Watch Season 2023 Kickoff on lolesports.com on January 10 + 11.

Jan 5, 2023

[Riot is Taking a Break to End the Year](https://www.riotgames.com/en/news/riot-is-taking-a-break-to-end-the-year-2022)

![](https://www.riotgames.com/darkroom/1000/af65cc63551d3996b7891f71eb2ea2e8:b4515cb1e424bd0c71f516882d8e1f66/riot-eoy-break-illo-2022.png)

Inside Riot

Dec 16, 2022

### Riot is Taking a Break to End the Year

We’re taking a break for the holidays and wanted to take this time to reflect on all the big moments from 2022.

Dec 16, 2022

[Re:presenting at AWS re:Invent](https://www.riotgames.com/en/news/re-presenting-at-aws-re-invent)

![](https://www.riotgames.com/darkroom/1000/8b82f9936efdc6187372d901c01dfbeb:0fca62c7c480131ce333863742d98d8a/reinvent-header.jpeg)

Tech Blog

Dec 14, 2022

### Re:presenting at AWS re:Invent

Earlier this month, nearly a hundred Rioters traveled to Las Vegas to attend AWS re:Invent, a weeklong conference hosted by AWS for the global cloud computing community.

Dec 14, 2022

[Now Streaming on Riot Mobile](https://www.riotgames.com/en/news/now-streaming-on-riot-mobile)

![](https://www.riotgames.com/darkroom/1000/c99180dd9326186d22ab2044e3190a85:f2c961f0caa689fed9c6795d59c07890/rg-com-lol-article-t1-cluttered-decluttered.jpg)

News

Dec 13, 2022

### Now Streaming on Riot Mobile

Watch your favorite players live on-the-go with Riot Mobile!

Dec 13, 2022

[Vote For the Cause You Want to Support!](https://www.riotgames.com/en/news/riot-games-social-impact-fund-vote-2022)

![](https://www.riotgames.com/darkroom/1000/b44009806fcf1e2a03003957cd3de777:ffcc762769ad5d6dd01745eccce84ec0/karma-charity-articlebanner.png)

News

Dec 12, 2022

### Vote For the Cause You Want to Support!

Riot Games Social Impact Fund has allotted over $6M to 84 charities in 28 regions. Your vote helps decide who gets the most.

Dec 12, 2022

[New Riot Games SEA Servers for LoL & TFT Go Live in January](https://www.riotgames.com/en/news/new-riot-games-sea-servers-for-lol-tft-go-live-in-january)

![](https://www.riotgames.com/darkroom/1000/8983729f21eee4895ab6a9d4d2e26a92:8ff3eb64d4c7b2b6eaae97028718551a/league-and-tft-transition-illustration-3.jpg)

News

Dec 12, 2022

### New Riot Games SEA Servers for LoL & TFT Go Live in January

On January 6, 2023, the transition from Garena to Riot will bring with it a month-long, prize-filled party to celebrate the new servers.

Dec 12, 2022

[Link Riot Account with Xbox Game Pass Today to Unlock Benefits](https://www.riotgames.com/en/news/link-riot-account-to-xbox-game-pass)

![](https://www.riotgames.com/darkroom/1000/e69e5ec20508361cd841bc66730f4797:2651bc6bea7cd3f81703426410705268/110322-xbox-pass-1920x1080-article.png)

News

Dec 8, 2022

### Link Riot Account with Xbox Game Pass Today to Unlock Benefits

Riot Games is coming to Xbox Game Pass! Link your Riot Account with Xbox Game Pass to unlock exclusive member-only benefits.

Dec 8, 2022

[/dev: Gameplay Basics & Tag in Project L](https://www.riotgames.com/en/news/dev-gameplay-basics-tag-in-project-l)

![](https://www.riotgames.com/darkroom/1000/b82457b06e066c8883cfe134a2e7bbc3:0585a9ff9e38d71702a5b9ed73fa93d5/projectl-eoy22-thumbnail-v010-r01-final.png)

News

Dec 5, 2022

### /dev: Gameplay Basics & Tag in Project L

Executive producer Tom Cannon and game director Shaun Rivera give an in-depth look at Project L’s gameplay and tag systems.

Dec 5, 2022

[Riot Games recognized as a Great Place to Work for Parents](https://www.riotgames.com/en/news/riot-games-recognized-as-a-great-place-to-work-for-parents)

![](https://www.riotgames.com/darkroom/1000/f4ff8e2e56e8427c1f8ce3a09c308279:2516c771fb78060a05730997d4da4587/greatplacetowork-parents-16x9.jpg)

News

Dec 2, 2022

### Riot Games recognized as a Great Place to Work for Parents

From parental leave to fertility benefits, Riot cares about current and future parents.

Dec 2, 2022

[Winners Announced Riot Games Designing for Digital Thriving Challenge](https://www.riotgames.com/en/news/winners-riot-ideo-digital-thriving-challenge)

![](https://www.riotgames.com/darkroom/1000/ac341bd28c792529f9b9a238fbc02109:0bd8973f2bb2a2e71d874dec06cf55fd/riot-games-ideo-challenge-winners-article-header.png)

News

Dec 1, 2022

### Winners Announced Riot Games Designing for Digital Thriving Challenge

IDEO and Riot are announcing the ten winners of grants for their proposals to make online spaces safer and more inclusive

Dec 1, 2022

[Black Leaders Take Central Stage at AfroTech 2022](https://www.riotgames.com/en/news/afrotech-riot-games-black-leaders-tech)

![](https://www.riotgames.com/darkroom/1000/809110dd4b4ff84bf6325c6181ce2c12:0fff7c401ab6dec0696c24b1ad437dfb/riot-games-afrotech-2022.png)

Inside Riot

Nov 22, 2022

### Black Leaders Take Central Stage at AfroTech 2022

Members of Riot Noir came together to network, build community, and connect with Black leaders across gaming and tech

Nov 22, 2022

[No Room for 1v5s: Ending the Hero Mentality in Game Dev](https://www.riotgames.com/en/news/work-life-balance-riot-games-nicolo-laurent-blog)

![](https://www.riotgames.com/darkroom/1000/e6659f8b1d87bac080e48ffc0a07dcc4:3f25f74973a38965b5d03b13b42d9661/rg-nicolo-one-shot-2022q3.png)

One-Shot

Nov 17, 2022

### No Room for 1v5s: Ending the Hero Mentality in Game Dev

Riot Games CEO Nicolo Laurent shares his thoughts on the importance of recharging, his own paternity leave, and Riot’s approach to work-life balance.

Nov 17, 2022

[Riot Games and Ubisoft Tackling Toxicity in Games With New Project](https://www.riotgames.com/en/news/riot-games-ubisoft-tackling-toxicity-in-games-with-new-project)

![](https://www.riotgames.com/darkroom/1000/059e22256cf33b8a6af860c0758a89d2:c676d923cbdb82c05a5c77861177046c/ubisoft-x-riot-16-9-1920x1080.jpg)

News

Nov 16, 2022

### Riot Games and Ubisoft Tackling Toxicity in Games With New Project

This new project will create a database to train AI systems to detect and mitigate harmful behavior in games.

Nov 16, 2022

[Sama Sama: Celebrating Filipino American History Month Together](https://www.riotgames.com/en/news/celebrating-filipino-history-month-2022)

![](https://www.riotgames.com/darkroom/1000/5e0b2ec74412a2c6715f41bd96b1b3f3:a60786beb75c79f8065fe24095c9a607/riot-games-filipino-american-hertitage-month-recap.png)

Inside Riot

Nov 11, 2022

### Sama Sama: Celebrating Filipino American History Month Together

Rioters came together in person at Riot HQ to celebrate Filipino culture. Plus, Riot is opening an office in the Philippines!

Nov 11, 2022

[LoL & TFT Will be Published by Riot Games in Southeast Asia](https://www.riotgames.com/en/news/riot-games-self-publish-league-legends-teamfight-tactics-southeast-asia)

![](https://www.riotgames.com/darkroom/1000/1d4875763dad3b2324feb914900e0e41:785328f672baa03ed4c70a4b062a884b/riot-games-self-publish-league-legends-teamfight-tactics-southeast-asia.png)

News

Nov 9, 2022

### LoL & TFT Will be Published by Riot Games in Southeast Asia

After a 12 year partnership with Garena, Riot will self-publish League and TFT in Southeast Asia. Here’s how it will impact players and how to transition your account.

Nov 9, 2022

[The Team That Makes the Worlds 2022 Broadcast Happen...](https://www.riotgames.com/en/news/meet-team-behind-worlds-2022-esports-broadcast)

![](https://www.riotgames.com/darkroom/1000/a4e88f6b04bf83f1c417e87292b85606:4b7a4dceb3862e036e33b18ae4715a20/riot-games-the-team-behind-worlds-2022-esports-broadcast-league-of-legends.png)

Inside Riot

Nov 3, 2022

### The Team That Makes the Worlds 2022 Broadcast Happen...

Putting together the global broadcast for Worlds 2022 takes a combination of talent from folks with backgrounds in music, sports, entertainment, and more.

Nov 3, 2022

[Culpa De Maggical: Javier España’s Legacy Lives on at Worlds 2022...](https://www.riotgames.com/en/news/remembering-javier-maggical-espana-2022-worlds-mexico-city)

![](https://www.riotgames.com/darkroom/1000/d3aec46bf0d73faeb43279cda6f301f4:06e664aa889461250245b733b772b5b3/ftoxdraxwaaam81.png)

Inside Riot

Nov 2, 2022

### Culpa De Maggical: Javier España’s Legacy Lives on at Worlds 2022...

With longtime Rioter Javier España passing in May, players lost a face of Riot, Riot lost one of its best, and Rioters in Mexico City lost a dear friend.

Nov 2, 2022

[How Riot Delivers a Global Broadcast for Worlds 2022](https://www.riotgames.com/en/news/riot-esports-delivering-custom-global-broadcasts-worlds-2022)

![](https://www.riotgames.com/darkroom/1000/2eed8cb1d5e52cedb429055a8552cc5c:9cfa7e63332ce31f4e198850785c59ce/52412687913-9e9c9874c0-k.jpg)

Inside Riot

Nov 1, 2022

### How Riot Delivers a Global Broadcast for Worlds 2022

Worlds 2022 has seen intense LPL vs. LCK matches, Faker and T1, LCS teams playing poorly, and so much more. Here’s how the League of Legends Esports broadcast comes together.

Nov 1, 2022

[Latin American Heritage Month and Partnering with Latinx in Gaming](https://www.riotgames.com/en/news/latin-american-heritage-month-and-partnering-with-latinx-in-gaming)

![](https://www.riotgames.com/darkroom/1000/7e1ebd85143c39115897bd04aecc9ced:25ae46297deb4b6d8eec58854473bcf2/riot-games-latin-american-heritage-month.png)

Inside Riot

Oct 25, 2022

### Latin American Heritage Month and Partnering with Latinx in Gaming

Rioters celebrated by coming together, working with our new partner, and pushing for more representation of Latin American culture in gaming.

Oct 25, 2022

[Riot Games Acquires Wargaming Sydney Studio...](https://www.riotgames.com/en/news/riot-games-acquires-wargaming-sydney-studio)

![](https://www.riotgames.com/darkroom/1000/b7d62caf51d289ab49388f6c6909898a:0d34019039e78d9ab78e13af0410e146/riot-games-sydney-austrailia-office.png)

News

Oct 17, 2022

### Riot Games Acquires Wargaming Sydney Studio...

The studio will be renamed Riot Sydney and will continue to operate out of the team’s current office.

Oct 17, 2022

[Inside The House of Worlds, Mexico City’s Premiere Worlds Watch Party](https://www.riotgames.com/en/news/inside-the-house-of-worlds-mexico-citys-premiere-worlds-watch-party)

![](https://www.riotgames.com/darkroom/1000/0d6a0762edad165bb4f936d019a8a5ce:eb3d42b72c32bd569b7dd3102a9bb118/52395811542-de096d65c0-o.png)

Inside Riot

Oct 11, 2022

### Inside The House of Worlds, Mexico City’s Premiere Worlds Watch Party

To give more fans the option to watch Worlds 2022 as it came through Mexico City, Rioters in our CDMX office put together “The House of Worlds.”

Oct 11, 2022

[Lucha Libre: Choosing a Unique Mexican Flair for Worlds 2022](https://www.riotgames.com/en/news/lucha-libre-choosing-a-unique-mexican-flair-for-worlds-2022)

![](https://www.riotgames.com/darkroom/1000/fab268f29412718502f833008d9c29e6:8953de7e43954b474e2f4e11081bf2a4/worlds-2022-playins-day1-399.png)

Inside Riot

Oct 7, 2022

### Lucha Libre: Choosing a Unique Mexican Flair for Worlds 2022

The team in Mexico City had to answer the question: “How do we welcome the Worlds to Mexico for the first time?” Lucha was the answer.

Oct 7, 2022

[Riot and IDEO Launch Open Innovation Challenge](https://www.riotgames.com/en/news/riot-and-ideo-launch-open-innovation-challenge)

![](https://www.riotgames.com/darkroom/1000/1845a8bf133888a820817c63f41d7331:93bd03c8ce5c70ae405e50bc9e41f418/riot-games-ideo-challenge-announcement.png)

News

Oct 5, 2022

### Riot and IDEO Launch Open Innovation Challenge

The Designing for Digital Thriving Challenge will champion responsible design practices that foster healthy, inclusive communities in a rapidly changing digital world.

Oct 5, 2022

[More Than a Label: Priscila Queiroz](https://www.riotgames.com/en/news/more-than-a-label-priscila-queiroz)

![](https://www.riotgames.com/darkroom/1000/29ae63963a50741cb861e00143772a7b:421b17be253d54e31fb93baa1cdf1fee/49189070076-ab32f6a1e7-o.jpg)

Inside Riot

Sep 27, 2022

### More Than a Label: Priscila Queiroz

For Latin American Heritage Month, we are telling stories of Rioters in Riot Unidos. First up, Priscila who has made a career out of curating communities and passion.

Sep 27, 2022

[Five Tips and Tricks to Help You Join the VALORANT Team](https://www.riotgames.com/en/news/five-tips-and-tricks-to-help-you-join-the-valorant-team)

![](https://www.riotgames.com/darkroom/1000/ad5fde9bb1402a573f91ead49a6e3a32:f7de61a0cc41549987471fe245571d4e/life-at-val-recruitertips-article-header.png)

Disciplines

Sep 23, 2022

### Five Tips and Tricks to Help You Join the VALORANT Team

We’re hiring! Like, a lot. Here are five tips from VALORANT’s Manager of Talent Acquisition to help you apply.

Sep 23, 2022

[Q and A with some of our VALORANT leaders](https://www.riotgames.com/en/news/q-and-a-with-some-of-our-valorant-leaders)

![](https://www.riotgames.com/darkroom/1000/faad36e5333f25fc36c6d926cf51165c:44bc4ca5463fcf2dc8c17aba2b246ef5/life-at-val-leadership-article-header.png)

Inside Riot

Sep 21, 2022

### Q and A with some of our VALORANT leaders

We asked a group of VALORANT leaders why they joined the team, what they're excited about for the future, and plenty more.

Sep 21, 2022

[Lil Nas X Becomes President of League of Legends](https://www.riotgames.com/en/news/lil-nas-x-becomes-president-of-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/3ad07c804c7bd8e856be8076d43c0cd1:8c0e409ed456c76457473b35c2d9bfbf/lnx-01.jpg)

News

Sep 15, 2022

### Lil Nas X Becomes President of League of Legends

Meet the new President of League of Legends

Sep 15, 2022

[Global Service Month 2022: From São Paulo to Shanghai](https://www.riotgames.com/en/news/global-service-month-2022-from-sao-paulo-to-shanghai)

![](https://www.riotgames.com/darkroom/1000/c0a64f2a92ef5ad03e9b1104ba906895:91cd62f1a1efedafa643ac51121633a2/cyw-riotkarma-264.JPG)

Inside Riot

Sep 15, 2022

### Global Service Month 2022: From São Paulo to Shanghai

Rioters Around the World came together to give back to their local communities by painting murals, cleaning beaches, becoming mentors, and so much more.

Sep 15, 2022

[Arcane Honored with Four Emmy® Awards](https://www.riotgames.com/en/news/arcane-honored-with-four-emmy-awards)

![](https://www.riotgames.com/darkroom/1000/2f3ebbb052e01b108ac15c7796601580:4549669e433fdb1725637d6706dcc29c/1199524-1920x1080.png)

News

Sep 6, 2022

### Arcane Honored with Four Emmy® Awards

Our hit Netflix Series from Riot Games and Fortiche Production becomes the first-ever video game IP to win a Series Emmy® for Outstanding Animated Series

Sep 6, 2022

[Riot Games Internship Study Guide Vol. 2: Tips and Tricks for Applying](https://www.riotgames.com/en/news/riot-games-internship-study-guide-vol-2-tips-and-tricks-for-applying)

![](https://www.riotgames.com/darkroom/1000/424256251c040907c2d3be46b6caea2a:f2884667c8fbee04bdbe5d5cfc35e7fa/up-posting-day-article-headers-vol-2-2.jpg)

Disciplines

Sep 1, 2022

### Riot Games Internship Study Guide Vol. 2: Tips and Tricks for Applying

Here is what you need to know to submit the best application for the US Summer Internship Program.

Sep 1, 2022

[Riot is joining forces with Take This](https://www.riotgames.com/en/news/riot-is-joining-forces-with-take-this)

![](https://www.riotgames.com/darkroom/1000/78ae7252a3489a05fa701afc2b5f1cf2:e9342b84022624564b210fb95a415bf5/rgxtakethis.png)

News

Sep 1, 2022

### Riot is joining forces with Take This

We will provide funding and resources to the nonprofit dedicated to increasing support for mental health in gaming

Sep 1, 2022

[Bug Blog: Esports Trade Issue](https://www.riotgames.com/en/news/bug-blog-esports-trade-issue)

![](https://www.riotgames.com/darkroom/1000/02f6b3a21ed190b65c7a4893f8341bc6:b5e876e56ef69c141c226630d0f0e973/tech-blog-delay-0.png)

Tech Blog

Aug 31, 2022

### Bug Blog: Esports Trade Issue

We will be walking you through a recent bug that was impacting our competitive leagues, and how we dove deep into one of League of Legends’ most legacy pieces of technology to mitigate it.

Aug 31, 2022

[Riot Games Internship Study Guide Vol. 1](https://www.riotgames.com/en/news/riot-games-internship-study-guide-vol-1-from-interns-to-full-time)

![](https://www.riotgames.com/darkroom/1000/129c5935443af674f143d1f29c71e458:2aec64825da0be125250ed217e891212/up-posting-day-article-headers-vol-1.jpg)

Disciplines

Aug 30, 2022

### Riot Games Internship Study Guide Vol. 1

On Posting Day, Riot Games US Summer Internships will open with applications. We brought previous Riot Interns to share their story.

Aug 30, 2022

[An Update on Player Dynamics](https://www.riotgames.com/en/news/an-update-on-player-dynamics)

![](https://www.riotgames.com/darkroom/1000/393b1e31aabdd610c46ae6657731371b:6dc0fabfc2d16b977f46b89481d316e1/pd-update-header.png)

News

Aug 29, 2022

### An Update on Player Dynamics

The numbers behind reports in our games and some things we are working on to make our games safer, more inclusive, more fair, and more fun.

Aug 29, 2022

[Hasan Colakoglu: From First Rioter in Turkey to Building Communities](https://www.riotgames.com/en/news/hasan-colakoglu-from-first-rioter-in-turkey-to-building-communities-around-the-world)

![](https://www.riotgames.com/darkroom/1000/c8a52714327891c242e8ee85f7563331:85c538d57ff205358685076a0a7d7159/hasan-colakoglu.png)

Inside Riot

Aug 17, 2022

### Hasan Colakoglu: From First Rioter in Turkey to Building Communities

Hasan started his career creating better experiences for Turkish gamers, himself included. Now he guides Riot’s international vision.

Aug 17, 2022

[We’re Deleting Some Very Inactive Accounts](https://www.riotgames.com/en/news/were-deleting-some-very-inactive-accounts)

![](https://www.riotgames.com/darkroom/1000/d456186c68b3e4dad739b5a77efb4c5a:41cc773cb09ca2fda82e22f7561e7385/riot-games-data-purge-deleting-old-accounts.png)

News

Aug 15, 2022

### We’re Deleting Some Very Inactive Accounts

We use your data to improve your experience and if you don’t need it, we don’t either.

Aug 15, 2022

[The Grand Opening of the SoLa Technology and Entrepreneurship Center](https://www.riotgames.com/en/news/the-grand-opening-of-the-sola-technology-and-entrepreneurship-center-powered-by-riot-games)

![](https://www.riotgames.com/darkroom/1000/08cdaaa0f6987d3d336d35dfaafa8e4b:6caa8b612bff44f6cc46125bf10a4bfd/riotgames-sola-technology-center-grand-opening-1.png)

News

Aug 12, 2022

### The Grand Opening of the SoLa Technology and Entrepreneurship Center

Funded in part by a $2.25M donation from Riot, the new center aims to inspire and develop the next generation of Black and Brown game developers, esports players, and tech professionals.

Aug 12, 2022

[Annual Diversity and Inclusion (D&I) Progress Report - 2021](https://www.riotgames.com/en/news/annual-diversity-and-inclusion-d-i-progress-report-2021)

![](https://www.riotgames.com/darkroom/1000/7d650dfba797623cfb4a155150ea8d4a:71e959ad6611b5e5eda58640e714afc3/00-d-i-report-original-english.png)

News

Aug 10, 2022

### Annual Diversity and Inclusion (D&I) Progress Report - 2021

This report is a snapshot of 2021, a building year that will set us up for what we believe will be a transformative next chapter at Riot.

Aug 10, 2022

[Rainbow Rioters Find Their Lane: Maria Pentinen](https://www.riotgames.com/en/news/rainbow-rioters-find-their-lane-maria-pentinen)

![](https://www.riotgames.com/darkroom/1000/8c5b809672be2aaa819eced68645d6c2:78e451f05c8ab0430ec19db59ccbb729/rg-rainbowrioters-maria-pentinen.png)

Inside Riot

Aug 9, 2022

### Rainbow Rioters Find Their Lane: Maria Pentinen

A gamer her entire life, Maria talks about what video games mean to her, what being an ally looks like, and her path throughout her career.

Aug 9, 2022

[Looking Back, and Forward, at Pride 2022](https://www.riotgames.com/en/news/looking-back-and-forward-at-pride-2022)

![](https://www.riotgames.com/darkroom/1000/c38d41f2f5dc3615fd093c86ec7b4682:1c1a79caaf6a7505b6457f0596d04494/riotpride-2022.png)

Inside Riot

Aug 4, 2022

### Looking Back, and Forward, at Pride 2022

Around the world and at Riot, Pride is a year-round celebration. Here’s what we did for Pride and what we are continuing to do.

Aug 4, 2022

[Riot is recognized as a Great Place to Work](https://www.riotgames.com/en/news/riot-is-recognized-as-a-great-place-to-work-2022)

![](https://www.riotgames.com/darkroom/1000/b960554385b6a41b2505d9af6045119a:e863db74248911222dd6974d7121e22c/riot-games-great-place-to-work-2022-2.png)

News

Aug 3, 2022

### Riot is recognized as a Great Place to Work

This year, Riot scored a 95% from Rioters who said it is a great place to work.

Aug 3, 2022

[/dev: Illaoi in Project L](https://www.riotgames.com/en/news/dev-illaoi-in-project-l)

![](https://www.riotgames.com/darkroom/1000/a292995e8cff5fe362be7748b72f0df3:11c2f187adf9f8d71a661450eae636d8/illaoi-dev-banner-1-web-1920x1080-ttan-v01.png)

Inside Riot

Aug 1, 2022

### /dev: Illaoi in Project L

Bringing the Kraken Priestess to a fighting game.

Aug 1, 2022

[Star Guardian Cosplay Guide: Part 2](https://www.riotgames.com/en/news/star-guardian-cosplay-guide-part-2)

![](https://www.riotgames.com/darkroom/1000/8bbf8ba11c0eb0209c7b158c3737d5c1:4cfbc9023fac988a8cadf6d11e841532/072622-star-guardian-akali-splash.png)

News

Jul 31, 2022

### Star Guardian Cosplay Guide: Part 2

Find all the outfit details for each member of this year's class in the second part of the official Star Guardian cosplay guide!

Jul 31, 2022

[Improving the Esports Experience with Remote Broadcast Centers...](https://www.riotgames.com/en/news/improving-the-esports-experience-with-project-stryker-and-aws)

![](https://www.riotgames.com/darkroom/1000/6b404e2be4a82ee353186d2df36b2167:975334871264ee2c7c577c6b93504bb6/riot-games-project-stryker.png)

News

Jul 20, 2022

### Improving the Esports Experience with Remote Broadcast Centers...

The first Remote Broadcast Center facility, powered by AWS, just opened in Dublin

Jul 20, 2022

[Star Guardian Cosplay Guide: Part 1](https://www.riotgames.com/en/news/star-guardian-cosplay-guide-part-1)

![](https://www.riotgames.com/darkroom/1000/3e958d74b324d8a725b4cbd4aaf2ca02:54a31f45a5c8658b38410581234d6880/starguardiankaisafinalpbechanges-1920-x-1080.png)

News

Jul 17, 2022

### Star Guardian Cosplay Guide: Part 1

Find all the outfit details for each member of this year's class in part 1 of this official Star Guardian cosplay guide!

Jul 17, 2022

[Hope Burns Brighter Than Fear in Star Guardian 2022](https://www.riotgames.com/en/news/hope-burns-brighter-than-fear-in-star-guardian-2022)

![](https://www.riotgames.com/darkroom/1000/1f9c3a260cb6e7a4da054ecbcdfa1c05:8c1fc2c1679e87a750e255368f14254c/sg22-franchise-kv-final.jpg)

News

Jul 12, 2022

### Hope Burns Brighter Than Fear in Star Guardian 2022

Learn more about everything Star Guardian has to offer this summer!

Jul 12, 2022

[Riot is Testing Out a New Affiliate Product](https://www.riotgames.com/en/news/riot-is-testing-out-a-new-affiliate-product)

![](https://www.riotgames.com/darkroom/1000/c1669c5b1bf270998876bb51abd2f20e:74c529385862aba78044e35afb3a0bbd/rpp-collections-article-image.jpg)

News

Jul 12, 2022

### Riot is Testing Out a New Affiliate Product

We’re testing a new way for our partners to promote content.

Jul 12, 2022

[We’re Taking a Break, BRB](https://www.riotgames.com/en/news/were-taking-a-break-brb)

![](https://www.riotgames.com/darkroom/1000/4fca8bbb17787287d451927912a38854:b3eb25fe56accb363ce8bda6d8796dd2/riot-mid-year-break-2022.png)

News

Jun 30, 2022

### We’re Taking a Break, BRB

It’s time for our annual summer break because sunburns beat burn out anyday

Jun 30, 2022

[2021 Social Impact Report](https://www.riotgames.com/en/news/2021-social-impact-report)

![](https://www.riotgames.com/darkroom/1000/cf60ddf6fe87eb89ecffaf1b4e704ec0:6566c5692d9a4de2607f2a2cc2c3ea29/screen-shot-2022-06-13-at-12-21-29-pm.png)

Inside Riot

Jun 28, 2022

### 2021 Social Impact Report

A look at how Riot was able to help mobilize communities and platforms in 2021 to create ripple effects far beyond our office walls or digital spaces.

Jun 28, 2022

[Rainbow Rioters Find Their Lane: Patrick Ryan](https://www.riotgames.com/en/news/rainbow-rioters-find-their-lane-patrick-ryan)

![](https://www.riotgames.com/darkroom/1000/79170c135b86e2fbcf06a0fa8cf8fcf5:63b952d0ed996504b470f0e8f3e29fef/rg-rainbowrioters-patrick-ryan.png)

Inside Riot

Jun 22, 2022

### Rainbow Rioters Find Their Lane: Patrick Ryan

From finance to Riot, Patrick Ryan’s career has evolved alongside his relationship

Jun 22, 2022

[Riot and Xbox are Coming Together on Game Pass](https://www.riotgames.com/en/news/riot-coming-to-game-pass)

![](https://www.riotgames.com/darkroom/1000/8136f3cda272877a691918dada74750a:4ba7d1d2166963ed7aa6c988770d1535/allgames-card-v6-final-lesstext-1920.jpg)

News

Jun 13, 2022

### Riot and Xbox are Coming Together on Game Pass

Unlocked content coming to Game Pass subscribers across Riot’s biggest games on PC and mobile

Jun 13, 2022

[Henna, Cats, and the Evil Eye, How Rioters in Turkey Helped Create...](https://www.riotgames.com/en/news/henna-cats-and-the-evil-eye-how-rioters-in-turkey-helped-create-valorants-fade)

![](https://www.riotgames.com/darkroom/1000/c6daec14635c52894ad2c81793749da4:2d21efeac5f91fa9410c9472ab8ab0ba/00-header-alt.jpg)

Inside Riot

Jun 10, 2022

### Henna, Cats, and the Evil Eye, How Rioters in Turkey Helped Create...

To make a Turkish agent, the VALORANT team talked with Turkish Rioters to create all the little details around Fade

Jun 10, 2022

[Choosing our lane: 2022 so far](https://www.riotgames.com/en/news/choosing-our-lane-2022-so-far)

![](https://www.riotgames.com/darkroom/1000/fe94628bf5b1f53b29f38afad8267909:f781d5a74306aa812693304af8828e9d/rg-nicolo.png)

One-Shot

Jun 9, 2022

### Choosing our lane: 2022 so far

Reflections from Riot’s CEO on in-person collaboration, our all-company trip to Barcelona, results of queue dodge, and more

Jun 9, 2022

[Asian American and Pacific Islander Heritage Month Recap](https://www.riotgames.com/en/news/asian-american-and-pacific-islander-heritage-month-recap)

![](https://www.riotgames.com/darkroom/1000/1f7b60367fa9f31c5f59a10dbcc340dc:422b751fd6274936649fe9fd4726786c/lny-sage-3-nightmarket.jpg)

Inside Riot

Jun 3, 2022

### Asian American and Pacific Islander Heritage Month Recap

To celebrate, we held talks, ate diverse food, and, most importantly, created a new RIG for API Rioters

Jun 3, 2022

[Celebrating Pride Together](https://www.riotgames.com/en/news/celebrating-pride-together)

![](https://www.riotgames.com/darkroom/1000/056b96aab9c107bfb72c1cc818be712a:5d0fc19e89d306a529ec507fe5cb5d08/tf-graves-pride-0.png)

News

May 31, 2022

### Celebrating Pride Together

We’re celebrating Pride in-game and out, here’s what you can expect throughout Pride month.

May 31, 2022

[Aim Lab and Riot Headed to the Next Level](https://www.riotgames.com/en/news/aim-lab-and-riot-headed-to-the-next-level)

![](https://www.riotgames.com/darkroom/1000/2353ba37b71a758b0fe3264fc9eab970:f732362f123581737d871cf460b1c720/rg-statespace-logo.png)

News

May 18, 2022

### Aim Lab and Riot Headed to the Next Level

Riot will become a minority shareholder of Statespace, the developers of Aim Lab.

May 18, 2022

[Being Multi-Minded: How Player Dynamics Designers Think](https://www.riotgames.com/en/news/being-multi-minded-how-player-dynamics-designers-think)

![](https://www.riotgames.com/darkroom/1000/b937236a1277907bad574b27f2931d70:5bea9c1d9e6b0cf7a25604b1ee61fe32/quinn-battle-boss-lv1.png)

News

May 16, 2022

### Being Multi-Minded: How Player Dynamics Designers Think

The second part of our series bringing more transparency to player dynamics and how it helps people play well together in game

May 16, 2022

[Player Dynamics Design: Looking Behind the Curtain](https://www.riotgames.com/en/news/player-dynamics-design-looking-behind-the-curtain)

![](https://www.riotgames.com/darkroom/1000/f229c7da368578501a0cead04a8dd93a:0eb8f0f465a90bd4b68018ac133e084c/arcade-riven-vs-blitz-splash.jpg)

News

May 12, 2022

### Player Dynamics Design: Looking Behind the Curtain

The evolution of Player Dynamics design and how it helps people play well together in games

May 12, 2022

[LA Times: Riot Games grows Los Angeles base](https://www.riotgames.com/en/work-with-us/life-at-riot/la-times-tour)

![](https://www.riotgames.com/darkroom/1000/104cd144fd0c802cd790d6d10ef6a3d0:2fc5302bb10f13488793d7c2b6e129df/la-times-image.webp)

Offices

May 9, 2022

### LA Times: Riot Games grows Los Angeles base

The LA Times recently took a tour of our LA office and learned about what makes Rioters tick.

May 9, 2022

[Two Riot Initiatives Honored by Fast Company](https://www.riotgames.com/en/news/two-riot-initiatives-honored-by-fast-company)

![](https://www.riotgames.com/darkroom/1000/606d94d7e639a708f893fe76ede86081:688b50f3fd0d9f198a608ceb932daebd/rg-fastco-award.png)

News

May 3, 2022

### Two Riot Initiatives Honored by Fast Company

VALORANT Game Changers and the Sentinels of Light campaign have been named to the World Changing Ideas list in 2022

May 3, 2022

[Women's History Month](https://www.riotgames.com/en/news/womens-history-month-2022)

![](https://www.riotgames.com/darkroom/1000/69b3a99a57801771f1fe226141f8c53d:782820e31fa3d944c4f4bd33f997eb3b/whm-riotnet-graphic.png)

Inside Riot

Apr 13, 2022

### Women's History Month

Women's History Month In March, Rioters from around the world came together to celebrate Women's History Month.

Apr 13, 2022

[How Rioters in Brazil Helped Create Raze, the Dancing Salvadorian...](https://www.riotgames.com/en/news/how-rioters-in-brazil-helped-create-raze-the-dancing-salvadorian-duelist)

![](https://www.riotgames.com/darkroom/1000/659ad672084360e550482dc6c1269f88:0f460d45caf9ccd4b16b8c4d2ab3b861/marquee-raze-thumb.png)

Inside Riot

Apr 7, 2022

### How Rioters in Brazil Helped Create Raze, the Dancing Salvadorian...

Every VALORANT player has thoughts about the flashy duelist. But in Brazil, where Raze is from, she means something more

Apr 7, 2022

[Riot Games Announces New Equity Investment in “Arcane” Animation...](https://www.riotgames.com/en/news/riot-games-announces-new-equity-investment-in-arcane-animation-studio-fortiche-production)

![](https://www.riotgames.com/darkroom/1000/871b25abd81a6bb6ea08afd510103759:fed52514ff77ef70bc68d6ce1b551c99/fortichexrg-4.png)

News

Mar 14, 2022

### Riot Games Announces New Equity Investment in “Arcane” Animation...

Companies Deepen Longtime Partnership as Riot Takes Significant, Non-Controlling Stake and Board Advisory Role

Mar 14, 2022

[Black History Month Recap](https://www.riotgames.com/en/news/black-history-month-recap)

![](https://www.riotgames.com/darkroom/1000/9518315d49af0249d7d817abe770f4c1:ca56d0266827683b8f8ba57d14c37091/noir-bhm22-keyart-16x9.jpg)

Inside Riot

Mar 7, 2022

### Black History Month Recap

In February, Rioters came together to celebrate Black History Month and shine a light on the Black community in gaming.

Mar 7, 2022

[Riot is Donating Battle Pass Sales to Humanitarian Relief...](https://www.riotgames.com/en/news/riot-is-donating-battle-pass-sales-to-humanitarian-relief-in-eastern-europe)

![](https://www.riotgames.com/darkroom/1000/e7634d6f7bba785db9ee4002a6388906:94d7cf2b567501d84e201112c4d87d71/2022-blurry-tigger-new-logo.png)

News

Mar 5, 2022

### Riot is Donating Battle Pass Sales to Humanitarian Relief...

We’re also contributing $1M more to help communities impacted by the war

Mar 5, 2022

[Getting Into the Guts of Berserk](https://www.riotgames.com/en/news/getting-guts-berserk)

![](https://www.riotgames.com/darkroom/1000/3fcb87bd6de793e215dab0ca7a70b3a1:5b69a02c2b25d682f3848fe057983459/renata-glasc-hero-image.png)

Tech Blog

Mar 1, 2022

### Getting Into the Guts of Berserk

We'll cover how we went from a hacky prototype spell to a game-changing ultimate that’s built to last.

Mar 1, 2022

[Riot is going AFK for a week in March](https://www.riotgames.com/en/news/riot-is-going-afk-for-a-week-in-march)

![](https://www.riotgames.com/darkroom/1000/cfd805a52999d7a2ae4b01f66432e1af:e0f3dadbb6b809e80cb89bf450ae3eef/riot-afk.png)

News

Feb 28, 2022

### Riot is going AFK for a week in March

We’re holding a series of company-wide meetings to connect, reflect, and discuss our goals.

Feb 28, 2022

[Riot Games Expands Publishing Business Into Asia Pacific](https://www.riotgames.com/en/news/riot-games-expands-publishing-business-into-asia-pacific)

![](https://www.riotgames.com/darkroom/1000/e7634d6f7bba785db9ee4002a6388906:94d7cf2b567501d84e201112c4d87d71/2022-blurry-tigger-new-logo.png)

News

Feb 25, 2022

### Riot Games Expands Publishing Business Into Asia Pacific

With a focus on hyperlocal publishing, Riot expands its Southeast Asia business to Asia Pacific, including Japan and new offices in India, Philippines, Indonesia/Malaysia and Thailand.

Feb 25, 2022

[Champion Insights: Renata Glasc](https://www.riotgames.com/en/news/champion-insights-renata-glasc)

![](https://www.riotgames.com/darkroom/1000/b5aa918b0d36f9632c95e5342dd8aa23:97b51248978c14720482e5f75896e6ca/2172022-renatachampinsightsarticle-00-header-web.jpeg)

Inside Riot

Feb 18, 2022

### Champion Insights: Renata Glasc

Gaslight. Gatekeep. Girlboss.

Feb 18, 2022

[Talking Tactics: The Game Dev’s Dilemma](https://www.riotgames.com/en/news/talking-tactics-the-game-devs-dilemma)

![](https://www.riotgames.com/darkroom/1000/6d3841c7cdf06033cd0e8f6c722d4c17:3124bca278c67b38fc98c0e6a2f8fe68/2-11-2022-tft-dev-dilemma.jpeg)

Inside Riot

Feb 14, 2022

### Talking Tactics: The Game Dev’s Dilemma

Mort reflects on the expectation to be highly ranked as a dev.

Feb 14, 2022

[From Vi to Diana, Creativity & Creators are the Heart of Riot Music](https://www.riotgames.com/en/news/from-vi-to-diana-creativity-and-creators-are-the-heart-of-riot-musics-sessions)

![](https://www.riotgames.com/darkroom/1000/f141d1c18406c23609f64d0cca8471e9:47a9d1448c9e911c4805e5db8b62cb88/riot-music-sessions-diana.jpg)

Inside Riot

Feb 10, 2022

### From Vi to Diana, Creativity & Creators are the Heart of Riot Music

Sessions: Diana shows a contemplative Diana exploring the wilderness with a notebook by her side as a two hour soundtrack accompanies her journey

Feb 10, 2022

[Two-Factor Authentication Has Arrived](https://www.riotgames.com/en/news/multi-factor-authentication-has-arrived)

![](https://www.riotgames.com/darkroom/1000/e48c692a3b78451652c5c8cb9322012c:0c32ce090c1259ed46cc50c9cffe8782/mfa-article-banner.jpg)

News

Jan 20, 2022

### Two-Factor Authentication Has Arrived

Protect your account and information with our new MFA system.

Jan 20, 2022

[Riot Games Announces Partnership with SoLa Impact’s I CAN Foundation](https://www.riotgames.com/en/news/riot-games-announces-partnership-with-sola-impacts-i-can-foundation)

![](https://www.riotgames.com/darkroom/1000/a855fa104755be8b0730cb0fa4a1d7c5:ac14521c8c9465f42175935ad51c1559/rgxsola-whitelogo.jpg)

News

Jan 13, 2022

### Riot Games Announces Partnership with SoLa Impact’s I CAN Foundation

Riot Games has teamed up with SoLa Impact’s I CAN Foundation to Fund South LA’s First Technology and Entrepreneurship Center to open later this month

Jan 13, 2022

[Calling the Shot: the Next Five Years at Riot Games](https://www.riotgames.com/en/news/calling-the-shot-the-next-five-years-at-riot-games)

![](https://www.riotgames.com/darkroom/1000/750cb6bfbe8b2a989b3e981dc6deb805:5099ff9c7bae06157d2d8153a711ec45/rg-nicolo-2.png)

One-Shot

Jan 11, 2022

### Calling the Shot: the Next Five Years at Riot Games

CEO Nicolo Laurent reflects on wins, losses, and the years ahead

Jan 11, 2022

[The Call \| Season 2022 Cinematic – League of Legends](https://www.riotgames.com/en/news/the-call-season-2022-cinematic-league-of-legends)

![](https://www.riotgames.com/darkroom/1000/0b792eece02bf2f10013a15a37cd70f0:69a030d675b9f8d800492275011960b5/ss22-thecall-thumbnail-gen.jpg)

News

Jan 7, 2022

### The Call \| Season 2022 Cinematic – League of Legends

For the fallen. For the deserted. For the downtrodden. And for those who will rise again. Season 2022 is here. How will you answer the Call?

Jan 7, 2022

[/dev: The Latest on Udyr's VGU](https://www.riotgames.com/en/news/dev-the-latest-on-udyrs-vgu)

![](https://www.riotgames.com/darkroom/1000/27d8c6f753d0544e33c2761904745128:6e99ec021a7bc63815b98ec6308b8caa/01-udyr-banner.jpeg)

News

Jan 7, 2022

### /dev: The Latest on Udyr's VGU

An update on Udyr’s VGU, including character modeling, animation, visual effects, and concept art.

Jan 7, 2022

[Spark - NEON Agent Trailer // VALORANT](https://www.riotgames.com/en/news/spark-neon-agent-trailer)

![](https://www.riotgames.com/darkroom/1000/a67c41acae0d1abdd4a95999a102d69a:bd387b7eaeb5b3c36d6ec6d8dc451f4c/010522-neontrailer-banner-v3.jpg)

News

Jan 7, 2022

### Spark - NEON Agent Trailer // VALORANT

Welcome to VALORANT, Neon. Our newest Manila-born Agent is sprinting onto the scene with Episode 4: Disruption. Lace up and get ready.

Jan 7, 2022

[Zeri: The Spark of Zaun \| Champion Trailer – League of Legends](https://www.riotgames.com/en/news/zeri-the-spark-of-zaun-champion-trailer)

![](https://www.riotgames.com/darkroom/1000/f2291c61396cf7323810ed478a373309:b6f611a013590f7c99e38ac8cee31d9b/ss22-zeri-champion-thumbnail-no-text-1.jpg)

News

Jan 7, 2022

### Zeri: The Spark of Zaun \| Champion Trailer – League of Legends

Lightning always strikes twice. Or three to seven times.

Jan 7, 2022

[Gameplay in Season 2022 \| Dev Video – League of Legends](https://www.riotgames.com/en/news/gameplay-in-season-2022-dev-video)

![](https://www.riotgames.com/darkroom/1000/c11d9acd6c2fe618e9db1cab9df484cb:e8a4c6a455ef1b0cf9b9b931040500f8/sn2022-lolpc-gameplay-thumbnail-gen.jpg)

News

Jan 7, 2022

### Gameplay in Season 2022 \| Dev Video – League of Legends

Hop in with Brightmoon to recap some of League’s recent updates and gameplay changes.

Jan 7, 2022

[Skins & Events in Season 2022 \| Dev Video – League of Legends](https://www.riotgames.com/en/news/skins-events-in-season-2022-dev-video)

![](https://www.riotgames.com/darkroom/1000/db249ca3665a0e00e4041d280cd88ac5:e8a6bd55a0146671b3b137db4a7281b7/sn2022-lolpc-pie-thumbnail-gen.jpg)

News

Jan 7, 2022

### Skins & Events in Season 2022 \| Dev Video – League of Legends

Join Bellissimoh for a first look at 2022’s skins, thematics, and events across League of Legends.

Jan 7, 2022

[Champions in Season 2022 \| Dev Video – League of Legends](https://www.riotgames.com/en/news/champions-in-season-2022-dev-video)

![](https://www.riotgames.com/darkroom/1000/e5c0a0c230672ad5724cebb8efa1342e:fc77a8e1f57e5f66082b49f6780d463e/sn2022-lolpc-champions-thumbnail-gen.jpg)

News

Jan 7, 2022

### Champions in Season 2022 \| Dev Video – League of Legends

Reav3 covers 2022's upcoming champions and VGUs—and introduces Zeri, an electrifying bot lane carry.

Jan 7, 2022

[Wild Rift in Season 2022 \| Dev Video – League of Legends: Wild Rift](https://www.riotgames.com/en/news/wild-rift-in-season-2022-dev-video)

![](https://www.riotgames.com/darkroom/1000/d18f99731f71347859b801e2fa0ee831:d51fb2c19dce6d6414a7949c74159bf5/sn2022-wildrift-thumbnail-gen.jpg)

News

Jan 7, 2022

### Wild Rift in Season 2022 \| Dev Video – League of Legends: Wild Rift

Uncover the newest Wild Rift updates, skins, and events for Season 2022. Look forward to new gameplay changes with the Elemental Rift, as well as new champions—including Sett and Yuumi.

Jan 7, 2022

[Season Start with Leo Faria \| Wild Rift Esports](https://www.riotgames.com/en/news/season-start-with-leo-faria-wild-rift-esports)

![](https://www.riotgames.com/darkroom/1000/a7b3a6cbbe1899375f603a815c21ab00:013bbc30a697155cbd7c5129652b47c9/sn2022-esports-thumbnail-wr-gen.jpg)

News

Jan 7, 2022

### Season Start with Leo Faria \| Wild Rift Esports

Check out what is in store for season one of Wild Rift Esports!

Jan 7, 2022

[League of Legends Esports in Season 2022 \| Esports – Riot Games](https://www.riotgames.com/en/news/league-of-legends-esports-in-season-2022)

![](https://www.riotgames.com/darkroom/1000/ed92279420b7cb591c0523025374a549:9bcbe4d285017d410f2c477fb5b60bb9/sn2022-lolpc-esports-thumbnail-gen.jpg)

News

Jan 7, 2022

### League of Legends Esports in Season 2022 \| Esports – Riot Games

Claim your season. Take a look at highlights from 2021 and look towards Season 2022 for League of Legends esports.

Jan 7, 2022

[Riot’s Opening an Office in the Seattle Area](https://www.riotgames.com/en/news/riots-opening-an-office-in-the-seattle-area)

![](https://www.riotgames.com/darkroom/1000/6658bd07ca0debedc0717770ffbd4592:baf98ab92a8a7b37b3c38059b2c12912/riot-games-seattle-office-announcement.jpg)

News

Dec 9, 2021

### Riot’s Opening an Office in the Seattle Area

The new office will house 400+ employees at the heart of Mercer Island, in between Bellevue and Seattle.

Dec 9, 2021

[RiotX Arcane: Until Next Time](https://www.riotgames.com/en/news/riotx-arcane-until-next-time)

![](https://www.riotgames.com/darkroom/1000/0a8b4600c88cc7e4bd392b90405f892a:aa7ab93ba3d3e8a04ce6d43862b1b73f/11-20-embargo-riot-riotxarcane-phase02-4k.jpg)

News

Nov 22, 2021

### RiotX Arcane: Until Next Time

Thank you to everyone who has watched Arcane and for making this an unforgettable moment. We’ve never been more excited about the future.

Nov 22, 2021

[Project L /dev: Finding our Game](https://www.riotgames.com/en/news/project-l-dev-finding-our-game)

![](https://www.riotgames.com/darkroom/1000/1ebd5a4795ff7c0ab3452a8fb66d62ec:66e7f57d1d08204a74b773ac9b942fb4/xp1-yt-thumbnails-r21-projectl-textless.jpg)

News

Nov 20, 2021

### Project L /dev: Finding our Game

As part of Undercity Nights, we’re sharing an update on Riot’s upcoming fighting game.

Nov 20, 2021

[Filipino American History Month Recap](https://www.riotgames.com/en/news/filipino-american-history-month-recap)

![](https://www.riotgames.com/darkroom/1000/8bd730f154c8e4427e9fc2860d8c1987:ae87555b4cc6bdbe8cbc6340934f1e62/riotgames-article-2880x1621.png)

Inside Riot

Nov 19, 2021

### Filipino American History Month Recap

To celebrate Filipino American History Month, Filipinos At Riot hosted activities centered around this year’s theme: A Quest for Emergence.

Nov 19, 2021

[Ruined King: A League of Legends Story™](https://www.riotgames.com/en/news/ruined-king-a-league-of-legends-story-available-now)

![](https://www.riotgames.com/darkroom/1000/119aa629ef0f3050212cbc7e8d07fa79:8a5566f2d8aa2e2ab9d083d229696e4e/rk-keyart-deluxe-16x9.jpg)

News

Nov 19, 2021

### Ruined King: A League of Legends Story™

Available Now

Nov 19, 2021

[Hextech Mayhem](https://www.riotgames.com/en/news/hextech-mayhem)

![](https://www.riotgames.com/darkroom/1000/6d1f7e37154b5f12a494800c4c033a86:08ea2075a40bc23e36fe09a2294929be/210820-jfg-ka.jpg)

News

Nov 19, 2021

### Hextech Mayhem

Available Now

Nov 19, 2021

[Global Undercity Nights Broadcast Co-Streaming and Rewards](https://www.riotgames.com/en/news/global-undercity-nights-broadcast-co-streaming-and-rewards)

![](https://www.riotgames.com/darkroom/1000/ebeeedc0abc13b93f77565f91432fe4b:98484bc17ca7b8003b519c60c8271fe4/riotgames-secret-cinema.jpg)

News

Nov 18, 2021

### Global Undercity Nights Broadcast Co-Streaming and Rewards

Learn about how to co-stream the two-day Undercity Nights broadcast on Twitch and YouTube.

Nov 18, 2021

[Introducing the Riot Gauntlet at Undercity Nights](https://www.riotgames.com/en/news/introducing-the-riot-gauntlet-at-undercity-nights)

![](https://www.riotgames.com/darkroom/1000/07f8a0cb9dfaf1bb3578346c122d1fdb:9fbc3b1c00de8d4676e61a6cf2f35bd7/ucn-riotgauntlet-rg-com-1920.jpg)

News

Nov 17, 2021

### Introducing the Riot Gauntlet at Undercity Nights

Players face off against devs across Riot’s games for charity

Nov 17, 2021

[Lands ho! Arcane is Joining Magic: The Gathering](https://www.riotgames.com/en/news/lands-ho-arcane-is-joining-magic-the-gathering)

![](https://www.riotgames.com/darkroom/1000/b23b18c80b45a72a20e7ac1db36666af:c080d030eda02393727792db8bbf9e79/secret-lair-arcane-announcement.jpg)

News

Nov 17, 2021

### Lands ho! Arcane is Joining Magic: The Gathering

Two new drops of Magic: The Gathering Secret Lair will feature characters, moments and locations from Arcane.

Nov 17, 2021

[Latinx Heritage Month Recap](https://www.riotgames.com/en/news/latinx-heritage-month-recap)

![](https://www.riotgames.com/darkroom/1000/59abd62842397f3ea9394496bc55d6e2:cda5a1f80ca40c0fef631aa4b0aa2b30/theme-rgb.jpg)

News

Nov 15, 2021

### Latinx Heritage Month Recap

How we celebrated Latinx Heritage Month

Nov 15, 2021

[Undercity Nights Takes Over](https://www.riotgames.com/en/news/undercity-nights-takes-over)

![](https://www.riotgames.com/darkroom/1000/ff86705fe4337578c54b001f648875f0:bcb4c06309c4cbe61b3a5f24bfe235ce/11-14-embargo-riotxarcane-phase02-keyart-4k.jpg)

News

Nov 14, 2021

### Undercity Nights Takes Over

It’s time to cross the bridge from Piltover to experience Undercity Nights. From rewards to competition, here’s what we have planned.

Nov 14, 2021

[Call an Emergency Meeting - Arcane is Coming to Among Us](https://www.riotgames.com/en/news/call-an-emergency-meeting-arcane-is-coming-to-among-us)

![](https://www.riotgames.com/darkroom/1000/5df13feb341e9948a7dfd45cc2ad2a99:9b3d23e7296ad7309eaa9e30f15c6084/riotx-arcane-among-us.png)

News

Nov 12, 2021

### Call an Emergency Meeting - Arcane is Coming to Among Us

Jinx, Vi, Jayce, and more will join the crew in celebration of Arcane.

Nov 12, 2021

[Riot Games Presents: Your Hobby Could Make For The Perfect Career](https://www.riotgames.com/en/news/riot-games-presents-your-hobby-could-make-for-the-perfect-career)

![](https://www.riotgames.com/darkroom/1000/7a8ee64862c9ca7902103e771be8ca5e:069160c4ff2883e6e4754fb4bd507697/afrotech-thumb-4.jpg)

Inside Riot

Nov 11, 2021

### Riot Games Presents: Your Hobby Could Make For The Perfect Career

Kahlief Adams (Program Manager for D&I) moderated a discussion for Afrotech World 2021 between REL Hunt (Marketing Creative Director, VALORANT) and Dwayne Sands (Software Engineer) about their journeys into gaming, the importance of finding community, and how Riot Noir has been a supportive network for them since coming to Riot.

Nov 11, 2021

[Progress Days: Celebrating Global Community Grant Winners](https://www.riotgames.com/en/news/progress-days-celebrating-global-community-grant-winners)

![](https://www.riotgames.com/darkroom/1000/a7b1bd0aebe0e7518db130515a3c5ead:81bedf24428e61f4e626cf9823d4880e/community-grants-announcement-header-1920x1080.png)

News

Nov 10, 2021

### Progress Days: Celebrating Global Community Grant Winners

Riot Games Social Impact Fund donates $10,000 to 30 nonprofits nominated by players around the world

Nov 10, 2021

[Happy Progress Days!](https://www.riotgames.com/en/news/happy-progress-days)

![](https://www.riotgames.com/darkroom/1000/b6c63cd9fac80601c5a882f1dfb0b8b0:34f3b8d35393f9af7b0b6e5a06d62391/pd-graphic.jpg)

News

Nov 7, 2021

### Happy Progress Days!

It’s Progress Day in Piltover and our games are joining in on the festivities.

Nov 7, 2021

[Fortnite Gets Jinxed to Celebrate Arcane](https://www.riotgames.com/en/news/fortnite-gets-jinxed-to-celebrate-arcane)

![](https://www.riotgames.com/darkroom/1000/e0ec14b3b7f35c371e754862b86375a8:02cf863809168a039eb00ce18975fc21/18br-jinx-arcane-loadingscreen-1920x1080-2.jpg)

News

Nov 4, 2021

### Fortnite Gets Jinxed to Celebrate Arcane

As Jinx drops in, most of Riot’s titles will become available on the Epic Games Store

Nov 4, 2021

[Runeterra Enters PUBG MOBILE to Celebrate RiotX Arcane](https://www.riotgames.com/en/news/runeterra-enters-pubg-mobile-to-celebrate-riotx-arcane)

![](https://www.riotgames.com/darkroom/1000/fff0c1fcbbf403f47ea864a1bec50786:657a7f5252d97ebd8adfc97c57132992/riotx-arcane-pubg-mobile.jpg)

News

Nov 2, 2021

### Runeterra Enters PUBG MOBILE to Celebrate RiotX Arcane

Erangel, meet Runeterra—That’s right, League’s coming to PUBG MOBILE.

Nov 2, 2021

[Welcome to RiotX Arcane](https://www.riotgames.com/en/news/welcome-to-riotx-arcane)

![](https://www.riotgames.com/darkroom/1000/d9a2f62dd5cdd3630d774173aa7da0ac:c06eced2380d64b961ab0555a8bcfe09/riotx-arcane-key-art.jpg)

News

Nov 1, 2021

### Welcome to RiotX Arcane

Celebrate Arcane’s release with us through games, events, rewards, and more!

Nov 1, 2021

[Global Arcane Premiere Co-Streaming and Rewards](https://www.riotgames.com/en/news/global-arcane-premiere-co-streaming-and-rewards)

![](https://www.riotgames.com/darkroom/1000/b540da2b9afe5ec83e842a2d84f6dbb1:9684eda5d229feec2930e4849eb9f819/arcane-final-poster-16x9-no-text-no-border.jpg)

News

Oct 29, 2021

### Global Arcane Premiere Co-Streaming and Rewards

Learn about how to co-stream the first episode of Arcane on Twitch and earn exclusive in-game rewards.

Oct 29, 2021

[Imagine Dragons & JID - Enemy Official Music Video](https://www.riotgames.com/en/news/imagine-dragons-jid-enemy-official-music-video)

![](https://www.riotgames.com/darkroom/1000/f5ddfe249d33ba93a50070cce9bb4fce:b077cb792dc7f80385061dc1ac4cfb5c/riot-arcane-enemy-music-video.png)

News

Oct 28, 2021

### Imagine Dragons & JID - Enemy Official Music Video

Watch the official music video for Arcane’s title track, Enemy.

Oct 28, 2021

[Prime Gaming and Riot Games Team Up to Bring In-Game Content](https://www.riotgames.com/en/news/prime-gaming-and-riot-games-team-up-to-bring-in-game-content-for-riot-games-biggest-titles-esports-sponsorship-and-more)

![](https://www.riotgames.com/darkroom/1000/a017d07f5c9e79b40794cb1fc9f5512e:fde7047b7b8673e2e213a548854e60cc/prime-gaming-x-riot-lol.png)

News

Oct 28, 2021

### Prime Gaming and Riot Games Team Up to Bring In-Game Content

Amazon Prime members can claim in-game offers for League of Legends, Legends of Runeterra, VALORANT, and League of Legends: Wild Rift, and emotes for Esports competitions

Oct 28, 2021

[Secret Cinema presents Arcane: An Immersive Live Experience](https://www.riotgames.com/en/news/secret-cinema-presents-arcane-an-immersive-live-experience)

![](https://www.riotgames.com/darkroom/1000/99bd056e22ba25dbb3bae357779e9b51:b7da0800c5707405cae51840c26da667/secret-cinema-full-poster.jpg)

News

Oct 14, 2021

### Secret Cinema presents Arcane: An Immersive Live Experience

Riot Games and Secret Cinema partner to launch an immersive Arcane experience this November in LA

Oct 14, 2021

[/Dev Teamfight Tactics: Reckoning Learnings](https://www.riotgames.com/en/news/dev-teamfight-tactics-reckoning-learnings)

![](https://www.riotgames.com/darkroom/1000/e00c7dbfd0fd5dfe687b6a98006d9dcb:4b83153da7f555acf495c1a8b13446af/100821-looking-back-reckoning-et5-keyart-horizontal-optimized.jpeg)

News

Oct 13, 2021

### /Dev Teamfight Tactics: Reckoning Learnings

Taking what we learned from Reckoning into Gizmos & Gadgets and beyond!

Oct 13, 2021

[Riot’s President of Games Helps to Inspire the Next Generation](https://www.riotgames.com/en/news/riots-president-of-games-helps-to-inspire-the-next-generation-in-st-louis)

![](https://www.riotgames.com/darkroom/1000/0e5fe3bb728f1864869ef049e044d35a:3dc1ce6ceab6fe7733ba577946890896/rg-stlouis-science-center.jpg)

News

Oct 7, 2021

### Riot’s President of Games Helps to Inspire the Next Generation

Rioters Participate in St. Louis Science Center's First Friday Event

Oct 7, 2021

[Naz Aletaha Named Global Head of LoL Esports](https://www.riotgames.com/en/news/naz-aletaha-named-global-head-of-lol-esports)

![](https://www.riotgames.com/darkroom/1000/da2f3003d32ce5e6e30e477e0857dbd3:89a3623f68987439bec5e1258ed9db92/49043404278-2f7eea2074-o.jpg)

News

Oct 5, 2021

### Naz Aletaha Named Global Head of LoL Esports

Creating the roadmap for the future of LoL Esports

Oct 5, 2021

[Riot’s Global Service Month 2021](https://www.riotgames.com/en/news/riots-global-service-month-2021)

![](https://www.riotgames.com/darkroom/1000/8a975de9f130d0dd7b1142dfbad138b5:44ee3f4d30f5637b93e48ac724610aec/2021-global-service-month-header.jpg)

News

Sep 29, 2021

### Riot’s Global Service Month 2021

Rioters around the world served their local communities during Global Service Month 2021

Sep 29, 2021

[Riot Games Elevates Ryan Crosby to President of Publishing](https://www.riotgames.com/en/news/riot-games-elevates-ryan-crosby-to-president-of-publishing)

![](https://www.riotgames.com/darkroom/1000/413f5cdaa5272fcbb94d62a44233861c:26c29b6def3e8e41f480e308e612ced6/ryan-crosby.jpg)

News

Sep 22, 2021

### Riot Games Elevates Ryan Crosby to President of Publishing

Riot sets sights on offerings across games, entertainment, and sports with Crosby helming 1,000+ worldwide publishing team

Sep 22, 2021

[New Riot Client Coming Soon](https://www.riotgames.com/en/news/new-riot-client-coming-soon)

![](https://www.riotgames.com/darkroom/1000/ee3e986a072982e9dac2faea9548d066:f3ab8f345ea68bc4a85af5d40cb29324/valorant.png)

News

Sep 16, 2021

### New Riot Client Coming Soon

Announcing the launch of the new Riot Client!

Sep 16, 2021

[Annual Diversity and Inclusion (D&I) Progress Report - 2020](https://www.riotgames.com/en/news/annual-diversity-and-inclusion-di-progress-report-august-2021)

![](https://www.riotgames.com/darkroom/1000/27681bc5bc281143598353b5aef66b22:fc3b881714ba5330c51b294e176d0ac2/riot-diversity-inclusion-report-2021-cover-2.jpg)

News

Sep 7, 2021

### Annual Diversity and Inclusion (D&I) Progress Report - 2020

In a year when we launched new games during a pandemic, we also kept focused on our commitment to fostering diverse perspectives and work towards creating an inclusive culture. Our second annual D&I report is live now!

Sep 7, 2021

[Hello Riot Mobile! Farewell League+](https://www.riotgames.com/en/news/hello-riot-mobile-farewell-league)

![](https://www.riotgames.com/darkroom/1000/342822937083c12b58367f03fcc19d58:8f1c6a2db9b5d33c2d5a9aec79fb148d/riot-mobile-multigame-promo-final-1920x1080.jpg)

News

Aug 26, 2021

### Hello Riot Mobile! Farewell League+

Download Riot Mobile - your companion for all things Riot Games!

Aug 26, 2021

[Erin Wayne Joins Riot Games as Its First-Ever Global Head of Player...](https://www.riotgames.com/en/news/erin-wayne-joins-riot-games-as-its-first-ever-global-head-of-player-community)

![](https://www.riotgames.com/darkroom/1000/1ee204183d726dc79c3c2c55c1278cd5:2aea644e8f343369fe8667243a42669e/erin-wayne.jpg)

News

Aug 25, 2021

### Erin Wayne Joins Riot Games as Its First-Ever Global Head of Player...

Shaping the Next Evolution of Community Engagement

Aug 25, 2021

[Riot Games Names Mark Sottosanti Chief Financial Officer](https://www.riotgames.com/en/news/riot-games-names-mark-sottosanti-chief-financial-officer)

![](https://www.riotgames.com/darkroom/1000/3822b116c9619d7445dfb4cf92d757f2:fde74b3a93e8b87a127388a04dce436f/mark-sottosanti.jpg)

News

Aug 23, 2021

### Riot Games Names Mark Sottosanti Chief Financial Officer

Veteran Riot executive to lead global finance and strategy as company enters its next phase of growth.

Aug 23, 2021

[Bug Blog: TFT Bugs And Patches](https://www.riotgames.com/en/news/bug-blog-tft-bugs-and-patches)

![](https://www.riotgames.com/darkroom/1000/b524dc361a7b89033cd6b6e321830278:15ebf66243b0273677cdd5bbb2464fb6/tftheader.png)

Tech Blog

Jul 30, 2021

### Bug Blog: TFT Bugs And Patches

See how we handle patches across PC and mobile for TFT, and how this relates to quality assurance.

Jul 30, 2021

[Changing the Face of Gaming at 2021 Virtual San Diego Comic-Con](https://www.riotgames.com/en/news/changing-the-face-of-gaming-at-2021-virtual-san-diego-comic-con)

![](https://www.riotgames.com/darkroom/1000/fe3c2d9c2958167a1114d161cd56a0ec:987f6930b1ebcde5e2a4477adea846e4/screen-shot-2021-06-10-at-9-22-04-am-1.png)

News

Jul 29, 2021

### Changing the Face of Gaming at 2021 Virtual San Diego Comic-Con

Jessica Nam, Candace Thomas, and Shauna Spenley discuss their journey into the gaming and entertainment industry

Jul 29, 2021

[Riot’s Mid-Year Break](https://www.riotgames.com/en/news/riots-mid-year-break)

![](https://www.riotgames.com/darkroom/1000/246330610c2c98c54dcc241f1612a649:f3c043c5940893fa2193888a8cf12623/riot-summer-break.png)

News

Jul 26, 2021

### Riot’s Mid-Year Break

We’re taking the week of August 2nd to recharge.

Jul 26, 2021

[Hollaback! Anti-Xenophobic Harassment Training](https://www.riotgames.com/en/news/hollaback-anti-xenophobic-harassment-training)

![](https://www.riotgames.com/darkroom/1000/94c5486f37adf1b2f6d229f2e1b772d8:3265fe015c52f0956578528ab1416863/legal.png)

News

Jul 26, 2021

### Hollaback! Anti-Xenophobic Harassment Training

Our continued work to educate Rioters on how to stand up against anti-Asian racism.

Jul 26, 2021

[R&D Foundations: Opportunity, Thesis, and Audience](https://www.riotgames.com/en/news/r-d-foundations-opportunity-thesis-and-audience)

![](https://www.riotgames.com/darkroom/1000/14d20808c5e0c5045272f3c047960633:443bc1894c8250e822211f3a99d217c1/00-header.jpg)

News

Jul 15, 2021

### R&D Foundations: Opportunity, Thesis, and Audience

Before development starts on an R&D game, the team needs to understand their game’s core foundations.

Jul 15, 2021

[Join The Sentinels Initiative](https://www.riotgames.com/en/news/join-the-sentinels-initiative)

![](https://www.riotgames.com/darkroom/1000/a548eef952ed4a7e992405c929795a2c:92c3286a89ca79f435e5378d66f3b2c7/06-sentinels-iniciative-annuncement-header-notext-riotwebsite-1.png)

News

Jul 8, 2021

### Join The Sentinels Initiative

Nominate a nonprofit or cause for a chance to grant them $10,000 for charity.

Jul 8, 2021

[Pride Month Recap](https://www.riotgames.com/en/news/pride-month-recap)

![](https://www.riotgames.com/darkroom/1000/24a0bbd1595a753dda7ee5638d20861f:c64c914dca86b607db1a7f2d26842e67/pridebg.png)

Inside Riot

Jul 7, 2021

### Pride Month Recap

How we celebrated Pride this year.

Jul 7, 2021

[Champion Insights: Akshan](https://www.riotgames.com/en/news/champion-insights-akshan)

![](https://www.riotgames.com/darkroom/1000/07622fe14b3d11ad2ff603ec7282e796:e4fccc2ce798b7caf40ca76bf5cb4af7/00-header.png)

Inside Riot

Jul 7, 2021

### Champion Insights: Akshan

A hero among scoundrels.

Jul 7, 2021

[Sessions: Vi - Out now](https://www.riotgames.com/en/news/sessions-vi-out-now)

![](https://www.riotgames.com/darkroom/1000/8c969ef9f267537ef0a6b092b629bfcb:c2fefe317bde437ff9a42fb060bf6903/sessions-article-hero.png)

News

Jul 2, 2021

### Sessions: Vi - Out now

Introducing Sessions: Vi. A 37 track album completely free to use in your content.

Jul 2, 2021

[Leveling Up Networking For A Multi-game Future](https://www.riotgames.com/en/news/leveling-networking-multi-game-future)

![](https://www.riotgames.com/darkroom/1000/ff91088c48e6adc9f0b4d22d038ccb3e:e36fd9df1b377ff4ed1c3629a61c45d7/rdheader.png)

Tech Blog

Jun 29, 2021

### Leveling Up Networking For A Multi-game Future

We’re going to tell you a bit about what we’ve done to reinforce consistent and stable connections, reduce latency, and improve the overall player experience for our entire multi-game portfolio.

Jun 29, 2021

[Fate of Demacia](https://www.riotgames.com/en/news/fate-of-demacia)

![](https://www.riotgames.com/darkroom/1000/d9987629b973b48a0fe3f71a52659276:a3bf1eabc7f878e7c673bea51ef7a7a3/sentinels-fate-of-demacia-01-no-text-1.jpg)

News

Jun 19, 2021

### Fate of Demacia

Ruination conquers all.

Jun 19, 2021

[Jason Bunge on Breakthrough Builders](https://www.riotgames.com/en/news/jason-bunge-on-breakthrough-builders)

![](https://www.riotgames.com/darkroom/1000/d7bfb91d74829b6b8b8a75464b4b4983:4310b8a97b9ff4500ef23859a06b3a17/jason-hero.png)

News

Jun 16, 2021

### Jason Bunge on Breakthrough Builders

CMO Jason Bunge talks to the Breakthrough Builders podcast about gaming culture, building a company's brand, and the importance of marketing.

Jun 16, 2021

[/dev diary: Arcane Animated Series](https://www.riotgames.com/en/news/dev-diary-arcane-animated-series)

![](https://www.riotgames.com/darkroom/1000/206acfa26271d305743c68e0e522ced4:a4be473281755c2ce29da57ddb180215/arcane-nx-geeked-thumbnail-devdiary-copy.png)

News

Jun 11, 2021

### /dev diary: Arcane Animated Series

Sit down with co-creators Christian Linke and Alex Yee to dive into Arcane.

Jun 11, 2021

[Arcane: Animated Series \| A Score To Settle](https://www.riotgames.com/en/news/arcane-animated-series-a-score-to-settle)

![](https://www.riotgames.com/darkroom/1000/0b6f19987c98609ca6c63ab6ae63fd6b:d643b7d377253d15dc9819059bea426d/e3nw7yiweauokae.jfif)

News

Jun 11, 2021

### Arcane: Animated Series \| A Score To Settle

Jinx has something to prove, especially to her sister. Arcane the animated series is coming to Netflix Fall 2021.

Jun 11, 2021

[Asian American & Pacific Islander Heritage Month Recap](https://www.riotgames.com/en/news/asian-american-pacific-islander-heritage-month-recap)

![](https://www.riotgames.com/darkroom/1000/6dc4d7e65c12b2bca4aca90165766d32:fc72120c448fcb1b55f15a669319c92b/apa-header-16x9.jpg)

Inside Riot

Jun 9, 2021

### Asian American & Pacific Islander Heritage Month Recap

How we celebrated AAPI Heritage Month.

Jun 9, 2021

[Underrepresented Founders Program One Year In](https://www.riotgames.com/en/news/underrepresented-founders-program-one-year-in)

![](https://www.riotgames.com/darkroom/1000/53970ae54f688597333abd5db416aa92:bfe32fb4b1d212ffb3396d6ab6d5dd8c/underrepresented-founders-program-graphic-1.png)

News

Jun 4, 2021

### Underrepresented Founders Program One Year In

Our continued support to drive change in underrepresented gaming communities.

Jun 4, 2021

[The Legends Of Runeterra CI/CD Pipeline](https://www.riotgames.com/en/news/legends-runeterra-cicd-pipeline)

![](https://www.riotgames.com/darkroom/1000/281b71e40f89e88f26947bba375efae4:3370a051f655c8eaece1346569f564d4/lorpipelineheader.png)

Tech Blog

May 31, 2021

### The Legends Of Runeterra CI/CD Pipeline

Our team will be sharing some details about how we build, test, and deploy Legends of Runeterra, a digital collectible card game.

May 31, 2021

[Free Mental Health Resources for Content Creators](https://www.riotgames.com/en/news/free-mental-health-resources-for-content-creators)

![](https://www.riotgames.com/darkroom/1000/105d041ab30a8492e5542674545e8200:3f1e07bf5135e57693c108ea67c0d9d6/mental-health-ad-article.png)

News

May 28, 2021

### Free Mental Health Resources for Content Creators

We partnered with The Public Good Projects to create a free mental health advocacy guide for streamers, content creators, and moderators.

May 28, 2021

[Dawnbringer Karma Wins Golden Halo Award](https://www.riotgames.com/en/news/dawnbringer-karma-wins-golden-halo-award)

![](https://www.riotgames.com/darkroom/1000/d8b51b2811cd8adc74b21cd14cbd3bee:5d689f36d8a785e81e1196a151b9c09c/00-karma-header.png)

News

May 27, 2021

### Dawnbringer Karma Wins Golden Halo Award

League of Legends players win the 2021 Golden Halo Award for their generosity with the Dawnbringer Karma Fundraiser.

May 27, 2021

[Collaborative Creative in R&D](https://www.riotgames.com/en/r-and-d-office/collaborative-creative-in-r-d)

![](https://www.riotgames.com/darkroom/1000/e0d519d1ffa0a42804032696144c5206:32aa50ce11ac7c1b7b63352610e2d559/runeterra-piltover-16.jpg)

Inside Riot

May 21, 2021

### Collaborative Creative in R&D

Looking at R&D games through a creative lens.

May 21, 2021

[Celebrating Pride With You](https://www.riotgames.com/en/news/celebrating-pride-with-you)

![](https://www.riotgames.com/darkroom/1000/17607b86edfca4347cc99776e1fd9cf9:ca06af01e9cc7d45efda69b9d375ce88/00-header-fluft-n-friends.jpg)

News

May 17, 2021

### Celebrating Pride With You

This year’s Pride celebration is bigger than ever, and it’s coming to every game!

May 17, 2021

[On Friction](https://www.riotgames.com/en/news/on-friction)

![](https://www.riotgames.com/darkroom/1000/edb2e8b2dab1bfa493f91352bf83ed04:961370095b68336bca2a646e1c0ea06f/0-qinxxuu6i7xtpy-2.jfif)

News

May 13, 2021

### On Friction

UX designer Jeff Zhang explains why friction in games creates a better experience.

May 13, 2021

[Arcane: Animated Series \| Official Netflix Announcement](https://www.riotgames.com/en/news/arcane-animated-series-official-netflix-announcement)

![](https://www.riotgames.com/darkroom/1000/58d08216a3d7fe7c43d1d8f91403be43:faca308a8ca043cb88d2eaa17c6ecd7d/ts-vi-overzaun-raw.png)

News

May 3, 2021

### Arcane: Animated Series \| Official Netflix Announcement

Arcane the animated series is coming to Netflix Fall 2021.

May 3, 2021

[Strategies for Working in Uncertain Systems](https://www.riotgames.com/en/news/strategies-working-uncertain-systems)

![](https://www.riotgames.com/darkroom/1000/1a48a21a85fddb48fd5a876b00046cd0:aa17ac838260f61c854adb8f943b38c7/uncertainsystemsheader.png)

Tech Blog

Apr 27, 2021

### Strategies for Working in Uncertain Systems

Learn some tips and tricks our team has used to overcome or avoid cases like the one above, and how we figure out why they happen in the first place.

Apr 27, 2021

[Updating the Privacy Notice and Terms of Service](https://www.riotgames.com/en/news/updating-the-privacy-notice-and-terms-of-service)

![](https://www.riotgames.com/darkroom/1000/94c5486f37adf1b2f6d229f2e1b772d8:3265fe015c52f0956578528ab1416863/legal.png)

News

Apr 30, 2021

### Updating the Privacy Notice and Terms of Service

We’re updating our player policies to implement a global refund policy and improve our behavior systems around voice chat.

Apr 30, 2021

[2020 Social Impact Year in Review](https://www.riotgames.com/en/news/2020-social-impact-year-in-review)

![](https://www.riotgames.com/darkroom/1000/c2428a7c1dd2306dfb114c22bb6f2b7f:1e025a64e241c5fb9b435963d30cafa0/2020yirheader.jpg)

News

Apr 26, 2021

### 2020 Social Impact Year in Review

Taking a look back at all the social good in 2020.

Apr 26, 2021

[Elderwood Ornn Charity Skin Results](https://www.riotgames.com/en/news/elderwood-ornn-charity-skin-results)

![](https://www.riotgames.com/darkroom/1000/5d1520e028637779e043bf15fba563c0:d19a7c6c2599086f29bd1f4efe9f94cf/elderwood-ornn-final-1.jpg)

News

Apr 22, 2021

### Elderwood Ornn Charity Skin Results

You raised more than $7 million for the Riot Games Social Impact Fund with Elderwood Ornn.

Apr 22, 2021

[Improving the Developer Experience for Operating Services](https://www.riotgames.com/en/news/improving-developer-experience-operating-services)

![](https://www.riotgames.com/darkroom/1000/f47dc6ca84f0510d6ca7415ddd4f4fae:8d9b349991263909eb6771cb764cb768/consoleheader.png)

Tech Blog

Mar 30, 2021

### Improving the Developer Experience for Operating Services

In this article, we’ll be talking about our one-stop-shop application for Rioters operating services - Console.

Mar 30, 2021

[Champion Insights: Gwen](https://www.riotgames.com/en/news/champion-insights-gwen)

![](https://www.riotgames.com/darkroom/1000/0443b93885825c57338b3d5aecbc945c:0c7dde0c10076202e171a0ffc0f677e4/00-header.jpg)

News

Mar 31, 2021

### Champion Insights: Gwen

Cut from a different cloth.

Mar 31, 2021

[League Frequencies: Audio Development Video Series](https://www.riotgames.com/en/news/league-frequencies-audio-development-video-series)

![](https://www.riotgames.com/darkroom/1000/4a78ee94964492708dccc363f8a2c678:1204bf8cd87881e0b09135eb81f95fa0/frequencies-ep1-part1-thumb.png)

Disciplines

Mar 24, 2021

### League Frequencies: Audio Development Video Series

Join audio engineers from across Riot as they share how the different areas of audio design work to create a cohesive experience for players.

Mar 24, 2021

[/dev: Clarity in League](https://www.riotgames.com/en/news/dev-clarity-in-league)

![](https://www.riotgames.com/darkroom/1000/3ac8c5277d757ed2237b5b13b55ab39a:029ecb45045c300fb2d443d7fb68eee4/01-clarity-banner-sona.jpg)

Inside Riot

Mar 12, 2021

### /dev: Clarity in League

Hitboxes, color palettes, silhouettes, cats, skins, and more.

Mar 12, 2021

[Client, Clarity, and State of the Game - lol pls](https://www.riotgames.com/en/news/client-clarity-and-state-of-the-game-lol-pls)

![](https://www.riotgames.com/darkroom/1000/2c19afd75b4cd47578a4c3cf9d4f3fc1:81c9a4d70a2a2818f528216baca0c7d6/lolplsq1-thumbnail.jpg)

News

Mar 12, 2021

### Client, Clarity, and State of the Game - lol pls

League’s executive producer Safelocked shares the latest on the client and game clarity and design director Meddler covers League’s current game state.

Mar 12, 2021

[Wild Rift Americas Server Launch](https://www.riotgames.com/en/news/wild-rift-americas-server-launch)

![](https://www.riotgames.com/darkroom/1000/51fe64c0f973a65c9d4ccfe736f09cbf:1c06be168fbdc20a6e545752d87f9502/wr-meta-homepage.png)

News

Mar 9, 2021

### Wild Rift Americas Server Launch

League of Legends: Wild Rift is coming to the Americas!

Mar 9, 2021

[Celebrating Black History Month](https://www.riotgames.com/en/news/celebrating-black-history-month)

![](https://www.riotgames.com/darkroom/1000/e2434f4962510d82b3e8b8e96f9cf985:b71ec0351b059df8dfc11ffb72801d48/noir-bhm-fight-night-so-noir-intro.jpg)

Inside Riot

Mar 2, 2021

### Celebrating Black History Month

How we celebrated Black Excellence in Gaming & Entertainment.

Mar 2, 2021

[Queue Dodge](https://www.riotgames.com/en/work-with-us/queue-dodge)

![](https://www.riotgames.com/darkroom/1000/08dc743df4aa26f832a3d2261551d8e9:dc30bab3eee00b9b171bbac666bc4354/dsc-1884.JPG)

Inside Riot

Aug 3, 2023

### Queue Dodge

We’ve designed Queue Dodge to help self-identified mismatches move on in an open, positive, and constructive way.

Aug 3, 2023

[The Art of Spell Casting, Part 2](https://www.riotgames.com/en/news/art-of-spell-casting-part-2)

![](https://www.riotgames.com/darkroom/1000/9b1536a2040a003773d024d465df8713:3643458817208aa473bab557ac78067b/hwei2.png)

Tech Blog

Feb 27, 2024

### The Art of Spell Casting, Part 2

In this two-part series, we explore the technology behind Hwei's unique spell casting paradigm.&nbsp;It took many ideas and iterations before we landed on a solution that&nbsp;makes Hwei’s spell casting feel precise and fluid, and allows&n…

Feb 27, 2024

[The Art of Spell Casting, Part 1](https://www.riotgames.com/en/news/art-of-spell-casting-part-1)

![](https://www.riotgames.com/darkroom/1000/f761c2017f6a540aeeded28e86b627a4:1a4b52de921e18cea8e9a71d4a5220ef/hwei1.png)

Tech Blog

Feb 27, 2024

### The Art of Spell Casting, Part 1

In this two-part series, we explore the technology behind Hwei's unique spell casting paradigm. It took many ideas and iterations before we landed on a solution that makes Hwei’s spell casting feel precise and fluid, and allows&nbsp;player…

Feb 27, 2024

[Reviving Nexus Blitz](https://www.riotgames.com/en/news/reviving-nexus-blitz)

![](https://www.riotgames.com/darkroom/1000/598d9cd14c14b4a1aff902189ef75f5c:514fdcd2b7d98c38bed329e14ea48814/nb-tb.jpg)

Tech Blog

Nov 28, 2023

### Reviving Nexus Blitz

In this article, we explore some of the challenges the team faced when reviving the Nexus Blitz game mode this year, from upgrading older systems to making them more accessible for future developers.

Nov 28, 2023

[Riot Games](https://www.riotgames.com/)

- [Press](https://www.riotgames.com/en/press)
- [Security](https://www.riotgames.com/en/security)
- [Legal](https://www.riotgames.com/en/legal)
- [Leadership](https://www.riotgames.com/en/leadership)
- [Candidate Privacy](https://www.riotgames.com/en/candidate-privacy)
- [Terms of Service](https://www.riotgames.com/en/terms-of-service)
- [Privacy Notice](https://www.riotgames.com/en/privacy-notice)
- [Player Support](https://support.riotgames.com/hc/en-us)
- [E-Verify](https://www.riotgames.com/en/e-verify-right-to-work)
- [Accessibility](https://www.riotgames.com/en/accessibility)
- [Annual Reports](https://www.riotgames.com/en/annual-reports)
- [Peering Information](https://www.riotgames.com/en/peering-information)
- [Community Pact](https://www.riotgames.com/en/community-pact)

[Follow us on Twitter](https://twitter.com/riotgames) [Follow us on Instagram](https://www.instagram.com/riotgames/) [Follow us on Tiktok](https://www.tiktok.com/@riotgames) [Share this on LinkedIn](https://www.linkedin.com/company/riot-games) [Follow us on Facebook](https://www.facebook.com/RiotGames/) [Watch on YouTube](https://www.youtube.com/riotgames)

Cookie Preferences

© 2025 Riot Games, Inc. All Rights Reserved.

[To the Surface](https://www.riotgames.com/en/news/vanguard-security-update-motherboard#top)

Check Session IFrame

OIDC OP Iframe

### Auth Error

|     |     |
| --- | --- |
| iss | https://auth.riotgames.com |
| state | 4e17d5bb664468a51fd988e6b4 |
| error | interaction\_required |
| error\_description | login\_required |