#ifndef PRESSORTEXT_H
#define PRESSORTEXT_H

#include <string>

std::string CompressData(const std::string& input);

void DecompressData(const std::string& input, std::string& output, size_t originalSize);

#endif // PRESSORTEXT_H
