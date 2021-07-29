#include "process.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <system_error>
#include <sys/wait.h>
#include <unistd.h>
#include "log.hpp"

// POSIX bindings
namespace {

struct pipe_t {
    fd_t in, out;
};

void execv(std::string_view epath,
                  std::vector<std::string> const& args) {
    char** argv = new char*[args.size() + 1];

    size_t i = 0;

    argv[i++] = const_cast<char*>(epath.data());
    for (auto& arg : args)
        argv[i++] = const_cast<char*>(arg.data());
    argv[i] = nullptr;

    ::execv(epath.data(), argv);
    throw std::system_error(errno, std::generic_category(), __func__);
}

pipe_t pipe() {
    int fd[2];
    if (::pipe(fd) == -1)
        throw std::system_error(errno, std::generic_category(), __func__);

    return {.in  = fd[0],
            .out = fd[1]};
}

void dup(fd_t oldfd, fd_t newfd) {
    if (::dup2(oldfd, newfd) == -1)
        throw std::system_error(errno, std::generic_category(), __func__);
}

template <class Function, class... Args>
pid_t fork(Function&& f, Args&&... args) {
    pid_t pid = ::fork();

    if (pid == -1)
        throw std::system_error(errno, std::generic_category(), __func__);

    if (pid == 0) {
        // Use std::invoke to support the caller of your code passing any callable
        // https://stackoverflow.com/questions/46388524/when-to-use-stdinvoke-instead-of-simply-calling-the-invokable
        std::invoke(std::forward<Function>(f),
                    std::forward<Args>(args)...);
        exit(0);
    }
    return pid;
}

bool fdgetline(int fd, std::string& str) {
    bool not_eof = false;
    char c;

    str.clear();
    while (read(fd, &c, 1) > 0) {
        not_eof = true;

        if (c == '\n') break;
        str += c;
    }

    return not_eof;
}

}

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
