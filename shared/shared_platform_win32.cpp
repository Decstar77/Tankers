#ifdef _WIN32

#include <stdio.h>
#include <Windows.h>

void PlatformAssert(const char *msg, const char *file, int line) {
    char buffer[1024];
    sprintf_s(buffer, sizeof(buffer), "Assertion failed: %s\nFile: %s\nLine: %d\n", msg, file, line);
    OutputDebugStringA(buffer);
    MessageBoxA(nullptr, buffer, "Assertion failed", MB_OK | MB_ICONERROR);
    exit(1);
}

#endif


