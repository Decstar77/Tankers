#ifdef _WIN32

#include <stdio.h>
#include <Windows.h>

void PlatformAssert(const char * msg, const char * file, int line) {
    char buffer[1024];
    sprintf_s(buffer, sizeof(buffer), "Assertion failed: %s\nFile: %s\nLine: %d\n", msg, file, line);
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

#endif

