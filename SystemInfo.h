#pragma once
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <iostream>

void printSystemInfo();


typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

void printWindowsVersion() {
    RTL_OSVERSIONINFOW verInfo = { 0 };
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);

    HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        auto func = (RtlGetVersionPtr)::GetProcAddress(hMod, "RtlGetVersion");
        if (func && func(&verInfo) == 0) {
            std::wcout << L"Операционная система: Windows "
                << verInfo.dwMajorVersion << L"." << verInfo.dwMinorVersion
                << L" (Build " << verInfo.dwBuildNumber << L")" << std::endl;
            return;
        }
    }
    std::cout << " Не удалось определить версию Windows ";
}
#endif

