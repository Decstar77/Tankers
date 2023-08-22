#pragma once

#define ALLOW_DEBUG_CODE 1

#define FILE_NAME __FILE__
#define LINE_NUMBER __LINE__

#define Assert(x) if (!(x)) { printf(#x); PlatformAssert(#x, FILE_NAME, LINE_NUMBER);  *(int *)0 = 0; }
#define ZeroStruct(s) memset(&(s), 0, sizeof(s))
#define ZeroMemory(s, n) memset((s), 0, (n))
#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Stringify(x) #x

typedef int i32;
typedef float f32;

void         PlatformAssert(const char * msg, const char * file, int line);
const char * PlatformFileDialogOpen(const char * path, const char * filter);