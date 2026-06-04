# PeTI To P2:CE Compilers

## What is this?

Three small applications I developed to be called by Portal 2 to take control of the compile process and call BEEMod's compilers followed by Portal 2: Community Edition's Strata compilers instead Portal 2's.
Not only will these compile a PeTI map with the Strata compilers, it will also make sure to copy over and run the PeTI map in P2:CE.

As of writing this only will work properly for Windows as Strata doesn't have native Linux map compilers.
While my current code has been developed in preparation for the day that happens, you'll need to use Wine or similar in the meantime.

BEEMod is recommended to be used with this since it was what it was intended for, but you technically can use these without it.

## What is the benefit of this over just copying over the BSP file made by PeTI?

Mainly just to automate the process. Copying the file over and over again can get annoying for some people, and so automating it and running the game for you can be quite helpful.
Other reason is to be able to use and take advantage of Strata's new engine features with PeTI.

However, these tools will still use PeTI's limits for compatibility with BEEMod since TeamSpen210 isn't going to update BEEMod to actively support this tool if there is issues with it and BEEMod.
I personally made a update to BEEMod's compilers to support this tool, but TeamSpen210 and I will not be actively changing BEEMod to support this tool.

## Does this mean that it's possible to make PeTI content for P2:CE's workshop?

Short answer: No. Other short answer: Kinda. You would still need to do a few other things manually with the compiled BSP, setting up the addon, etc., which I will not be automating since this isn't what this tool is for.
In fact, I do not recommend using this at all if you want to make PeTI content and post it to the P2:CE workshop.
P2:CE will one day get its own puzzle maker so if you want something more refined, then just wait for that.

## Because this is using Strata's compilers, does this mean it can use the most if not all the Strata engine's newer features?

Theoretically, yes. You can make an instance with P2:CE entities and turn it into a UCP for BEEMod to add into PeTI, but do not expect it to just work out of the box for everything.
While I was developing this tool to try and make some P2:CE features work in tandem with PeTI and BEEMod, I have not tested everything, as this is geared to projects I am working on and not anything else others are doing.
One of those features is custom AngelScript entities. In my testing those work fine as long as the required scripts and other assets are available on P2:CE's end and your instance/UCP isn't doing anything too wacky.

> [WARNING!]
> There is the issue where custom AngelScript entities in instances won't be processed correctly if they don't have an entry in the "main" FGD defined in P2:CE's `gameinfo.txt`.
> This "main" FGD normally is `p2ce.fgd`. If your AngelScript entity does not have an entry with all its KeyValues and I/O defined, it won't be properly compiled into the map.

## Ok so how do I set it up?

I tried to make the process easy as possible, but there is some action on your part to make sure it works.

1. In your Portal 2's bin directory (`Portal 2/bin`), you will need to rename the current BEEMod compilers to `vbsp_bee.exe` and `vrad_bee.exe`.
   For VVIS, you will just need to rename the original Portal 2 VVIS to something else since BEEMod doesn't have a replacement for it. This tool's version will be run directly by the game.
   If you are not using BEEMod for this, you just need to rename all the original Portal 2's compilers to something else.
2. This program is not able to find your P2:CE installation for you.  You will need to symlink your P2:CE's VBSP, just the VBSP, to the `Portal 2/bin` directory and rename the symlink to `vbsp_p2ce.exe`.
   If you do not know how to symlink on Windows, you will need just need to open a Command Prompt (not Powershell) window with Administrator to use this command:
   `mklink vbsp_p2ce.exe "PATH TO P2:CE VBSP EXE"`
   This will make a connection between P2:CE and Portal 2 via the symlink. The program does not run VBSP using the symlink and simply uses it to pull paths from there using it.
   If you don't want to mess with terminal commands, you can also use Link Shell Extension to "Pick Link Source" the P2:CE VBSP compiler and then "Drop As... Symbolic Link" into the Portal 2 bin directory.
   <https://schinagl.priv.at/nt/hardlinkshellext/linkshellextension.html#download>
3. Once you have renamed the compilers and symlinked P2:CE's VBSP, go ahead and download the latest compilers from the [Releases](https://github.com/TimeStall-Collective/peti2p2ce-compiler/releases/latest) page on the GitHub repository.
4. Copy the downloaded compilers into Portal 2's bin directory. If you get any dialog that says to replace existing files, close the prompt and make sure you renamed everything as mentioned in step 1.
5. If using BEEMod: You will need to copy over or symlink the `bee2` folder that is copied from the BEEMod application to P2's base folder into P2:CE's main folder so assets get loaded by the game when map is ran.
   Once that is done, go into P2:CE's `gameinfo.txt` under the `p2ce` folder and add `Game |gameinfo_path|../bee2` as the last item in the `SearchPaths` section.
6. That should be it! Open the game with Steam or BEEMod's application, open PeTI, and then compile a map. It should compile, copy over to P2:CE, and run it in P2:CE.

> [CAUTION!]
> When PeTI finishes compiling after the VRAD stage, it will report that it failed to compile. THIS IS NORMAL! Assuming that nothing actually went wrong.
> If something does go wrong, then P2:CE will not load the map and the whole console output will be in the game's console.
> Because the PeTI map has been compiled with Strata's compilers, Portal 2 will fail to load the PeTI BSP and will exit the game with an error box.
> To prevent this from happening, this tool has to return an "error" return code back to the game so it stops in it tracks before it can load the BSP. Anything that isn't `0` is considered an error for the game, so `-1` is passed to it.

## What if I need help with something or something went wrong?

Feel free to make an issue post in the GitHub [Issues](https://github.com/TimeStall-Collective/peti2p2ce-compiler/issues) page and I should be able to get around to it.

Please do not ask TeamSpen210 for help with any issues with this tool and making it work with BEEMod since this is overall just a hack and isn't something that will be actively supported by him.
If there is an issue with this and BEEMod at all, bug me, not him.
