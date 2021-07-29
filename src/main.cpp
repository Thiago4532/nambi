#include <iostream>
#include "log.hpp"
#include <vector>
#include "process.hpp"

using namespace std;

int main() {
    process proc("/bin/sh", {"-c", "sleep 3"});

    auto x = proc.wait();

    if (x == process::code::EXITED)
        cout << "Status: " << proc.getExitStatus() << endl;
    else if (x == process::code::SIGNALED)
        cout << "Signal: " << proc.getSignal() << endl;
    else if (x == process::code::RUNNING)
        cout << "RODANDO" << endl;
    else if (x == process::code::UNKNOWN)
        cout << "BRUH" << endl;

    return 0;
}
