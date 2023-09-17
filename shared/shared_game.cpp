#include "shared_game.h"

#include <memory>
#include <math.h>
#include <cstring> // For ubuntu
#include <cstdio>  // For ubuntu

void MapCreate(Map & map, bool singlePlayer) {
    ZeroStruct(map);
    map.isSinglePlayer = singlePlayer;
    map.turnNumber = 0;
    MapSpawnGeneral(map, 1, V2fp(-50, 0));
    MapSpawnGeneral(map, 2, V2fp(50, 0));
}

void MapStart(Map & map) {
    map.turnNumber = 1;
}

Circle EntityGetSelectionBounds(Entity * entity) {
    Circle result = {};
    result.pos = V2(entity->general.pos);
    result.radius = 7.0f;
    return result;
}

Entity * MapSpawnEntity(Map & map, i32 playerNumber) {
    for (int i = 0; i < ArrayCount(map.entities); i++) {
        Entity * entity = &map.entities[i];
        if (!entity->inUse) {
            entity->inUse = true;
            entity->id.idx = i;
            entity->id.gen = entity->id.gen + 1;
            entity->playerNumber = playerNumber;
            return entity;
        }
    }

    Assert(false && "No more entities");

    return nullptr;
}

Entity * MapSpawnGeneral(Map & map, i32 playerNumber) {
    Entity * entity = MapSpawnEntity(map, playerNumber);
    if (entity) {
        entity->general.pos = { 0, 0 };
        entity->general.gruntCount = 0;
    }
    return entity;
}

Entity * MapSpawnGeneral(Map & map, i32 playerNumber, v2fp pos) {
    Entity * entity = MapSpawnEntity(map, playerNumber);
    if (entity) {
        entity->general.pos = pos;
        entity->general.target = pos;
        entity->general.visPos = V2(pos);
        entity->general.gruntCount = 0;
    }
    return entity;
}

void MapCreateCommand(Map & map, MapCommand & action, MapCommandType type) {
    ZeroStruct(action);
    action.type = type;
}

void MapCreateCommandMoveSelectedUnits(Map & map, MapCommand & cmd, v2fp target) {
    MapCreateCommand(map, cmd, MAP_COMMAND_MOVE_UNITS);
    cmd.target = target;

    Assert(map.selectionCount <= ArrayCount(cmd.entities));
    cmd.entitiesCount = map.selectionCount;
    for (int i = 0; i < map.selectionCount; i++) {
        cmd.entities[i] = map.selection[i];
    }
}

void MapApplyCommand(Map & map, const MapCommand & cmd) {
    switch (cmd.type) {
    case MAP_COMMAND_MOVE_UNITS: {
        for (int i = 0; i < cmd.entitiesCount; i++) {
            EntityId id = cmd.entities[i];
            Entity * entity = &map.entities[id.idx];
            if (entity->inUse) {
                entity->general.target = cmd.target;
            }
        }
    } break;

    default: {
        Assert(false && "Invalid action type");
    } break;
    }
}

i64 MapMakeTurnCheckSum(Map & map) {
    i64 checkSum = 0;
    for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
        Entity * entity = &map.entities[i];
        if (entity->inUse) {
            checkSum += entity->id.idx;
            checkSum += entity->id.gen;
            checkSum += I64(entity->general.pos.x);
            checkSum += I64(entity->general.pos.y);
            checkSum += I64(entity->general.target.x);
            checkSum += I64(entity->general.target.y);
            checkSum += entity->general.gruntCount;
        }
    }

    return checkSum;
}

void MapDoTurn(Map & map, MapTurn & player1Turn, MapTurn & player2Turn) {
    // printf("Turn %d\n", map.turnNumber);
    // printf("Player 1: %d\n", player1Turn.turnNumber);
    Assert(map.turnNumber == player1Turn.turnNumber);
    Assert(map.turnNumber == player2Turn.turnNumber);
    Assert(player1Turn.checkSum == player2Turn.checkSum);

    for (i32 i = 0; i < player1Turn.commandCount; i++) {
        MapApplyCommand(map, player1Turn.cmds[i]);
    }

    for (i32 i = 0; i < player2Turn.commandCount; i++) {
        MapApplyCommand(map, player2Turn.cmds[i]);
    }

    for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
        Entity * entity = &map.entities[i];
        if (entity->inUse) {
            fp speed = Fp(0.1f);
            v2fp dir = entity->general.target - entity->general.pos;
            entity->general.pos = entity->general.pos + dir * speed;
            entity->general.visPos = Lerp(entity->general.visPos, V2(entity->general.pos), 0.1f);
        }
    }

    map.turnNumber++;
}









