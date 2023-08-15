#include "game_local.h"
#include "game_client.h"

#include <math.h>

void LocalPlayerMove(Map & map, Player * player, v2 dir) {
    f32 speed = 2.23f;

    if (RoughlyZero(dir)) {
        return;
    }

    dir = Normalize(dir);
    v2 currentDir = { cosf(player->tankRot), sinf(player->tankRot) };
    f32 angle = SignedAngle(currentDir, dir);
    if (fabsf(angle) > 0.1f) {
        player->tankRot += angle * 0.1f;
    }
    else {
        player->tankRot = atan2f(dir.y, dir.x);
    }

    currentDir = { cosf(player->tankRot), sinf(player->tankRot) };
    player->pos = player->pos + currentDir * speed;

    // Wrap player movement
    if (player->pos.x < player->size) {
        player->pos.x = player->size;
    }
    else if (player->pos.x > map.width - player->size) {
        player->pos.x = map.width - player->size;
    }
    if (player->pos.y < player->size) {
        player->pos.y = player->size;
    }
    else if (player->pos.y > map.height - player->size) {
        player->pos.y = map.height - player->size;
    }

    Circle c = PlayerGetCollider(player);
    for (i32 i = 0; i < map.tileCount; i++) {
        MapTile & tile = map.tiles[i];
        CollisionManifold manifold = {};
        if (CircleVsRect(c, tile.rect, &manifold)) {
            player->pos = player->pos + manifold.normal * manifold.penetration;
        }
    }
}

void LocalPlayerLook(Map & map, Player * player, v2 point) {
    v2 dir = point - player->pos;
    player->turretRot = atan2f(dir.y, dir.x);
}

void LocalPlayerShoot(Map & map, Player * player) {
    if (player->fireCooldown > 0.0f) {
        return;
    }

    player->fireCooldown = 0.2f;

    v2 dir = { cosf(player->turretRot), sinf(player->turretRot) };
    LocalMapSpawnBullet(map, player->pos, dir);
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
    packet.playerStreamData.pos = map.localPlayer.pos;
    packet.playerStreamData.tankRot = map.localPlayer.tankRot;
    packet.playerStreamData.turretRot = map.localPlayer.turretRot;
    NetworkSendPacket(packet, false);
}