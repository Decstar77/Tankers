#include "../shared/shared_ini.h"
#include "../shared/shared_game.h"

#include "game_client.h"
#include "game_local.h"

#include "../vendor/raylib/include/raylib.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <cstring>

static v2 surfaceMouse = {};

struct GameSettings {
    int width;
    int height;
    int posX;
    int posY;
    bool fullscreen;
    char serverIp[64];
};

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

    return settings;
};

enum ColorSlot {
    COLOR_SLOT_INVALID = 0,
    COLOR_SLOT_BACKGROUND,
    COLOR_SLOT_COUNT,
};

struct UIColors {
    i32     counts[COLOR_SLOT_COUNT];
    Color   stacks[COLOR_SLOT_COUNT][8];
};

static UIColors uiColors = {};

void UIColorsCreate() {
    uiColors.counts[COLOR_SLOT_BACKGROUND] = 1;
    uiColors.stacks[COLOR_SLOT_BACKGROUND][0] = LIGHTGRAY;
}

void UIColorsPush(ColorSlot slot, Color c) {
    Assert(uiColors.counts[slot] < ArrayCount(uiColors.stacks[slot]));
    Assert(slot != COLOR_SLOT_INVALID);
    uiColors.stacks[slot][uiColors.counts[slot]] = c;
    uiColors.counts[slot]++;
}

void UIColorsPop(ColorSlot slot) {
    Assert(uiColors.counts[slot] > 0);
    Assert(slot != COLOR_SLOT_INVALID);
    uiColors.counts[slot]--;
}

Color UIColorsGet(ColorSlot slot) {
    Assert(uiColors.counts[slot] > 0);
    Assert(slot != COLOR_SLOT_INVALID);
    return uiColors.stacks[slot][uiColors.counts[slot] - 1];
}

static bool DrawButtonCenter(i32 centerX, i32 centerY, const char * text, Rectangle * bb = nullptr) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
    Rectangle rect = { (float)centerX - rectSize.x / 2, (float)centerY - rectSize.y / 2, rectSize.x, rectSize.y };

    DrawRectangleRec(rect, UIColorsGet(COLOR_SLOT_BACKGROUND));

    if (bb != nullptr) {
        *bb = rect;
    }

    Vector2 textPos = { rect.x + rect.width / 2 - textSize.x / 2, rect.y + rect.height / 2 - textSize.y / 2 };
    DrawTextEx(GetFontDefault(), text, textPos, 20, 1, BLACK);

    Vector2 mousePos = { surfaceMouse.x, surfaceMouse.y };
    if (CheckCollisionPointRec(mousePos, rect)) {
        DrawRectangleLinesEx(rect, 2, RED);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}

static bool DrawButtonTopLeft(i32 centerX, i32 centerY, const char * text, Rectangle * bb = nullptr) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
    Rectangle rect = { (float)centerX, (float)centerY, rectSize.x, rectSize.y };

    DrawRectangleRec(rect, UIColorsGet(COLOR_SLOT_BACKGROUND));

    if (bb != nullptr) {
        *bb = rect;
    }

    Vector2 textPos = { rect.x + rect.width / 2 - textSize.x / 2, rect.y + rect.height / 2 - textSize.y / 2 };
    DrawTextEx(GetFontDefault(), text, textPos, 20, 1, BLACK);

    Vector2 mousePos = { surfaceMouse.x, surfaceMouse.y };
    if (CheckCollisionPointRec(mousePos, rect)) {
        DrawRectangleLinesEx(rect, 2, RED);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}

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

enum ScreenType {
    SCREEN_TYPE_MAIN_MENU,
    SCREEN_TYPE_GAME,
    SCREEN_TYPE_GAME_OVER,
    SCREEN_TYPE_LEVEL_EDITOR,
};

struct UIToolBar {
    bool vertical;
    i32 currentP;
};

static bool ToolBarButton(UIToolBar & tb, const char * text) {
    bool res = false;
    if (tb.vertical) {
        res = DrawButtonTopLeft(0, tb.currentP, text);
        tb.currentP += 50;
        return res;
    }
    else {
        Rectangle bb = {};
        res = DrawButtonTopLeft(tb.currentP, 0, text, &bb);
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
    UIColorsPush(COLOR_SLOT_BACKGROUND, Fade(ORANGE, 0.5f));
    if (editor.toolMode == mode) {
        UIColorsPush(COLOR_SLOT_BACKGROUND, Fade(GREEN, 0.5f));
        if (ToolBarButton(editor.menuTB, text)) {
            editor.toolMode = mode;
        }
        UIColorsPop(COLOR_SLOT_BACKGROUND);
    }
    else {
        if (ToolBarButton(editor.menuTB, text)) {
            editor.toolMode = mode;
        }
    }
    UIColorsPop(COLOR_SLOT_BACKGROUND);
}

static void DoCameraPanning(Camera2D & cam) {
    f32 speed = 10.0f;
    if (IsKeyDown(KEY_W)) {
        cam.target.y -= speed;
    }
    if (IsKeyDown(KEY_S)) {
        cam.target.y += speed;
    }
    if (IsKeyDown(KEY_A)) {
        cam.target.x -= speed;
    }
    if (IsKeyDown(KEY_D)) {
        cam.target.x += speed;
    }
    if (IsKeyDown(KEY_Q)) {
        cam.zoom -= 0.1f;
    }
    if (IsKeyDown(KEY_E)) {
        cam.zoom += 0.1f;
    }
}

static void DrawGeneral(Entity & entity) {
    Vector2 pos = { FpToFloat(entity.general.pos.x), FpToFloat(entity.general.pos.y) };
    if (entity.selected) {
        DrawCircleV(pos, 10, GREEN);
    }
    DrawCircleV(pos, 7, RED);
}

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

    static LevelEditor editor = {};
    static ScreenType screen = SCREEN_TYPE_MAIN_MENU;

    bool debugPauseSim = false;

    Map map = {};
    MapSpawnGeneral(map);

    Camera2D cam = {};
    cam.zoom = 1.0f;
    cam.offset = { surfaceWidth / 2.0f, surfaceHeight / 2.0f };

    SetTargetFPS(60);

    f32 accumulator = 0.0f;
    while (!WindowShouldClose()) {
        f32 scale = Min((f32)GetScreenWidth() / surfaceWidth, (f32)GetScreenHeight() / surfaceHeight);

        Vector2 mouse = GetMousePosition();
        Vector2 mouseDelta = GetMouseDelta();
        bool mouseMoved = true;
        if (mouseDelta.x == 0 && mouseDelta.y == 0) {
            mouseMoved = false;
        }

        surfaceMouse = {};
        surfaceMouse.x = (mouse.x - (GetScreenWidth() - (surfaceWidth * scale)) * 0.5f) / scale;
        surfaceMouse.y = (mouse.y - (GetScreenHeight() - (surfaceHeight * scale)) * 0.5f) / scale;
        surfaceMouse = Clamp(surfaceMouse, { 0, 0 }, { (float)surfaceWidth, (float)surfaceHeight });

        DoCameraPanning(cam);

        Vector2 rayMouseWorld = GetScreenToWorld2D({ surfaceMouse.x, surfaceMouse.y }, cam);
        v2 mouseWorld = { rayMouseWorld.x, rayMouseWorld.y };

        static v2 startDrag = {};
        static v2 endDrag = {};
        static bool dragging = false;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            startDrag = mouseWorld;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && mouseMoved) {
            dragging = true;
        }

        if (dragging == true) {
            endDrag = mouseWorld;
        }

        if (dragging == true && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            dragging = false;
            startDrag = {};
            endDrag = {};

            Rect selectionRect = {};
            selectionRect.min = Min(startDrag, endDrag);
            selectionRect.max = Max(startDrag, endDrag);

            for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
                Entity * entity = &map.entities[i];
                if (entity->inUse) {
                    Circle c = EntityGetSelectionBounds(entity);
                    if (CircleVsRect(c, selectionRect)) {
                        entity->selected = true;
                    }
                    else {
                        entity->selected = false;
                    }
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
                Entity * entity = &map.entities[i];
                if (entity->inUse) {
                    if (entity->selected) {
                        entity->general.pos = V2fp(mouseWorld);
                    }
                }
            }
        }

        BeginTextureMode(surface);
        BeginMode2D(cam);
        ClearBackground(RAYWHITE);

        DrawGeneral(map.entities[0]);

        if (dragging == true) {
            v2 topLeft = {};
            topLeft.x = Min(startDrag.x, endDrag.x);
            topLeft.y = Min(startDrag.y, endDrag.y);
            v2 bottomRight = {};
            bottomRight.x = Max(startDrag.x, endDrag.x);
            bottomRight.y = Max(startDrag.y, endDrag.y);
            DrawRectangleRec({ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y }, Fade(GREEN, 0.5f));
        }

        DrawTempCenterText(surfaceWidth, surfaceHeight);

        EndMode2D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle r1 = { 0.0f, 0.0f, (float)surface.texture.width, (float)-surface.texture.height };
        Rectangle r2 = {};
        r2.x = (GetScreenWidth() - ((float)surfaceWidth * scale)) * 0.5f;
        r2.y = (GetScreenHeight() - ((float)surfaceHeight * scale)) * 0.5f;
        r2.width = (float)surfaceWidth * scale;
        r2.height = (float)surfaceHeight * scale;
        DrawTexturePro(surface.texture, r1, r2, {}, 0.0f, WHITE);

        EndDrawing();
    }

    NetoworkDisconnectFromServer();

    CloseWindow();

    return 0;
}
