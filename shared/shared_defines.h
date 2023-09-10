#pragma once

#define ALLOW_DEBUG_CODE 1

#define FILE_NAME __FILE__
#define LINE_NUMBER __LINE__

#define Assert(x) if (!(x)) { printf(#x); PlatformAssert(#x, FILE_NAME, LINE_NUMBER);  *(int *)0 = 0; }
#define ZeroStruct(s) memset(&(s), 0, sizeof(s))
#define ZeroMemory(s, n) memset((s), 0, (n))
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

void         PlatformAssert(const char * msg, const char * file, int line);
const char * PlatformFileDialogOpen(const char * path, const char * filter);
const char * PlatformFileDialogSave(const char * path, const char * filter);

struct StringView {
    const char * data;
    int          length;
};

struct SmallString {
    char data[64];
    int  length;

    operator StringView() const {
        return { data, length };
    }
};

struct LargeString {
    char data[256];
    int  length;

    operator StringView() const {
        return { data, length };
    }
};

// bool StringEquals(StringView * a, StringView * b);
// bool StringStartsWith(StringView * str, StringView * substr);
// bool StringEndsWith(StringView * str, StringView * substr);
// bool StringContains(StringView * str, StringView * substr);

// void StringCopy(StringView * dst, StringView * src);
// void StringAppend(StringView * dst, StringView * src);
// void StringAppend(StringView * dst, const char * src);
// void StringAppend(StringView * dst, char c);
// void StringAppend(StringView * dst, int i);
// void StringAppend(StringView * dst, float f);

// void StringFormat(StringView * dst, const char * fmt, ...);