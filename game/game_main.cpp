
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

static void DrawPlayer(Player * player) {
    Color color = player->playerNumber == 1 ? RED : BLUE;
    DrawCircle((int)player->pos.x, (int)player->pos.y, player->size, color);
    f32 gunRad = 15;
    Vector2 start = { player->pos.x, player->pos.y };
    Vector2 end = { player->pos.x + gunRad * cosf(player->rot), player->pos.y + gunRad * sinf(player->rot) };
    DrawLineEx(start, end, 2.0f, BLACK);
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

    const char * iniPath = "bin/player_one.ini";
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
    map.width = gameSettings.width;
    map.height = gameSettings.height;

    InitWindow(gameSettings.width, gameSettings.height, "Yes. This is a game you slut!");

    SetWindowPosition(gameSettings.posX, gameSettings.posY);

    bool isMainMenu = true;
    bool isGameOver = false;

    SetTargetFPS(60);

    i32 tickRate = 60;
    f32 tickTime = 1.0f / (f32)tickRate;
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
                    ZeroStruct(map);
                    map.width = gameSettings.width;
                    map.height = gameSettings.height;
                    map.localPlayer = packet.mapStart.localPlayer;
                    map.remotePlayer = packet.mapStart.remotePlayer;
                    printf("Map start, local player number %d\n", map.localPlayer.playerNumber);
                    isMainMenu = false;
                } break;
                case GAME_PACKET_TYPE_MAP_PLAYER_SHOOT: {
                    MapSpawnBullet(map, packet.playerShoot.pos, packet.playerShoot.dir);
                } break;
                case GAME_PACKET_TYPE_MAP_STREAM_DATA: {
                    map.remotePlayer.remotePos = packet.streamData.pos;
                    map.remotePlayer.remoteRot = packet.streamData.rot;
                } break;
                case GAME_PACKET_TYPE_MAP_GAME_OVER: {
                    printf("Game over, reason: %s\n", MapGameOverReasonToString(packet.gameOver.reason));
                    isGameOver = true;
                } break;
                }
            }
        }

        if (isMainMenu == false && isGameOver == false) {
            float deltaTime = GetFrameTime();
            accumulator += deltaTime;

            while (accumulator >= tickTime) {
                accumulator -= tickTime;

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

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    LocalPlayerShoot(map, &map.localPlayer);
                }

                GamePacket packet = {};
                GamePacketCreateStreamData(packet, map);
                NetworkSendPacket(packet, false);

                MapUpdate(map, tickTime);
            }

            map.remotePlayer.pos = Lerp(map.remotePlayer.pos, map.remotePlayer.remotePos, 0.1f);
            map.remotePlayer.rot = Lerp(map.remotePlayer.rot, map.remotePlayer.remoteRot, 0.1f);

            DrawPlayer(&map.localPlayer);
            DrawPlayer(&map.remotePlayer);

            // Draw bullets
            for (i32 i = 0; i < map.bulletCount; ++i) {
                Bullet & bullet = map.bullets[i];
                DrawCircle((int)bullet.pos.x, (int)bullet.pos.y, 5, ORANGE);
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
