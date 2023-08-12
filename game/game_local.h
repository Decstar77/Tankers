#pragma once

#include "../shared/shared_game.h"

void LocalPlayerMove(Map & map, Player * player, v2 dir);
void LocalPlayerLook(Map & map, Player * player, v2 point);
void LocalPlayerShoot(Map & map, Player * player);
void LocalMapSpawnBullet(Map & map, v2 pos, v2 dir);


