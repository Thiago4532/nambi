#ifndef _SPAWN_HPP
#define _SPAWN_HPP

#include <vector>
#include <string>
#include <string_view>
#include <sys/types.h>

typedef int fd_t;

class process {
public:
    process(std::string_view epath,
            std::vector<std::string> const& args);
    ~process();

    enum class code {
        RUNNING,
        EXITED,
        SIGNALED,
        UNKNOWN
    };

    code status(bool wait=false);
    bool isRunning() { return status() == code::RUNNING; }
    code wait() { return status(true); }

    int getExitStatus();
    std::string_view getSignal();

// private:
    fd_t _stdin, _stdout, _stderr;

    pid_t _pid {};

    int _wstatus;
    code _status { code::RUNNING };
};

#endif
