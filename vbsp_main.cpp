/**
 *
 */

#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

/**
 * @brief Portal 2 will run this version of VBSP instead of its own.
 *        Here various actions will occur before the Portal 2: Community Edition VBSP executable is called.
 * @param argc Number of arguments passed to the program.
 * @param argv Arguments passed to the program.
 *             Portal 2 passes in to vbsp.exe:
 *             (0) VBSP location, normally located at "Portal 2/bin/vbsp.exe"
 *             (1) "-entity_limit" The flag to override the entity limit is its own argument in the array.
 *             (2) "1750" The set entity limit for PeTI.
 *             (3) "-game" Flag to set the game folder to use with VBSP, where the gameinfo.txt is location.
 *             (4) Game folder location, should default to "Portal 2/portal2".
 *             (5) Map VMF location, should default to "Portal 2/sdk_content/maps/preview.vmf"
 *
 *             -entity_limit
 *             1750
 *             -game
 *             /home/.../.local/share/steam/steamapps/common/portal 2/portal2/
 *             /home/.../.local/share/steam/steamapps/common/portal 2/sdk_content\maps/preview.vmf
 * @return Return code of the program. Portal 2 will recognize any return code that is non-zero to be a error code.
 */
int main( const int argc, char** argv )
{
    for (int i = 0; i < argc; i++ )
        std::cout << argv[i] << std::endl;

    //return -1;

#ifdef _WIN32
    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput =  GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi = { 0 };



    // Start the child process.
    if( !CreateProcess( nullptr,   // No module name (use command line)
        "vbsp_bee.exe",        // Command line
        nullptr,           // Process handle not inheritable
        nullptr,           // Thread handle not inheritable
        false,          // Set handle inheritance to FALSE
        0,              // No creation flags
        nullptr,           // Use parent's environment block
        nullptr,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )       // Pointer to PROCESS_INFORMATION structure
    )
    {
        std::cout << std::format("CreateProcess failed (%d).", GetLastError()) << std::endl;
        return -1;
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else

    // Fork the BEE VBSP into it's own process.
    const pid_t childPID = fork();
    if (childPID < 0)
    {
        // Fork failed to be made.
        std::cout << std::format("Failed to create forked ! Error: %s", strerror(errno)) << std::endl;
        return -1;
    }
    if (!childPID)
    {
        std::filesystem::path p2ceVBSPPath = argv[0];
        p2ceVBSPPath.replace_filename("vbsp_linux_p2ce"); // TODO: Replace with getting the path to the P2:CE VBSP.

        std::filesystem::path modifiedBEEVBSPPath = argv[0];
        modifiedBEEVBSPPath.replace_filename("vbsp_linux_bee");

        // Run the BEE VBSP in the forked child process.
        if (execl(modifiedBEEVBSPPath.string().c_str(), argv[1], argv[2], argv[3], argv[4], argv[5], nullptr) == -1)
            std::cout << std::format("Executable failed to execute! Error: %s", strerror(errno)) << std::endl;

        // Exit out the forked process so the parent process can continue.
        exit(0);
    }

    // Wait for the child process to finish.
    int returnStatus;
    waitpid(childPID, &returnStatus, 0);

    // Check if the program finished successfully.
    if (WIFEXITED(returnStatus))
    {
        const int returnCode = WEXITSTATUS(returnStatus);
        std::cout << "BEE VBSP Return Code: " << returnCode << std::endl;
        return -1;
        return returnCode;
    }

    std::cout << "BEE VBSP process status could not be retrieved!" << std::endl;
    return -1;
#endif
}
