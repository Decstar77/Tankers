#pragma once

#define Assert(x) if (!(x)) { printf(#x);  *(int *)0 = 0; }
#define ZeroStruct(s) memset(&(s), 0, sizeof(s))
#define ZeroMemory(s, n) memset((s), 0, (n))

typedef int i32;
typedef float f32;