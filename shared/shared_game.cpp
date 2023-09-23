#include "shared_game.h"

#include <memory>
#include <math.h>
#include <cstring> // For ubuntu
#include <cstdio>  // For ubuntu

void MapCreate(Map & map, bool singlePlayer) {
    ZeroStruct(map);
    map.isSinglePlayer = singlePlayer;
    map.turnNumber = 0;

    for (i32 i = 0; i < MAX_MAP_TILES; i++) {
        MapTile tile = {};
        tile.flatIndex = i;
        tile.xIndex = i % MAX_MAP_TILE_W_COUNT;
        tile.yIndex = i / MAX_MAP_TILE_W_COUNT;
        tile.isWalkable = true;

        //tile.pos = V2fp(tile.xIndex * MAP_TILE_WIDTH, tile.yIndex * MAP_TILE_HEIGHT);
        //tile.visPos = V2(tile.pos);

        // We offset the tiles by half a tile so that the center of the map is (0, 0)
        tile.pos = V2fp(
            ((f32)tile.xIndex - (f32)MAX_MAP_TILE_W_COUNT / 2.0f) * (f32)MAP_TILE_WIDTH,
            ((f32)tile.yIndex - (f32)MAX_MAP_TILE_H_COUNT / 2.0f) * (f32)MAP_TILE_HEIGHT
        );
        tile.visPos = V2(tile.pos);

        map.tiles.Add(tile);
    }

    MapSpawnGeneral(map, 1, V2fp(-50, 20));
    MapSpawnGeneral(map, 2, V2fp(50, 20));

    MapSpawnBuildingCenter(map, 1, MapTileGetFlatIndex(47, 45));
    MapSpawnBuildingCenter(map, 2, MapTileGetFlatIndex(51, 45));
}

void MapStart(Map & map) {
    map.turnNumber = 1;
}

Bounds EntityGetSelectionBounds(Entity * entity) {
    switch (entity->type) {
    case ENTITY_TYPE_GENERAL: {
        Bounds result = {};
        result.type = BOUNDS_TYPE_CIRCLE;
        result.circle.pos = entity->general.visPos;
        result.circle.radius = 7.0f;
        return result;
    } break;
    case ENTITY_TYPE_BUILDING_TOWN_CENTER: {
        v2 dims = V2(MAP_TILE_WIDTH, MAP_TILE_HEIGHT) * V2(BUILDING_TOWN_CENTER_TILE_W_COUNT, BUILDING_TOWN_CENTER_TILE_H_COUNT);
        Bounds result = {};
        result.type = BOUNDS_TYPE_RECT;
        result.rect.min = entity->townCenter.visPos;
        result.rect.max = entity->townCenter.visPos + dims;
        return result;
    } break;
    default: {
        Assert(false && "Invalid entity type, you probably forget to add it here");
        return {};
    } break;
    };

    return {};
}

Entity * MapSpawnEntity(Map & map, EntityType type, i32 playerNumber) {
    for (int i = 0; i < ArrayCount(map.entities); i++) {
        Entity * entity = &map.entities[i];
        if (!entity->inUse) {
            entity->type = type;
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
    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_GENERAL, playerNumber);
    if (entity) {
        entity->general.pos = { 0, 0 };
        entity->general.gruntCount = 0;
    }
    return entity;
}

Entity * MapSpawnGeneral(Map & map, i32 playerNumber, v2fp pos) {
    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_GENERAL, playerNumber);
    if (entity) {
        entity->general.pos = pos;
        entity->general.target = pos;
        entity->general.visPos = V2(pos);
        entity->general.gruntCount = 0;
    }
    return entity;
}

Entity * MapSpawnBuildingCenter(Map & map, i32 playerNumber, i32 baseTileIndex) {
    Assert(baseTileIndex >= 0 && baseTileIndex < MAX_MAP_TILES);
    Assert(map.tiles[baseTileIndex].isWalkable);

    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_BUILDING_TOWN_CENTER, playerNumber);
    if (entity) {
        entity->townCenter.pos = MapTileFlatIndexToWorldPos(map, baseTileIndex);
        entity->townCenter.visPos = V2(entity->townCenter.pos);
        entity->townCenter.baseTileIndex = baseTileIndex;

        // Mark the tiles as unwalkable
        for (i32 y = 0; y < BUILDING_TOWN_CENTER_TILE_H_COUNT; y++) {
            for (i32 x = 0; x < BUILDING_TOWN_CENTER_TILE_W_COUNT; x++) {
                i32 tileXIndex = entity->townCenter.baseTileIndex % MAX_MAP_TILE_W_COUNT;
                i32 tileYIndex = entity->townCenter.baseTileIndex / MAX_MAP_TILE_W_COUNT;
                i32 tileFlatIndex = MapTileGetFlatIndex(tileXIndex + x, tileYIndex + y);
                map.tiles[tileFlatIndex].isWalkable = false;
            }
        }
    }

    return entity;
}

i32 MapTileGetFlatIndex(i32 xIndex, i32 yIndex) {
    Assert(xIndex >= 0 && xIndex < MAX_MAP_TILE_W_COUNT);
    Assert(yIndex >= 0 && yIndex < MAX_MAP_TILE_H_COUNT);

    i32 flatIndex = yIndex * MAX_MAP_TILE_W_COUNT + xIndex;
    return flatIndex;
}

v2fp MapTileFlatIndexToWorldPos(Map & map, i32 flatIndex) {
    i32 xIndex = flatIndex % MAX_MAP_TILE_W_COUNT;
    printf("xIndex: %d\n", xIndex);
    i32 yIndex = flatIndex / MAX_MAP_TILE_W_COUNT;
    v2fp pos = V2fp(
        ((f32)xIndex - (f32)MAX_MAP_TILE_W_COUNT / 2.0f) * (f32)MAP_TILE_WIDTH,
        ((f32)yIndex - (f32)MAX_MAP_TILE_H_COUNT / 2.0f) * (f32)MAP_TILE_HEIGHT
    );

    return pos;
}

i32 MapTileWorldPosToFlatIndex(Map & map, v2fp pos) {
    // We have to solve for tile.xIndex (The yIndex is solved the same way)
    // We have the following equation:
    // px = (tile.xIndex - MAX_MAP_TILE_W_COUNT / 2.0f) * MAP_TILE_WIDTH
    // px / MAP_TILE_WIDTH = tile.xIndex - MAX_MAP_TILE_W_COUNT / 2.0f
    // px / MAP_TILE_WIDTH + MAX_MAP_TILE_W_COUNT / 2.0f = tile.xIndex
    // Therefore:
    // tile.xIndex = px / MAP_TILE_WIDTH + MAX_MAP_TILE_W_COUNT / 2.0f

    fp xIndexF = pos.x / Fp(MAP_TILE_WIDTH) + Fp(MAX_MAP_TILE_W_COUNT) / Fp(2);
    fp yIndexF = pos.y / Fp(MAP_TILE_HEIGHT) + Fp(MAX_MAP_TILE_H_COUNT) / Fp(2);

    if (xIndexF < Fp(0) || xIndexF >= Fp(MAX_MAP_TILE_W_COUNT)) return -1;
    if (yIndexF < Fp(0) || yIndexF >= Fp(MAX_MAP_TILE_H_COUNT)) return -1;

    i32 xIndex = FpToInt(xIndexF);
    i32 yIndex = FpToInt(yIndexF);

    i32 flatIndex = yIndex * MAX_MAP_TILE_W_COUNT + xIndex;
    return flatIndex;
}

void MapCreateCommand(Map & map, MapCommand & action, MapCommandType type) {
    ZeroStruct(action);
    action.type = type;
}

void MapCreateCommandMoveSelectedUnits(Map & map, MapCommand & cmd, v2fp target) {
    MapCreateCommand(map, cmd, MAP_COMMAND_MOVE_UNITS);
    cmd.target = target;

    Assert(map.selection.GetCount() <= ArrayCount(cmd.entities));
    cmd.entitiesCount = map.selection.GetCount();
    for (int i = 0; i < map.selection.GetCount(); i++) {
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









