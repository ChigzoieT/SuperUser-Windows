#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <stdint.h>

// Function to retrieve the number of logical CPU cores
unsigned int get_cpu_info();

// Function to calculate the CPU clock speed in GHz
double get_cpu_clock_speed();

#endif // CPU_INFO_H
