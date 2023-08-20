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

static void DrawEnemy(Enemy * enemy) {
    Color color = LIGHTGRAY;
    switch (enemy->type) {
    case ENEMY_TYPE_LIGHT_BROWN: color = { 196, 164, 132, 255 }; break;
    case ENEMY_TYPE_DARK_BROWN: color = { 128, 64, 0, 255 }; break;
    }

    DrawTank(enemy->tank.pos, enemy->tank.size, enemy->tank.rot, enemy->tank.turretRot, color);
}

static bool DrawButton(i32 centerX, i32 centerY, const char * text) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
    Rectangle rect = { (float)centerX - rectSize.x / 2, (float)centerY - rectSize.y / 2, rectSize.x, rectSize.y };
    DrawRectangleRec(rect, LIGHTGRAY);


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

enum ScreenType {
    SCREEN_TYPE_MAIN_MENU,
    SCREEN_TYPE_GAME,
    SCREEN_TYPE_GAME_OVER,
};

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

    i32 surfaceWidth = 1280;
    i32 surfaceHeight = 720;

    RenderTexture2D surface = LoadRenderTexture(surfaceWidth, surfaceHeight);
    SetTextureFilter(surface.texture, TEXTURE_FILTER_BILINEAR);

    Map map = {};
    ScreenType screen = SCREEN_TYPE_MAIN_MENU;

    bool debugPauseSim = false;

    SetTargetFPS(60);

    Camera2D camera = {};
    camera.zoom = 1.0f;

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
                if (DrawButton(surfaceWidth / 2, surfaceHeight / 2, text)) {
                    if (NetworkConnectToServer("127.0.0.1", 27164) == false) {
                        //if (NetworkConnectToServer(gameSettings.serverIp, 27164) == false) {
                        text = "Connection failed please try again";
                    }
                }
                if (DrawButton(surfaceWidth / 2, surfaceHeight / 2 - 100, "Single Pringle")) {
                    MapStart(map, surfaceWidth, surfaceHeight, true);
                    MapSpawnPlayer(map);
                    MapSpawnPlayer(map);
                    screen = SCREEN_TYPE_GAME;
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
                    MapStart(map, surfaceWidth, surfaceHeight, false);
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

        BeginMode2D(camera);

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
                    bool moved = false;
                    if (IsKeyDown(KEY_W)) {
                        dir.y += -1.0f;
                        moved = true;
                    }
                    if (IsKeyDown(KEY_S)) {
                        dir.y += 1.0f;
                        moved = true;
                    }
                    if (IsKeyDown(KEY_A)) {
                        dir.x += -1.0f;
                        moved = true;
                    }
                    if (IsKeyDown(KEY_D)) {
                        dir.x += 1.0f;
                        moved = true;
                    }

                    if (moved) {
                        LocalPlayerMove(map, &map.localPlayer, dir);
                    }
                    Vector2 p = GetMousePosition();
                    LocalPlayerLook(map, &map.localPlayer, { p.x, p.y });

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

            DrawPlayer(&map.localPlayer);
            DrawPlayer(&map.remotePlayer);

            // DrawEnemies
            for (i32 i = 0; i < MAX_ENEMIES; i++) {
                if (map.enemies[i].active == false) {
                    continue;
                }

                DrawEnemy(&map.enemies[i]);
            }

            for (i32 i = 0; i < map.tileCount; i++) {
                MapTile & tile = map.tiles[i];
                Rect r = tile.rect;
                Rectangle rect = { r.min.x, r.min.y, r.max.x - r.min.x, r.max.y - r.min.y };
                DrawRectangleRec(rect, DARKGRAY);
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

        EndMode2D();

        if (screen == SCREEN_TYPE_GAME_OVER) {
            const char * gameOverText = "Game over!!";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), gameOverText, 20, 1);
            Vector2 centerPos = {};
            centerPos.x = surfaceWidth / 2 - textSize.x / 2;
            centerPos.y = surfaceHeight / 2 - textSize.y / 2;
            DrawText(gameOverText, (int)centerPos.x, (int)centerPos.y - 40, 20, BLACK);

            const char * text = "Main Menu";
            if (DrawButton(surfaceWidth / 2, surfaceHeight / 2, text)) {
                screen = SCREEN_TYPE_MAIN_MENU;
            }
        }

        std::string fps = "FPS: " + std::to_string(GetFPS());
        std::string ping = "Ping: " + std::to_string(NetworkGetPing());

        DrawText(fps.c_str(), 10, 10, 20, BLACK);
        DrawText(ping.c_str(), 10, 30, 20, BLACK);

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
