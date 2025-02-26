#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

// Adjusts the given speed (in GHz) to the nearest allowed endpoint.
// Allowed endpoints are: 1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0,
//                        3.2, 3.4, 3.6, 3.8, 4.0, 4.2, 4.4, 4.6, 4.8, 5.0 GHz.
double adjust_speed(double inputSpeed);

// Returns the thread count for the specified number of cores and speed.
// The "speed" parameter should be one of the allowed speeds.
// Returns -1 if the core count or speed is invalid.
int get_thread_count(int cores, double speed);

#ifdef __cplusplus
}
#endif

#endif // ALLOCATOR_H
