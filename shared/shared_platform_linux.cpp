#ifdef __linux__

#include <stdio.h>
void PlatformAssert(const char * msg, const char * file, int line) {
    printf("Assertion failed: %s\nFile: %s\nLine: %d\n", msg, file, line);
    *(int *)0 = 0;
}

const char * PlatformFileDialogOpen(const char * path, const char * filter) {
    return nullptr;
}

const char * PlatformFileDialogSave(const char * path, const char * filter) {
    return nullptr;
}

#endif