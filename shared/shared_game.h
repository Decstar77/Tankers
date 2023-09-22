#pragma once

#include "shared_math.h"
#include "shared_containers.h"

enum MapSize {
    MAP_SIZE_INVALID = 0,
    MAP_SIZE_SMALL,
    MAP_SIZE_MEDIUM,
    MAP_SIZE_LARGE,
    MAP_SIZE_COUNT,
};

enum MapVersion {
    MAP_VERSION_INVALID = 0,
    MAP_VERSION_1,
    MAP_VERSION_COUNT,
};

struct Grunt {

};

struct EntityId {
    i32 idx;
    i32 gen;
};

enum EntityType {
    ENTITY_TYPE_INVALID = 0,
    ENTITY_TYPE_GENERAL,
    ENTITY_TYPE_BUILDING_TOWN_CENTER,
    ENTITY_TYPE_COUNT,
};

#define MAX_GRUNTS 6
struct General {
    v2fp pos;
    v2fp target;
    v2 visPos;
    i32 gruntCount;
    Grunt grunts[MAX_GRUNTS];
};

constexpr i32 BUILDING_TOWN_CENTER_TILE_W_COUNT = 3;
constexpr i32 BUILDING_TOWN_CENTER_TILE_H_COUNT = 3;

struct BuildingTownHall {
    v2fp pos;
    v2 visPos;
    i32 baseTileIndex;
};

struct Entity {
    bool inUse;
    EntityId id;
    EntityType type;
    bool selected;
    i32 playerNumber;
    union {
        General general;
        BuildingTownHall townCenter;
    };
};

struct MapTile {
    v2fp    pos;
    v2      visPos;
    i32 flatIndex;
    i32 xIndex;
    i32 yIndex;
    bool isWalkable;
};

constexpr i32 MAX_MAP_ENTITIES = 100;
constexpr i32 MAX_MAP_TILE_W_COUNT = 100;
constexpr i32 MAX_MAP_TILE_H_COUNT = 100;
constexpr i32 MAX_MAP_TILES = MAX_MAP_TILE_W_COUNT * MAX_MAP_TILE_H_COUNT;
constexpr i32 MAP_TILE_WIDTH = 25;
constexpr i32 MAP_TILE_HEIGHT = 25;

struct Map {
    MapSize size;
    MapVersion version;
    i32 width;
    i32 height;

    bool isSinglePlayer;
    i32 turnNumber;

    i32         selectionCount;
    EntityId    selection[MAX_MAP_ENTITIES];

    Entity      entities[MAX_MAP_ENTITIES];

    FixedList<MapTile, MAX_MAP_TILES> tiles;
};

enum MapCommandType {
    MAP_COMMAND_INVALID = 0,
    MAP_COMMAND_MOVE_UNITS,
    MAP_COMMAND_COUNT,
};

struct MapCommand {
    MapCommandType   type;
    i32             entitiesCount;
    EntityId        entities[5];
    v2fp            target;
};

struct MapTurn {
    i64 checkSum;
    i32 turnNumber;
    i32 commandCount;
    MapCommand cmds[5];
};

Circle EntityGetSelectionBounds(Entity * entity);

void MapCreate(Map & map, bool singlePlayer);
void MapStart(Map & map);

Entity * MapSpawnEntity(Map & map, EntityType type, i32 playerNumber);
Entity * MapSpawnGeneral(Map & map, i32 playerNumber);
Entity * MapSpawnGeneral(Map & map, i32 playerNumber, v2fp pos);

Entity * MapSpawnBuildingCenter(Map & map, i32 playerNumber, i32 baseTileIndex);

i32     MapTileGetFlatIndex(i32 xIndex, i32 yIndex);
v2fp    MapTileFlatIndexToWorldPos(Map & map, i32 flatIndex);
i32     MapTileWorldPosToFlatIndex(Map & map, v2fp pos);

void MapCreateCommand(Map & map, MapCommand & action, MapCommandType type);
void MapCreateCommandMoveSelectedUnits(Map & map, MapCommand & action, v2fp target);

void MapApplyCommand(Map & map, const MapCommand & action);
void MapApplyTurn(Map & map, const MapTurn & turn);
i64  MapMakeTurnCheckSum(Map & map);
void MapDoTurn(Map & map, MapTurn & player1, MapTurn & player2);

void MapDestroy(Map & map);

enum GamePacketType {
    GAME_PACKET_TYPE_INVALID = 0,
    GAME_PACKET_TYPE_MAP_START,
    GAME_PACKET_TYPE_MAP_TURN,
    GAME_PACKET_TYPE_COUNT,
};

struct GamePacket {
    GamePacketType type;
    union {
        struct {
            i32 localPlayerNumber;
        } mapStart;
        struct {
            i32 playerNumber;
            MapTurn mapTurn;
        };
    };
};

// Setting values
constexpr i32 GAME_TICKS_PER_SECOND = 60;
constexpr i32 GAME_MAX_BYTES_PER_MS = 30;

// Calculated values
constexpr f32 GAME_TICK_TIME = 1.0f / (f32)GAME_TICKS_PER_SECOND;
constexpr f32 GAME_TICKS_PER_MS = (f32)GAME_TICKS_PER_SECOND / 1000.0f;
constexpr i32 GAME_PACKET_SIZE_BYTES = sizeof(GamePacket);
constexpr f32 GAME_MS_PER_TICK = 1.0f / GAME_TICKS_PER_MS;
constexpr f32 GAME_BYTES_PER_TICK = GAME_MS_PER_TICK * GAME_MAX_BYTES_PER_MS;

// Checks
static_assert(GAME_PACKET_SIZE_BYTES <= GAME_BYTES_PER_TICK, "Game packet size is too large");
