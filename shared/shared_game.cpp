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

    MapSpawnBuildingTownCenter(map, 1, MapTileGetFlatIndex(47, 45));
    MapSpawnBuildingTownCenter(map, 2, MapTileGetFlatIndex(51, 45));

    MapSpawnResourceNodeR1(map, MapTileGetFlatIndex(47, 38), 100);
    MapSpawnResourceNodeR2(map, MapTileGetFlatIndex(52, 38), 100);
}

void MapStart(Map & map) {
    map.turnNumber = 1;
}

bool MapSelectionContainsType(Map & map, EntityType type) {
    const i32 count = map.selection.GetCount();
    for (i32 i = 0; i < count; i++) {
        Entity * entity = &map.entities[map.selection[i].idx];
        if (entity->type == type) {
            return true;
        }
    }

    return false;
}

Entity * MapLookUpEntityFromId(Map & map, EntityId id) {
    if (id.idx < 0 || id.idx >= MAX_MAP_ENTITIES) {
        return nullptr;
    }

    Entity * entity = &map.entities[id.idx];
    if (entity->inUse && entity->id.gen == id.gen) {
        return entity;
    }

    return nullptr;
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
    case ENTITY_TYPE_RESOURCE_NODE_R1: // Fallthrough
    case ENTITY_TYPE_RESOURCE_NODE_R2: {
        v2 dims = V2(MAP_TILE_WIDTH, MAP_TILE_HEIGHT) * V2(RESOURCE_NODE_TILE_W_COUNT, RESOURCE_NODE_TILE_H_COUNT);
        Bounds result = {};
        result.type = BOUNDS_TYPE_RECT;
        result.rect.min = entity->resourceNode.visPos;
        result.rect.max = entity->resourceNode.visPos + dims;
        return result;
    } break;
    default: {
        Assert(false && "Invalid entity type, you probably forget to add it here");
        return {};
    } break;
    };

    return {};
}

Boundsfp EntityGetCollider(Entity * entity) {
    switch (entity->type) {
    case ENTITY_TYPE_GENERAL: {
        Boundsfp result = {};
        result.type = BOUNDS_TYPEFP_CIRCLE;
        result.circle.pos = entity->general.pos;
        result.circle.radius = Fp(7.0f);
        return result;
    } break;
    case ENTITY_TYPE_BUILDING_TOWN_CENTER: {
        v2fp dims = V2fp(MAP_TILE_WIDTH, MAP_TILE_HEIGHT) * V2fp(BUILDING_TOWN_CENTER_TILE_W_COUNT, BUILDING_TOWN_CENTER_TILE_H_COUNT);
        Boundsfp result = {};
        result.type = BOUNDS_TYPEFP_RECT;
        result.rect.min = entity->townCenter.pos;
        result.rect.max = entity->townCenter.pos + dims;
        return result;
    } break;
    case ENTITY_TYPE_RESOURCE_NODE_R1: // Fallthrough
    case ENTITY_TYPE_RESOURCE_NODE_R2: {
        v2fp dims = V2fp(MAP_TILE_WIDTH, MAP_TILE_HEIGHT) * V2fp(RESOURCE_NODE_TILE_W_COUNT, RESOURCE_NODE_TILE_H_COUNT);
        Boundsfp result = {};
        result.type = BOUNDS_TYPEFP_RECT;
        result.rect.min = entity->resourceNode.pos;
        result.rect.max = entity->resourceNode.pos + dims;
        return result;
    } break;
    default: {
        Assert(false && "Invalid entity type, you probably forget to add it here");
        return {};
    } break;
    };

    return {};
}

bool EntityIsResourceNode(Entity * entity) {
    const i32 begin = ENTITY_TYPE_RESOURCE_NODE_BEGIN;
    const i32 end = ENTITY_TYPE_RESOURCE_NODE_END;
    return entity->type > begin && entity->type < end;
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

Entity * MapSpawnGeneral(Map & map, i32 playerNumber, v2fp pos) {
    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_GENERAL, playerNumber);
    if (entity) {
        entity->general.pos = pos;
        entity->general.target = pos;
        entity->general.visPos = V2(pos);

        entity->general.equipment = GRUNT_EQUIPMENT_NONE;
        // Spawn the grunts in a circle around the general
        for (i32 i = 0; i < MAX_GRUNTS; i++) {

            fp angle = Fp(i) / Fp(MAX_GRUNTS) * Fp(2) * FP_PI;
            v2fp offset = V2fp(Cos(angle), Sin(angle)) * Fp(25.0f);

            Grunt grunt = {};
            grunt.pos = pos + offset;
            grunt.visPos = V2(grunt.pos);
            entity->general.grunts.Add(grunt);
        }
    }
    return entity;
}

Entity * MapSpawnBuildingTownCenter(Map & map, i32 playerNumber, i32 baseTileIndex) {
    Assert(baseTileIndex >= 0 && baseTileIndex < MAX_MAP_TILES);
    Assert(map.tiles[baseTileIndex].isWalkable);

    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_BUILDING_TOWN_CENTER, playerNumber);
    if (entity) {
        entity->townCenter.pos = MapTileFlatIndexToWorldPos(map, baseTileIndex);
        entity->townCenter.visPos = V2(entity->townCenter.pos);
        entity->townCenter.baseTileIndex = baseTileIndex;

        // Mark the tiles as unwalkable
        MapTileMarkAsWalkable(map, baseTileIndex, BUILDING_TOWN_CENTER_TILE_W_COUNT, BUILDING_TOWN_CENTER_TILE_H_COUNT, false);
    }

    return entity;
}

Entity * MapSpawnResourceNodeR1(Map & map, i32 baseTileIndex, i32 resourceCount) {
    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_RESOURCE_NODE_R1, 0);
    if (entity) {
        entity->resourceNode.pos = MapTileFlatIndexToWorldPos(map, baseTileIndex);
        entity->resourceNode.visPos = V2(entity->resourceNode.pos);
        entity->resourceNode.baseTileIndex = baseTileIndex;
        entity->resourceNode.resourceCount = resourceCount;

        // Mark the tiles as unwalkable
        MapTileMarkAsWalkable(map, baseTileIndex, RESOURCE_NODE_TILE_W_COUNT, RESOURCE_NODE_TILE_H_COUNT, false);
    }

    return entity;
}

Entity * MapSpawnResourceNodeR2(Map & map, i32 baseTileIndex, i32 resourceCount) {
    Entity * entity = MapSpawnEntity(map, ENTITY_TYPE_RESOURCE_NODE_R2, 0);
    if (entity) {
        entity->resourceNode.pos = MapTileFlatIndexToWorldPos(map, baseTileIndex);
        entity->resourceNode.visPos = V2(entity->resourceNode.pos);
        entity->resourceNode.baseTileIndex = baseTileIndex;
        entity->resourceNode.resourceCount = resourceCount;

        // Mark the tiles as unwalkable
        MapTileMarkAsWalkable(map, baseTileIndex, RESOURCE_NODE_TILE_W_COUNT, RESOURCE_NODE_TILE_H_COUNT, false);
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

void MapTileMarkAsWalkable(Map & map, i32 baseTileIndex, i32 wCount, i32 hCount, bool isWalkable) {
    for (i32 y = 0; y < hCount; y++) {
        for (i32 x = 0; x < wCount; x++) {
            i32 tileXIndex = baseTileIndex % MAX_MAP_TILE_W_COUNT;
            i32 tileYIndex = baseTileIndex / MAX_MAP_TILE_W_COUNT;
            i32 tileFlatIndex = MapTileGetFlatIndex(tileXIndex + x, tileYIndex + y);
            map.tiles[tileFlatIndex].isWalkable = isWalkable;
        }
    }
}

void MapCreateCommand(Map & map, MapCommand & action, MapCommandType type) {
    ZeroStruct(action);
    action.type = type;
}

void MapCreateCommandMoveSelectedUnits(Map & map, MapCommand & cmd, v2fp target) {
    MapCreateCommand(map, cmd, MAP_COMMAND_MOVE_UNITS);
    cmd.target = target;

    const i32 count = map.selection.GetCount();
    Assert(count <= ArrayCount(cmd.entities));

    for (i32 i = 0; i < count; i++) {
        Entity * entity = MapLookUpEntityFromId(map, map.selection[i]);
        if (entity && entity->type == ENTITY_TYPE_GENERAL) {
            cmd.entities.Add(entity->id);
        }
    }
}

void MapApplyCommand(Map & map, const MapCommand & cmd) {
    switch (cmd.type) {
    case MAP_COMMAND_MOVE_UNITS: {
        for (int i = 0; i < cmd.entities.GetCount(); i++) {
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
            checkSum += entity->general.grunts.GetCount();
        }
    }

    checkSum += map.player1Resource1Count;
    checkSum += map.player1Resource2Count;
    checkSum += map.player2Resource1Count;
    checkSum += map.player2Resource2Count;

    return checkSum;
}

void MapDoTurn(Map & map, MapTurn & player1Turn, MapTurn & player2Turn) {
    // printf("Turn %d\n", map.turnNumber);
    // printf("Player 1: %d\n", player1Turn.turnNumber);
    Assert(map.turnNumber == player1Turn.turnNumber);
    Assert(map.turnNumber == player2Turn.turnNumber);
    Assert(player1Turn.checkSum == player2Turn.checkSum);

    const i32 player1CommandCount = player1Turn.cmds.GetCount();
    for (i32 i = 0; i < player1CommandCount; i++) {
        MapApplyCommand(map, player1Turn.cmds[i]);
    }

    const i32 player2CommandCount = player2Turn.cmds.GetCount();
    for (i32 i = 0; i < player2CommandCount; i++) {
        MapApplyCommand(map, player2Turn.cmds[i]);
    }

    for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
        Entity * entity = &map.entities[i];
        if (entity->inUse) {
            switch (entity->type) {
            case ENTITY_TYPE_GENERAL: {
                fp speed = Fp(0.1f);
                v2fp dir = entity->general.target - entity->general.pos;
                entity->general.pos = entity->general.pos + dir * speed;
                entity->general.visPos = Lerp(entity->general.visPos, V2(entity->general.pos), 0.1f);
                const i32 gruntCount = entity->general.grunts.GetCount();
                for (i32 i = 0; i < gruntCount; i++) {
                    Grunt * grunt = &entity->general.grunts[i];
                    grunt->pos = grunt->pos + dir * speed;
                    grunt->visPos = Lerp(grunt->visPos, V2(grunt->pos), 0.1f);
                }

                // // Check if we're near a resource node
                // for (i32 resourceNodeIndex = 0; resourceNodeIndex < MAX_MAP_ENTITIES; resourceNodeIndex++) {
                //     Entity * resourceNode = &map.entities[resourceNodeIndex];
                //     if (resourceNode->inUse && EntityIsResourceNode(resourceNode)) {
                //         Boundsfp b = EntityGetCollider(resourceNode);
                //         v2fp c = GetBoundsFpCenter(b);
                //         fp d = DistanceSqrd(c, entity->general.pos);
                //         printf("d: %f\n", FpToFloat(d));
                //         if (d < Fp(20.0f)) {
                //             printf("Near resource node\n");
                //         }
                //     }
                // }

            } break;
            case ENTITY_TYPE_BUILDING_TOWN_CENTER: {

            } break;
            case ENTITY_TYPE_RESOURCE_NODE_R1: {

            } break;
            case ENTITY_TYPE_RESOURCE_NODE_R2: {

            } break;
            default: {
                Assert(false && "Invalid entity type, you probably forget to add it here");
            } break;
            }

        }
    }

    map.turnNumber++;
}









