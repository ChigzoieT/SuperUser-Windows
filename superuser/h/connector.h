#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <windows.h>
#include <wininet.h>
#include <string>

std::string sendToPHP(const std::string& userkey, const std::string& password, 
                      const std::string& state, const std::string& cmptext); // Changed from void to std::string

#endif // CONNECTOR_H
