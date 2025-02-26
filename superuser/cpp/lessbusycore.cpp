#include "../h/lessbusycore.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <stdlib.h>
#include <wchar.h>  // For swprintf and wide character support

#pragma comment(lib, "pdh.lib")

int SetAffinityToLeastBusyCore(void)
{
    // Retrieve the number of logical processors.
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int numCores = sysInfo.dwNumberOfProcessors;

    // Open a PDH query.
    PDH_HQUERY query;
    PDH_STATUS status = PdhOpenQuery(NULL, 0, &query);
    if (status != ERROR_SUCCESS) {
        return -1;
    }

    // Allocate an array for one counter per core.
    PDH_HCOUNTER* counters = (PDH_HCOUNTER*)malloc(sizeof(PDH_HCOUNTER) * numCores);
    if (!counters) {
        PdhCloseQuery(query);
        return -1;
    }

    // Use a wide character array for the counter path.
    wchar_t counterPath[256];
    // Add a counter for each core using the path "\\Processor(<n>)\\% Processor Time"
    for (int i = 0; i < numCores; i++) {
        swprintf(counterPath, 256, L"\\Processor(%d)\\%% Processor Time", i);
        status = PdhAddCounter(query, counterPath, 0, &counters[i]);
        if (status != ERROR_SUCCESS) {
            free(counters);
            PdhCloseQuery(query);
            return -1;
        }
    }

    // Collect two samples (with a 1-second delay) to obtain valid data.
    status = PdhCollectQueryData(query);
    Sleep(1000);
    status = PdhCollectQueryData(query);

    double minLoad = 101.0;  // Start with a value higher than 100%
    int minCore = 0;
    PDH_FMT_COUNTERVALUE counterVal;

    // Determine which core is the least busy.
    for (int i = 0; i < numCores; i++) {
        status = PdhGetFormattedCounterValue(counters[i], PDH_FMT_DOUBLE, NULL, &counterVal);
        if (status != ERROR_SUCCESS)
            continue;
        double load = counterVal.doubleValue;
        if (load < minLoad) {
            minLoad = load;
            minCore = i;
        }
    }

    // Set the process affinity mask to run only on the least busy core.
    DWORD_PTR affinityMask = ((DWORD_PTR)1 << minCore);
    HANDLE hProcess = GetCurrentProcess();
    if (!SetProcessAffinityMask(hProcess, affinityMask)) {
        free(counters);
        PdhCloseQuery(query);
        return -1;
    }

    free(counters);
    PdhCloseQuery(query);

    return minCore;
}
