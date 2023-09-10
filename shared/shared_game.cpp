#include "shared_game.h"

#include <memory>
#include <math.h>
#include <cstring> // For ubuntu
#include <cstdio>  // For ubuntu

void MapCreate(Map & map) {
    ZeroStruct(map);
}

void MapStart(Map & map) {

}

Circle EntityGetSelectionBounds(Entity * entity) {
    Circle result = {};
    result.pos = V2(entity->general.pos);
    result.radius = 7.0f;
    return result;
}

Entity * MapSpawnEntity(Map & map) {
    for (int i = 0; i < ArrayCount(map.entities); i++) {
        Entity * entity = &map.entities[i];
        if (!entity->inUse) {
            entity->inUse = true;
            entity->id.idx = i;
            entity->id.gen = entity->id.gen + 1;
            return entity;
        }
    }

    Assert(false && "No more entities");

    return nullptr;
}

Entity * MapSpawnGeneral(Map & map) {
    Entity * entity = MapSpawnEntity(map);
    if (entity) {
        entity->general.pos = { 0, 0 };
        entity->general.gruntCount = 0;
    }
    return entity;
}

void MapApplyAction(Map & map, const MapAction & action) {
    switch (action.type) {
        case MAP_ACTION_MOVE_UNITS: {
            for (int i = 0; i < action.entitiesCount; i++) {
                EntityId id = action.entities[i];
                Entity * entity = &map.entities[id.idx];
                if (entity->inUse) {
                    entity->general.pos = action.target;
                }
            }
        } break;

        default: {
            Assert(false && "Invalid action type");
        } break;
    }
}

void MapDoTurn(Map & map, const MapTurn & turn) {
    map.tickNumber++;
    Assert(map.tickNumber == turn.tickNumber);
    for (int i = 0; i < turn.actionsCount; i++) {
        MapApplyAction(map, turn.actions[i]);
    }
}

void MapDoTick(Map & map) {

}








