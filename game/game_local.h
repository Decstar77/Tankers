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

enum UISubmissionType {
    UI_SUBMISSION_TYPE_INVALID = 0,
    UI_SUBMISSION_TYPE_BUTTON,
    UI_SUBMISSION_TYPE_BLOCK_BUTTON,
    UI_SUBMISSION_TYPE_COUNT,
};

struct UISubmission {
    UISubmissionType type;
    LargeString text;
    Vector2 textSize;
    Rectangle rect;
    bool isHovered;
    bool isPressed;
    Color baseColor;
    Color hoveredColor;
    Color pressedColor;
};

struct UIState {
    UIColors                    uiColors;
    FixedList<UISubmission, 32> submissions;
    v2                          surfaceMouse;
    bool                        elementHovered;
};

void UIStateReset();
void UIStateDraw();
void UIColorsCreate();
void UIColorsPush(UIColorSlot slot, Color c);
void UIColorsPop(UIColorSlot slot);
Color UIColorsGet(UIColorSlot slot);
bool UIDrawButtonCenter(i32 centerX, i32 centerY, const char * text, Rectangle * bb = nullptr);
bool UIDrawButtonTopLeft(i32 topLeftX, i32 topLeftY, const char * text, Rectangle * bb = nullptr);
bool UIDrawBlockButton(v2 center, v2 dims, Color c, const char * text);

void DoCameraPanning(Camera2D & cam);
void DoCameraZooming(Camera2D & cam);

void DoScreenMainMenu(GameLocal & gameLocal);
void DoScreenGame(GameLocal & gameLocal, i32 surfaceWidth, i32 surfaceHeight, RenderTexture2D surface);