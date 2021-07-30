#include "process.hpp"

#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include "log.hpp"
#include "posix_bindings.hpp"

process::process(std::string_view epath,
                 std::vector<std::string> const& args,
                 stdio const& io) : _stdin(-1), _stdout(-1), _stderr(-1) {
    int fdIn, fdOut, fdErr;

    if (io.stdin.empty()) std::tie(fdIn, _stdin) = pipe();
    else fdIn = open(io.stdin, O_RDONLY);
    if (io.stdout.empty()) std::tie(_stdout, fdOut) = pipe();
    else fdOut = open(io.stdout, O_WRONLY);
    if (io.stderr.empty()) std::tie(_stderr, fdErr) = pipe();
    else fdErr = open(io.stderr, O_WRONLY);

    _pid = fork([&]() {
        dup(fdIn, STDIN_FILENO);
        dup(fdOut, STDOUT_FILENO);
        dup(fdErr, STDERR_FILENO);

        close_all_fds();

        execv(epath, args);
    });

    close(fdIn);
    close(fdOut);
    close(fdErr);
}

process::code process::status(bool wait) {
    if (_status != code::RUNNING)
        return _status;

    if (waitpid(_pid, &_wstatus, wait ? 0 : WNOHANG) > 0) {
        if      (WIFEXITED(_wstatus))   _status = code::EXITED;
        else if (WIFSIGNALED(_wstatus)) _status = code::SIGNALED;
        else                            _status = code::UNKNOWN;
    }

    return _status;
}

int process::getExitStatus() {
    if (_status == code::RUNNING)
        status();

    if (_status != code::EXITED)
        return -1; // Invalid status

    return WEXITSTATUS(_wstatus);
}

std::string_view process::getSignal() {
    if (_status == code::RUNNING)
        status();

    if (_status != code::SIGNALED)
        return ""; // Invalid status

    char* ptr = strsignal( WTERMSIG(_wstatus) );
    if (ptr == nullptr) {
        LOG("unknown signal!");
        return ""; // Unknown status
    }

    return ptr;
}

bool process::readLine(std::string& str) { return fdgetline(_stdout, str); }
bool process::readErrLine(std::string& str) { return fdgetline(_stderr, str); }

process::~process() {
    LOG("called!");
    close(_stdin);
    close(_stdout);
    close(_stderr);
}
