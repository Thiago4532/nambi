#include "process.hpp"

#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include "log.hpp"
#include "posix_bindings.hpp"

process::process(std::string_view epath,
                 std::vector<std::string> const& args) {

    pipe_t pIn  = pipe();
    pipe_t pOut = pipe();
    pipe_t pErr = pipe();

    _pid = fork([&]() {
        close(pIn.out);
        close(pOut.in);
        close(pErr.in);

        dup(pIn.in, STDIN_FILENO);
        dup(pOut.out, STDOUT_FILENO);
        dup(pErr.out, STDERR_FILENO);

        close_all_fds();

        execv(epath, args);
    });

    close(pIn.in);
    close(pOut.out);
    close(pErr.out);

    _stdin  = pIn.out;
    _stdout = pOut.in;
    _stderr = pErr.in;
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
