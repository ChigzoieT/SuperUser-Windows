#ifndef CPU_INFO_PLUGGER_H
#define CPU_INFO_PLUGGER_H

#include <Windows.h>
#include <string>

extern int threaddata;

void UpdateCPUInfo(HWND hwnd, HWND hCoresValue, HWND hSpeedValue, HWND hThreadValue);

#endif // CPU_INFO_PLUGGER_H
