#include "h/depressorfile.h"
#include "h/pressorfile.h"
#include "h/events.h"
#include "h/superuser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lzma.h>
#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>

std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

void cleanupResources(ResourceManager* resources) {
    if (resources->inputFile) fclose(resources->inputFile);
    if (resources->outputFile) fclose(resources->outputFile);
    if (resources->extension) free(resources->extension);
    if (resources->fullOutputPath) free(resources->fullOutputPath);
    if (resources->inputBuffer) free(resources->inputBuffer);
    if (resources->outputBuffer) free(resources->outputBuffer);
    if (resources->strm) {
        lzma_end(resources->strm);
        free(resources->strm);
    }
}

void decompressFileWithFixedFilename(HWND hwnd, const std::wstring& inputFilePath, int numThreads) {
    ResourceManager resources = {0};
    resources.strm = (lzma_stream*)malloc(sizeof(lzma_stream));
    if (!resources.strm) {
        validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
        return;
    }
    *resources.strm = LZMA_STREAM_INIT;

    std::string narrowInputPath = wstringToString(inputFilePath);
    resources.inputFile = fopen(narrowInputPath.c_str(), "rb");
    if (!resources.inputFile) {
        validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
        cleanupResources(&resources);
        return;
    }

    // Read the stored filename (first 256 bytes)
    char storedFilename[256] = {0};
    if (fread(storedFilename, 1, 256, resources.inputFile) != 256) {
        validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
        cleanupResources(&resources);
        return;
    }

    // Extract the directory from the input file path
    std::wstring inputDir;
    size_t pos = inputFilePath.find_last_of(L"/\\");
    if (pos != std::wstring::npos) {
        inputDir = inputFilePath.substr(0, pos);
    } else {
        inputDir = L".";  // fallback to current directory
    }
    std::string narrowInputDir = wstringToString(inputDir);

    // Construct the output file path using the extracted directory and stored filename
    char outputFilePath[512];
    snprintf(outputFilePath, sizeof(outputFilePath), "%s/%s", narrowInputDir.c_str(), storedFilename);

    resources.outputFile = fopen(outputFilePath, "wb");
    if (!resources.outputFile) {
        validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
        cleanupResources(&resources);
        return;
    }

    if (lzma_stream_decoder(resources.strm, UINT64_MAX, LZMA_CONCATENATED) != LZMA_OK) {
        validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
        cleanupResources(&resources);
        return;
    }

    size_t bufferSize = 1024;
    resources.inputBuffer = (uint8_t*)malloc(bufferSize);
    resources.outputBuffer = (uint8_t*)malloc(bufferSize);
    if (!resources.inputBuffer || !resources.outputBuffer) {
        validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
        cleanupResources(&resources);
        return;
    }

    resources.strm->next_out = resources.outputBuffer;
    resources.strm->avail_out = bufferSize;

    fseek(resources.inputFile, 0, SEEK_END);
    long fileSize = ftell(resources.inputFile);
    rewind(resources.inputFile);
    long processed = 0;

    while (!feof(resources.inputFile)) {
        size_t bytesRead = fread(resources.inputBuffer, 1, bufferSize, resources.inputFile);
        resources.strm->next_in = resources.inputBuffer;
        resources.strm->avail_in = bytesRead;
        processed += bytesRead;

        double progress = (static_cast<double>(processed) / fileSize) * 100;
        wchar_t progressText[50];
        swprintf(progressText, 50, L"Decompressing: %.1f%%", progress);
        validateclickz(hwnd, progressText, RGB(0, 255, 0));

        while (resources.strm->avail_in > 0 || resources.strm->avail_out == 0) {
            lzma_ret ret = lzma_code(resources.strm, LZMA_RUN);
            if (ret == LZMA_STREAM_END) break;
            if (ret != LZMA_OK) {
                validateclickz(hwnd, L"Error loading file!!", RGB(255, 0, 0));
                cleanupResources(&resources);
                return;
            }
            if (resources.strm->avail_out == 0) {
                fwrite(resources.outputBuffer, 1, bufferSize, resources.outputFile);
                resources.strm->next_out = resources.outputBuffer;
                resources.strm->avail_out = bufferSize;
            }
        }
    }

    if (resources.strm->avail_out < bufferSize) {
        fwrite(resources.outputBuffer, 1, bufferSize - resources.strm->avail_out, resources.outputFile);
    }

    cleanupResources(&resources);
    
    // Show the "Decompression Complete!" message for 5 seconds using TIMER_ID.
    validateclickzWithTimer(hwnd, L"Decompression Complete!", RGB(0, 255, 0), 5000);
}
