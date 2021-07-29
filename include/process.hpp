#ifndef _PROCESS_HPP
#define _PROCESS_HPP

#include <vector>
#include <string>
#include <string_view>
#include <sys/types.h>

class process {
public:
    process(std::string_view epath,
            std::vector<std::string> const& args);
    ~process();

    bool readLine(std::string& str);
    bool readErrLine(std::string& str);

    enum class code {
        RUNNING,
        EXITED,
        SIGNALED,
        UNKNOWN
    };

    struct stdio {
        std::string_view stdin;
        std::string_view stdout;
        std::string_view stderr;
    };

    code status(bool wait=false);
    bool isRunning() { return status() == code::RUNNING; }
    code wait() { return status(true); }

    int getExitStatus();
    std::string_view getSignal();

// private:
    int _stdin, _stdout, _stderr;

    pid_t _pid {};

    int _wstatus;
    code _status { code::RUNNING };
};

#endif
