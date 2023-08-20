#include "game_local.h"
#include "game_client.h"

#include <math.h>

void LocalPlayerMove(Map & map, Player * player, v2 dir) {
    f32 speed = 2.23f;

    if (RoughlyZero(dir)) {
        return;
    }

    dir = Normalize(dir);
    v2 currentDir = { cosf(player->tank.rot), sinf(player->tank.rot) };
    f32 angle = SignedAngle(currentDir, dir);
    if (fabsf(angle) > 0.1f) {
        player->tank.rot += angle * 0.1f;
    }
    else {
        player->tank.rot = atan2f(dir.y, dir.x);
    }

    currentDir = { cosf(player->tank.rot), sinf(player->tank.rot) };
    player->tank.pos = player->tank.pos + currentDir * speed;

    // Wrap player movement
    if (player->tank.pos.x < player->tank.size) {
        player->tank.pos.x = player->tank.size;
    }
    else if (player->tank.pos.x > map.width - player->tank.size) {
        player->tank.pos.x = map.width - player->tank.size;
    }
    if (player->tank.pos.y < player->tank.size) {
        player->tank.pos.y = player->tank.size;
    }
    else if (player->tank.pos.y > map.height - player->tank.size) {
        player->tank.pos.y = map.height - player->tank.size;
    }

    Circle c = PlayerGetCollider(player);
    for (i32 i = 0; i < map.tileCount; i++) {
        MapTile & tile = map.tiles[i];
        CollisionManifold manifold = {};
        if (CircleVsRect(c, tile.rect, &manifold)) {
            player->tank.pos = player->tank.pos + manifold.normal * manifold.penetration;
        }
    }
}

void LocalPlayerLook(Map & map, Player * player, v2 point) {
    v2 dir = point - player->tank.pos;
    player->tank.turretRot = atan2f(dir.y, dir.x);
}

void LocalPlayerShoot(Map & map, Player * player) {
    if (player->fireCooldown > 0.0f) {
        return;
    }

    player->fireCooldown = 0.2f;

    v2 dir = { cosf(player->tank.turretRot), sinf(player->tank.turretRot) };
    LocalMapSpawnBullet(map, player->tank.pos, dir);
}

void LocalMapSpawnBullet(Map & map, v2 pos, v2 dir) {
    GamePacket packet = {};
    packet.type = GAME_PACKET_TYPE_MAP_SHOT_FIRED;
    packet.shotFired.pos = pos;
    packet.shotFired.dir = dir;
    NetworkSendPacket(packet, true);
}

void LocalSendStreamData(Map & map) {
    GamePacket packet = {};
    packet.type = GAME_PACKET_TYPE_MAP_PLAYER_STREAM_DATA;
    packet.playerStreamData.pos = map.localPlayer.tank.pos;
    packet.playerStreamData.tankRot = map.localPlayer.tank.rot;
    packet.playerStreamData.turretRot = map.localPlayer.tank.turretRot;
    NetworkSendPacket(packet, false);
}