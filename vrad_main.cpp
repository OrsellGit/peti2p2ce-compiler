#include <filesystem>
#include <iostream>
// TODO: Get subprocess_h
#include <unistd.h>

/**
 * @brief Portal 2 will run this version of VBSP instead of its own.
 *        Here various actions will occur before the original VBSP EXE is called.
 * @param argc Number of arguments passed to the program.
 * @param argv Arguments passed to the program.
 *             Portal 2 passes in:
 *
 * @return Return code of the program. Portal 2 will recognize any return code that is non-zero to be a error code.
 */
int main( const int argc, char** argv )
{
    for (int i = 0; i < argc; i++ )
        std::cout << argv[i] << std::endl;

    std::filesystem::path originalVBSPPath = argv[0];
    originalVBSPPath.replace_filename("vbsp_original.exe");

    std::filesystem::path modifiedBEEVBSPPath = argv[0];
    originalVBSPPath.replace_filename("vbsp_bee.exe");

    return execl(originalVBSPPath.string().c_str(), argv[1], argv[2], argv[3], argv[4], argv[5], static_cast<char*>(nullptr));
}
