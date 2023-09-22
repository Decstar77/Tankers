#pragma once

#include "game_sync.h"

#include "../shared/shared_game.h"
#include "../vendor/raylib/include/raylib.h" // Not the cleanest way to do this, but it works for now.

struct GameSettings {
    int width;
    int height;
    int posX;
    int posY;
    bool fullscreen;
    char serverIp[64];
    int serverPort;
};

enum ScreenType {
    SCREEN_TYPE_MAIN_MENU,
    SCREEN_TYPE_GAME,
    SCREEN_TYPE_GAME_OVER,
    SCREEN_TYPE_LEVEL_EDITOR,
};

struct GameLocal {
    bool running = true;
    GameSettings gameSettings = {};
    ScreenType screen = SCREEN_TYPE_MAIN_MENU;
    Map map = {};
    MapTurn mapTurn = {};
    SyncQueues sync = {};
    Camera2D cam = {};
    f32 turnAccumulator = 0;
    i32 playerNumber = 0;
};

enum UIColorSlot {
    UI_COLOR_SLOT_INVALID = 0,
    UI_COLOR_SLOT_BACKGROUND,
    UI_COLOR_SLOT_COUNT,
};

struct UIColors {
    i32     counts[UI_COLOR_SLOT_COUNT];
    Color   stacks[UI_COLOR_SLOT_COUNT][8];
};

struct UIState {
    UIColors    uiColors;
    v2          surfaceMouse;
};

void UIColorsCreate();
void UIColorsPush(UIColorSlot slot, Color c);
void UIColorsPop(UIColorSlot slot);
Color UIColorsGet(UIColorSlot slot);
bool UIDrawButtonCenter(i32 centerX, i32 centerY, const char * text, Rectangle * bb = nullptr);
bool UIDrawButtonTopLeft(i32 centerX, i32 centerY, const char * text, Rectangle * bb = nullptr);

void DoCameraPanning(Camera2D & cam);
void DoCameraZooming(Camera2D & cam);

void DoScreenMainMenu(GameLocal & gameLocal);
void DoScreenGame(GameLocal & gameLocal, i32 surfaceWidth, i32 surfaceHeight, RenderTexture2D surface);