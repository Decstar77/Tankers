#include "shared_game.h"

#include <math.h>

Player * MapSpawnPlayer(Map & map) {
    if (map.localPlayer.active == false) {
        map.localPlayer.active = true;
        map.localPlayer.playerNumber = 1;
        map.localPlayer.pos = { 100.0f, 100.0f };
        map.localPlayer.rot = 0.0f;
        map.localPlayer.fireCooldown = 0.0f;
        map.localPlayer.size = 10.0f;
        return &map.localPlayer;
    }

    if (map.remotePlayer.active == false) {
        map.remotePlayer.active = true;
        map.remotePlayer.playerNumber = 2;
        map.remotePlayer.pos = { 200.0f, 200.0f };
        map.remotePlayer.rot = 0.0f;
        map.remotePlayer.fireCooldown = 0.0f;
        map.remotePlayer.size = 10.0f;
        return &map.remotePlayer;
    }

    return nullptr;
}

Bullet * MapSpawnBullet(Map & map, v2 pos, v2 dir) {
    if (map.bulletCount >= MAX_BULLETS) {
        return nullptr;
    }

    Bullet & bullet = map.bullets[map.bulletCount++];
    bullet.pos = pos;
    bullet.dir = dir;
    return &bullet;
}

Enemy * MapSpawnEnemy(Map & map, EnemyType type, v2 pos) {
    if (map.enemyCount >= MAX_ENEMIES) {
        return nullptr;
    }

    Enemy & enemy = map.enemies[map.enemyCount++];
    enemy.type = type;
    enemy.pos = pos;
    enemy.rot = 0.0f;
    enemy.fireCooldown = 0.0f;
    enemy.size = 25.0f;
    return &enemy;
}

void MapUpdate(Map & map, f32 dt) {
    // Update player fire cooldown
    map.localPlayer.fireCooldown -= dt;
    map.remotePlayer.fireCooldown -= dt;

    for (i32 i = 0; i < map.bulletCount; ++i) {
        Bullet & bullet = map.bullets[i];
        bullet.pos = bullet.pos + bullet.dir * 10.0f;
    }
}

void GamePacketCreateStreamData(GamePacket & packet, Map & map) {
    packet.type = GAME_PACKET_TYPE_MAP_STREAM_DATA;
    packet.streamData.playerNumber = map.localPlayer.playerNumber;
    packet.streamData.pos = map.localPlayer.pos;
    packet.streamData.rot = map.localPlayer.rot;
}