#include <iostream>
#include <string>
#include <zlib.h>
#include "../h/pressortext.h"

std::string CompressData(const std::string& input) {
    uLongf compressedSize = compressBound(input.size());
    std::string output(compressedSize, '\0'); // Allocate enough space for compression

    if (compress(reinterpret_cast<Bytef*>(&output[0]), &compressedSize,
                 reinterpret_cast<const Bytef*>(input.data()), input.size()) != Z_OK) {
        std::cerr << "Compression failed!" << std::endl;
        return "";
    }

    output.resize(compressedSize); // Resize to actual compressed size
    return output;
}

std::string DecompressData(const std::string& input, size_t originalSize) {
    std::string output(originalSize, '\0'); // Allocate space for decompression

    if (uncompress(reinterpret_cast<Bytef*>(&output[0]), reinterpret_cast<uLongf*>(&originalSize),
                   reinterpret_cast<const Bytef*>(input.data()), input.size()) != Z_OK) {
        std::cerr << "Decompression failed!" << std::endl;
        return "";
    }

    output.resize(originalSize); // Resize to match decompressed size
    return output;
}
