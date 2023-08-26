#include "shared_game.h"

#include <memory>
#include <math.h>
#include <cstring> // For ubuntu

Tank PlayerCreateTank(v2 pos) {
    Tank tank = {};
    tank.startingPos = pos;
    tank.pos = pos;
    tank.rot = 0.0f;
    tank.turretRot = 0.0f;
    tank.size = 25.0f;
    return tank;
}

Circle PlayerGetCollider(Player * player) {
    return { player->tank.pos, player->tank.size / 2.0f };
}

MapTile MapEditorCreateGhostTile(Map & map, v2 pos) {
    i32 x = (i32)(pos.x / map.tileSize);
    i32 y = (i32)(pos.y / map.tileSize);

    pos = { (f32)x * map.tileSize, (f32)y * map.tileSize };

    MapTile tile = {};
    tile.pos = pos;
    tile.size = (f32)map.tileSize;
    tile.rect = { tile.pos, tile.pos + v2{ tile.size, tile.size } };
    return tile;
}

void MapAddTile(Map & map, i32 x, i32 y) {
    if (x < 0 || x >= map.tilesHCount || y < 0 || y >= map.tilesVCount) {
        return;
    }

    i32 flatIndex = y * map.tilesHCount + x;

    MapTile & tile = map.tiles[flatIndex];
    tile.active = true;
    tile.xIndex = x;
    tile.yIndex = y;
    tile.flatIndex = y * map.tilesHCount + x;
    tile.pos = { (f32)x * map.tileSize, (f32)y * map.tileSize };
    tile.size = (f32)map.tileSize;
    tile.rect = { tile.pos, tile.pos + v2{ tile.size, tile.size } };
}

void MapSizeGetDimensions(MapSize size, i32 * width, i32 * height) {
    // @TODO: Try some of these sizes
    /* List of 16:9 resolutions/sizes
        640x360
        1024x576
        1152x648
        1280x720
        1366x768
        1600x900
        1920x1080
        2560x1440
        3840x2160
    */
    switch (size) {
    case MAP_SIZE_SMALL: {
        *width = 640;
        *height = 360;
    } break;
    case MAP_SIZE_MEDIUM: {
        *width = 1200;
        *height = 800;
    } break;
    case MAP_SIZE_LARGE: {
        *width = 1600;
        *height = 1200;
    } break;
    default: {
        *width = 0;
        *height = 0;
        Assert(false);
    } break;
    }
}

const char * MapSizeToString(MapSize size) {
    switch (size) {
    case MAP_SIZE_SMALL:    return Stringify(MAP_SIZE_SMALL);
    case MAP_SIZE_MEDIUM:   return Stringify(MAP_SIZE_MEDIUM);
    case MAP_SIZE_LARGE:    return Stringify(MAP_SIZE_LARGE);
    default: {
        return nullptr;
        Assert(false);
    } break;
    }
    return nullptr;
}

MapSize MapSizeFromString(const char * str) {
    if (strcmp(str, Stringify(MAP_SIZE_SMALL)) == 0) {
        return MAP_SIZE_SMALL;
    }
    else if (strcmp(str, Stringify(MAP_SIZE_MEDIUM)) == 0) {
        return MAP_SIZE_MEDIUM;
    }
    else if (strcmp(str, Stringify(MAP_SIZE_LARGE)) == 0) {
        return MAP_SIZE_LARGE;
    }
    else {
        Assert(false);
        return MAP_SIZE_SMALL;
    }
}

void MapStart(Map & map, bool isAuthoritative) {
    Assert(map.version != MAP_VERSION_INVALID);
    Assert(map.size != MAP_SIZE_INVALID);
    map.isAuthoritative = isAuthoritative;

    // MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, v2{ 200.0f, 500.0f });
    // MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, v2{ 600.0f, 500.0f });

    // MapAddTile(map, 0, 0);
    // for (i32 x = 2; x < 14; x++) {
    //     MapAddTile(map, x, 5);
    // }
}

Player * MapSpawnPlayer(Map & map) {
    if (map.localPlayer.active == false) {
        map.localPlayer.active = true;
        map.localPlayer.playerNumber = 1;
        map.localPlayer.tank = PlayerCreateTank({ 100.0f, 100.0f });
        map.localPlayer.fireCooldown = 0.0f;
        return &map.localPlayer;
    }

    if (map.remotePlayer.active == false) {
        map.remotePlayer.active = true;
        map.remotePlayer.playerNumber = 2;
        map.remotePlayer.tank = PlayerCreateTank({ 200.0f, 100.0f });
        map.remotePlayer.fireCooldown = 0.0f;
        return &map.remotePlayer;
    }

    return nullptr;
}

Bullet * MapSpawnBullet(Map & map, v2 pos, v2 dir, BulletType type) {
    for (i32 i = 0; i < MAX_BULLETS; i++) {
        Bullet & bullet = map.bullets[i];
        if (bullet.active == false) {
            ZeroStruct(bullet);
            bullet.active = true;
            bullet.type = type;
            bullet.bounced = false;
            bullet.pos = pos;
            bullet.dir = dir;
            return &bullet;
        }
    }

    return nullptr;
}

Enemy * MapSpawnEnemy(Map & map, EnemyType type, v2 pos) {
    for (i32 i = 0; i < MAX_ENEMIES; i++) {
        Enemy & enemy = map.enemies[i];
        if (enemy.active == false) {
            ZeroStruct(enemy);
            enemy.active = true;
            enemy.type = type;
            enemy.fireCooldown = 0.0f;
            enemy.tank = EnemyCreateTank(pos, type);
            return &enemy;
        }
    }
    return nullptr;
}

MapTile * MapGetTileAtPos(Map & map, v2 pos) {
    i32 x = (i32)(pos.x / map.tileSize);
    i32 y = (i32)(pos.y / map.tileSize);
    i32 flatIndex = y * map.tilesHCount + x;
    if (flatIndex < 0 || flatIndex >= MAX_MAP_TILES) {
        return nullptr;
    }
    return &map.tiles[flatIndex];
}

void MapDestroyEnemy(Map & map, i32 index) {
    if (index < 0 || index >= MAX_ENEMIES) {
        return;
    }

    map.enemies[index].active = false;
}

void MapClearPackets(Map & map) {
    map.packetCount = 0;
}

GamePacket * MapAddGamePacket(Map & map) {
    if (map.packetCount >= MAX_MAP_PACKETS) {
        return nullptr;
    }

    GamePacket & packet = map.mpPackets[map.packetCount];
    map.packetCount++;
    ZeroStruct(packet);
    return &packet;
}

f32 BulletSpeedFromType(BulletType type) {
    switch (type) {
    case BULLET_TYPE_NORMAL: return 10.0f;
    case BULLET_TYPE_ROCKET: return 15.0f;
    default: return 0.0f;
    }

    return 0.0f;
}

f32 BulletSizeFromType(BulletType type) {
    switch (type) {
    case BULLET_TYPE_NORMAL: return 7.0f;
    case BULLET_TYPE_ROCKET: return 10.0f;
    default: return 0.0f;
    }

    return 0.0f;
}

i32 MapGetEnemyCount(Map & map) {
    i32 count = 0;
    for (i32 i = 0; i < MAX_ENEMIES; i++) {
        if (map.enemies[i].active) {
            count++;
        }
    }
    return count;
}

Circle BulletSizeColliderFromType(v2 p, BulletType type) {
    switch (type) {
    case BULLET_TYPE_NORMAL: return { p, BulletSizeFromType(type) / 2.0f }; // TODO: This is a hack
    case BULLET_TYPE_ROCKET: return { p, BulletSizeFromType(type) };
    default: return {};
    }
    return {};
}

Tank EnemyCreateTank(v2 pos, EnemyType type) {
    Tank tank = {};
    tank.startingPos = pos;
    tank.pos = pos;
    tank.rot = 0.0f;
    tank.turretRot = 0.0f;
    tank.size = 25.0f;
    tank.remotePos = pos;
    tank.remoteRot = 0.0f;
    tank.remoteTurretRot = 0.0f;
    return tank;
}

void MapUpdate(Map & map, f32 dt) {
    MapClearPackets(map);

    // Update player fire cooldown
    map.localPlayer.fireCooldown -= dt;
    map.remotePlayer.fireCooldown -= dt;

    if (map.isAuthoritative) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            Enemy & enemy = map.enemies[i];
            if (enemy.active) {
                enemy.fireCooldown -= GAME_TICK_TIME;

                v2 toPlayer = map.localPlayer.tank.pos - enemy.tank.pos;
                enemy.tank.turretRot = atan2f(toPlayer.y, toPlayer.x);

                if (enemy.fireCooldown <= 0.0f) {
                    enemy.fireCooldown = 1.5f;
                    v2 dir = { cosf(enemy.tank.turretRot), sinf(enemy.tank.turretRot) };
                    v2 spawnPos = enemy.tank.pos + dir * 21.0f;
                    MapSpawnBullet(map, spawnPos, dir, BULLET_TYPE_NORMAL);

                    GamePacket * packet = MapAddGamePacket(map);
                    packet->type = GAME_PACKET_TYPE_MAP_SHOT_FIRED;
                    packet->shotFired.pos = spawnPos;
                    packet->shotFired.dir = dir;
                }
            }
        }
    }

    for (i32 currentBulletIndex = 0; currentBulletIndex < MAX_BULLETS; currentBulletIndex++) {
        Bullet & bullet = map.bullets[currentBulletIndex];
        const f32 bulletSpeed = BulletSpeedFromType(bullet.type);
        const f32 bulletSize = BulletSizeFromType(bullet.type);
        if (bullet.active) {
            v2 bv = bullet.dir * bulletSpeed;
            Circle bulletCollider = BulletSizeColliderFromType(bullet.pos, bullet.type);

            bool hitBullet = false;
            for (i32 otherBulletIndex = 0; otherBulletIndex < MAX_BULLETS; otherBulletIndex++) {
                Bullet & otherBullet = map.bullets[otherBulletIndex];
                if (otherBullet.active && otherBulletIndex != currentBulletIndex) {
                    Circle otherBulletCollider = BulletSizeColliderFromType(otherBullet.pos, otherBullet.type);
                    v2 obv = otherBullet.dir * bulletSpeed;
                    SweepResult sweep = {};
                    if (SweepCircleVsCircle(bulletCollider, bv, otherBulletCollider, obv, &sweep)) {
                        bullet.active = false;
                        otherBullet.active = false;
                        hitBullet = true;
                    }
                }
            }

            if (hitBullet) {
                continue;
            }

            for (i32 tileIndex = 0; tileIndex < MAX_MAP_TILES; tileIndex++) {
                MapTile & tile = map.tiles[tileIndex];
                if (tile.active == false) {
                    continue;
                }

                SweepResult sweep;
                if (SweepCircleVsRect(bulletCollider, bv, tile.rect, {}, &sweep)) {
                    if (bullet.bounced == false) {
                        bullet.bounced = true;
                        bullet.pos = bullet.pos + bv * sweep.t;
                        bullet.dir = Reflect(bullet.dir, sweep.normal);
                        bv = bullet.dir * bulletSpeed;
                    }
                    else {
                        bullet.active = false;
                    }
                }
            }

            bullet.pos = bullet.pos + bv;
            if (bullet.pos.x < 0.0f || bullet.pos.x > map.width || bullet.pos.y < 0.0f || bullet.pos.y > map.height) {
                if (bullet.bounced == false) {
                    bullet.bounced = true;
                    if (bullet.pos.x < 0.0f || bullet.pos.x > map.width) {
                        bullet.dir.x = -bullet.dir.x;
                    }
                    if (bullet.pos.y < 0.0f || bullet.pos.y > map.height) {
                        bullet.dir.y = -bullet.dir.y;
                    }
                }
                else {
                    bullet.active = false;
                }
            }

            for (i32 enemyIndex = 0; enemyIndex < MAX_ENEMIES; enemyIndex++) {
                Enemy & enemy = map.enemies[enemyIndex];
                if (enemy.active) {
                    Circle enemyCollider = { enemy.tank.pos, enemy.tank.size / 2.0f };
                    if (CircleVsCircle(bulletCollider, enemyCollider)) {
                        bullet.active = false;
                        enemy.active = false;
                    }
                }
            }
        }
    }
}

#include "../vendor/json/json.hpp"
#include <fstream>

bool MapSaveFile(Map & map, const char * filename) {
    std::ofstream ss;
    ss.open(filename, std::ios::out);

    if (!ss.is_open()) {
        printf("Failed to open file: %s\n", filename);
        return false;
    }

    nlohmann::json j;

    j["MapVersion"] = map.version;
    j["Size"] = MapSizeToString(map.size);
    j["SinglePlayerMap"] = map.isSinglePlayerMap;

    j["Player1"] = { map.localPlayer.tank.pos.x, map.localPlayer.tank.pos.y };
    j["Player2"] = { map.remotePlayer.tank.pos.x, map.remotePlayer.tank.pos.y };

    for (i32 i = 0; i < MAX_ENEMIES; i++) {
        Enemy & enemy = map.enemies[i];
        if (enemy.active) {
            j["Enemies"].push_back({ enemy.type, enemy.tank.pos.x, enemy.tank.pos.y });
        }
    }

    for (i32 i = 0; i < MAX_MAP_TILES; i++) {
        MapTile & tile = map.tiles[i];
        if (tile.active) {
            j["Tiles"].push_back({ tile.xIndex, tile.yIndex });
        }
    }

    ss << j.dump(4);

    ss.close();

    return true;
}

bool MapLoadFile(Map & map, const char * filename) {
    ZeroStruct(map);

    std::ifstream ss;
    ss.open(filename, std::ios::in);

    if (!ss.is_open()) {
        printf("Failed to open file: %s\n", filename);
        return false;
    }

    nlohmann::json j;
    ss >> j;

    MapVersion version = (MapVersion)j["MapVersion"];
    if (version != MAP_VERSION_1) {
        printf("Map version not supported: %d\n", version);
        return false;
    }

    map.version = version;

    std::string sizeStr = j["Size"];
    map.size = MapSizeFromString(sizeStr.c_str());
    map.isSinglePlayerMap = j["SinglePlayerMap"];

    map.localPlayer.tank.pos.x = j["Player1"][0];
    map.localPlayer.tank.pos.y = j["Player1"][1];

    map.remotePlayer.tank.pos.x = j["Player2"][0];
    map.remotePlayer.tank.pos.y = j["Player2"][1];

    for (auto & enemy : j["Enemies"]) {
        EnemyType type = (EnemyType)enemy[0];
        v2 pos = { enemy[1], enemy[2] };
        MapSpawnEnemy(map, type, pos);
    }

    MapSizeGetDimensions(map.size, &map.width, &map.height);
    map.tileSize = 50;
    map.tilesHCount = map.width / map.tileSize;
    map.tilesVCount = map.height / map.tileSize;
    for (auto & tile : j["Tiles"]) {
        i32 x = tile[0];
        i32 y = tile[1];
        MapAddTile(map, x, y);
    }

    return true;
}
