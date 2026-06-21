/**
* @brief   Managing the child processes created by the compilers.
* @details This is used by all compilers to properly start their child processes as well as capturing stdout and stderr from them.
* @authors Orsell
*
* @license Distributed under the MIT license.
*/

// #include <assert.h>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else // Linux
#include <unistd.h>
#include <signal.h>
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

#ifdef _WIN32

/**
 * @brief Turns a vector of wstrings into one wstring.
 * @param args Vector of wstrings to be assembled.
 * @return Assembled wstring.
 */
std::wstring BuildCommandLine(const std::vector<std::string>& args)
{
    std::wstringstream stream;

    for (const auto& arg : args)
    {
        stream << std::wstring(arg.begin(), arg.end()) << L' ';
    }

    std::wstring result = stream.str();
    if (!result.empty())
        result.pop_back();

    return result;
}

/**
 * @brief Convert a standard string to a wstring for use with Unicode.
 * @param str String to convert.
 * @return wstring version of string.
 */
std::wstring ConvertStringToWString(const std::string& str)
{
    std::wstringstream stream;
    stream << std::wstring(str.begin(), str.end());
    std::wstring result = stream.str();
    return result;
}

#endif

/**
 * @brief Dtor for the class which will make sure the child process early terminates.
 */
Process::~Process()
{
    if (this->running)
    {
        Terminate();
        Wait();
    }

#ifdef _WIN32

    if (this->stdoutRead)
        CloseHandle(this->stdoutRead);

    if (this->stderrRead)
        CloseHandle(this->stderrRead);

    if (this->childProcessHandle)
        CloseHandle(this->childProcessHandle);

#else // Linux

    if (this->stdoutRead >= 0)
        close(this->stdoutRead);

    if (this->stderrRead >= 0)
        close(this->stderrRead);

    this->stdoutRead = -1;
    this->stderrRead = -1;

#endif
}

/**
 * @brief Check if the process is running.
 * @return True if the process is still running.
 */
bool Process::IsRunning() const
{
    return this->running;
}

/**
 * @brief Terminate the child process early.
 */
void Process::Terminate()
{
    if (!this->running)
        return;

#ifdef _WIN32

    if (this->childProcessHandle)
        TerminateProcess(this->childProcessHandle, 1);

#else // Linux

    if (this->pid > 0)
        kill(this->pid, SIGKILL);

#endif
}

#ifdef _WIN32

/**
 * @brief Run a child process and get back stdout and stderr information from the child process.
 * @param cmd Array of wstring arguments that will be used for the process. wstrings are used to support Unicode paths.
 * @param execDir If this child process needs to be executed from a certain directory to make sure paths are right for it.
 * @param callback_ Function callback used for passing the stdout and stderr to the program that is running the child process.
 * @param detached If the process should be detached from this compile tool.
 * @return If the main program was able to make the pipes, and run the child process, then this will return true.
 */
bool Process::Start( const std::string& cmd, const std::filesystem::path& execDir, OutputCallback callback_, const bool detached )
{
    DevLog(std::format("Starting process with parameters:\ncmd: {}\nexecDir: {}\ncallback: {}\ndetached: {}", cmd, execDir.string(), callback_ ? "Has Callback" : "No Callback", detached));

    callback = std::move(callback_);

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = !detached;

    HANDLE stdoutRead_ = nullptr;
    HANDLE stdoutWrite_ = nullptr;
    HANDLE stderrRead_ = nullptr;
    HANDLE stderrWrite_ = nullptr;

    if (!CreatePipe(&stdoutRead_, &stdoutWrite_, &sa, 0))
    {
        Log(std::format("[Process::Start - Windows] CreatePipe for stdout failed! Error: {}", GetLastError()), true);
        return false;
    }

    if (!CreatePipe(&stderrRead_, &stderrWrite_, &sa, 0))
    {
        Log(std::format("[Process::Start - Windows] CreatePipe for stderr failed! Error: {}", GetLastError()), true);

        // Close the stdout handles since those would have successfully been made before getting here.
        CloseHandle(stdoutRead_);
        CloseHandle(stdoutWrite_);
        return false;
    }

    // Make sure children are able to get an inherited handle to the stdout and stderr handles.
    if (!SetHandleInformation(stdoutRead_, HANDLE_FLAG_INHERIT, 0))
    {
        Log(std::format("[Process::Start - Windows] SetHandleInformation failed! Error: {}", GetLastError()), true);

        CloseHandle(stdoutWrite_);
        CloseHandle(stderrWrite_);
        CloseHandle(stdoutRead_);
        CloseHandle(stderrRead_);

        return false;
    }
    if (!SetHandleInformation(stderrRead_, HANDLE_FLAG_INHERIT, 0))
    {
        Log(std::format("[Process::Start - Windows] SetHandleInformation failed! Error: {}", GetLastError()), true);

        CloseHandle(stdoutWrite_);
        CloseHandle(stderrWrite_);
        CloseHandle(stdoutRead_);
        CloseHandle(stderrRead_);

        return false;
    }

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = detached ? nullptr : stdoutWrite_;
    si.hStdError  = detached ? nullptr : stderrWrite_;
    si.hStdInput  = nullptr;

    PROCESS_INFORMATION pi{};

    std::wstring cmdFinal = ConvertStringToWString(cmd);
    DWORD flags = CREATE_NO_WINDOW;
    if (detached)
        flags = DETACHED_PROCESS;
    const bool success = CreateProcessW(
        nullptr,
        cmdFinal.data(),
        nullptr,
        nullptr,
        !detached,
        flags,
        nullptr,
        execDir.empty() ? nullptr : execDir.c_str(),
        &si,
        &pi
    );

    if (!success)
    {
        Log(std::format("[Process::Start - Windows] CreateProcess failed! Error: {}", GetLastError()), true);

        CloseHandle(stdoutWrite_);
        CloseHandle(stderrWrite_);
        CloseHandle(stdoutRead_);
        CloseHandle(stderrRead_);

        return false;
    }

    CloseHandle(stdoutWrite_);
    CloseHandle(stderrWrite_);
    CloseHandle(pi.hThread);

    this->childProcessHandle = pi.hProcess;
    this->stdoutRead = stdoutRead_;
    this->stderrRead = stderrRead_;

    this->running = true;

    this->stdoutThread = std::thread(&Process::ReadSTDOut, this);
    this->stderrThread = std::thread(&Process::ReadSTDErr, this);

    return true;
}

#else // Linux

/**
 * @brief Run a child process and get back stdout and stderr information from the child process.
 * @param args Array of arguments that will be used for the process.
 * @param execDir If this child process needs to be executed from a certain directory to make sure paths are right for it.
 * @param callback_ Function callback used for passing the stdout and stderr to the program that is running the child process.
 * @return If the main program was able to make the pipes, and run the child process, then this will return true.
 */
bool Process::Start( const std::vector<std::string>& args, const std::filesystem::path& execDir, OutputCallback callback_, const bool detached )
{
    this->callback = std::move(callback_);

    int outPipe[2];
    int errPipe[2];

    if (pipe(outPipe) != 0)
    {
        Log(std::format("[Process::Start - Linux] Failed to create pipe! Error: {}", std::strerror(errno), true);
        return false;
    }

    if (pipe(errPipe) != 0)
    {
        Log(std::format("[Process::Start - Linux] Failed to create pipe! Error: {}", std::strerror(errno), true);

        // Close the stdout pipes since those would have successfully been made before getting here.
        close(outPipe[0]);
        close(outPipe[1]);
        return false;
    }

    this->pid = fork();

    if (this->pid == 0)
    {
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(errPipe[1], STDERR_FILENO);

        close(outPipe[0]);
        close(outPipe[1]);

        close(errPipe[0]);
        close(errPipe[1]);

        assert(false); //

        /*std::vector<char*> argv;
        for (const auto& s : args)
        {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
        argv.push_back(nullptr);

        if (!execDir.empty())
            chdir(execDir.c_str());

        execvp(argv[0], argv.data());*/

        exit(0);
    }
    else if (this->pid < 0)
    {
        Log(std::format("[Process::Start - Linux] Failed to fork! Error: {}", std::strerror(errno), true);

        close(outPipe[0]);
        close(outPipe[1]);

        close(errPipe[0]);
        close(errPipe[1]);

        return false;
    }

    close(outPipe[1]);
    close(errPipe[1]);

    this->stdoutRead = outPipe[0];
    this->stderrRead = errPipe[0];

    this->running = true;

    this->stdoutThread = std::thread(&Process::ReadSTDOut, this);
    this->stderrThread = std::thread(&Process::ReadSTDErr, this);

    return true;
}

#endif

/**
 * @brief Wait for the child process to finish before cleaning up handles and returning an exit code.
 * @return Exit code of the child process that was run.
 */
unsigned long Process::Wait( const bool fullyWait )
{
    // If process was never started or already waited for.
    if (!running)
        return 1;

#ifdef _WIN32

    DWORD exitCode = 0;
    if (this->childProcessHandle == INVALID_HANDLE_VALUE)
    {
        Log("[Process::Wait - Windows] Invalid child process handle!");
        return 1;
    }

    // No need to wait for processes don't need to be waited on or have timed out. This more specifically applies to P2:CE.
    if (const DWORD returnCode = WaitForSingleObject(this->childProcessHandle, fullyWait ? INFINITE : 0); returnCode == WAIT_OBJECT_0 || returnCode == WAIT_TIMEOUT)
    {
        DevLog(std::format("[Process::Wait - Windows] WaitForSingleObject returnCode: {}", returnCode));
        GetExitCodeProcess(this->childProcessHandle, &exitCode);
        if (exitCode == STILL_ACTIVE && fullyWait)
        {
            Log("[Process::Wait - Windows] Child process is still running even when fullyWait is true, something has gone wrong!", true);
            return 1;
        }
    }
    else
    {
        Log(std::format("[Process::Wait - Windows] WaitForSingleObject failed! returnCode: {} Error: {}", returnCode, GetLastError()), true);

        CloseHandle(this->childProcessHandle);
        CloseHandle(this->stdoutRead);
        CloseHandle(this->stderrRead);
        return 1;
    }

#else // Linux

    int status = 0;
    waitpid(this->pid, &status, 0);

    const unsigned long exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : 1;

#endif

    this->running = false;

    if (this->stdoutThread.joinable())
        this->stdoutThread.join();

    if (this->stderrThread.joinable())
        this->stderrThread.join();

    return exitCode;
}

/**
 * @brief Call the callback for the child process to pass along its stdout to the parent process.
 */
void Process::ReadSTDOut()
{
#ifdef _WIN32

    char buffer[4096];
    DWORD bytesRead;

    std::string pending;

    while (ReadFile(this->stdoutRead, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0)
    {
        pending.append(buffer, bytesRead);

        size_t newlinePos;

        while ((newlinePos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, newlinePos + 1);
            pending.erase(0, newlinePos + 1);

            if (const auto cb = this->callback)
                cb(line, false);
        }
    }

    // Flush remaining partial line.
    if (!pending.empty())
    {
        if (const auto cb = this->callback)
            cb(pending, false);
    }

#else // Linux

    char buffer[4096];
    ssize_t bytesRead;

    std::string pending;

    while ((bytesRead = read(this->stdoutRead, buffer, sizeof(buffer))) > 0)
    {
        pending.append(buffer, bytesRead);

        size_t newlinePos;

        while ((newlinePos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, newlinePos + 1);
            pending.erase(0, newlinePos + 1);

            if (const auto cb = this->callback)
                cb(line, false);
        }
    }

    // Flush remaining partial line.
    if (!pending.empty())
    {
        if (const auto cb = this->callback)
            cb(pending, false);
    }

#endif
}

/**
 * @brief Call the callback for the child process to pass along its stderr to the parent process.
 */
void Process::ReadSTDErr()
{
#ifdef _WIN32

    char buffer[4096];
    DWORD bytesRead;

    std::string pending;

    while (ReadFile(this->stderrRead, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0)
    {
        pending.append(buffer, bytesRead);

        size_t newlinePos;

        while ((newlinePos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, newlinePos + 1);
            pending.erase(0, newlinePos + 1);

            if (const auto cb = this->callback)
                cb(line, true);
        }
    }

    // Flush remaining partial line.
    if (!pending.empty())
    {
        if (const auto cb = this->callback)
            cb(pending, true);
    }

#else // Linux

    char buffer[4096];
    ssize_t bytesRead;

    std::string pending;

    while ((bytesRead = read(this->stderrRead, buffer, sizeof(buffer))) > 0)
    {
        pending.append(buffer, bytesRead);

        size_t newlinePos;

        while ((newlinePos = pending.find('\n')) != std::string::npos)
        {
            std::string line = pending.substr(0, newlinePos + 1);
            pending.erase(0, newlinePos + 1);

            if (auto cb = this->callback)
                cb(line, true);
        }
    }

    // Flush remaining partial line.
    if (!pending.empty())
    {
        if (auto cb = this->callback)
            cb(pending, true);
    }

#endif
}
