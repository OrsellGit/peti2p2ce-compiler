/**
* @brief   Handling logging for the program.
* @details
* @authors Orsell
*
* @license Distributed under the MIT license.
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <filesystem>
#include <fstream>
#include <string>

class Logger
{
protected:
    std::ofstream logFile;
    std::filesystem::path logPath;
    std::string compilerName;

    Logger() = default;
    ~Logger() = default;

public:
    Logger( const Logger& rhs ) = delete;
    Logger( Logger&& rhs ) = delete;
    Logger& operator=( const Logger& rhs ) = delete;
    Logger& operator=( Logger&& rhs ) = delete;

    static Logger* GetInstance();

    static bool InitLogging(const std::filesystem::path& execDir, const std::string& execName);
    static void ShutdownLogging();

    static void Log( const std::string& text, bool error = false, bool excPrefix = false );
    static void DevLog( const std::string& text, bool error = false, bool excPrefix = false );

    static void ShowPopUp( const std::string& title, const std::string& message, bool error = false );
 };

#endif //LOGGER_H
