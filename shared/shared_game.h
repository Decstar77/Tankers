#pragma once

#include "shared_math.h"

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

#define MAX_GRUNTS 50
struct General {
    v2fp pos;
    i32 gruntCount;
    Grunt grunts[MAX_GRUNTS];
};

struct EntityId {
    i32 idx;
    i32 gen;
};

struct Entity {
    bool inUse;
    EntityId id;
    bool selected;
    union {
        General general;
    };
};

struct MapTiles {

};

#define MAX_MAP_ENTITIES 100

struct Map {
    MapSize size;
    MapVersion version;
    i32 width;
    i32 height;

    i32 tickNumber;

    i32         selectionCount;
    EntityId    selection[MAX_MAP_ENTITIES];

    Entity      entities[MAX_MAP_ENTITIES];
};

enum MapActionType {
    MAP_ACTION_INVALID = 0,
    MAP_ACTION_MOVE_UNITS,
    MAP_ACTION_COUNT,
};

struct MapAction {
    MapActionType   type;
    i32             entitiesCount;
    EntityId        entities[10];
    v2fp            target;
};

struct MapTurn {
    i32 tickNumber;
    i32 actionsCount;
    MapAction actions[10];
};

Circle EntityGetSelectionBounds(Entity * entity);

void MapCreate(Map & map);
void MapStart(Map & map);

Entity * MapSpawnEntity(Map & map);
Entity * MapSpawnGeneral(Map & map);

void MapApplyAction(Map & map, const MapAction & action);

void MapDoTurn(Map & map, const MapTurn & turn);
void MapDoTick(Map & map);

void MapDestroy(Map & map);

struct GamePacket {

};

// Setting values
constexpr i32 GAME_TICKS_PER_SECOND = 30;
constexpr i32 GAME_MAX_BYTES_PER_MS = 30;

// Calculated values
constexpr f32 GAME_TICK_TIME = 1.0f / (f32)GAME_TICKS_PER_SECOND;
constexpr f32 GAME_TICKS_PER_MS = (f32)GAME_TICKS_PER_SECOND / 1000.0f;
constexpr i32 GAME_PACKET_SIZE_BYTES = sizeof(GamePacket);
constexpr f32 GAME_MS_PER_TICK = 1.0f / GAME_TICKS_PER_MS;
constexpr f32 GAME_BYTES_PER_TICK = GAME_MS_PER_TICK * GAME_MAX_BYTES_PER_MS;

// Checks
static_assert(GAME_PACKET_SIZE_BYTES <= GAME_BYTES_PER_TICK, "Game packet size is too large");
