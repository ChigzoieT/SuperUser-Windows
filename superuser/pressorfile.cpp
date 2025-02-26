#include "h/pressorfile.h"
#include "h/superuser.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <lzma.h>
#include <fstream>
#include <codecvt>
#include <locale>
#include <cstring>
#include <windows.h>
#include "h/events.h"

const size_t FIXED_NAME_SIZE = 256;

void validateclickz(HWND hwnd, const wchar_t* text, COLORREF color) {
    SendMessage(hTextView2, WM_SETTEXT, 0, (LPARAM)text);
    SendMessage(hTextView2, EM_SETBKGNDCOLOR, 0, (LPARAM)color);

    PARAFORMAT pf = { sizeof(PARAFORMAT) };
    pf.dwMask = PFM_ALIGNMENT;
    pf.wAlignment = PFA_CENTER;
    SendMessage(hTextView2, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}

void CALLBACK ClearText(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    validateclickz(hwnd, L"", RGB(255, 255, 255));
    KillTimer(hwnd, idEvent);
}

void validateclickzWithTimer(HWND hwnd, const wchar_t* text, COLORREF color, UINT delay) {
    validateclickz(hwnd, text, color);
    SetTimer(hwnd, TIMER_ID, delay, ClearText);
}

std::string wstringToUtf8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

void compressFileWithExtension(HWND hwnd, const std::wstring& inputFilePath, int numThreads) {
    size_t extPos = inputFilePath.find_last_of(L'.');
    size_t namePos = inputFilePath.find_last_of(L"/\\");

    if (extPos == std::wstring::npos) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;
    }

    std::wstring filename = (namePos != std::wstring::npos)
                                ? inputFilePath.substr(namePos + 1, extPos - namePos - 1)
                                : inputFilePath.substr(0, extPos);
    std::wstring outputDirectory = L"compressed\\";
    CreateDirectory(outputDirectory.c_str(), NULL);
    std::wstring outputFilePath = outputDirectory + filename + L".su";

    std::string inputFile = wstringToUtf8(inputFilePath);
    std::string outputFile = wstringToUtf8(outputFilePath);
    std::string filenameUtf8 = wstringToUtf8(filename);

    std::ifstream inFile(inputFile, std::ios::binary | std::ios::ate);
    if (!inFile) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;
    }

    std::streamsize fileSize = inFile.tellg();
    if (fileSize <= 0) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;
    }

    inFile.seekg(0, std::ios::beg);
    std::vector<uint8_t> inputBuffer(fileSize);
    if (!inFile.read(reinterpret_cast<char*>(inputBuffer.data()), fileSize)) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;
    }
    inFile.close();

    lzma_stream strm = LZMA_STREAM_INIT;

    lzma_mt mt = {0};
    mt.threads = numThreads;
    mt.block_size = 0;
    mt.timeout = 300;
    mt.check = LZMA_CHECK_CRC32;

    if (lzma_stream_encoder_mt(&strm, &mt) != LZMA_OK) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        return;
    }

    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        validateclickzWithTimer(hwnd, L"Error loading file!", RGB(255, 0, 0), 3000);
        lzma_end(&strm);
        return;
    }

    char filenameBuffer[FIXED_NAME_SIZE] = {0};
    std::memcpy(filenameBuffer, filenameUtf8.c_str(), std::min(filenameUtf8.size(), FIXED_NAME_SIZE));
    outFile.write(filenameBuffer, FIXED_NAME_SIZE);

    strm.next_in = inputBuffer.data();
    strm.avail_in = fileSize;
    std::vector<uint8_t> outputBuffer(1024);
    strm.next_out = outputBuffer.data();
    strm.avail_out = outputBuffer.size();

    size_t processed = 0;
    while (strm.avail_in > 0) {
        size_t prev_avail_in = strm.avail_in;
        if (lzma_code(&strm, LZMA_RUN) != LZMA_OK) break;

        processed += (prev_avail_in - strm.avail_in);
        double progress = (static_cast<double>(processed) / fileSize) * 100;

        wchar_t progressText[50];
        swprintf(progressText, 50, L"Compressing: %.1f%%", progress);
        validateclickz(hwnd, progressText, RGB(0, 255, 0));

        if (strm.avail_out == 0) {
            outFile.write(reinterpret_cast<char*>(outputBuffer.data()), outputBuffer.size());
            strm.next_out = outputBuffer.data();
            strm.avail_out = outputBuffer.size();
        }
    }

    while (lzma_code(&strm, LZMA_FINISH) == LZMA_OK) {
        if (strm.avail_out == 0) {
            outFile.write(reinterpret_cast<char*>(outputBuffer.data()), outputBuffer.size());
            strm.next_out = outputBuffer.data();
            strm.avail_out = outputBuffer.size();
        }
    }

    if (strm.avail_out < outputBuffer.size()) {
        outFile.write(reinterpret_cast<char*>(outputBuffer.data()), outputBuffer.size() - strm.avail_out);
    }

    outFile.close();
    lzma_end(&strm);

    validateclickzWithTimer(hwnd, L"Compression Complete!", RGB(0, 255, 0), 5000);
}
