
#include "../shared/shared_game.h"
#ifdef _WIN32
#undef ZeroMemory
#endif

#define ENET_IMPLEMENTATION
#include "../vendor/enet.h"

#include <stdio.h>
#include <stdarg.h>
#include <atomic>
#include <thread>  // Sed
#include <chrono>  // Sed

static void * Allocate(size_t size) {
    void * mem = malloc(size);
    memset(mem, 0, size);
    return mem;
}

static void Deallocate(void * mem) {
    free(mem);
}

static void LogTrace(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

struct PeerData {
    int gameSessionIndex;
    int peerIndex;
    int playerNumber;
    char name[32];
};

struct GameSession {
    bool running;
    i32 tickNumber;
    double currentTime;
    union {
        ENetPeer * peers[2];
        struct {
            ENetPeer * peer1;
            ENetPeer * peer2;
        };
    };

    i32 packetCount;
    GamePacket incomingPackets[128];
};

#define MAX_SESSIONS 32
static GameSession sessions[MAX_SESSIONS];

static int GetSessionsRunningCount() {
    int count = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].running) {
            count++;
        }
    }
    return count;
}

static void GameSessionSendToPeer(GameSession * session, GamePacket * packet, int peerIndex, bool reliable) {
    ENetPacket * enetPacket = enet_packet_create(packet, sizeof(GamePacket), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    enet_peer_send(session->peers[peerIndex], 0, enetPacket);
}

static void GameSessionSendToPeersExpect(GameSession * session, GamePacket * packet, int peerIndex, bool reliable) {
    for (int i = 0; i < 2; i++) {
        if (i != peerIndex) {
            GameSessionSendToPeer(session, packet, i, reliable);
        }
    }
}

static void GameSessionSendToAllPeers(GameSession * session, GamePacket * packet, bool reliable) {
    for (int i = 0; i < 2; i++) {
        GameSessionSendToPeer(session, packet, i, reliable);
    }
}

static void GameSessionGameOverPlayerDisconnected(GameSession * session, int diconnectedPeerIndex) {
    // GamePacket packet = {};
    // packet.type = GAME_PACKET_TYPE_MAP_GAME_OVER;
    // packet.gameOver.reason = MAP_GAME_OVER_REASON_PLAYER_DISCONNECTED;
    // GameSessionSendToPeersExpect(session, &packet, diconnectedPeerIndex, true);

    session->running = false;
    Deallocate(session->peer1->data);
    Deallocate(session->peer2->data);

    for (int i = 0; i < 2; i++) {
        ((PeerData *)session->peers[i]->data)->gameSessionIndex = -1;
        ((PeerData *)session->peers[i]->data)->playerNumber = -1;
    }
}

std::atomic_bool running = true;

static void ReadServerConsole() {
    char str[256] = {};
    while (fgets(str, 256, stdin) != nullptr) {
        if (strcmp(str, "quit\n") == 0) {
            printf("Quitting...\n");
            running = false;
            break;
        }

        if (strcmp(str, "status\n") == 0) {
            printf("Status:\n");
            printf("Sessions running: %d\n", GetSessionsRunningCount());
            printf("Session tick rate: %d\n", GAME_TICKS_PER_SECOND);
            printf("Session tick time: %f\n", GAME_TICK_TIME);
        }

        ZeroMemory(str, 256);
    }
}

struct TimeResult {
    float delta_seconds;
    float delta_milliseconds;
    float delta_microseconds;
};

class Clock {
public:
    inline void Start() { this->start_time = std::chrono::steady_clock::now(); }
    inline void End() { this->end_time = std::chrono::steady_clock::now(); }

    inline TimeResult Mark() {
        std::chrono::time_point<std::chrono::steady_clock> current_time = std::chrono::steady_clock::now();
        std::chrono::microseconds elapsed_time(std::chrono::duration_cast<std::chrono::microseconds>(current_time - this->start_time));
        float time = static_cast<float>(elapsed_time.count());

        TimeResult result = {};
        result.delta_microseconds = time;
        result.delta_milliseconds = time * 0.001f;
        result.delta_seconds = time * 0.001f * 0.001f;

        return result;
    }

    inline TimeResult Get() {
        std::chrono::microseconds elapsed_time(std::chrono::duration_cast<std::chrono::microseconds>(this->end_time - this->start_time));
        float time = static_cast<float>(elapsed_time.count());

        TimeResult result = {};
        result.delta_microseconds = time;
        result.delta_milliseconds = time * 0.001f;
        result.delta_seconds = time * 0.001f * 0.001f;

        return result;
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::chrono::time_point<std::chrono::steady_clock> end_time;
};

int main(int argc, char * argv[]) {
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    ENetAddress address = {};
    address.host = ENET_HOST_ANY;
    address.port = 27164;

    ENetHost * server = enet_host_create(&address, 32, 2, 0, 0);
    if (server == nullptr) {
        fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
        return 0;
    }

    printf("Server started on port %d\n", address.port);

    ENetPeer * holdingPeer = nullptr;

    running = true;
    std::thread consoleThread(ReadServerConsole);

    double deltaTime = 0.0;
    Clock clock = {};
    clock.Start();

    ENetEvent event = {};
    while (enet_host_service(server, &event, 30) >= 0) {
        if (running == false) {
            break;
        }

        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            char ip[32] = {};
            enet_address_get_host_ip(&event.peer->address, ip, 32);
            printf("A new client connected from %s\n", ip);

            event.peer->data = Allocate(sizeof(PeerData));
            PeerData * peerData = (PeerData *)event.peer->data;
            peerData->gameSessionIndex = -1;
            peerData->peerIndex = -1;
            peerData->name[0] = '\0';

            if (holdingPeer != nullptr) {
                for (int i = 0; i < MAX_SESSIONS; i++) {
                    if (!sessions[i].running) {
                        ZeroStruct(sessions[i]);
                        sessions[i].running = true;
                        sessions[i].peer1 = holdingPeer;
                        sessions[i].peer2 = event.peer;
                        sessions[i].tickNumber = 0;

                        PeerData * peerData1 = (PeerData *)sessions[i].peer1->data;
                        PeerData * peerData2 = (PeerData *)sessions[i].peer2->data;

                        peerData1->gameSessionIndex = i;
                        peerData1->peerIndex = 0;
                        peerData2->gameSessionIndex = i;
                        peerData2->peerIndex = 1;

                        GamePacket packet = {};
                        packet.type = GAME_PACKET_TYPE_MAP_START;
                        packet.mapStart.localPlayerNumber = 1;
                        GameSessionSendToPeer(&sessions[i], &packet, 0, true);

                        packet.mapStart.localPlayerNumber = 2;
                        GameSessionSendToPeer(&sessions[i], &packet, 1, true);

                        printf("Starting game session %d\n", i);

                        holdingPeer = nullptr;
                        break;
                    }
                }

                holdingPeer = nullptr;
            }
            else {
                holdingPeer = event.peer;
            }
        } break;
        case ENET_EVENT_TYPE_RECEIVE: {
            GamePacket * gamePacket = (GamePacket *)event.packet->data;
            PeerData * peerData = (PeerData *)event.peer->data;
            if (peerData->gameSessionIndex != -1) {
                GameSessionSendToPeersExpect(&sessions[peerData->gameSessionIndex], gamePacket, peerData->peerIndex, true);
            }
            else {
                printf("Received packet from peer without game session\n");
            }
            enet_packet_destroy(event.packet);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
            char ip[32] = {};
            enet_address_get_host_ip(&event.peer->address, ip, 32);
            printf("%s disconnected.\n", ip);
            PeerData * peerData = (PeerData *)event.peer->data;
            if (peerData->gameSessionIndex != -1) {
                GameSessionGameOverPlayerDisconnected(&sessions[peerData->gameSessionIndex], peerData->peerIndex);
            }
        }   break;
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
            char ip[32] = {};
            enet_address_get_host_ip(&event.peer->address, ip, 32);
            printf("%s disconnected due to timeout.\n", ip);
            PeerData * peerData = (PeerData *)event.peer->data;
            if (peerData->gameSessionIndex != -1) {
                GameSessionGameOverPlayerDisconnected(&sessions[peerData->gameSessionIndex], peerData->peerIndex);
            }
        } break;
        case ENET_EVENT_TYPE_NONE: {
            // Nothing happened
        } break;
        }

        clock.End();
        deltaTime = clock.Get().delta_seconds;
        clock.Start();
    }

    consoleThread.join();

    printf("Server stopped\n");

    enet_host_destroy(server);
    enet_deinitialize();

    return 0;
}
