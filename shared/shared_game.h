#pragma once

#include "shared_math.h"

struct Bullet {
    bool active;
    bool bounced;
    v2 pos;
    v2 dir;
};

struct Player {
    bool active;
    i32 playerNumber;
    v2 remotePos;
    v2 pos;
    f32 tankRot;
    f32 remoteTankRot;
    f32 turretRot;
    f32 remoteTurretRot;
    f32 fireCooldown;
    f32 size;
};

enum EnemyType {
    ENEMY_TYPE_INVALID = 0,
    ENEMY_TYPE_TURRET,
    ENEMY_TYPE_COUNT,
};

struct Enemy {
    EnemyType type;
    v2 pos;
    f32 rot;
    f32 fireCooldown;
    f32 size;
};

#define MAX_BULLETS 256
#define MAX_ENEMIES 256

struct Map {
    i32 width;
    i32 height;

    Bullet bullets[MAX_BULLETS];

    i32 enemyCount;
    Enemy enemies[MAX_ENEMIES];

    union {
        Player players[2];
        struct {
            Player localPlayer;
            Player remotePlayer;
        };
    };
};

Player *    MapSpawnPlayer(Map & map);
Bullet *    MapSpawnBullet(Map & map, v2 pos, v2 dir);
Enemy *     MapSpawnEnemy(Map & map, EnemyType type, v2 pos);

void        MapUpdate(Map & map, f32 dt);

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
    GAME_PACKET_TYPE_MAP_PLAYER_STREAM_DATA,
    GAME_PACKET_TYPE_MAP_SHOT_FIRED,
    GAME_PACKET_TYPE_MAP_GAME_OVER,
    GAME_PACKET_TYPE_MAP_ENTITY_STREAM_DATA,
};

struct GamePacket {
    GamePacketType type;
    union {
        struct {
            Player localPlayer;
            Player remotePlayer;
        } mapStart;
        struct {
            v2 pos;
            f32 tankRot;
            f32 turretRot;
        } playerStreamData;
        struct {
            v2 pos;
            v2 dir;
        } shotFired;
        struct {
            MapGameOverReason reason;
        } gameOver;
        struct {
            //i32 entityCount;
            //i32 indices[20];
            //v2  pos[20];
            //f32 rot[20];
        } entityStreamData;
    };
};

// Setting values
constexpr i32 GAME_TICKS_PER_SECOND = 60;
constexpr i32 GAME_MAX_BYTES_PER_MS = 30; 

// Calculated values
constexpr f32 GAME_TICKS_PER_MS = (f32)GAME_TICKS_PER_SECOND / 1000.0f;
constexpr i32 GAME_PACKET_SIZE_BYTES = sizeof(GamePacket);
constexpr f32 GAME_MS_PER_TICK = 1.0f / GAME_TICKS_PER_MS;
constexpr f32 GAME_BYTES_PER_TICK = GAME_MS_PER_TICK * GAME_MAX_BYTES_PER_MS;

// Checks
static_assert(GAME_PACKET_SIZE_BYTES <= GAME_BYTES_PER_TICK , "Game packet size is too large");
