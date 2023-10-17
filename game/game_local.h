#pragma once
/*
#include "game_sync.h"
#include "game_settings.h"

#include "../shared/shared_game.h"
#include "../vendor/raylib/include/raylib.h" // Not the cleanest way to do this, but it works for now.

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


void DoCameraPanning(Camera2D & cam);
void DoCameraZooming(Camera2D & cam);

void DoScreenMainMenu(GameLocal & gameLocal);
void DoScreenGame(GameLocal & gameLocal, i32 surfaceWidth, i32 surfaceHeight, RenderTexture2D surface);

Entity * GetEntityUnderMouse(Map &map, v2 worldMouse);
*/