#include "shared_game.h"

#include <memory>
#include <math.h>
#include <cstring> // For ubuntu

Circle PlayerGetCollider(Player * player) {
    return { player->pos, player->size / 2.0f };
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

void MapStart(Map & map) {
    MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, v2{ 200.0f, 500.0f });
    //MapSpawnEnemy(map, ENEMY_TYPE_LIGHT_BROWN, v2{ 600.0f, 500.0f });

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
        map.localPlayer.pos = { 100.0f, 100.0f };
        map.localPlayer.tankRot = 0.0f;
        map.localPlayer.turretRot = 0.0f;
        map.localPlayer.fireCooldown = 0.0f;
        map.localPlayer.size = 25.0f;
        return &map.localPlayer;
    }

    if (map.remotePlayer.active == false) {
        map.remotePlayer.active = true;
        map.remotePlayer.playerNumber = 2;
        map.remotePlayer.pos = { 200.0f, 200.0f };
        map.remotePlayer.tankRot = 0.0f;
        map.remotePlayer.turretRot = 0.0f;
        map.remotePlayer.fireCooldown = 0.0f;
        map.remotePlayer.size = 25.0f;
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
    if (map.enemyCount >= MAX_ENEMIES) {
        return nullptr;
    }

    Enemy & enemy = map.enemies[map.enemyCount++];
    enemy.active = true;
    enemy.type = type;
    enemy.pos = pos;
    enemy.tankRot = 0.0f;
    enemy.fireCooldown = 0.0f;
    enemy.size = 25.0f;
    enemy.remotePos = pos;
    enemy.remoteTankRot = 0.0f;
    enemy.remoteTurretRot = 0.0f;
    return &enemy;
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

static f32 BulletSpeedFromType(BulletType type) {
    switch (type) {
        case BULLET_TYPE_NORMAL: return 10.0f;
        case BULLET_TYPE_ROCKET: return 15.0f;
        default: return 0.0f;
    }

    return 0.0f;
}

static f32 BulletSizeFromType(BulletType type) {
    switch (type) {
        case BULLET_TYPE_NORMAL: return 5.0f;
        case BULLET_TYPE_ROCKET: return 10.0f;
        default: return 0.0f;
    }

    return 0.0f;
}

void MapUpdate(Map & map, f32 dt) {
    // Update player fire cooldown
    map.localPlayer.fireCooldown -= dt;
    map.remotePlayer.fireCooldown -= dt;

    for (i32 currentBulletIndex = 0; currentBulletIndex < MAX_BULLETS; currentBulletIndex++) {
        Bullet & bullet = map.bullets[currentBulletIndex];
        const f32 bulletSpeed = BulletSpeedFromType(bullet.type);
        const f32 bulletSize = BulletSizeFromType(bullet.type);
        if (bullet.active) {
            v2 bv = bullet.dir * bulletSpeed;
            Circle bulletCollider = { bullet.pos, bulletSize };

            bool hitBullet = false;
            for (i32 otherBulletIndex = 0; otherBulletIndex < MAX_BULLETS; otherBulletIndex++) {
                Bullet & otherBullet = map.bullets[otherBulletIndex];
                if (otherBullet.active && otherBulletIndex != currentBulletIndex) {
                    Circle otherBulletCollider = { otherBullet.pos, bulletSize };
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

            // for (i32 j = 0; j < MAX_ENEMIES; j++) {
            //     Enemy & enemy = map.enemies[j];
            //     if (enemy.active) {
            //         Circle enemyCollider = { enemy.pos, enemy.size / 2.0f };
            //         if (CircleVsCircle(bulletCollider, enemyCollider)) {
            //             bullet.active = false;
            //             enemy.active = false;
            //         }
            //     }
            // }
        }
    }
}
