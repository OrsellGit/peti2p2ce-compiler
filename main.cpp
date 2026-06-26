/**
* @brief   Main entry point for all the compiler.
* @details This VBSP compiler will replace Portal 2's VBSP getting called by Portal 2
*          to perform various operations before running P2:CE's compilers.
* @authors Orsell
*
* @license Distributed under the MIT license.
*/

// TODO: Automatically append the `Game    |gameinfo_path|../bee2` line to P2:CE's `gameinfo.txt` for the user.
// TODO: Finish Linux support and make native Linux P2:CE map compilers work with the compiler.
// TODO: Detect and properly load cooperative mode PeTI maps.

#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "logger.h"
#include "process_man.h"

#define TOOL_VERSION "1.1.0"

/**
 * @brief Get what the file type of executables are on the system. ".exe" - Windows "" - Linux
 * @return Executable file type for the system.
 */
static std::string GetExecutableExtension()
{
    #ifdef _WIN32
        return ".exe";
    #else
        return "";
    #endif
}

/**
 * @brief Function call used to run Portal 2: Community Edition, supplying it with the information it needs to run.
 * @warning Side effect of starting P2:CE through
 * @param p2ceGameDir Location of P2:CE's game directory. This should normally be P2:CE's "p2ce" game directory.
 * @param exePath Location of P2:CE's executable path.
 * @param bspPath Location of the BSP that has been compiled. This is used to get the map name to be run.
 * @param usingBEE If BEEMod is being used.
 * @return Return code of Portal 2: Community Edition.
 */
static int RunP2CE( const std::filesystem::path& p2ceGameDir, const std::filesystem::path& exePath, const std::filesystem::path& bspPath, const bool usingBEE )
{
    Logger::DevLog(std::format("Running P2:CE with p2ceGameDir: {}, exePath: {}, bspPath: {} & usingBEE: {}", p2ceGameDir.string(), exePath.string(), bspPath.string(), usingBEE));
    Logger::DevLog(std::format("[RunP2CE] Copy Destination: {}", (p2ceGameDir / "maps" / bspPath.filename()).string()));
    std::filesystem::copy_file(bspPath, p2ceGameDir / "maps" / bspPath.filename(), std::filesystem::copy_options::overwrite_existing);

    Process p2ce;
    if (!p2ce.Start(
        std::format(R"("{}" -as_debug -multirun -hijack -dev -conclearlog -condebug +map {})", exePath.string(), bspPath.stem().string()),
        "",
        nullptr, // Portal 2 does not need to get P2:CE's output.
        true
    ))
    {
        Logger::Log("[RunP2CE] Failed to start Portal 2: Community Edition! Check above for errors!", true);
        return 1;
    }

    // P2:CE will not be waited on to finish running. Need to get back control of PeTI to continue making edits!
    int exitCode = p2ce.Wait(false);
    Logger::Log(std::format("[RunP2CE] Portal 2: Community Edition exited with code: {}", exitCode));
    return exitCode;
}

/**
 *
 * @param p2ceGameDir Game folder that BEEMod and P2:CE compilers will use. Folder that contains gameinfo.txt. This should normally be P2:CE's "p2ce" game directory.
 * @param vmfPath Path to the VMF file that will be processed.
 * @param p2ceBinDir Path to the P2:CE's bin folder.
 * @param usingBEE If BEEMod is being used, will make sure to perform BEEMod related steps.
 * @return 0 if whole process was successful, else 1.
 */
static int RunVBSP(const std::filesystem::path& p2ceGameDir, const std::filesystem::path& vmfPath, const std::filesystem::path& p2ceBinDir, const bool usingBEE)
{
    Logger::Log("", false, true);
    Logger::Log("------------------ BEGIN PETI TO P2CE VBSP COMPILE ------------------", false, true);

    int exitCode;

    if (usingBEE)
    {
        Process vbspBEE;
        if (!vbspBEE.Start(
            std::format(R"(./vbsp_bee.exe -force_peti -skip_vbsp -game "{}" "{}")", p2ceGameDir.string(), vmfPath.string()),
            "",
            [](const std::string& text, const bool isSTDErr)
            {
                // Can't use Log functions as it will add another return carriage.
                if (isSTDErr)
                    std::cout << "[vbsp-peti2p2ce] [ERROR] [RunVBSP] [BEEMod VBSP] " << text;
                else
                    std::cout << "[vbsp-peti2p2ce] [RunVBSP] [BEEMod VBSP] " << text;
            }
        ))
        {
            Logger::Log("[RunVBSP] Failed to start BEEMod VBSP! Check above for errors!", true);
            Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
            Logger::Log("", false, true);
            return 1;
        }

        exitCode = vbspBEE.Wait();
        if (exitCode != 0)
        {
            Logger::Log( std::format("[RunVBSP] BEEMod VBSP returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
            Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
            Logger::Log("", false, true);
            return 1;
        }

        Logger::Log(std::format("[RunVBSP] BEEMod VBSP exited with code: {}", exitCode));
    }

    // BEEMod's VBSP generates a corrected/styled VMF that should be used by the other compilers when BEEMod is used.
    std::filesystem::path newVMFPath = vmfPath;
    if (usingBEE)
        newVMFPath = vmfPath.parent_path() / "styled" / vmfPath.filename();

    Process vbspP2CE;
    if (!vbspP2CE.Start(
        std::format(R"({}/vbsp.exe -game "{}" "{}")", p2ceBinDir.string(), p2ceGameDir.string(), newVMFPath.string()),
        p2ceBinDir,
        [](const std::string& text, const bool isSTDErr)
        {
            // Can't use Log functions as it will add another return carriage.
            if (isSTDErr)
                std::cout << "[vbsp-peti2p2ce] [ERROR] [RunVBSP] [P2:CE VBSP] " << text;
            else
                std::cout << "[vbsp-peti2p2ce] [RunVBSP] [P2:CE VBSP] " << text;
        }
    ))
    {
        Logger::Log("[RunVBSP] Failed to start P2:CE VBSP! Check above for errors!", true);
        Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    exitCode = vbspP2CE.Wait();
    if (exitCode != 0)
    {
        Logger::Log( std::format("[RunVBSP] P2:CE VBSP returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    Logger::Log(std::format("[RunVBSP] P2:CE VBSP exited with code: {}", exitCode));

    if (!std::filesystem::exists(newVMFPath.parent_path() / "preview.lin"))
    {
        Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return exitCode;
    }

    // TODO: There is probably a better way I could go about this without running BEEMod again

    if (!usingBEE)
    {
        Logger::Log("[RunVBSP] LEAK DETECTED!!! A leak has been detected and BEEMod isn't being used! Please verify Portal 2's files through Steam if you get this error!", true);

        Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    Logger::Log("[RunVBSP] LEAK DETECTED!!! Running BEEMod VBSP again but allowing it to sort out the leak.", true);

    Process vbspBEE;
    if (!vbspBEE.Start(
        std::format(R"(./vbsp_bee.exe -force_peti -game "{}" "{}")", p2ceGameDir.string(), vmfPath.string()),
        "",
        [](const std::string& text, const bool isSTDErr)
        {
            // Can't use Log functions as it will add another return carriage.
            if (isSTDErr)
                std::cout << "[vbsp-peti2p2ce] [ERROR] [RunVBSP] [BEEMod VBSP] " << text;
            else
                std::cout << "[vbsp-peti2p2ce] [RunVBSP] [BEEMod VBSP] " << text;
        }
    ))
    {
        Logger::Log("[RunVBSP] Failed to start BEEMod VBSP! Check above for errors!", true);
        Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    exitCode = vbspBEE.Wait();
    if (exitCode != 0)
    {
        Logger::Log( std::format("[RunVBSP] BEEMod VBSP returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    Logger::Log(std::format("[RunVBSP] BEEMod VBSP exited with code: {}", exitCode));
    Logger::Log("------------------ END PETI TO P2CE VBSP COMPILE ------------------", false, true);
    Logger::Log("", false, true);
    return exitCode;
}

/**
 * @brief BEEMod doesn't have a VVIS compiler, so this will just call P2:CE's VVIS.
 * @param p2ceGameDir Game folder that BEEMod P2:CE compilers will use. Folder that contains gameinfo.txt. This should normally be P2:CE's "p2ce" game directory.
 * @param bspPath Path to BSP generated by VBSP prior.
 * @param p2ceBinDir Path to the P2:CE's bin folder.
 * @return 0 if whole process was successful, else 1.
 */
static int RunVVIS( const std::filesystem::path& p2ceGameDir, const std::filesystem::path& bspPath, const std::filesystem::path& p2ceBinDir )
{
    Logger::Log("", false, true);
    Logger::Log("------------------ BEGIN PETI TO P2CE VVIS COMPILE ------------------", false, true);

    Process vvisP2CE;
    if (!vvisP2CE.Start(
    std::format(R"({}/vvis.exe -game "{}" "{}")", p2ceBinDir.string(), p2ceGameDir.string(), bspPath.string()),
    "",
    [](const std::string& text, const bool isSTDErr)
    {
        // Can't use Log functions as it will add another return carriage.
        if (isSTDErr)
            std::cout << "[vvis-peti2p2ce] [ERROR] [RunVVIS] [P2:CE VVIS] " << text;
        else
            std::cout << "[vvis-peti2p2ce] [RunVVIS] [P2:CE VVIS] " << text;
    }
    ))
    {
        Logger::Log("[RunVVIS] Failed to start P2:CE VVIS! Check above for errors!", true);
        Logger::Log("------------------ END PETI TO P2CE VVIS COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    int exitCode = vvisP2CE.Wait();
    if (exitCode != 0)
    {
        Logger::Log( std::format("[RunVVIS] P2:CE VVIS returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        Logger::Log("NOTE: ERRORS FROM VVIS WILL BE IGNORED AS NORMAL PETI STILL WORKS WITH MAP LEAKS!");
        Logger::Log("------------------ END PETI TO P2CE VVIS COMPILE ------------------", false, true);
        Logger::Log("", false, true);

        //! For whatever reason, Valve had programmed PeTI to not fail when there are map leaks. Normally VVIS needs a .prt file generated by VBSP to process, but leaks don't generate one.
        //! While VVIS errors, PeTI is programmed to ignore that error and continue. This behavior is NOT caused by BEEMod, but is utilized by BEEMod to make the error server to work when BEEMod's VRAD compile is run.
        //! We must return 0 for VVIS whenever it errors due to leaks so BEEMod's VRAD error server can take over.
        return 0;
    }

    Logger::Log(std::format("[RunVVIS] P2:CE VVIS exited with code: {}", exitCode));
    Logger::Log("------------------ END PETI TO P2CE VVIS COMPILE ------------------", false, true);
    Logger::Log("", false, true);

    return exitCode;
}

/**
 * @brief Run BEEMod's VRAD followed by P2:CE's VRAD. If a leak or some other error occurred in prior steps, BEEMod should return a error and also start the error server for the user.
 * @param p2ceGameDir Game folder that BEEMod VBSP and P2:CE VBSP will use. Folder that contains gameinfo.txt. This should normally be P2:CE's "p2ce" game directory.
 * @param bspPath Path to BSP generated by VBSP prior.
 * @param p2ceBinDir Path to the P2:CE's bin folder.
 * @param usingBEE If BEEMod is being used, will make sure to perform BEEMod related steps.
 * @return 0 if whole process was successful, else 1.
 */
static int RunVRAD( const std::filesystem::path& p2ceGameDir, const std::filesystem::path& bspPath, const std::filesystem::path& p2ceBinDir, const bool usingBEE )
{
    Logger::Log("", false, true);
    Logger::Log("------------------ BEGIN PETI TO P2CE VRAD COMPILE ------------------", false, true);

    int exitCode;

    if (usingBEE)
    {
        Process vradBEE;
        if (!vradBEE.Start(
            std::format(R"(./vrad_bee.exe -force_peti -skip_vrad -game "{}" "{}")", p2ceGameDir.string(), bspPath.string()),
            "",
            [](const std::string& text, const bool isSTDErr)
            {
                // Can't use Log functions as it will add another return carriage.
                if (isSTDErr)
                    std::cout << "[vrad-peti2p2ce] [ERROR] [RunVRAD] [BEEMod VRAD] " << text;
                else
                    std::cout << "[vrad-peti2p2ce] [RunVRAD] [BEEMod VRAD] " << text;
            }
        ))
        {
            Logger::Log("[RunVRAD] Failed to start BEEMod VRAD! Check above for errors!", true);
            Logger::Log("------------------ END PETI TO P2CE VRAD COMPILE ------------------", false, true);
            Logger::Log("", false, true);
            return 1;
        }

        exitCode = vradBEE.Wait();
        if (exitCode != 0)
        {
            Logger::Log( std::format("[RunVRAD] BEEMod VRAD returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
            Logger::Log("------------------ END PETI TO P2CE VRAD COMPILE ------------------", false, true);
            Logger::Log("", false, true);
            return 1;
        }

        Logger::Log(std::format("[RunVRAD] BEEMod VRAD exited with code: {}", exitCode));
    }

    Process vradP2CE;
    if (!vradP2CE.Start(
    std::format(R"({}/vrad.exe -fast -staticproplighting -staticproppolys -game "{}" "{}")", p2ceBinDir.string(), p2ceGameDir.string(), bspPath.string()),
    "",
    [](const std::string& text, const bool isSTDErr)
    {
        // Can't use Log functions as it will add another return carriage.
        if (isSTDErr)
            std::cout << "[vrad-peti2p2ce] [ERROR] [RunVRAD] [P2:CE VRAD] " << text;
        else
            std::cout << "[vrad-peti2p2ce] [RunVRAD] [P2:CE VRAD] " << text;
    }
    ))
    {
        Logger::Log("[RunVRAD] Failed to start P2:CE VRAD! Check above for errors!", true);
        Logger::Log("------------------ END PETI TO P2CE VRAD COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    exitCode = vradP2CE.Wait();
    if (exitCode != 0)
    {
        Logger::Log( std::format("[RunVRAD] P2:CE VRAD returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        Logger::Log("------------------ END PETI TO P2CE VRAD COMPILE ------------------", false, true);
        Logger::Log("", false, true);
        return 1;
    }

    Logger::Log(std::format("[RunVRAD] P2:CE VRAD exited with code: {}", exitCode));
    Logger::Log("------------------ END PETI TO P2CE VRAD COMPILE ------------------", false, true);
    Logger::Log("", false, true);

    return exitCode;
}

/**
 * @brief Portal 2 will run this version of the VBSP, VVIS, or VRAD instead of its normal version.
 *        Here some setup occurs before running the respective BEEMod compilers if applicable before then running Portal 2: Community Edition's map compilers.
 * @param argc Number of arguments passed to the program.
 * @param argv Arguments passed to the program.
 *             Portal 2 passes in to vbsp.exe:
 *             (0) Executable location, normally located at "Portal 2/bin/vbsp.exe"
 *             (1) "-entity_limit" The flag to override the entity limit which is its own argument in the array. Only used to determine if this is a PeTI compile or not.
 *             (2) "1750" The set entity limit for PeTI.
 *             (3) "-game" Flag to set the game folder where the gameinfo.txt is location.
 *             (4) Game folder location, defaults to "Portal 2/portal2".
 *             (5) Map VMF location, defaults to "Portal 2/sdk_content/maps/preview.vmf"
 *
 *             .../Steam/steamapps/common/Portal 2/bin/vbsp.exe
 *             -entity_limit
 *             1750
 *             -game
 *             .../Steam/steamapps/common/Portal 2/portal2/
 *             .../Steam/steamapps/common/Portal 2/sdk_content\maps/preview.vmf
 *
 *             Portal 2 passes in to vvis.exe:
 *             (0) Executable location, normally located at "Portal 2/bin/vvis.exe"
 *             (1) "-game" Flag to set the game folder where the gameinfo.txt is location.
 *             (2) Game folder location, defaults to "Portal 2/portal2".
 *             (3) Map BSP location, defaults to "Portal 2/sdk_content/maps/preview.bsp"
 *
 *             .../Steam/steamapps/common/Portal 2/bin/vvis.exe
 *             -game
 *             .../Steam/steamapps/common/Portal 2/portal2/
 *             .../Steam/steamapps/common/Portal 2/sdk_content\maps/preview.bsp
 *
 *             Portal 2 passes in to vrad.exe:
 *             (0) Executable location, normally located at "Portal 2/bin/vrad.exe"
 *             (1-5) VRAD compile flags for making sure lighting is compiled for the map.
 *             (6) "-game" Flag to set the game folder where the gameinfo.txt is location.
 *             (7) Game folder location, defaults to "Portal 2/portal2".
 *             (8) Map BSP location, defaults to "Portal 2/sdk_content/maps/preview.bsp"
 *
 *             .../Steam/steamapps/common/Portal 2/bin/vrad.exe
 *             -final
 *             -staticproppolys
 *             -staticproplighting
 *             -textureshadows
 *             -hdr
 *             -game
 *             .../Steam/steamapps/common/Portal 2/portal2/
 *             .../Steam/steamapps/common/Portal 2/sdk_content\maps/preview.bsp
 *
 * @return Return code of the program. Portal 2 will recognize any return code that is not zero to be an error code.
 */
int main( const int argc, char* argv[] )
{
#ifdef _DEBUG
    for (int i = 0; i < argc; i++)
        std::cout << argv[i] << std::endl;
    std::cout << std::endl;
#else
    (void)argc;
#endif

    const std::filesystem::path curExecPath = argv[0];
    const std::string execName = curExecPath.stem().string();

    Logger::InitLogging(curExecPath.parent_path(), execName);

    // Check if the symlink for P2:CE's VBSP is present.
    if (!std::filesystem::exists(std::format("./vbsp_p2ce{}", GetExecutableExtension())))
    {
        Logger::Log("[main] Symlinked P2:CE VBSP not found! Needed for getting P2:CE installation location!", true);
        return 1;
    }

    Logger::DevLog(std::format("[main] curExecPath: {}", curExecPath.string()));
    Logger::DevLog(std::format("[main] execName: {}", execName));

    // BEEMod usage is solely dependant if the renamed BEEMod VBSP exists.
    const bool usingBEE = std::filesystem::exists(std::format("./vbsp_bee{}", GetExecutableExtension()));
    Logger::DevLog(std::format("[main] usingBEE?: {}", usingBEE));

    Logger::Log(std::format("Running PeTI to P2:CE Compiler! | Version: {} | Compiler Behavior: {} | Using BEEMod?: {}", TOOL_VERSION, execName, usingBEE));

    // Check if this is in fact a PeTI compile. This assumes that the first argument is going to be "-entity_limit". This check is really the only safeguard against Hammer, should probably be changed.
    if (std::strcmp(argv[1], "-entity_limit") != 0 && execName == "vbsp")
    {
        Logger::Log("[main] This is not a PeTI compile! Assuming this is a standard Hammer compile! Please go into Hammer and set it to use the '_original' variants of the Portal 2 compilers!", true);
        return 1;
    }

    // After verifying that the symlink exists, it can be used to pull directory information so canonical paths can be used with the compilers.
    const std::filesystem::path p2ceVBSPExecPath = std::filesystem::read_symlink(std::format("./vbsp_p2ce{}", GetExecutableExtension()));
    const std::filesystem::path p2ceBinDir = p2ceVBSPExecPath.parent_path();
    const std::filesystem::path p2ceGameDir = std::filesystem::canonical(p2ceBinDir / "../../p2ce");
    const std::filesystem::path p2ceGameBaseDir = std::filesystem::canonical(p2ceGameDir / "../");

    Logger::DevLog(std::format("[main] p2ceVBSPExecPath?: {}", p2ceVBSPExecPath.string()));
    Logger::DevLog(std::format("[main] p2ceBinDir?: {}", p2ceBinDir.string()));
    Logger::DevLog(std::format("[main] p2ceGameDir?: {}", p2ceGameDir.string()));
    Logger::DevLog(std::format("[main] p2ceGameBaseDir?: {}", p2ceGameBaseDir.string()));

    // Run the compilers.
    if (execName == "vbsp")
    {
        Logger::Log("Running VBSP compiler behavior!");

        std::filesystem::path vmfPath = argv[5];
        const std::filesystem::path p2GameBaseDir = std::filesystem::canonical(std::filesystem::path(argv[4]) / "../" );

        Logger::DevLog(std::format("[main] vmfPath: {}", vmfPath.string()));
        Logger::DevLog(std::format("[main] p2GameBaseDir: {}", p2GameBaseDir.string()));

        // Add the Sentry consent file so that the consent popup doesn't happen each compile with PeTI.
        if (!std::filesystem::exists(p2GameBaseDir.parent_path() / "_sentry") || !std::filesystem::exists(p2GameBaseDir.parent_path() / "_sentry/user-consent"))
        {
            Logger::Log("No '_sentry' folder or 'user-consent' file not found! Adding a folder with a 'user-consent' file defaulting to no consent!");

            std::filesystem::create_directory(p2GameBaseDir.parent_path() / "_sentry");
            std::fstream consentFile(p2GameBaseDir.parent_path() / "_sentry/user-consent", std::ios_base::out);
            if (!consentFile.is_open() || !consentFile.good() || consentFile.bad() || consentFile.fail())
            {
                Logger::Log("Failed to create consent file for Sentry! Not required but Sentry prompt will appear on every compiler run!", true);
                Logger::ShowPopUp("WARNING",
                    "The PeTI To P2:CE compiler failed to automatically add the 'user-consent' file to silence Sentry popups.\n\n"
                                "This is not required to compile but helps prevent the consent popup from showing for every P2:CE compiler when they're run.\n"
                                "If you wish to not get constant popups, please make a '_sentry' folder in your 'Portal 2/bin' directory and add a 'user-consent' file with a single 0 inside as its contents.\n"
                                "This will also help prevent any Sentry reports from getting sent to the P2:CE team as those reports will be invalid and do not apply to P2:CE properly.\n\n"
                                "Click OK to continue."
                );
            }
            else
            {
                consentFile << 0 << std::endl; // Sentry should not run when used with this compiler tool!
                consentFile.close();
            }
        }

        // Check if P2:CE's FGD properly exists.
        if (!std::filesystem::exists(p2ceGameDir / "p2ce.fgd"))
        {
            Logger::Log("Failed to find P2:CE's FGD! This is required for compiling! Please verify your files for Portal 2: Community Edition on Steam!", true);
            Logger::ShowPopUp("ERROR", "Failed to find P2:CE's FGD! This is required for compiling! Please verify your files for Portal 2: Community Edition on Steam!");
            return 1;
        }

        // Parse the contents of custom FGDs into the "main" P2:CE FGD "p2ce.fgd" to make custom AngelScript entities work.
        if (std::filesystem::exists(p2ceGameDir / "custom_fgds"))
        {
            Logger::Log("[main] Backing up current P2:CE FGD.");
            std::filesystem::copy_file(p2ceGameDir / "p2ce.fgd", p2ceGameDir / "p2ce_original.fgd");

            Logger::Log("[main] Iterating through 'custom_fgds' folder for any FGDs to copy into the 'p2ce.fgd'.");

            std::fstream p2ceFGDFileStream(p2ceGameDir / "p2ce.fgd", std::ios_base::app);
            if (!p2ceFGDFileStream.is_open() || !p2ceFGDFileStream.good() || p2ceFGDFileStream.bad() || p2ceFGDFileStream.fail())
            {
                Logger::Log("[main] Failed to open P2:CE's FGD for copying in custom AngelScript entity entries! Make sure the file isn't opened and getting modified in any other program.", true);

                if (std::filesystem::exists(p2ceGameDir / "p2ce_original.fgd"))
                {
                    Logger::DevLog("Cleaning up FGDs before stopping compile.");
                    std::filesystem::rename(p2ceGameDir / "p2ce_original.fgd", p2ceGameDir / "p2ce.fgd");
                }

                Logger::ShowPopUp("ERROR", "Failed to open P2:CE's FGD for copying in custom AngelScript entity entries! Make sure the file isn't opened and getting modified in any other program.", true);
                return 1;
            }

            int numCopiedFGDs = 0;
            for (const auto& directoryEntry : std::filesystem::recursive_directory_iterator(p2ceGameDir / "custom_fgds", std::filesystem::directory_options::follow_directory_symlink | std::filesystem::directory_options::skip_permission_denied))
            {
                const std::filesystem::path& fgdPath = directoryEntry.path();
                Logger::DevLog(std::format("Current path in 'custom_fgds': {}", fgdPath.string()));

                if (fgdPath.extension() != ".fgd")
                    continue;

                std::fstream fgdFileStream(fgdPath, std::ios_base::in);
                if (!fgdFileStream.is_open() || !fgdFileStream.good() || fgdFileStream.bad() || fgdFileStream.fail())
                {
                    Logger::Log(std::format("[main] Unable to open FGD file '{}' for reading! Ignoring.", fgdPath.filename().string()), true);
                    continue;
                }

                p2ceFGDFileStream << std::format("\n//------------------------------------------------ {}", fgdPath.filename().string()) << std::endl;
                p2ceFGDFileStream << fgdFileStream.rdbuf();
                p2ceFGDFileStream << std::format("//------------------------------------------------", fgdPath.filename().string()) << std::endl;

                fgdFileStream.close();
                numCopiedFGDs++;

                Logger::Log(std::format("[main] Copied in contents of the FGD file '{}'!", fgdPath.filename().string()));
            }

            p2ceFGDFileStream.close();
            Logger::Log(std::format("[main] Copied in the contents of {} FGD files into 'p2ce.fgd'!", numCopiedFGDs));
        }

        // Copy BEEMod assets folder so compiles work and game shows the right assets.
        if (std::filesystem::exists(p2GameBaseDir / "bee2") && usingBEE)
        {
            Logger::Log("Copying Portal 2 'bee2' folder to P2:CE base game folder...");
            std::filesystem::copy(p2GameBaseDir / "bee2", p2ceGameBaseDir / "bee2", std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
        }

        // Make sure this directory exists for VRAD to read and write caches to it.
        if (!std::filesystem::exists(p2ceGameBaseDir / "bin/bee2") && usingBEE)
            std::filesystem::create_directory(p2ceGameBaseDir / "bin/bee2");

        int exitCode = RunVBSP(p2ceGameDir, vmfPath, p2ceBinDir, usingBEE);
        if (exitCode != 0)
        {
            Logger::Log(std::format("[main] VBSP operation failed! Check above for errors! exitCode: {}", exitCode), true);

            if (std::filesystem::exists(p2ceGameDir / "p2ce_original.fgd"))
            {
                Logger::DevLog("[main] Cleaning up FGDs before stopping compile.");
                std::filesystem::rename(p2ceGameDir / "p2ce_original.fgd", p2ceGameDir / "p2ce.fgd");
            }

            Logger::ShowPopUp("ERROR", "VBSP operation failed! Please check Portal 2's console for errors!", true);
            return 1;
        }

        if (std::filesystem::exists(p2ceGameDir / "p2ce_original.fgd"))
        {
            Logger::DevLog("[main] Cleaning up FGDs.");
            std::filesystem::rename(p2ceGameDir / "p2ce_original.fgd", p2ceGameDir / "p2ce.fgd");
        }

        Logger::ShutdownLogging();

        return 0;
    }

    if (execName == "vvis")
    {
        Logger::Log("Running VVIS compiler behavior!");

        std::filesystem::path bspPath = argv[3];

        // Need to make sure to use the "styled" preview map file generated by BEEMod is used.
        if (usingBEE)
            bspPath = bspPath.parent_path() / "styled" / bspPath.filename();

        int exitCode = RunVVIS(p2ceGameDir, bspPath, p2ceBinDir);
        if (exitCode != 0)
        {
            Logger::Log("[main] VVIS operation failed! Check above for errors!", true);
            Logger::ShowPopUp("ERROR", "VVIS operation failed! Please check Portal 2's console for errors!", true);
            return 1;
        }

        Logger::ShutdownLogging();

        return 0;
    }

    if (execName == "vrad")
    {
        Logger::Log("Running VRAD compiler behavior!");

        std::filesystem::path bspPath = argv[8];
        int exitCode;

        // Need to make sure to use the "styled" preview map file generated by BEEMod is used.
        if (usingBEE)
            bspPath = bspPath.parent_path() / "styled" / bspPath.filename();

        exitCode = RunVRAD(p2ceGameDir, bspPath, p2ceBinDir, usingBEE); // TODO-FIXME: BEEMod error server is not getting run when there is a leak!
        if (exitCode != 0)
        {
            Logger::Log("[main] VRAD operation failed! Check above for errors!", true);
            Logger::ShowPopUp("ERROR", "VRAD operation failed! Please check Portal 2's console for errors!", true);
            return 1;
        }

        // Compiler does not care if P2:CE is still running as PeTI needs to be returned to when it finishes compiling.
        exitCode = RunP2CE(p2ceGameDir, p2ceBinDir / std::format("p2ce{}", GetExecutableExtension()), bspPath, usingBEE);
        if (exitCode != 0 && exitCode != STILL_ACTIVE)
        {
            Logger::Log( std::format("[main] Portal 2: Community Edition returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
            Logger::ShowPopUp("ERROR", "Portal 2: Community Edition returned with a error code! Please check Portal 2's console for errors!");
            return 1;
        }

        Logger::Log("\n\n---------------------\nTHE PETI TO P2:CE COMPILER HAS COMPLETED SUCCESSFULLY!!!\n---------------------\n\n");

        Logger::ShutdownLogging();

        return 1; //! The VRAD compiler MUST end with a error code because it must prevent Portal 2 from running the map or it will cause the game to error!
    }

    Logger::Log("[main] Invalid compiler file name! Compiler must be named appropriately with 'vbsp.exe', 'vvis.exe', or 'vrad.exe'!");
    return 1;
}
