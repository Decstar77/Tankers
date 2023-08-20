
#include "../shared/shared_ini.h"
#include "../shared/shared_game.h"

#include "game_client.h"
#include "game_local.h"

#include "../vendor/raylib/include/raylib.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <cstring>

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
    DrawTank(player->pos, player->size, player->tankRot, player->turretRot, color);
}

static void DrawEnemy(Enemy * enemy) {
    Color color = LIGHTGRAY;
    switch (enemy->type) {
    case ENEMY_TYPE_LIGHT_BROWN: color = { 196, 164, 132, 255 }; break;
    case ENEMY_TYPE_DARK_BROWN: color = { 128, 64, 0, 255 }; break;
    }

    DrawTank(enemy->pos, enemy->size, enemy->tankRot, enemy->turretRot, color);
}

static bool DrawButton(i32 centerX, i32 centerY, const char * text) {
    Rectangle rect = { (float)centerX - 100 / 2, (float)centerY - 50 / 2, 100, 50 };
    DrawRectangleRec(rect, LIGHTGRAY);

    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    Vector2 textPos = { rect.x + rect.width / 2 - textSize.x / 2, rect.y + rect.height / 2 - textSize.y / 2 };
    DrawTextEx(GetFontDefault(), text, textPos, 20, 1, BLACK);

    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, rect)) {
        DrawRectangleLinesEx(rect, 2, RED);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
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

    Map map = {};
    map.isAuthoritative = false;
    map.width = gameSettings.width;
    map.height = gameSettings.height;

    InitWindow(gameSettings.width, gameSettings.height, "Yes. This is a game you slut!");

    SetWindowPosition(gameSettings.posX, gameSettings.posY);

    bool isMainMenu = true;
    bool isGameOver = false;

    bool debugPauseSim = false;

    SetTargetFPS(60);

    f32 accumulator = 0.0f;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (isMainMenu) {
            // Connect button
            if (NetworkIsConnected() == false) {
                static const char * text = "Connect";
                if (DrawButton(gameSettings.width / 2, gameSettings.height / 2, text)) {
                    if (NetworkConnectToServer("127.0.0.1", 27164) == false) {
                        //if (NetworkConnectToServer(gameSettings.serverIp, 27164) == false) {
                        text = "Connection failed please try again";
                    }
                }
                if (DrawButton(gameSettings.width / 2, gameSettings.height / 2 - 100, "Single Pringle")) {
                    MapStart(map, gameSettings.width, gameSettings.height, true);
                    MapSpawnPlayer(map);
                    MapSpawnPlayer(map);
                    isMainMenu = false;
                }
            }
            else {
                // Show waiting for game text
                const char * text = "Waiting for game, please wait";
                Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
                Vector2 centerPos = {};
                centerPos.x = gameSettings.width / 2 - textSize.x / 2;
                centerPos.y = gameSettings.height / 2 - textSize.y / 2;
                DrawText(text, (int)centerPos.x, (int)centerPos.y, 20, BLACK);
            }
        }

        if (NetworkIsConnected()) {
            GamePacket packet = {};
            while (NetworkPoll(packet)) {
                switch (packet.type) {
                case GAME_PACKET_TYPE_MAP_START: {
                    MapStart(map, gameSettings.width, gameSettings.height, false);
                    map.localPlayer = packet.mapStart.localPlayer;
                    map.remotePlayer = packet.mapStart.remotePlayer;

                    printf("Map start, local player number %d\n", map.localPlayer.playerNumber);
                    isMainMenu = false;
                } break;
                case GAME_PACKET_TYPE_MAP_SHOT_FIRED: {
                    MapSpawnBullet(map, packet.shotFired.pos, packet.shotFired.dir, BULLET_TYPE_NORMAL);
                } break;
                case GAME_PACKET_TYPE_MAP_PLAYER_STREAM_DATA: {
                    map.remotePlayer.remotePos = packet.playerStreamData.pos;
                    map.remotePlayer.remoteTankRot = packet.playerStreamData.tankRot;
                    map.remotePlayer.remoteTurretRot = packet.playerStreamData.turretRot;
                } break;
                case GAME_PACKET_TYPE_MAP_GAME_OVER: {
                    printf("Game over, reason: %s\n", MapGameOverReasonToString(packet.gameOver.reason));
                    isGameOver = true;
                } break;
                case GAME_PACKET_TYPE_MAP_ENTITY_STREAM_DATA: {
                    for (i32 i = 0; i < packet.entityStreamData.entityCount; i++) {
                        map.enemies[i].remotePos = packet.entityStreamData.pos[i];
                        map.enemies[i].remoteTankRot = packet.entityStreamData.tankRot[i];
                        map.enemies[i].remoteTurretRot = packet.entityStreamData.turretRot[i];
                    }
                } break;
                }
            }
        }

        if (isMainMenu == false && isGameOver == false) {
            static bool shouldShoot = false;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                shouldShoot = true;
            }

            if (IsKeyPressed(KEY_F1)) {
                debugPauseSim = !debugPauseSim;
            }

            bool debugStepOne = false;
            if (IsKeyPressed(KEY_F2)) {
                debugStepOne = true;
            }

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

            map.remotePlayer.pos = Lerp(map.remotePlayer.pos, map.remotePlayer.remotePos, 0.1f);
            map.remotePlayer.tankRot = LerpAngle(map.remotePlayer.tankRot, map.remotePlayer.remoteTankRot, 0.1f);
            map.remotePlayer.turretRot = LerpAngle(map.remotePlayer.turretRot, map.remotePlayer.remoteTurretRot, 0.1f);

            for (i32 i = 0; i < MAX_ENEMIES; i++) {
                if (map.enemies[i].active == false) {
                    continue;
                }

                Enemy & enemy = map.enemies[i];
                enemy.pos = Lerp(enemy.pos, enemy.remotePos, 0.1f);
                enemy.tankRot = LerpAngle(enemy.tankRot, enemy.remoteTankRot, 0.1f);
                enemy.turretRot = LerpAngle(enemy.turretRot, enemy.remoteTurretRot, 0.1f);
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

        if (isGameOver) {
            const char * text = "Game over restart the game sloot!";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
            Vector2 centerPos = {};
            centerPos.x = gameSettings.width / 2 - textSize.x / 2;
            centerPos.y = gameSettings.height / 2 - textSize.y / 2;
            DrawText(text, (int)centerPos.x, (int)centerPos.y, 20, BLACK);
        }

        std::string fps = "FPS: " + std::to_string(GetFPS());
        std::string ping = "Ping: " + std::to_string(NetworkGetPing());

        DrawText(fps.c_str(), 10, 10, 20, BLACK);
        DrawText(ping.c_str(), 10, 30, 20, BLACK);

        //DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    NetoworkDisconnectFromServer();

    CloseWindow();

    return 0;
}
