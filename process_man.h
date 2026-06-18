/**
* @brief   Managing the child processes created by the compilers.
* @details This is used by all compilers to properly start their child processes as well as capturing stdout and stderr from them.
* @authors Orsell
*
* @license Distributed under the MIT license.
*/

#ifndef PROCESS_H
#define PROCESS_H

#pragma once

#include <atomic>
#include <functional>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

/**
 * @brief Class for handling the child processes run by the tool, managing their state, and getting stdout and stderr streams from them.
 */
class Process
{
public:
    using OutputCallback = std::function<void ( const std::string&, bool isSTDErr )>;

    Process() = default;
    ~Process();

    // The child process handler shouldn't be copied around.
    Process( const Process& ) = delete;
    Process& operator=( const Process& ) = delete;

    [[nodiscard]] bool IsRunning() const;

    void Terminate();

    bool Start(const std::string& cmd, const std::filesystem::path& execDir = {}, OutputCallback callback_ = nullptr, bool detached = false);

    unsigned long Wait(bool fullyWait = true);

private:
    void ReadSTDOut();
    void ReadSTDErr();

    OutputCallback callback = nullptr;

    std::thread stdoutThread{};
    std::thread stderrThread{};

    // Need to atomically check if the child process is in fact running.
    std::atomic<bool> running = false;

#ifdef _WIN32
    HANDLE childProcessHandle = nullptr;
    HANDLE stdoutRead = nullptr;
    HANDLE stderrRead = nullptr;
#else // Linux
    int pid = -1;
    int stdoutRead = -1;
    int stderrRead = -1;
#endif
};

#endif //PROCESS_H
