#include "../shared/shared_ini.h"
#include "../shared/shared_game.h"

#include "game_client.h"
#include "game_local.h"

#include "../vendor/raylib/include/raylib.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <cstring>


static GameSettings ParseConfigFileForGameSettings(Config & config) {
    GameSettings settings = {};
    for (int i = 0; i < config.entryCount; i++) {
        if (strcmp(config.entries[i].key, "window_x") == 0) {
            settings.posX = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_y") == 0) {
            settings.posY = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_width") == 0) {
            settings.width = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_height") == 0) {
            settings.height = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_fullscreen") == 0) {
            settings.fullscreen = atoi(config.entries[i].value) != 0;
        }
        else if (strcmp(config.entries[i].key, "server_ip") == 0) {
            strcpy_s(settings.serverIp, config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "server_port") == 0) {
            settings.serverPort = atoi(config.entries[i].value);
        }
    }

    return settings;
}

static GameSettings CreateDefaultGameSettings() {
    GameSettings settings = {};
    settings.width = 1280;
    settings.height = 720;
    settings.posX = 200;
    settings.posY = 200;
    settings.fullscreen = false;
    settings.serverPort = 27164;
    strcpy_s(settings.serverIp, "127.0.0.1");

    return settings;
};

static f32 tempTextTimer = 0.0f;
static const char * tempCeterText = "";
static void SetTempCenterText(const char * text, f32 time) {
    tempTextTimer = time;
    tempCeterText = text;
}

static void DrawTempCenterText(i32 w, i32 h) {
    if (tempTextTimer > 0.0f) {
        tempTextTimer -= GetFrameTime();
        DrawText(tempCeterText, w / 2 - MeasureText(tempCeterText, 20) / 2, h / 2 - 20 / 2, 20, BLACK);
    }
    else {
        tempCeterText = "";
    }
}

struct UIToolBar {
    bool vertical;
    i32 currentP;
};

static bool ToolBarButton(UIToolBar & tb, const char * text) {
    bool res = false;
    if (tb.vertical) {
        res = UIDrawButtonTopLeft(0, tb.currentP, text);
        tb.currentP += 50;
        return res;
    }
    else {
        Rectangle bb = {};
        res = UIDrawButtonTopLeft(tb.currentP, 0, text, &bb);
        tb.currentP += (i32)bb.width;
    }

    return res;
}

enum LevelEditorToolMode {
    LEVEL_EDITOR_TOOL_MODE_INVALID = 0,
    LEVEL_EDITOR_TOOL_MODE_TILE,
    LEVEL_EDITOR_TOOL_MODE_P1,
    LEVEL_EDITOR_TOOL_MODE_P2,
    LEVEL_EDITOR_TOOL_MODE_BROWN_ENEMY,
    LEVEL_EDITOR_TOOL_MODE_COUNT,
};

struct LevelEditor {
    std::string mapName;
    LevelEditorToolMode toolMode;
    UIToolBar menuTB;
};

static void ToolBarButtonEditorMode(LevelEditor & editor, LevelEditorToolMode mode, const char * text) {
    UIColorsPush(UI_COLOR_SLOT_BACKGROUND, Fade(ORANGE, 0.5f));
    if (editor.toolMode == mode) {
        UIColorsPush(UI_COLOR_SLOT_BACKGROUND, Fade(GREEN, 0.5f));
        if (ToolBarButton(editor.menuTB, text)) {
            editor.toolMode = mode;
        }
        UIColorsPop(UI_COLOR_SLOT_BACKGROUND);
    }
    else {
        if (ToolBarButton(editor.menuTB, text)) {
            editor.toolMode = mode;
        }
    }
    UIColorsPop(UI_COLOR_SLOT_BACKGROUND);
}

#include "../vendor/fixed/Fixed64.h"

int main(int argc, char * argv[]) {

    const char * iniPath = nullptr;
    if (argc == 2) {
        iniPath = argv[1];
    }

    GameSettings gameSettings = CreateDefaultGameSettings();
    if (iniPath != nullptr) {
        printf("Using ini file: %s\n", iniPath);
        static Config cfg = {};
        ReadEntireConfigFile(iniPath, cfg);
        gameSettings = ParseConfigFileForGameSettings(cfg);
    }

    printf("width: %d\n", gameSettings.width);
    printf("height: %d\n", gameSettings.height);
    printf("posX: %d\n", gameSettings.posX);
    printf("posY: %d\n", gameSettings.posY);
    printf("using server: %s\n", gameSettings.serverIp);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(gameSettings.width, gameSettings.height, "Yes. This is a game you slut!");

    SetWindowPosition(gameSettings.posX, gameSettings.posY);

    bool recreateSufrace = false;
    i32 surfaceWidth = 1280;
    i32 surfaceHeight = 720;

    RenderTexture2D surface = LoadRenderTexture(surfaceWidth, surfaceHeight);
    SetTextureFilter(surface.texture, TEXTURE_FILTER_BILINEAR);

    UIColorsCreate();

    static GameLocal gameLocal = {};
    gameLocal.gameSettings = gameSettings;
    gameLocal.cam.zoom = 1.0f;
    gameLocal.cam.offset = { surfaceWidth / 2.0f, surfaceHeight / 2.0f };

    SetTargetFPS(60);
    while (!WindowShouldClose() && gameLocal.running) {
        switch (gameLocal.screen) {
        case SCREEN_TYPE_MAIN_MENU: DoScreenMainMenu(gameLocal); break;
        case SCREEN_TYPE_GAME:      DoScreenGame(gameLocal, surfaceWidth, surfaceHeight, surface); break;
        default: Assert(false); break;
        }
    }

    NetoworkDisconnectFromServer();

    CloseWindow();

    return 0;
}
