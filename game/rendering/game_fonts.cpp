#include "game_fonts.h"

#undef ZeroMemory

#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define FONTSTASH_IMPLEMENTATION
#include "../../vendor/fontstash/src/fontstash.h"

#include "../../vendor/glfw/include/glfw/glfw3.h"

#define GLFONTSTASH_IMPLEMENTATION
#include "../../vendor/fontstash/src/glfontstash.h"


static FONScontext * fs = nullptr;
int fontId = FONS_INVALID;

void InitFontRendering() {
    fs = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
    Assert(fs != nullptr);
    fontId = fonsAddFont(fs, "sans", "C:\\Projects\\Play2\\content\\DroidSerif-Regular.ttf");
    Assert(fontId != FONS_INVALID);
}

void FontRenderSomeText() {
    float dx = 10, dy = 10;
    unsigned int white = glfonsRGBA(255, 255, 255, 255);
    unsigned int brown = glfonsRGBA(192, 128, 0, 128);

    fonsSetFont(fs, fontId);
    fonsSetSize(fs, 124.0f);
    fonsSetColor(fs, white);
    fonsDrawText(fs, dx, dy, "The big ", NULL);

    fonsSetSize(fs, 24.0f);
    fonsSetColor(fs, brown);
    fonsDrawText(fs, dx,dy,"brown fox", NULL);
}
