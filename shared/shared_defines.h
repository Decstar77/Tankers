#pragma once

#define ALLOW_DEBUG_CODE 1

#define FILE_NAME __FILE__
#define FUNCTION_NAME __func__
#define LINE_NUMBER __LINE__

#define Assert(x)                                                               \
    {                                                                           \
        if (!(x)) {                                                             \
            PlatformPrint(#x);                                                  \
            PlatformAssert(#x, FILE_NAME, FUNCTION_NAME, LINE_NUMBER);          \
            *(int *)0 = 0;                                                      \
        }                                                                       \
    }

#define AssertMsg(expr, msg)                                                    \
    {                                                                           \
        if (expr) {                                                             \
        } else {                                                                \
            PlatformAssert(msg, FILE_NAME, FUNCTION_NAME, LINE_NUMBER);         \
        }                                                                       \
    }


#define ZeroStruct(s) MemorySet(&(s), 0, sizeof(s))
#define ZeroMemory(s, n) MemorySet((s), 0, (n))
#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Stringify(x) #x

typedef short       i16;
typedef int         i32;
typedef long long   i64;
typedef float       f32;
typedef double      f64;

static_assert(sizeof(i16) == 2, "i16 is not 2 bytes");
static_assert(sizeof(i32) == 4, "i32 is not 4 bytes");
static_assert(sizeof(i64) == 8, "i64 is not 8 bytes");
static_assert(sizeof(f32) == 4, "f32 is not 4 bytes");
static_assert(sizeof(f64) == 8, "f64 is not 8 bytes");

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

static_assert(sizeof(u8) == 1, "Expected uint8 to be 1 byte.");
static_assert(sizeof(u16) == 2, "Expected uint16 to be 2 bytes.");
static_assert(sizeof(u32) == 4, "Expected uint32 to be 4 bytes.");
static_assert(sizeof(u64) == 8, "Expected uint64 to be 8 bytes.");

typedef int b32;
typedef bool b8;
typedef u8 byte;

void         PlatformAssert(const char * msg, const char * file, const char * functionName, int line);
const char * PlatformFileDialogOpen(const char * path, const char * filter);
const char * PlatformFileDialogSave(const char * path, const char * filter);

void         PlatformPrint(const char * msg, ...);

void        MemorySet(void * dest, u8 value, u64 size);