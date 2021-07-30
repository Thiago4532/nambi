#ifndef _POSIX_BINDINGS_HPP
#define _POSIX_BINDINGS_HPP

#include "log.hpp"
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <functional>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace {

typedef std::pair<int, int> pipe_t;

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

    return {fd[0], fd[1]};
}

void dup(int oldfd, int newfd) {
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
    if (fd < 0) return false;

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

void close_all_fds() {
    /* Close all file descriptors except 0, 1, 2 */

    DIR *dir = opendir("/proc/self/fd");
    if (!dir)
        throw std::system_error(errno, std::generic_category(), "opendir");

    int dir_fd = dirfd(dir);

    dirent *e;
    while ((e = readdir(dir))) {
        char *end;
        int fd = strtol(e->d_name, &end, 10);

        if (*end)
            continue;
        if (fd >= 0 && fd <= 2)
            continue;

        close(fd);
    }

    closedir(dir);
}

int open(std::string_view pathname, int flags) {
    int fd = ::open(pathname.data(), flags);
    if (fd == -1)
        throw std::system_error(errno, std::generic_category(), "open");

    return fd;
}

}

#endif
