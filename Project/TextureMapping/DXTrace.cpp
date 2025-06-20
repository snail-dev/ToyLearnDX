#include "DXTrace.h"
#include <Windows.h>
#include <cstdio>

HRESULT WINAPI DXTraceW(
        _In_z_ const WCHAR* file, _In_ DWORD line, _In_ HRESULT hr, _In_opt_ const WCHAR* msg, _In_ bool popMsgBox) {
    WCHAR strBufferLine[128];
    WCHAR strBufferFile[MAX_PATH];
    WCHAR strBufferHR[40];
    WCHAR strBufferErrors[300];
    WCHAR strBufferMsg[1024];
    WCHAR strBuffer[3000];

    swprintf_s(strBufferLine, 128, L"%lu", line);

    if (file) {
        swprintf_s(strBuffer, 3000, L"%ls(%ls):", file, strBufferLine);
        OutputDebugStringW(strBuffer);
    }

    size_t nMsgLen = (msg) ? wcsnlen_s(msg, 1024) : 0;
    if (nMsgLen > 0) {
        OutputDebugStringW(msg);
        OutputDebugStringW(L"");
    }

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), strBufferErrors, 256, nullptr);

    WCHAR* errorStr = wcsrchr(strBufferErrors, L'\r');
    if (errorStr) {
        errorStr[0] = L'\0';
    }

    swprintf_s(strBufferHR, 40, L"(%0x0.8x)", hr);
    wcscat_s(strBufferErrors, strBufferHR);
    swprintf_s(strBuffer, 3000, L"错误码含义: %ls", strBufferErrors);
    OutputDebugStringW(strBuffer);

    OutputDebugStringW(L"\n");

    if (popMsgBox) {
        wcscpy_s(strBufferFile, MAX_PATH, L"");
        if (file) {
            wcscpy_s(strBufferFile, MAX_PATH, file);
        }

        wcscpy_s(strBufferMsg, 1024, L"");
        if (nMsgLen > 0) {
            swprintf_s(strBufferMsg, 1024, L"当前调用: %ls\n", msg);
        }

        swprintf_s(strBuffer, 3000, L"文件名:%ls\n行号:%ls\n错误码含义:%ls\n%ls是否调试当前程序?", strBufferFile,
                strBufferLine, strBufferErrors, strBufferMsg);

        int nResult = MessageBoxW(GetForegroundWindow(), strBuffer, L"错误", MB_YESNO | MB_ICONERROR);
        if (nResult == IDYES)
            DebugBreak();
    }

    return hr;
}
