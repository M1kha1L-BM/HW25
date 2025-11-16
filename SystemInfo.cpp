#include "SystemInfo.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <fstream>
#include <limits>
#endif

void printSystemInfo() {
    std::cout << "-------------------------------------\n";

#ifdef _WIN32

    printWindowsVersion();

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    std::cout << "Количество процессоров: " << sysInfo.dwNumberOfProcessors << "\n";


#else

    struct utsname buffer;
    if (uname(&buffer) == 0) {
        std::cout << "Операционная система: " << buffer.sysname
            << " " << buffer.release
            << " (" << buffer.version << ")\n";
        std::cout << "Архитектура: " << buffer.machine << "\n";
    }

    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    std::cout << "Количество процессоров: " << cpus << "\n";

#endif

    std::cout << "-------------------------------------\n";
}

