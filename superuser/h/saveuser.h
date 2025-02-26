#ifndef SAVEUSER_H
#define SAVEUSER_H

#include <string>

// Writes the given user key into "data.txt" if the response contains "welcome".
// Returns "done" after the operation.
void saveUserKeyIfWelcome(const std::string& response);

// Extern declaration for the super_user variable.
extern std::string super_user;

// Checks if "data.txt" exists, loads its content into super_user, and returns "exists" if it exists or "not exists" if it does not.
std::string loadSuperUser();

#endif // SAVEUSER_H
