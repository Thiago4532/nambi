#include <iostream>
#include "log.hpp"
#include <vector>
#include "process.hpp"

using namespace std;

int main() {
    process proc("/bin/sh", {"-c", "lolcat"}, {.stdin=""});

    proc.sendEOF();
    auto x = proc.wait();

    // string s;
    // while (proc.readLine(s)) {
    //     cout << s << '\n';
    // }
    return 0;
}
