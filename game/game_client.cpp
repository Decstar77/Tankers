#include "game_client.h"

#ifdef ZeroMemory
#undef ZeroMemory
#endif

#define ENET_IMPLEMENTATION
#include "../vendor/enet.h"

static bool enetInitialized = false;
static ENetHost * client = nullptr;
static ENetPeer * server = nullptr;

bool NetworkConnectToServer(const char * address, int port) {
    if (enetInitialized == false && enet_initialize() != 0) {
        PlatformPrint("An error occurred while initializing ENet.\n");
        return false;
    }

    enetInitialized = true;

    ENetAddress enetAddress = {};
    enetAddress.port = port;
    enet_address_set_host(&enetAddress, address);

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    server = enet_host_connect(client, &enetAddress, 2, 0);

    if (server == nullptr) {
        PlatformPrint("No available peers for initiating an ENet connection.\n");
        return false;
    }

    ENetEvent event = {};
    if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        PlatformPrint("Connection to %s:%d succeeded.\n", address, port);
        return true;
    }
    else {
        enet_peer_reset(server);
        server = nullptr;

        enet_host_destroy(client);
        client = nullptr;

        PlatformPrint("Connection to %s:%d failed.\n", address, port);
        return false;
    }
}

void NetoworkDisconnectFromServer() {
    if (server != nullptr) {
        enet_peer_disconnect(server, 0);
        ENetEvent event = {};
        bool disconnected = false;
        while (!disconnected && enet_host_service(client, &event, 3000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                PlatformPrint("Disconnection succeeded.\n");
                disconnected = true;
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                PlatformPrint("Disconnection failed.\n");
                break;
            }
            case ENET_EVENT_TYPE_NONE: {
                break;
            }
            }
        }
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
                PlatformPrint("Server disconnected.\n");
                server = nullptr;
            }
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
            if (event.peer == server) {
                PlatformPrint("Server disconnected.\n");
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

i32 NetworkGetPing() {
    if (server == nullptr) {
        return 0;
    }

    return (i32)server->roundTripTime;
}
