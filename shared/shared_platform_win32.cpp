#ifdef _WIN32

#include <stdio.h>
#include <Windows.h>

void PlatformAssert(const char * msg, const char * file, const char * functionName, int line) {
    char buffer[1024];
    sprintf_s(buffer, sizeof(buffer), "Assertion failed: %s\nFile: %s\n Func: %s\n Line: %d\n", msg, file, functionName, line);
    OutputDebugStringA(buffer);
    MessageBoxA(nullptr, buffer, "Assertion failed", MB_OK | MB_ICONERROR);
    exit(1);
}

const char * PlatformFileDialogOpen(const char * path, const char * filter) {
    static char buffer[1024] = {};
    buffer[0] = 0;

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = sizeof(buffer);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = path;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        return buffer;
    }

    return nullptr;
}

const char * PlatformFileDialogSave(const char * path, const char * filter) {
    static char buffer[1024] = {};
    buffer[0] = 0;

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = sizeof(buffer);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = path;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetSaveFileNameA(&ofn)) {
        return buffer;
    }

    return nullptr;
}

void PlatformPrint(const char * msg, ...) {
    char buffer[1024] = {};
    va_list args;
    va_start(args, msg);
    vsprintf_s(buffer, sizeof(buffer), msg, args);
    va_end(args);
    OutputDebugStringA(buffer);
}

#endif


