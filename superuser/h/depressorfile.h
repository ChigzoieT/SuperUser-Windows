#ifndef DEPRESSORFILE_H
#define DEPRESSORFILE_H

#include <stdint.h>
#include <stdio.h>
#include <lzma.h>
#include <string>
#include <windows.h>

// Structure to manage resources
typedef struct {
    FILE* inputFile;
    FILE* outputFile;
    char* extension;
    char* fullOutputPath;
    uint8_t* inputBuffer;
    uint8_t* outputBuffer;
    lzma_stream* strm;
} ResourceManager;

// Function declarations
void cleanupResources(ResourceManager* resources);
void decompressFileWithFixedFilename(HWND hwnd, const std::wstring& inputFilePath, int numThreads);

#endif // DEPRESSORFILE_H
