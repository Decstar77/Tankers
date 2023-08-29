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

static Config cfg = {};

static void DrawTank(v2 p, f32 size, f32 r, f32 tr, Color c) {
    Rectangle rect = { p.x, p.y, size, size };
    Vector2 origin = { size / 2, size / 2 };
    DrawRectanglePro(rect, origin, RadToDeg(r), c);

    f32 gunRad = 15;
    Vector2 start = { p.x, p.y };
    Vector2 end = { p.x + gunRad * cosf(tr), p.y + gunRad * sinf(tr) };
    DrawLineEx(start, end, 2.0f, BLACK);

#if 0
    // Draw the colliders
    DrawCircle((int)p.x, (int)p.y, (int)size / 2.0f, Fade(RED, 0.5f));
#endif
}

static void DrawPlayer(Player * player) {
    Color color = player->playerNumber == 1 ? RED : BLUE;
    DrawTank(player->tank.pos, player->tank.size, player->tank.rot, player->tank.turretRot, color);
}

static Color GetColorForEnemyType(EnemyType type) {
    switch (type) {
    case ENEMY_TYPE_LIGHT_BROWN: return { 196, 164, 132, 255 };
    case ENEMY_TYPE_DARK_BROWN: return { 128, 64, 0, 255 };
    }

    return LIGHTGRAY;
}

static void DrawEnemy(Enemy * enemy) {
    Color color = GetColorForEnemyType(enemy->type);
    DrawTank(enemy->tank.pos, enemy->tank.size, enemy->tank.rot, enemy->tank.turretRot, color);
}

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

static void DrawMapTile(MapTile & tile) {
    Rectangle rect = { tile.rect.min.x, tile.rect.min.y, tile.rect.max.x - tile.rect.min.x, tile.rect.max.y - tile.rect.min.y };
    DrawRectangleRec(rect, DARKGRAY);
}

static void DrawMapGhostTile(MapTile & tile, Color c = Fade(DARKGRAY, 0.5f)) {
    Rectangle rect = { tile.rect.min.x, tile.rect.min.y, tile.rect.max.x - tile.rect.min.x, tile.rect.max.y - tile.rect.min.y };
    DrawRectangleRec(rect, c);
}

static void DrawMap(Map & map) {
    DrawPlayer(&map.localPlayer);
    DrawPlayer(&map.remotePlayer);

    // DrawEnemies
    for (i32 i = 0; i < MAX_ENEMIES; i++) {
        if (map.enemies[i].active == false) {
            continue;
        }

        DrawEnemy(&map.enemies[i]);
    }

    for (i32 i = 0; i < MAX_MAP_TILES; i++) {
        MapTile & tile = map.tiles[i];
        if (tile.active) {
            DrawMapTile(tile);
        }
    }

    // Draw bullets
    for (i32 i = 0; i < MAX_BULLETS; i++) {
        Bullet & bullet = map.bullets[i];
        if (bullet.active) {
            switch (bullet.type) {
            case BULLET_TYPE_NORMAL: {
                f32 size = BulletSizeFromType(bullet.type);
                DrawCircle((int)bullet.pos.x, (int)bullet.pos.y, size, ORANGE);
            } break;
            }
        }
    }
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

static void DoLevelModePlayer(LevelEditor & editor, Map & map, Player & player, v2 surfaceMouse) {
    bool validPlace = true;
    Circle c = PlayerGetColliderAtPos(&player, surfaceMouse);
    for (i32 i = 0; i < MAX_MAP_TILES; i++) {
        MapTile & tile = map.tiles[i];
        if (tile.active) {
            CollisionManifold manifold = {};
            if (CircleVsRect(c, tile.rect, &manifold)) {
                validPlace = false;
                break;
            }
        }
    }

    if (validPlace == true) {
        Color c = player.playerNumber == 1 ? RED : BLUE;
        DrawTank(surfaceMouse, player.tank.size, 0.0f, 0.0f, Fade(c, 0.5f));
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            player.tank.pos = surfaceMouse;
        }
    }
}

int main(int argc, char * argv[]) {
    const char * iniPath = "player_one.ini";
    if (argc == 2) {
        iniPath = argv[1];
    }

    printf("Using ini file: %s\n", iniPath);
    ReadEntireConfigFile(iniPath, cfg);
    GameSettings gameSettings = ParseConfigFileForGameSettings(cfg);

    printf("width: %d\n", gameSettings.width);
    printf("height: %d\n", gameSettings.height);
    printf("posX: %d\n", gameSettings.posX);
    printf("posY: %d\n", gameSettings.posY);
    printf("using server: %s\n", gameSettings.serverIp);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(gameSettings.width, gameSettings.height, "Yes. This is a game you slut!");

    SetWindowPosition(gameSettings.posX, gameSettings.posY);

    bool recreateSufrace = false;
    i32 surfaceWidth = 0;
    i32 surfaceHeight = 0;
    MapSize surfaceMapSize = MAP_SIZE_MEDIUM;
    MapSizeGetDimensions(surfaceMapSize, &surfaceWidth, &surfaceHeight);  // TODO: This will effect text rendering !!

    RenderTexture2D surface = LoadRenderTexture(surfaceWidth, surfaceHeight);
    SetTextureFilter(surface.texture, TEXTURE_FILTER_BILINEAR);

    UIColorsCreate();

    static LevelEditor editor = {};
    static Map map = {};
    static ScreenType screen = SCREEN_TYPE_MAIN_MENU;

    bool debugPauseSim = false;

    SetTargetFPS(60);

    f32 accumulator = 0.0f;
    while (!WindowShouldClose()) {
        f32 scale = Min((f32)GetScreenWidth() / surfaceWidth, (f32)GetScreenHeight() / surfaceHeight);

        Vector2 mouse = GetMousePosition();
        surfaceMouse = {};
        surfaceMouse.x = (mouse.x - (GetScreenWidth() - (surfaceWidth * scale)) * 0.5f) / scale;
        surfaceMouse.y = (mouse.y - (GetScreenHeight() - (surfaceHeight * scale)) * 0.5f) / scale;
        surfaceMouse = Clamp(surfaceMouse, { 0, 0 }, { (float)surfaceWidth, (float)surfaceHeight });

        BeginTextureMode(surface);
        ClearBackground(RAYWHITE);

        if (screen == SCREEN_TYPE_MAIN_MENU) {
            // Connect button
            if (NetworkIsConnected() == false) {
                static const char * text = "Connect";
                if (DrawButtonCenter(surfaceWidth / 2, surfaceHeight / 2, text)) {
                    if (NetworkConnectToServer("127.0.0.1", 27164) == false) {
                    //if (NetworkConnectToServer(gameSettings.serverIp, 27164) == false) {
                        text = "Connection failed please try again";
                    }
                }
                if (DrawButtonCenter(surfaceWidth / 2, surfaceHeight / 2 - 100, "Single Pringle")) {
                    MapLoadFile(map, "maps/demo.map");
                    MapStart(map, true);
                    MapSpawnPlayer(map);
                    MapSpawnPlayer(map);
                    screen = SCREEN_TYPE_GAME;
                }

                if (DrawButtonCenter(surfaceWidth / 2, surfaceHeight / 2 - 50, "Level Editor")) {
                    screen = SCREEN_TYPE_LEVEL_EDITOR;
                }
            }
            else {
                // Show waiting for game text
                const char * text = "Waiting for game, please wait";
                Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
                Vector2 centerPos = {};
                centerPos.x = surfaceWidth / 2 - textSize.x / 2;
                centerPos.y = surfaceHeight / 2 - textSize.y / 2;
                DrawText(text, (int)centerPos.x, (int)centerPos.y, 20, BLACK);
            }
        }

        if (NetworkIsConnected()) {
            GamePacket packet = {};
            while (NetworkPoll(packet)) {
                switch (packet.type) {
                case GAME_PACKET_TYPE_MAP_START: {
                    MapLoadFile(map, "maps/mp_01.map");
                    MapStart(map, false);
                    map.localPlayer = packet.mapStart.localPlayer;
                    map.remotePlayer = packet.mapStart.remotePlayer;

                    screen = SCREEN_TYPE_GAME;

                    printf("Map start, local player number %d\n", map.localPlayer.playerNumber);
                } break;
                case GAME_PACKET_TYPE_MAP_SHOT_FIRED: {
                    MapSpawnBullet(map, packet.shotFired.pos, packet.shotFired.dir, BULLET_TYPE_NORMAL);
                } break;
                case GAME_PACKET_TYPE_MAP_PLAYER_STREAM_DATA: {
                    map.remotePlayer.tank.remotePos = packet.playerStreamData.pos;
                    map.remotePlayer.tank.remoteRot = packet.playerStreamData.tankRot;
                    map.remotePlayer.tank.remoteTurretRot = packet.playerStreamData.turretRot;
                } break;
                case GAME_PACKET_TYPE_MAP_GAME_OVER: {
                    printf("Game over, reason: %s\n", MapGameOverReasonToString(packet.gameOver.reason));
                    screen = SCREEN_TYPE_GAME_OVER;
                } break;
                case GAME_PACKET_TYPE_MAP_ENTITY_STREAM_DATA: {
                    for (i32 i = 0; i < packet.entityStreamData.entityCount; i++) {
                        i32 index = packet.entityStreamData.indices[i];
                        map.enemies[index].tank.remotePos = packet.entityStreamData.pos[i];
                        map.enemies[index].tank.remoteRot = packet.entityStreamData.tankRot[i];
                        map.enemies[index].tank.remoteTurretRot = packet.entityStreamData.turretRot[i];
                    }
                } break;
                }
            }
        }

        if (screen == SCREEN_TYPE_GAME) {
            static bool shouldShoot = false;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                shouldShoot = true;
            }
#if ALLOW_DEBUG_CODE
            if (IsKeyPressed(KEY_F1)) {
                debugPauseSim = !debugPauseSim;
            }

            bool debugStepOne = false;
            if (IsKeyPressed(KEY_F2)) {
                debugStepOne = true;
            }
#endif

            if (debugPauseSim == false || debugStepOne == true) {
                float deltaTime = GetFrameTime();
                accumulator += deltaTime;

                while (accumulator >= GAME_TICK_TIME) {
                    accumulator -= GAME_TICK_TIME;

                    v2 dir = {};
                    if (IsKeyDown(KEY_W)) {
                        dir.y += -1.0f;
                    }
                    if (IsKeyDown(KEY_S)) {
                        dir.y += 1.0f;
                    }
                    if (IsKeyDown(KEY_A)) {
                        dir.x += -1.0f;
                    }
                    if (IsKeyDown(KEY_D)) {
                        dir.x += 1.0f;
                    }

                    LocalPlayerMove(map, &map.localPlayer, dir);
                    LocalPlayerLook(map, &map.localPlayer, surfaceMouse);

                    if (shouldShoot) {
                        LocalPlayerShoot(map, &map.localPlayer);
                        shouldShoot = false;
                    }

                    LocalSendStreamData(map);
                    MapUpdate(map, GAME_TICK_TIME);
                }
            }

            map.remotePlayer.tank.pos = Lerp(map.remotePlayer.tank.pos, map.remotePlayer.tank.remotePos, 0.1f);
            map.remotePlayer.tank.rot = LerpAngle(map.remotePlayer.tank.rot, map.remotePlayer.tank.remoteRot, 0.1f);
            map.remotePlayer.tank.turretRot = LerpAngle(map.remotePlayer.tank.turretRot, map.remotePlayer.tank.remoteTurretRot, 0.1f);

            for (i32 i = 0; i < MAX_ENEMIES; i++) {
                if (map.enemies[i].active == false) {
                    continue;
                }

                Enemy & enemy = map.enemies[i];
                enemy.tank.pos = Lerp(enemy.tank.pos, enemy.tank.remotePos, 0.1f);
                enemy.tank.rot = LerpAngle(enemy.tank.rot, enemy.tank.remoteRot, 0.1f);
                enemy.tank.turretRot = LerpAngle(enemy.tank.turretRot, enemy.tank.remoteTurretRot, 0.1f);
            }

            DrawMap(map);
        }
        else if (screen == SCREEN_TYPE_LEVEL_EDITOR) {
            if (editor.mapName.empty()) {
                editor.mapName = "maps/demo.map";
                //MapLoadFile(map, "C:/Projects/Play/maps/demo.map");
                MapLoadFile(map, "maps/demo.map");
            }

            DrawMap(map);
            editor.menuTB = {};
            UIColorsPush(COLOR_SLOT_BACKGROUND, SKYBLUE);
            if (ToolBarButton(editor.menuTB, "New")) {
                const char * file = PlatformFileDialogSave("maps", "Map Files\0*.map\0");
                if (file != nullptr) {
                    printf("Creating map: %s\n", file);
                    MapCreateEditorNew(map, MAP_SIZE_MEDIUM);
                    editor.mapName = file;
                    // Ensure .map extension
                    if (editor.mapName.find(".map") == std::string::npos) {
                        editor.mapName += ".map";
                    }
                    printf("Saving map: %s\n", file);
                    bool saved = MapSaveFile(map, editor.mapName.c_str());
                    if (saved) {
                        SetTempCenterText("New map!", 3.0f);
                    }
                    else {
                        SetTempCenterText("Failed to create new map!", 3.0f);
                    }
                }
            }
            if (ToolBarButton(editor.menuTB, "Open")) {
                const char * file = PlatformFileDialogOpen("maps", "Map Files\0*.map\0");
                if (file != nullptr) {
                    printf("Opening file: %s\n", file);
                    bool loaded = MapLoadFile(map, file);
                    if (loaded) {
                        editor.mapName = file;
                        SetTempCenterText("Opened!", 3.0f);
                        surfaceMapSize = map.size;
                        recreateSufrace = true;
                    }
                    else {
                        SetTempCenterText("Failed to open!", 3.0f);
                    }
                }
            }
            if (ToolBarButton(editor.menuTB, "Save")) {
                if (MapSaveFile(map, editor.mapName.c_str())) {
                    SetTempCenterText("Saved!", 1.0f);
                }
                else {
                    SetTempCenterText("Failed to save!", 1.0f);
                }
            }
            if (ToolBarButton(editor.menuTB, "Main Menu")) {
                screen = SCREEN_TYPE_MAIN_MENU;
            }
            UIColorsPop(COLOR_SLOT_BACKGROUND);

            ToolBarButtonEditorMode(editor, LEVEL_EDITOR_TOOL_MODE_TILE, "Tile");
            ToolBarButtonEditorMode(editor, LEVEL_EDITOR_TOOL_MODE_P1, "P1");
            ToolBarButtonEditorMode(editor, LEVEL_EDITOR_TOOL_MODE_P2, "P2");
            ToolBarButtonEditorMode(editor, LEVEL_EDITOR_TOOL_MODE_BROWN_ENEMY, "Brown Enemy");

            switch (editor.toolMode) {
            case LEVEL_EDITOR_TOOL_MODE_TILE: {
                v2 mousePos = surfaceMouse;
                if (mousePos.x < 0.0f || mousePos.y < 0.0f || mousePos.x >= surfaceWidth || mousePos.y >= surfaceHeight) {
                    break;
                }
                MapTile * tile = MapGetTileAtPos(map, mousePos);
                bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
                if (altDown == false) {
                    if (tile != nullptr && tile->active == false) {
                        MapTile gTile = MapEditorCreateGhostTile(map, mousePos);
                        DrawMapGhostTile(gTile);
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            i32 x = (i32)mousePos.x / map.tileSize;
                            i32 y = (i32)mousePos.y / map.tileSize;
                            MapAddTile(map, x, y);
                        }
                    }
                }
                else {
                    if (tile != nullptr && tile->active == true) {
                        MapTile gTile = MapEditorCreateGhostTile(map, mousePos);
                        DrawMapGhostTile(gTile, Fade(RED, 0.5f));
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            i32 x = (i32)mousePos.x / map.tileSize;
                            i32 y = (i32)mousePos.y / map.tileSize;
                            MapRemoveTile(map, x, y);
                        }
                    }
                }
            } break;
            case LEVEL_EDITOR_TOOL_MODE_P1: {
                if (surfaceMouse.x < 0.0f || surfaceMouse.y < 0.0f || surfaceMouse.x >= surfaceWidth || surfaceMouse.y >= surfaceHeight) {
                    break;
                }
                DoLevelModePlayer(editor, map, map.localPlayer, surfaceMouse);
            } break;
            case LEVEL_EDITOR_TOOL_MODE_P2: {
                if (surfaceMouse.x < 0.0f || surfaceMouse.y < 0.0f || surfaceMouse.x >= surfaceWidth || surfaceMouse.y >= surfaceHeight) {
                    break;
                }
                DoLevelModePlayer(editor, map, map.remotePlayer, surfaceMouse);
            } break;
            case LEVEL_EDITOR_TOOL_MODE_BROWN_ENEMY: {
                if (surfaceMouse.x < 0.0f || surfaceMouse.y < 0.0f || surfaceMouse.x >= surfaceWidth || surfaceMouse.y >= surfaceHeight) {
                    break;
                }

                bool altDown = IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT);
                if (altDown == false) {
                    Tank dummyTank = EnemyCreateTank(surfaceMouse, ENEMY_TYPE_LIGHT_BROWN);
                    Circle c = TankGetColliderAtPos(&dummyTank, surfaceMouse);
                    bool validPlace = true;
                    for (i32 i = 0; i < MAX_MAP_TILES; i++) {
                        MapTile & tile = map.tiles[i];
                        if (tile.active) {
                            CollisionManifold manifold = {};
                            if (CircleVsRect(c, tile.rect, &manifold)) {
                                validPlace = false;
                                break;
                            }
                        }
                    }

                    if (validPlace == true) {
                        DrawTank(surfaceMouse, dummyTank.size, 0.0f, 0.0f, Fade(GetColorForEnemyType(ENEMY_TYPE_LIGHT_BROWN), 0.5f));
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, surfaceMouse);
                        }
                    }
                }
                else {
                    Tank dummyTank = EnemyCreateTank(surfaceMouse, ENEMY_TYPE_LIGHT_BROWN);
                    Circle c1 = TankGetColliderAtPos(&dummyTank, surfaceMouse);
                    bool validDelete = false;
                    i32 deleteIndex = -1;
                    for (i32 i = 0; i < MAX_ENEMIES; i++) {
                        Enemy & enemy = map.enemies[i];
                        if (enemy.active == true) {
                            Circle c2 = TankGetColliderAtPos(&enemy.tank, enemy.tank.pos);
                            CollisionManifold manifold = {};
                            if (CircleVsCircle(c1, c2, &manifold)) {
                                validDelete = true;
                                deleteIndex = i;
                                break;
                            }
                        }
                    }

                    if (validDelete == true) {
                        DrawTank(surfaceMouse, dummyTank.size, 0.0f, 0.0f, Fade(RED, 0.5f));
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            MapDestroyEnemy(map, deleteIndex);
                        }
                    }
                }
            } break;
            }
        }
        else if (screen == SCREEN_TYPE_GAME_OVER) {
            const char * gameOverText = "Game over!!";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), gameOverText, 20, 1);
            Vector2 centerPos = {};
            centerPos.x = surfaceWidth / 2 - textSize.x / 2;
            centerPos.y = surfaceHeight / 2 - textSize.y / 2;
            DrawText(gameOverText, (int)centerPos.x, (int)centerPos.y - 40, 20, BLACK);

            const char * text = "Main Menu";
            if (DrawButtonCenter(surfaceWidth / 2, surfaceHeight / 2, text)) {
                screen = SCREEN_TYPE_MAIN_MENU;
            }
        }

        if (screen == SCREEN_TYPE_GAME) {
            std::string fps = "FPS: " + std::to_string(GetFPS());
            std::string ping = "Ping: " + std::to_string(NetworkGetPing());

            DrawText(fps.c_str(), 10, 10, 20, BLACK);
            DrawText(ping.c_str(), 10, 30, 20, BLACK);
        }

        DrawTempCenterText(surfaceWidth, surfaceHeight);

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

        if (recreateSufrace) {
            UnloadRenderTexture(surface);
            MapSizeGetDimensions(surfaceMapSize, &surfaceWidth, &surfaceHeight);
            surface = LoadRenderTexture(surfaceWidth, surfaceHeight);
            SetTextureFilter(surface.texture, TEXTURE_FILTER_BILINEAR);
            recreateSufrace = false;
        }
    }

    NetoworkDisconnectFromServer();

    CloseWindow();

    return 0;
}
