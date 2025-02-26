#include "../h/cpu_info_plugger.h"
#include "../h/allocator.h"
#include "../h/cpu_info.h"
#include <sstream>
#include <iomanip> // For std::fixed and std::setprecision
#include <string>
#include <windows.h>

// Global variable to hold the thread count as an integer.
int threaddata;

void UpdateCPUInfo(HWND hwnd, HWND hCoresValue, HWND hSpeedValue, HWND hThreadValue) {
    // Get the number of cores and clock speed (in GHz)
    unsigned int numCores = get_cpu_info();
    double clockSpeed = get_cpu_clock_speed(); // Already in GHz

    // Convert the number of cores to a wide string
    std::wostringstream coreStream;
    coreStream << numCores;
    std::wstring coreStr = coreStream.str();

    // Convert the clock speed to a wide string with 2 decimal places and append " GHz"
    std::wostringstream speedStream;
    speedStream << std::fixed << std::setprecision(2) << clockSpeed << L" GHz";
    std::wstring speedStr = speedStream.str();

    // Set text to respective labels
    SetWindowTextW(hCoresValue, coreStr.c_str());
    SetWindowTextW(hSpeedValue, speedStr.c_str());

    // Adjust the clock speed to the nearest allowed endpoint.
    double adjustedClockSpeed = adjust_speed(clockSpeed);

    // Get the thread count for the given number of cores and adjusted speed.
    threaddata = get_thread_count(numCores, adjustedClockSpeed); // Now an int

    // Set the thread count as the text of the hThreadValue control.
    SetWindowTextW(hThreadValue, std::to_wstring(threaddata).c_str());
}
