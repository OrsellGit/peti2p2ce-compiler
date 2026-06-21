# PeTI To P2:CE Compilers

* [General Info](#general-info)
  * [What is this?](#what-is-this)
  * [What is the benefit of this over just copying over the BSP file made by PeTI?](#what-is-the-benefit-of-this-over-just-copying-over-the-bsp-file-made-by-peti)
  * [Does this mean that it's possible to make PeTI content for P2:CE's workshop?](#does-this-mean-that-its-possible-to-make-peti-content-for-p2ces-workshop)
  * [Because this is using Strata's compilers, does this mean it can use the most if not all the Strata engine's newer features?](#because-this-is-using-stratas-compilers-does-this-mean-it-can-use-the-most-if-not-all-the-strata-engines-newer-features)
* [Setup](#setup)
  * [Uninstalling](#uninstalling)
  * [Custom FGDs For AngelScript Entities](#custom-fgds-for-angelscript-entities)
* [Other Information](#other-information)
  * [Cooperative Partner PeTI Map](#cooperative-partner-peti-map)
  * [What if I need help with something or something went wrong?](#what-if-i-need-help-with-something-or-something-went-wrong)
  * [Why Does It Still Say Portal 2 Is Running When P2:CE Is Running?](#why-does-it-still-say-portal-2-is-running-when-p2ce-is-running)
  * [Stuff I Still Have To Do With This Tool](#stuff-i-still-have-to-do-with-this-tool)
* [Credits](#credits)
  * [Special Thanks](#special-thanks)
  * [Crediting](#crediting) PLEASE READ IF YOU USE THIS TOOL!

## General Info

### What is this?

A compiler tool I developed to be called by Portal 2 to take control of the compile process and call BEEMod's compilers followed by Portal 2: Community Edition's Strata compilers instead Portal 2's.
Not only will these compile a PeTI map with the Strata compilers, it will also make sure to copy over and run the PeTI map in P2:CE.

This tool is designed with mappers, modders, and puzzle designers in mind who will not be posting PeTI maps publicly to the workshop and instead using it to prototype puzzles for P2:CE related mods, maps, or other content.

As of writing this only will work properly for Windows as I haven't had the time to properly work on this for Linux.
While my current code has been developed to some extent with Linux support, you'll need to use Wine or similar in the meantime.

BEEMod is recommended to be used with this since it was what it was intended for, but you technically can use these without it. If BEEMod's VBSP compiler is not found, only P2CE's compilers will be run.

### What is the benefit of this over just copying over the BSP file made by PeTI?

Mainly just to automate the process. Copying the file over and over again can get annoying for some people, and so automating it and running the game for you can be quite helpful.
Other reason is to be able to use and take advantage of Strata's new engine features with PeTI.

However, these tools will still use PeTI's limits for compatibility with BEEMod since TeamSpen210 isn't going to update BEEMod to actively support this tool if there is issues with it and BEEMod.
I personally made an update to BEEMod's compilers to support this tool, but TeamSpen210 and I will not be actively changing BEEMod to support this tool.

### Does this mean that it's possible to make PeTI content for P2:CE's workshop?

Short answer: **No**. Other short answer: *Kinda*.

You would still need to do a few other things manually with the compiled BSP, setting up the addon, etc. Those processes I will not be automating since this isn't what this tool is for.

**In fact, I do not recommend using this at all if you want to make PeTI content and post it to the P2:CE workshop.**

P2:CE will one day get its own puzzle maker so if you want something more refined, then just wait for that.

### Because this is using Strata's compilers, does this mean it can use the most if not all the Strata engine's newer features?

Theoretically, yes. You can make an instance with P2:CE entities and turn it into a UCP for BEEMod to add into PeTI, but do not expect it to just work out of the box for everything.

While I was developing this tool to try and make some P2:CE features work in tandem with PeTI and BEEMod, I have not tested everything, as this is geared to projects I am working on and not everything else others are doing.

One of those features is custom AngelScript entities. In my testing those work fine as long as the required scripts and other assets are available on P2:CE's end and your instance/UCP isn't doing anything too wacky.

This compiler has special setup needed if you want custom AngelScript entities to work correctly. Please read [Custom FGDs For AngelScript Entities](#custom-fgds-for-angelscript-entities) for more info after setting up this compiler below.

## Setup

I tried to make the process easy as possible, but there is some action on your part to make sure it works.

1. Due to BEEMod not getting updated yet with the compiler changes needed to make this work, you will need to download the updated BEEMod compilers from the [Releases](https://github.com/TimeStall-Collective/peti2p2ce-compiler/releases/latest) page.
   In order to not need to redownload/recopy the PeTI To P2:CE compilers over the existing Portal 2 compilers everytime BEEMod exports, it is recommended that you replace the current BEEMod compilers in your BEEMod's application `bin/compiler` folder with the ones from the Releases page.
   If you wish to keep the original BEEMod compilers, rename the current VBSP and VRAD compilers there to something else that doesn't have `_original` or `_bee` in its name. These will always get copied to Portal 2 and you need to make sure they wont interfere with the game.
   For VVIS, you will just need to rename the original Portal 2 VVIS to something else since BEEMod doesn't have a replacement for it. This tool's version will be run directly by the game.
   If you are not using BEEMod, you just need to rename all the original Portal 2's compilers to have `_original` at the end of the filename.
2. This program is not able to find your P2:CE installation for you. You will need to symlink your P2:CE's VBSP, just the VBSP, to the `Portal 2/bin` directory and rename the symlink to `vbsp_p2ce.exe`.
   If you do not know how to symlink on Windows, you will need just need to open a Command Prompt (not PowerShell) window with Administrator **inside the `Portal 2/bin` directory** and use this command:
   `mklink vbsp_p2ce.exe "PATH TO P2:CE VBSP"`
   This will make a connection between P2:CE and Portal 2 via the symlink. The program does not run VBSP using the symlink and simply uses it to gather file paths using it.
   If you don't want to mess with terminal commands, you can also use Link Shell Extension to "Pick Link Source" the P2:CE VBSP compiler and then "Drop As... Symbolic Link" into the Portal 2 bin directory.
   <https://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html#download>
3. Once you have renamed the compilers and symlinked P2:CE's VBSP, go ahead and download the latest version of the compilers from the [Releases](https://github.com/TimeStall-Collective/peti2p2ce-compiler/releases/latest) page on the GitHub repository.
4. Copy the downloaded compilers into Portal 2's bin directory. If you get any dialog that says to replace existing files, close the prompt and make sure you renamed everything as mentioned in step 1.
5. Go into P2:CE's `gameinfo.txt` under the `p2ce` folder and add `Game    |gameinfo_path|../bee2` as the last item in the `SearchPaths` section.
6. That should be it! Open Portal 2 with Steam or BEEMod's application, open PeTI, and then compile a map. It should compile, copy over to P2:CE, and run it in P2:CE.

> [!CAUTION]
> 
> When PeTI finishes compiling after the VRAD stage, it will report that it failed to compile. ***THIS IS NORMAL!*** Assuming that nothing actually went wrong.
> 
> If something does go wrong, a popup will appear (on Windows) and P2:CE will not launch or be called to load the map. The whole console output will be in Portal 2's console for you to check for errors.
> 
> Because the PeTI map has been compiled with Strata's compilers, Portal 2 will fail to load the PeTI BSP and will exit the game with an error box.
> 
> To prevent this from happening, this tool has to give an "error" return code back to the game so it stops in it tracks before it can load the BSP. Anything that isn't `0` is considered an error for the game, so `1` is passed to it.

### Uninstalling

There are multiple ways of going about it. Easiest way is to delete the compilers then rename the original BEEMod or Portal 2 compilers back to normal.

You can also verify the game files and that should replace them with the originals. But if you did have `vbsp_original` and such, then you'll have two original compilers.

Another way is to remove any compiler of sorts of VBSP and such, then verify the game files and rerun the BEEMod export if you still wanted BEEMod.

### Custom FGDs For AngelScript Entities

Due to how Valve made instances in VBSP work, and in turn how Strata's VBSP works, instances and their entities inside them all must have an entry in the "main" FGD of the game. This FGD is defined in the `gameinfo.txt`, and for P2:CE's case it is the `p2ce.fgd`.

All standard entities both in Portal 2 and P2:CE are already in here so you don't have to worry about them, but you will for custom AngelScript entities.

For custom AngelScript entities to work that are in BEEMod UCPs, which are instances, the custom AngelScript entity must be defined in the P2:CE's FGD.

While putting them into `p2ce.fgd` works, it is annoying if the game files are verified or updated and the `p2ce.fgd` is reset or updated wiping your entity entries.

This compiler has been programmed to support having separate FGDs by parsing FGD files from a `custom_fgds` folder.  This folder is not made by default and is made by the user in the `p2ce` directory.

Once made, FGDs can be placed or symlinked into this folder which will be used during compile to make AngelScript entities work. Make sure to check the P2:CE console for any AngelScript errors if something doesn't run correctly.

## Other Information

### Cooperative Partner PeTI Map

As of writing, the program can't detect if you are making a cooperative map in PeTI. Once P2:CE loads the map, you will need to exit back to the menu and put in the console:
```
   sv_portal_gamemode_override coop
   ss_map preview
```

This should load the PeTI map properly and allow you to play the map in splitscreen coop. Future compiles will still allow you to play the map properly in splitscreen after each map change.

In order to switch between the cooperative robots in the map, run `in_forceuser 1` to switch to PBody. I recommend binding this key using `BindToggle <key> in_forceuser`.

### What if I need help with something or something went wrong?

Feel free to make an issue post on the GitHub [Issues](https://github.com/TimeStall-Collective/peti2p2ce-compiler/issues) page, and I should be able to get around to it one day.

Please **do not** ask TeamSpen210 for help with any issues with this tool and making it work with BEEMod since this is overall just a hack and isn't something that will be actively supported by him.
If there is an issue with this and BEEMod at all, bug me, not him.

### Why Does It Still Say Portal 2 Is Running When P2:CE Is Running?

This is a side effect of running P2:CE from Portal 2, which causes P2:CE to run off of Portal 2's AppID according to Steam. There isn't anything inherently bad that will happen in P2:CE from this so far as I can tell.

One thing that has happened is that it won't load any keybinds for P2:CE if you have custom keybinds, but apart from that, nothing too bad.

If anything too major comes up that does have a big impact, I'll take another look at making sure it's run as its own proper process.

### Stuff I Still Have To Do With This Tool

* Automatically append the `Game    |gameinfo_path|../bee2` line to P2:CE's `gameinfo.txt` for the user.
* Finish Linux support and make native Linux P2:CE map compilers work with the compiler.
* Detect and properly load cooperative mode PeTI maps.

## Credits

### Special Thanks

Thanks a ton to [TeamSpen210](https://github.com/TeamSpen210) for helping me work through all the details of dealing with BEEMod's compilers with this tool, along with reviewing and accepting the BEEMod PR that makes this tool work properly with BEEMod.

### Crediting

It would be greatly appreciated if you used this tool for your project and gave the tool a shoutout. If you distribute the tool anywhere, whether with or without the source code, the MIT license in the repository must be included with it. Thank you!
