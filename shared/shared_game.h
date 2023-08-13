#pragma once

#include "shared_math.h"

struct Bullet {
    v2 pos;
    v2 dir;
};

struct Player {
    bool active;
    i32 playerNumber;
    v2 remotePos;
    v2 pos;
    f32 remoteRot;
    f32 rot;
    f32 fireCooldown;
    f32 size;
};

#define MAX_BULLETS 256

struct Map {
    i32 width;
    i32 height;

    i32 bulletCount;
    Bullet bullets[MAX_BULLETS];

    union {
        Player players[2];
        struct {
            Player localPlayer;
            Player remotePlayer;
        };
    };
};

Player * MapSpawnPlayer(Map & map);
Bullet * MapSpawnBullet(Map & map, v2 pos, v2 dir);
void MapUpdate(Map & map, f32 dt);

enum MapGameOverReason {
    MAP_GAME_OVER_REASON_INVALID = 0,
    MAP_GAME_OVER_REASON_PLAYER_DISCONNECTED,
    MAP_GAME_OVER_REASON_PLAYER_WON,
    MAP_GAME_OVER_REASON_PLAYER_LOST,
};

inline const char * MapGameOverReasonToString(MapGameOverReason reason) {
    switch (reason) {
    case MAP_GAME_OVER_REASON_PLAYER_DISCONNECTED: return "Player disconnected";
    case MAP_GAME_OVER_REASON_PLAYER_WON: return "Player won";
    case MAP_GAME_OVER_REASON_PLAYER_LOST: return "Player lost";
    default: return "Unknown";
    }
}

enum GamePacketType {
    GAME_PACKET_TYPE_INVALID = 0,
    GAME_PACKET_TYPE_MAP_START,
    GAME_PACKET_TYPE_MAP_STREAM_DATA,
    GAME_PACKET_TYPE_MAP_PLAYER_SHOOT,
    GAME_PACKET_TYPE_MAP_GAME_OVER,
};

struct GamePacket {
    GamePacketType type;
    union {
        struct {
            Player localPlayer;
            Player remotePlayer;
        } mapStart;
        struct {
            i32 playerNumber;
            v2 pos;
            f32 rot;
        } streamData;
        struct {
            v2 pos;
            v2 dir;
        } playerShoot;
        struct {
            MapGameOverReason reason;
        } gameOver;
    };
};

void GamePacketCreateStreamData(GamePacket & packet, Map & map);
