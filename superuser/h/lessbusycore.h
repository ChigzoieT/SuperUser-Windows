#ifndef LESSBUSYCORE_H
#define LESSBUSYCORE_H

#ifdef __cplusplus
extern "C" {
#endif

// Sets the process affinity to the least busy core (based on current processor load).
// Returns the selected core index (0-based) if successful, or -1 on failure.
int SetAffinityToLeastBusyCore(void);

#ifdef __cplusplus
}
#endif

#endif // LESSBUSYCORE_H
