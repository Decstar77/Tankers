#include "shared_game.h"

#include <memory>
#include <math.h>
#include <cstring> // For ubuntu

Tank PlayerCreateTank(v2 pos) {
    Tank tank = {};
    tank.pos = pos;
    tank.rot = 0.0f;
    tank.turretRot = 0.0f;
    tank.size = 25.0f;
    return tank;
}

Circle PlayerGetCollider(Player * player) {
    return { player->tank.pos, player->tank.size / 2.0f };
}

static void MapAddTile(Map & map, i32 x, i32 y) {
    if (map.tileCount >= MAX_MAP_TILES) {
        return;
    }
    if (x < 0 || x >= map.tilesHCount || y < 0 || y >= map.tilesVCount) {
        return;
    }

    MapTile & tile = map.tiles[map.tileCount++];
    tile.xIndex = x;
    tile.yIndex = y;
    tile.flatIndex = y * map.tilesHCount + x;
    tile.pos = { (f32)x * map.tileSize, (f32)y * map.tileSize };
    tile.size = (f32)map.tileSize;
    tile.rect = { tile.pos, tile.pos + v2{ tile.size, tile.size } };
}

void MapStart(Map & map, i32 mapWidth, i32 mapHeight, bool isAuthoritative) {
    ZeroStruct(map);
    map.width = mapWidth;
    map.height = mapHeight;
    map.isAuthoritative = isAuthoritative;

    MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, v2{ 200.0f, 500.0f });
    MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, v2{ 600.0f, 500.0f });

    map.tileSize = 50;
    map.tilesHCount = map.width / map.tileSize;
    map.tilesVCount = map.height / map.tileSize;

    MapAddTile(map, 0, 0);

    for (i32 x = 2; x < 14; x++) {
        MapAddTile(map, x, 5);
    }
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
    if (flatIndex < 0 || flatIndex >= map.tileCount) {
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

            for (i32 tileIndex = 0; tileIndex < map.tileCount; tileIndex++) {
                MapTile & tile = map.tiles[tileIndex];
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
