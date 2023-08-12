#include "game_local.h"
#include "game_client.h"

#include <math.h>

void LocalPlayerMove(Map & map, Player * player, v2 dir) {
    f32 speed = 1.0f;
    player->pos = player->pos + dir * speed;

    // Wrap player movement
    if (player->pos.x < 0.0f) {
        player->pos.x += (f32)map.width;
    }
    if (player->pos.x >= (f32)map.width) {
        player->pos.x -= (f32)map.width;
    }
    if (player->pos.y < 0.0f) {
        player->pos.y += (f32)map.height;
    }
    if (player->pos.y >= (f32)map.height) {
        player->pos.y -= (f32)map.height;
    }
}

void LocalPlayerLook(Map & map, Player * player, v2 point) {
    v2 dir = point - player->pos;
    player->rot = atan2f(dir.y, dir.x);
}

void LocalPlayerShoot(Map & map, Player * player) {
    if (player->fireCooldown > 0.0f) {
        return;
    }

    player->fireCooldown = 0.2f;

    v2 dir = { cosf(player->rot), sinf(player->rot) };
    LocalMapSpawnBullet(map, player->pos, dir);
}

void LocalMapSpawnBullet(Map & map, v2 pos, v2 dir){
    GamePacket packet = {};
    packet.type = GAME_PACKET_TYPE_MAP_PLAYER_SHOOT;
    packet.playerShoot.pos = pos;
    packet.playerShoot.dir = dir;
    NetworkSendPacket(packet, true);
}