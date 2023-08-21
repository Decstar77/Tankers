#pragma once

#include "shared_math.h"

enum BulletType {
    BULLET_TYPE_INVALID = 0,
    BULLET_TYPE_NORMAL,
    BULLET_TYPE_ROCKET,
    BULLET_TYPE_MINE,
    BULLET_TYPE_COUNT,
};

struct Bullet {
    bool active;
    BulletType type;
    bool bounced;
    v2 pos;
    v2 dir;
};

struct Tank {
    v2 startingPos;
    v2 pos;
    v2 remotePos;
    f32 rot;
    f32 remoteRot;
    f32 turretRot;
    f32 remoteTurretRot;
    f32 size;
};

struct Player {
    bool active;
    i32 playerNumber;
    f32 fireCooldown;
    Tank tank;
};

/*
1. Light Brown: These tanks are stationary and only shoot one bullet at a time. These are the easiest to take care of.
2. Dark Brown: These tanks move around and shoot one bullet at a time. They can try to sneak up on you in the later stages if you're not careful.
3. Blue: These tanks shoot rocket missiles. They can be a huge pain since I think these are one of the main ones that dodge my bullets.
4. Yellow: These tanks plant three mines at a time. Some mines can appear offscreen and can explode you by surprise. Be on the lookout by shooting straight offscreen.
5. Pink/Red: These tanks shoot up to three bullets at a time. They're not that bad, but can be a pain if there is a number of them in the later stages.
6. Green: These tanks are stationary, but they're rocket missiles ricochet three times. These tanks will cause you anxiety because they are crazy accurate!
6. Purple: These tanks increase their speed and can be hard to hit. In my opinion, these are the hardest slow bullet tanks to defeat.
7. White: These tanks turn invisible. They're not as bad as the purple ones as you can not only see their tracks, but they're pretty slow for an easier target.
8. Black: These tanks appear in stage 50 and beyond. They are the most difficult tank out of them all with their fast movements, quick reflexes, and rocket missiles!
*/

enum EnemyType {
    ENEMY_TYPE_INVALID = 0,
    ENEMY_TYPE_LIGHT_BROWN,
    ENEMY_TYPE_DARK_BROWN,
    ENEMY_TYPE_COUNT,
};

struct Enemy {
    bool active;
    EnemyType type;
    f32 fireCooldown;
    i32 lastSend;
    Tank tank;
};

struct MapTile {
    v2 pos;
    f32 size;
    i32 flatIndex;
    i32 xIndex;
    i32 yIndex;
    Rect rect;
};

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
            i32 entityCount;
            i32 indices[20];
            v2  pos[20];
            f32 tankRot[20];
            f32 turretRot[20];
        } entityStreamData;
    };
};

enum MapSize {
    MAP_SIZE_INVALID = 0,
    MAP_SIZE_SMALL,
    MAP_SIZE_MEDIUM,
    MAP_SIZE_LARGE,
    MAP_SIZE_COUNT,
};

enum MapVersion {
    MAP_VERSION_INVALID = 0,
    MAP_VERSION_1,
    MAP_VERSION_COUNT,
};

#define MAX_BULLETS 256
#define MAX_ENEMIES 256
#define MAX_MAP_TILES 256
#define MAX_MAP_PACKETS 256

struct Map {
    bool isAuthoritative;
    bool isSinglePlayerMap;
    MapSize size;
    MapVersion version;
    i32 width;
    i32 height;

    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];

    i32 tileSize;
    i32 tileCount;
    i32 tilesHCount;
    i32 tilesVCount;
    MapTile tiles[MAX_MAP_TILES];

    i32 packetCount;
    GamePacket mpPackets[MAX_MAP_PACKETS];

    union {
        Player players[2];
        struct {
            Player localPlayer;
            Player remotePlayer;
        };
    };
};

Tank        PlayerCreateTank(v2 pos);
Circle      PlayerGetCollider(Player * player);

f32         BulletSpeedFromType(BulletType type);
f32         BulletSizeFromType(BulletType type);
Circle      BulletSizeColliderFromType(v2 p, BulletType type);

Tank        EnemyCreateTank(v2 pos, EnemyType type);

i32         MapGetEnemyCount(Map & map);

void            MapSizeGetDimensions(MapSize size, i32 * width, i32 * height);
const char *    MapSizeToString(MapSize size);
MapSize         MapSizeFromString(const char * str);

void        MapStart(Map & map, bool isAuthoritative);

Player *    MapSpawnPlayer(Map & map);
Bullet *    MapSpawnBullet(Map & map, v2 pos, v2 dir, BulletType type);
Enemy *     MapSpawnEnemy(Map & map, EnemyType type, v2 pos);
void        MapDestroyEnemy(Map & map, i32 index);
MapTile *   MapGetTileAtPos(Map & map, v2 pos);

void         MapClearPackets(Map & map);
GamePacket * MapAddGamePacket(Map & map);

void        MapUpdate(Map & map, f32 dt);

bool        MapSaveFile(Map & map, const char * filename);
bool        MapLoadFile(Map & map, const char * filename);

// Setting values
constexpr i32 GAME_TICKS_PER_SECOND = 30;
constexpr i32 GAME_MAX_BYTES_PER_MS = 30;

// Calculated values
constexpr f32 GAME_TICK_TIME = 1.0f / (f32)GAME_TICKS_PER_SECOND;
constexpr f32 GAME_TICKS_PER_MS = (f32)GAME_TICKS_PER_SECOND / 1000.0f;
constexpr i32 GAME_PACKET_SIZE_BYTES = sizeof(GamePacket);
constexpr f32 GAME_MS_PER_TICK = 1.0f / GAME_TICKS_PER_MS;
constexpr f32 GAME_BYTES_PER_TICK = GAME_MS_PER_TICK * GAME_MAX_BYTES_PER_MS;

// Checks
static_assert(GAME_PACKET_SIZE_BYTES <= GAME_BYTES_PER_TICK, "Game packet size is too large");
