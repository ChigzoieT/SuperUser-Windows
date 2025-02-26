#include "../h/winsys.h"
#include <windows.h>

unsigned int get_logical_processor_count() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);  // Retrieve system information
    return sysInfo.dwNumberOfProcessors;  // Number of logical processors
}

unsigned int get_cpu_base_speed() {
    unsigned int baseSpeed = 0;
    HKEY hKey;
    
    // Open the registry key where the CPU base frequency is stored
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType = 0;
        DWORD dwSize = sizeof(baseSpeed);
        if (RegQueryValueEx(hKey, L"~MHz", NULL, &dwType, reinterpret_cast<LPBYTE>(&baseSpeed), &dwSize) == ERROR_SUCCESS) {
            // baseSpeed will now contain the CPU base frequency in MHz
        }
        RegCloseKey(hKey);
    }
    
    return baseSpeed;
}
