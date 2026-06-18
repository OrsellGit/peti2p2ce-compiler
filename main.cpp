/**
* @brief   Main entry point for all the compiler. What sets
* @details This VBSP compiler will replace Portal 2's VBSP getting called by Portal 2
*          to perform various operations before running P2:CE's compilers.
* @authors Orsell
*
* @license Distributed under the MIT license.
*/

#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "process_man.h"

static void Log( const std::string& text, const bool error = false)
{
    if (error)
        std::cout << "[PETI-TO-P2CE] [ERROR] " << text << std::endl;
    else
        std::cout << "[PETI-TO-P2CE] " << text << std::endl;
}

static void DevLog( const std::string& text, const bool error = false )
{
#if _DEBUG
    Log("[DEV] " + text, error);
#else
    (void)text;
    (void)error;
#endif
}

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
 * @param p2ceGameDir Location of P2:CE's game directory. This should normally be P2:CE's "p2ce" game directory.
 * @param exePath Location of P2:CE's executable path.
 * @param bspPath Location of the BSP that has been compiled. This is used to get the map name to be run.
 * @param usingBEE If BEEMod is being used.
 * @return Return code of Portal 2: Community Edition.
 */
static int RunP2CE( const std::filesystem::path& p2ceGameDir, const std::filesystem::path& exePath, const std::filesystem::path& bspPath, const bool usingBEE )
{
    DevLog(std::format("Running P2:CE with p2ceGameDir: {}, exePath: {}, bspPath: {} & usingBEE: {}", p2ceGameDir.string(), exePath.string(), bspPath.string(), usingBEE));

    //const std::filesystem::path mapPath = vmfPath.parent_path() / std::format("{}.bsp", vmfPath.stem().string());
    //DevLog(std::format("[RunP2CE] mapPath: {}", mapPath.string()));
    DevLog(std::format("[RunP2CE] Copy Destination: {}", (p2ceGameDir / "maps" / bspPath.filename()).string()));
    std::filesystem::copy_file(bspPath, p2ceGameDir / "maps" / bspPath.filename(), std::filesystem::copy_options::overwrite_existing);

    Process p2ce;
    if (!p2ce.Start(
        std::format(R"("{}" -as_debug -multirun -hijack -dev -condebug +map {})", exePath.string(), bspPath.stem().string()),
        "",
        [](const std::string& text, const bool isSTDErr)
        {
            // Can't use Log functions as it will add another return carriage.
            if (isSTDErr)
                std::cout << "[PETI-TO-P2CE] [ERROR] [RunP2CE] " << text;
            else
                std::cout << "[PETI-TO-P2CE] [RunP2CE] " << text;
        },
        true
    ))
    {
        Log("[RunP2CE] Failed to run Portal 2: Community Edition!", true);
        return 1;
    }

    // P2:CE will not be waited on to finish running. Need to get back control of PeTI to continue making edits!
    int exitCode = p2ce.Wait(false);
    Log(std::format("[RunP2CE] Portal 2: Community Edition exited with code: {}", exitCode));
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
    std::cout << "------------------ BEGIN PETI TO P2CE VBSP ------------------" << std::endl;

    int exitCode = 1;

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
                    std::cout << "[PETI-TO-P2CE] [ERROR] [RunVBSP] [BEEMod VBSP] " << text;
                else
                    std::cout << "[PETI-TO-P2CE] [RunVBSP] [BEEMod VBSP] " << text;
            }
        ))
        {
            Log("[RunVBSP] Failed to start BEEMod VBSP!", true);
            std::cout << "------------------ END PETI TO P2CE VBSP ------------------" << std::endl;
            return 1;
        }

        exitCode = vbspBEE.Wait();
        if (exitCode != 0)
        {
            Log( std::format("[RunVBSP] BEEMod VBSP returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
            std::cout << "------------------ END PETI TO P2CE VBSP ------------------" << std::endl;
            return 1;
        }

        Log(std::format("[RunVBSP] BEEMod VBSP exited with code: {}", exitCode));
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
                std::cout << "[PETI-TO-P2CE] [ERROR] [RunVBSP] [P2:CE VBSP] " << text;
            else
                std::cout << "[PETI-TO-P2CE] [RunVBSP] [P2:CE VBSP] " << text;
        }
    ))
    {
        Log("[RunVBSP] Failed to start P2:CE VBSP!", true);
        std::cout << "------------------ END PETI TO P2CE VBSP ------------------" << std::endl;
        return 1;
    }

    exitCode = vbspP2CE.Wait();
    if (exitCode != 0)
    {
        Log( std::format("[RunVBSP] P2:CE VBSP returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        std::cout << "------------------ END PETI TO P2CE VBSP ------------------" << std::endl;
        return 1;
    }

    Log(std::format("[RunVBSP] P2:CE VBSP exited with code: {}", exitCode));
    std::cout << "------------------ END PETI TO P2CE VBSP ------------------" << std::endl;
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
    std::cout << "------------------ BEGIN PETI TO P2CE VVIS ------------------" << std::endl;

    int exitCode = 1;

    Process vvisP2CE;
    if (!vvisP2CE.Start(
    std::format(R"({}/vvis.exe -game "{}" "{}")", p2ceBinDir.string(), p2ceGameDir.string(), bspPath.string()),
    "",
    [](const std::string& text, const bool isSTDErr)
    {
        // Can't use Log functions as it will add another return carriage.
        if (isSTDErr)
            std::cout << "[PETI-TO-P2CE] [ERROR] [RunVVIS] [P2:CE VVIS] " << text;
        else
            std::cout << "[PETI-TO-P2CE] [RunVVIS] [P2:CE VVIS] " << text;
    }
    ))
    {
        Log("[RunVVIS] Failed to start P2:CE VVIS!", true);
        std::cout << "------------------ END PETI TO P2CE VVIS ------------------" << std::endl;
        return 1;
    }

    exitCode = vvisP2CE.Wait();
    if (exitCode != 0)
    {
        Log( std::format("[RunVVIS] P2:CE VVIS returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        std::cout << "------------------ END PETI TO P2CE VVIS ------------------" << std::endl;
        return 1;
    }

    Log(std::format("[RunVVIS] P2:CE VVIS exited with code: {}", exitCode));
    std::cout << "------------------ END PETI TO P2CE VVIS ------------------" << std::endl;

    return 0;
}

/**
 * @brief Run BEEMod's VRAD followed by P2:CE's VRAD. If a leak or some other error occured in prior steps, BEEMod should return a error and also start the error server for the user.
 * @param p2ceGameDir Game folder that BEEMod VBSP and P2:CE VBSP will use. Folder that contains gameinfo.txt. This should normally be P2:CE's "p2ce" game directory.
 * @param bspPath Path to BSP generated by VBSP prior.
 * @param p2ceBinDir Path to the P2:CE's bin folder.
 * @param usingBEE If BEEMod is being used, will make sure to perform BEEMod related steps.
 * @return 0 if whole process was successful, else 1.
 */
static int RunVRAD( const std::filesystem::path& p2ceGameDir, const std::filesystem::path& bspPath, const std::filesystem::path& p2ceBinDir, const bool usingBEE )
{
    std::cout << "------------------ BEGIN PETI TO P2CE VRAD ------------------" << std::endl;

    int exitCode = 1;

    Process vradP2CE;
    if (!vradP2CE.Start(
    std::format(R"({}/vrad.exe -game "{}" "{}")", p2ceBinDir.string(), p2ceGameDir.string(), bspPath.string()),
    "",
    [](const std::string& text, const bool isSTDErr)
    {
        // Can't use Log functions as it will add another return carriage.
        if (isSTDErr)
            std::cout << "[PETI-TO-P2CE] [ERROR] [RunVRAD] [P2:CE VRAD] " << text;
        else
            std::cout << "[PETI-TO-P2CE] [RunVRAD] [P2:CE VRAD] " << text;
    }
    ))
    {
        Log("[RunVRAD] Failed to start P2:CE VRAD!", true);
        std::cout << "------------------ END PETI TO P2CE VRAD ------------------" << std::endl;
        return 1;
    }

    exitCode = vradP2CE.Wait();
    if (exitCode != 0)
    {
        Log( std::format("[RunVRAD] P2:CE VRAD returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
        std::cout << "------------------ END PETI TO P2CE VRAD ------------------" << std::endl;
        return 1;
    }

    Log(std::format("[RunVRAD] P2:CE VRAD exited with code: {}", exitCode));
    std::cout << "------------------ END PETI TO P2CE VRAD ------------------" << std::endl;

    return 0;
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
 *             (1) "-game" Flag to set the game folder where the gameinfo.txt is location.
 *             (2) Game folder location, defaults to "Portal 2/portal2".
 *             (3) Map BSP location, defaults to "Portal 2/sdk_content/maps/preview.bsp"
 *
 *             .../Steam/steamapps/common/Portal 2/bin/vrad.exe
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
#endif

    // Check if the symlink for P2:CE's VBSP is present.
    if (!std::filesystem::exists(std::format("./vbsp_p2ce{}", GetExecutableExtension())))
    {
        Log("[main] Symlinked P2:CE VBSP not found! Needed for getting P2:CE installation location!", true);
        return 1;
    }

    const std::filesystem::path curExecPath = argv[0];
    const std::string execName = curExecPath.stem().string();

    DevLog(std::format("[main] curExecPath: {}", curExecPath.string()));
    DevLog(std::format("[main] execName: {}", execName));

    // Check if this is in fact a PeTI compile. This assumes that the first argument is going to be "-entity_limit". This check is really the only safeguard against Hammer, should probably be changed.
    if (std::strcmp(argv[1], "-entity_limit") != 0 && execName == "vbsp")
    {
        Log("[main] This is not a PeTI compile! Assuming this is a standard Hammer compile! Please go into Hammer and set it to use the '_original' variants of the Portal 2 compilers!", true);
        return 1;
    }

    // BEEMod usage is solely dependant if the renamed BEEMod VBSP exists.
    const bool usingBEE = std::filesystem::exists(std::format("./vbsp_bee{}", GetExecutableExtension()));
    DevLog(std::format("[main] usingBEE?: {}", usingBEE));

    // After verifying that the symlink exists, we can use it to pull directory information so canonical paths can be used with the compilers.
    const std::filesystem::path p2ceVBSPExecPath = std::filesystem::read_symlink(std::format("./vbsp_p2ce{}", GetExecutableExtension()));
    const std::filesystem::path p2ceBinDir = p2ceVBSPExecPath.parent_path();
    const std::filesystem::path p2ceGameDir = std::filesystem::canonical(p2ceBinDir / "../../p2ce");
    const std::filesystem::path p2ceGameBaseDir = std::filesystem::canonical(p2ceGameDir / "../");

    DevLog(std::format("[main] p2ceVBSPExecPath?: {}", p2ceVBSPExecPath.string()));
    DevLog(std::format("[main] p2ceBinDir?: {}", p2ceBinDir.string()));
    DevLog(std::format("[main] p2ceGameDir?: {}", p2ceGameDir.string()));
    DevLog(std::format("[main] p2ceGameBaseDir?: {}", p2ceGameBaseDir.string()));

    // Run the compilers.
    if (execName == "vbsp")
    {
        std::filesystem::path vmfPath = argv[5];
        const std::filesystem::path p2GameBaseDir = std::filesystem::canonical(std::filesystem::path(argv[4]) / "../" );

        DevLog(std::format("[main] vmfPath: {}", vmfPath.string()));
        DevLog(std::format("[main] p2GameBaseDir: {}", p2GameBaseDir.string()));

        // Add the Sentry consent file so that the consent popup doesn't happen each compile with PeTI.
        if (!std::filesystem::exists(curExecPath.parent_path() / "_sentry") || !std::filesystem::exists(curExecPath.parent_path() / "_sentry/user-consent"))
        {
            Log("No '_sentry' folder or user consent file not found! Adding a folder with a user consent file defaulting to no consent!");

            std::filesystem::create_directory(curExecPath.parent_path() / "_sentry");
            std::fstream consentFile;
            consentFile.open(curExecPath.parent_path() / "_sentry/user-consent", std::ios_base::out);
            if (!consentFile.is_open() || !consentFile.good() || consentFile.bad() || consentFile.fail())
                Log("Failed to create consent file for Sentry! Not required but Sentry prompt will appear on every compiler run!", true);
            else
            {
                consentFile << 0 << std::endl; // Sentry should not run when used with this compiler tool!
                consentFile.close();
            }
        }

        // TODO: Consider adding `Game	|gameinfo_path|../bee2` to `gameinfo.txt` for them.

        // Copying over BEEMod assets along with PeTI and BEEMod instances. This will always be done to keep files up to date.`
        if (std::filesystem::exists(p2GameBaseDir / "bee2"))
        {
            Log("Copying Portal 2 'bee2' folder to P2:CE base game folder...");
            std::filesystem::copy(p2GameBaseDir / "bee2", p2ceGameBaseDir / "bee2", std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);
        }

        Log("Copying Portal 2 instances to P2:CE...");
        std::filesystem::copy(p2GameBaseDir / "sdk_content/maps/instances", p2ceGameBaseDir / "sdk_content/maps/instances", std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing);

        int exitCode = RunVBSP(p2ceGameDir, vmfPath, p2ceBinDir, usingBEE);
        if (exitCode == 1)
        {
            Log(std::format("[main] VBSP operation failed! Check above for errors! exitCode: {}", exitCode), true);
            return 1;
        }

        return 0;
    }

    if (execName == "vvis")
    {
        std::filesystem::path bspPath = argv[3];

        // Need to make sure to use the "styled" preview map file generated by BEEMod is used.
        if (usingBEE)
            bspPath = bspPath.parent_path() / "styled" / bspPath.filename();

        int exitCode = RunVVIS(p2ceGameDir, bspPath, p2ceBinDir);
        if (exitCode == 1)
        {
            Log("[main] VVIS operation failed! Check above for errors!", true);
            return 1;
        }

        return 0; //! CHANGE TO 0 LATER!
    }

    if (execName == "vrad")
    {
        return 1;
        std::filesystem::path bspPath = argv[3];
        int exitCode = 1;

        // Need to make sure to use the "styled" preview map file generated by BEEMod is used.
        if (usingBEE)
            bspPath = bspPath.parent_path() / "styled" / bspPath.filename();

        exitCode = RunVRAD(p2ceGameDir, bspPath, p2ceBinDir, usingBEE);
        if (exitCode == 1)
        {
            Log("[main] VRAD operation failed! Check above for errors!", true);
            return 1;
        }

        // Tool does not care if P2:CE is still running as PeTI needs to be returned to when it finishes compiling.
        exitCode = RunP2CE(p2ceGameDir, p2ceBinDir / std::format("p2ce{}", GetExecutableExtension()), bspPath, usingBEE);
        if (exitCode != 0 && exitCode != STILL_ACTIVE)
        {
            Log( std::format("[main] Portal 2: Community Edition returned with a error code! Check above for errors! exitCode: {}", exitCode), true);
            return 1;
        }

        return 1; //! The VRAD compiler MUST end with a error code because we must prevent Portal 2 from running the map or it will cause the game to error!
    }

    Log("[main] Invalid compiler file name! Compiler must be named appropriately with 'vbsp.exe', 'vvis.exe', or 'vrad.exe'!");
    return 1;
}
