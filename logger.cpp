/**
* @brief   Handling logging for the program.
* @details
* @authors Orsell
*
* @license Distributed under the MIT license.
*/

#include "logger.h"

#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else // Linux
#endif

/**
 * @brief Get the singleton instance of the Logger class.
 * @return Pointer to class instance.
 */
Logger* Logger::GetInstance()
{
    static Logger logger;
    return &logger;
}

/**
 * @brief Initialize logging for the program.
 * @param execDir Directory for log file to be created in.
 * @param execName Name of compiler being run.
 * @return True if log file stream opened.
 */
bool Logger::InitLogging(const std::filesystem::path& execDir, const std::string& execName)
{
    GetInstance()->compilerName = execName;
    GetInstance()->logPath = execDir / std::format("{}-peti2p2ce.log", execName);
    GetInstance()->logFile.open(GetInstance()->logPath, std::ios::out);
    if (!GetInstance()->logFile.is_open())
    {
        std::cout << std::format("[{}-peti2p2ce] [ERROR] Failed to open log file! Will continue without it!", execName) << std::endl;
        return false;
    }

    DevLog(std::format("Started logging for {}!", GetInstance()->compilerName));
    return true;
}

void Logger::ShutdownLogging()
{
    if (GetInstance()->logFile.is_open())
    {
        GetInstance()->logFile.close();
        std::cout << std::format("[{}-peti2p2ce] Close logging file.", GetInstance()->compilerName) << std::endl;
    }
}

/**
 * @brief Log to the output and log file stream of the program.
 * @param text Text to log.
 * @param error If this is a error message display as so.
 * @param excPrefix Exclude the compiler prefix in front of the log line.
 */
void Logger::Log(const std::string& text, const bool error, bool excPrefix)
{
    std::string prefixString;
    if (!excPrefix)
    {
        prefixString = error ? std::format("[{}-peti2p2ce] [ERROR] ", GetInstance()->compilerName)
                                          : std::format("[{}-peti2p2ce] ", GetInstance()->compilerName);
    }

    std::cout << prefixString << text << std::endl;
    if (GetInstance()->logFile.is_open())
        GetInstance()->logFile << prefixString << text << std::endl;
}

/**
 * @brief Debug log to the output and log file stream of the program. Doesn't run in release builds.
 * @param text Text to log.
 * @param error If this is a error message display as so.
 * @param excPrefix Exclude the compiler prefix in front of the log line.
 */
void Logger::DevLog( const std::string& text, const bool error, const bool excPrefix )
{
#if _DEBUG
    Log("[DEV] " + text, error, excPrefix);
#else
    (void)text;
    (void)error;
    (void)excPrefix;
#endif
}

// TODO: Make notifications and text boxes show up for notifying the user of errors or attention required for the program. Figure out using xmessage and/or notify-send for Linux.
void Logger::ShowPopUp( const std::string& title, const std::string& message, const bool error )
{
    DevLog(std::format("Showing message box:\ntitle: '{}'\nmessage: '{}'\nerror: {}", title, message, error));
#ifdef _WIN32

    MessageBox(nullptr, message.c_str(), title.c_str(), error ? MB_OK | MB_ICONERROR : MB_OK | MB_ICONEXCLAMATION);

#else // Linux

    (void)title;
    (void)message;
    (void)error;

#endif

}
