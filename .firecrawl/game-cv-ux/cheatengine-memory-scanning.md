# Cheat Engine:Memory Scanning

From Cheat Engine

[Jump to navigation](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#mw-head) [Jump to search](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#p-search)

Memory scanning is one of the most major features of Cheat Engine.
Memory scanning means searching for a specific value or a pattern in the memory provided to the open (target) process.
Cheat Engine's scanning makes use of multiple processor cores when available. The final result of a scan is a list of addresses, and this list can be refined (reduced) by other next scans.

Cheat Engine allows you to search in various ways using different types of searching. All options are provided by the main window of CE: value to search, scan type, value type, many memory scan options, and some other extra options.

**Note:** You must open a target process to active these options.

## Contents

- [1Value to scan](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#Value_to_scan)
- [2Scan Type](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#Scan_Type)
- [3Value Type](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#Value_Type)
- [4Memory Scan Options](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#Memory_Scan_Options)
- [5Other extra options](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#Other_extra_options)
- [6Performing a memory scan](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning#Performing_a_memory_scan)

## Value to scan\[ [edit](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=edit&section=1 "Edit section: Value to scan")\]

The defaut base for **Value** is decimal but you can check the **Hex** option to use an hexadecimal value or **Lua formula** option to use a valid Lua expression. Latter option actives an extra option **Separate Lua State** by default checked.

## Scan Type\[ [edit](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=edit&section=2 "Edit section: Scan Type")\]

Sometimes you may not know the exact number of a value you want to search for, or it changes too often to scan normally. That's why Cheat Engine provides a dropbox with several ways of looking for values. These scan options make it possible to find any value, whether you know it or not.
The available options depend on the scan order (initial/first or subsequent/next):

For the First Scan the scan types options are:

- **Exact Value**

- **Bigger than...**

- **Smaller than...**

- **Value between...**

- **Unknown initial value**

For the Next Scan the scan types options are:

- **Exact Value**

- **Bigger than...**

- **Smaller than...**

- **Value between...**

- **Increased Value**

- **Increased Value by...**

- **Decreased Value**

- **Decreased Value by...**

- **Changed Value**

- **Unchanged Value**

- **Same as First Scan**

All these options speak for themselves.

## Value Type\[ [edit](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=edit&section=3 "Edit section: Value Type")\]

The available types Cheat Engine can scan for are:

- **Binary**

- **Byte** (values between 0 and 255 or -128 to 127 if signed)

- **2 Bytes** (values between 0 and 65535 or -32768 to 32767 if signed)

- **4 Bytes** (values between 0 and 4294967295 or -2147483648 and 2147483647 if signed)

- **8 Bytes** (values between 0 and 18446744073709551615 or -9223372036854775808 and 9223372036854775807 if signed)

- **Float** (values between 1.5 x 10^-45 and 3.4 x 10^38 )

- **Double** (values between 5.0 x 10^-324 and 1.7 x 10^308 )

- **String** (/Text)

- **Array of byte**

- **All** (from **Byte** to **Double**)

- **Grouped** : Grouped scanning allows you to find a structure more quickly when you know its layout. Individual values are in the format "type:value", separated by spaces. For example if you know that health is an integer and your current health is 75/100, and that the structure has the current value, an unknown 4 byte quantity, and then the max value, you can search for this:

```
4:75 4:* 4:100
```

Which one you should choose depends on the value and type of the value you want to search for.
A value that only switches between 1 and 0 is often a Binary.
While an integer number (1, 3, 4599, 15686, etc...) is most often a 4-byte value.
Strictly non-integer numbers are always Float or Double. Small numbers are often Float, bigger ones Double.
Text is often stored as a String/Text.

## Memory Scan Options\[ [edit](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=edit&section=4 "Edit section: Memory Scan Options")\]

Cheat Engine offers you a dropbox option to scan all the memory space used by the target process or only specific areas used by the main process or any of the depending DLL. Depending on selected options, the scan range defined by the **Start** and the **Stop** addresses is automatically adjusted by CE (from 0000000000000000 to 7fffffffffffffff for all memory space) but you can edit/overwrite these limits.

A tri-state option **Executable** allow you to scan only the executable, non-executable, or any kind of executable memory.
The options **Writable** and **CopyOnWrite** allow to scan only the writable and/or copy-on-write ranges of memory.

The **Fast Scan** option can speeds up the scanning by testing only the **Aligned** memory addresses or the **Last Digits** of the specified value. For aligned address you can choose the multiplier with a defaut value equal to 4 (the most used value used by games and any other applications). Mathematically, an aligned address is a multiple of the multiplier value. For example, address 0x40000 is aligned by 4 and also aligned by 0x1000 because after dividing it by both alignment numbers, remainder of the result is 0 in both cases. Address 0x40004 is aligned by 4 but isn't aligned by 0x1000 because when you divide it by 4, the remainder is 0, but when you divide it by 0x1000, the remainder is 4. Address 0x40001 is not aligned by neither 4, nor 0x1000.

If the searched value can change in real-time in the target process then you can activate the option **Pause the game while scanning** which will freeze the target process while doing the memory scan.

## Other extra options\[ [edit](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=edit&section=5 "Edit section: Other extra options")\]

**Unrandomizer** option: TODO

**Enable Speedhack** option: see [Cheat Engine Internals](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Internals "Cheat Engine:Internals")

## Performing a memory scan\[ [edit](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=edit&section=6 "Edit section: Performing a memory scan")\]

If you choose the right options, you can perform a new scan by clicking on **First Scan** button. Cheat Engine will provide you with a list of addresses matching your search. Shown values of variables at found addresses are updated as the actual in-game values of variables change, at a rate set in the Settings menu. There are two types of addresses in this list: green ones and black ones.

- Green means static addresses. Whenever you load the application these addresses will stay the same/hold the same value.

These green addresses show up as absolute virtual addresses in the list, but are actually offsets to base address of one of the loaded modules in the process' address space. So when you have static(green) address 4075FFB0 and module's base address is 40000000, it is calculated as 40000000+75FFB0, which Cheat Engine often shows you as ModuleName.exe+75FFB0.

- Black means dynamic addresses. Variables at these addresses will change location(variables will change their addresses) whenever you load the application, and even while the application is running. Using pointers you can find static addresses for these dynamic ones.

Then you can refine/reduce the list of addresses performing a **Next Scan**. Cheat Engine remembers the values found in the previous scan, allowing it to compare new values with the old ones and revert to a previous scan. It also remembers the values of the **First Scan**.

Retrieved from " [https://wiki.cheatengine.org/index.php?title=Cheat\_Engine:Memory\_Scanning&oldid=7205](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&oldid=7205)"

[Category](https://wiki.cheatengine.org/index.php?title=Special:Categories "Special:Categories"):

- [Help](https://wiki.cheatengine.org/index.php?title=Category:Help "Category:Help")

## Navigation menu

### Personal tools

- Not logged in
- [Talk](https://wiki.cheatengine.org/index.php?title=Special:MyTalk "Discussion about edits from this IP address [alt-shift-n]")
- [Contributions](https://wiki.cheatengine.org/index.php?title=Special:MyContributions "A list of edits made from this IP address [alt-shift-y]")
- [Create account](https://wiki.cheatengine.org/index.php?title=Special:CreateAccount&returnto=Cheat+Engine%3AMemory+Scanning "You are encouraged to create an account and log in; however, it is not mandatory")
- [Log in](https://wiki.cheatengine.org/index.php?title=Special:UserLogin&returnto=Cheat+Engine%3AMemory+Scanning "You are encouraged to log in; however, it is not mandatory [alt-shift-o]")

### Namespaces

- [Project page](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning "View the project page [alt-shift-a]")
- [Discussion](https://wiki.cheatengine.org/index.php?title=Cheat_Engine_talk:Memory_Scanning&action=edit&redlink=1 "Discussion about the content page (page does not exist) [alt-shift-t]")

### Variants

### Views

### More

### Search

[Visit the main page](https://wiki.cheatengine.org/index.php?title=Main_Page "Visit the main page")

### Navigation

- [Main page](https://wiki.cheatengine.org/index.php?title=Main_Page "Visit the main page [alt-shift-z]")
- [Recent changes](https://wiki.cheatengine.org/index.php?title=Special:RecentChanges "A list of recent changes in the wiki [alt-shift-r]")
- [Random page](https://wiki.cheatengine.org/index.php?title=Special:Random "Load a random page [alt-shift-x]")
- [Help about MediaWiki](https://www.mediawiki.org/wiki/Special:MyLanguage/Help:Contents)

### Tools

- [What links here](https://wiki.cheatengine.org/index.php?title=Special:WhatLinksHere/Cheat_Engine:Memory_Scanning "A list of all wiki pages that link here [alt-shift-j]")
- [Related changes](https://wiki.cheatengine.org/index.php?title=Special:RecentChangesLinked/Cheat_Engine:Memory_Scanning "Recent changes in pages linked from this page [alt-shift-k]")
- [Special pages](https://wiki.cheatengine.org/index.php?title=Special:SpecialPages "A list of all special pages [alt-shift-q]")
- [Printable version](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&printable=yes "Printable version of this page [alt-shift-p]")
- [Permanent link](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&oldid=7205 "Permanent link to this revision of the page")
- [Page information](https://wiki.cheatengine.org/index.php?title=Cheat_Engine:Memory_Scanning&action=info "More information about this page")