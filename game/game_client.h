#pragma once

#include "../shared/shared_game.h"

bool NetworkConnectToServer(const char * address, int port);
void NetoworkDisconnectFromServer();
bool NetworkPoll(GamePacket & packet);

bool NetworkIsConnected();

void NetworkSendPacket(GamePacket & packet, bool reliable);


