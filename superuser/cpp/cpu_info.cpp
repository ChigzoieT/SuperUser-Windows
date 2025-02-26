#include "../h/cpu_info.h"
#include <stdio.h>
#include <time.h>

unsigned int get_cpu_info() {
    unsigned int eax, ebx, ecx, edx;
    unsigned int num_physical_cores = 0;

    // Call CPUID with EAX=1 for basic processor info (get logical processor count)
    __asm__ volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (1)
    );

    // Extract logical processors count from EBX[23:16]
    unsigned int logical_processors = (ebx >> 16) & 0xFF;

    // Try another CPUID instruction with EAX=0x80000008 for more information
    __asm__ volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (0x80000008)
    );

    // If ECX holds a value, extract physical cores
    num_physical_cores = (ecx & 0xFF);

    // If we still get zero, assume a fallback method:
    if (num_physical_cores == 0) {
        // Fallback for systems that might not report cores using CPUID
        num_physical_cores = logical_processors / 2; // Best guess on hyperthreaded processors
    }

    return num_physical_cores;
}

double get_cpu_clock_speed() {
    uint64_t start, end;
    unsigned int low, high;

    // Measure TSC at the start
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    start = ((uint64_t)high << 32) | low;

    // Sleep for 1 second
    struct timespec ts = {1, 0};
    nanosleep(&ts, NULL);

    // Measure TSC at the end
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    end = ((uint64_t)high << 32) | low;

    // Calculate clock speed in GHz
    return (double)(end - start) / 1e9;
}