#ifndef EVENTS_H
#define EVENTS_H

#include <windows.h>
#include <richedit.h>
#include <commctrl.h> 

void OnButtonUpdateClicked(HWND hwnd);
void OnButtonCopyClicked(HWND hwnd); 
void OnButtonCompressClicked(HWND hwnd); 
void OnButtonDecompressClicked(HWND hwnd);  
void OnButtonNextClicked(HWND hwnd);
void OnButtonAttachFileClicked(HWND hwnd);
void OnButtonPrevClicked(HWND hwnd);
void OnButtonNextClicked(HWND hwnd);
void OnButtonDeleteClicked(HWND hwnd);
void UpdateListBoxWithTextViewContent(HWND hwnd, HWND hTextView, HWND hListBox);
void validateclick(HWND hwnd, const wchar_t* text, COLORREF color);

#endif // EVENTS_H
