#include "game_local.h"
#include "game_client.h"

#include <math.h>

void LocalPlayerMove(Map & map, Player * player, v2 dir) {
    f32 speed = 1.0f;
    player->pos = player->pos + dir * speed;

    // Wrap player movement
    if (player->pos.x < player->size) {
        player->pos.x = player->size;
    }
    else if (player->pos.x > map.width - player->size) {
        player->pos.x = map.width - player->size;
    }
    if (player->pos.y < player->size){
        player->pos.y = player->size;
    }
    else if (player->pos.y > map.height - player->size) {
        player->pos.y = map.height - player->size;
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
    packet.type = GAME_PACKET_TYPE_MAP_SHOT_FIRED;
    packet.shotFired.pos = pos;
    packet.shotFired.dir = dir;
    NetworkSendPacket(packet, true);
}

void LocalSendStreamData(Map & map){
    GamePacket packet = {};
    packet.type = GAME_PACKET_TYPE_MAP_PLAYER_STREAM_DATA;
    packet.playerStreamData.pos = map.localPlayer.pos;
    packet.playerStreamData.rot = map.localPlayer.rot;
    NetworkSendPacket(packet, false);
}