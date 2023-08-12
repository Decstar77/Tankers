#include "game_client.h"

#include <stdio.h>

#define ENET_IMPLEMENTATION
#include "../vendor/enet.h"

static bool enetInitialized = false;
static ENetHost * client = nullptr;
static ENetPeer * server = nullptr;

bool NetworkConnectToServer(const char * address, int port) {
    if (enetInitialized == false && enet_initialize() != 0) {
        printf("An error occurred while initializing ENet.\n");
        return false;
    }

    enetInitialized = true;

    ENetAddress enetAddress = {};
    enetAddress.port = port;
    enet_address_set_host(&enetAddress, address);

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    server = enet_host_connect(client, &enetAddress, 2, 0);

    if (server == nullptr) {
        printf("No available peers for initiating an ENet connection.\n");
        return false;
    }

    ENetEvent event = {};
    if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connection to %s:%d succeeded.\n", address, port);
        return true;
    }
    else {
        enet_peer_reset(server);
        server = nullptr;

        enet_host_destroy(client);
        client = nullptr;

        printf("Connection to %s:%d failed.\n", address, port);
        return false;
    }
}

void NetoworkDisconnectFromServer() {
    if (server != nullptr) {
        enet_peer_disconnect(server, 0);
    }

    if (client != nullptr) {
        enet_host_destroy(client);
    }
}

bool NetworkIsConnected() {
    return server != nullptr;
}

bool NetworkPoll(GamePacket & packet) {
    if (client == nullptr) {
        return false;
    }

    if (server == nullptr) {
        return false;
    }

    ENetEvent event = {};
    if (enet_host_service(client, &event, 0) >= 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            Assert(event.packet->dataLength == sizeof(GamePacket));
            packet = *(GamePacket *)event.packet->data;

            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            if (event.peer == server) {
                printf("Server disconnected.\n");
                server = nullptr;
            }
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
            if (event.peer == server) {
                printf("Server disconnected.\n");
                server = nullptr;
            }
            break;
        }
        case ENET_EVENT_TYPE_NONE: {
            return false;
        }
        }
    }

    return true;
}

void NetworkSendPacket(GamePacket & packet, bool reliable) {
    if (server == nullptr) {
        return;
    }

    ENetPacket * enetPacket = enet_packet_create(&packet, sizeof(packet), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    enet_peer_send(server, 0, enetPacket);
}
