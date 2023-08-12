#pragma once

#define Assert(x) if (!(x)) { printf(#x);  *(int *)0 = 0; }
#define ZeroStruct(s) memset(&(s), 0, sizeof(s))

typedef int i32;
typedef float f32;