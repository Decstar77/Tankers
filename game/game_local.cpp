/*

#include "game_local.h"
#include "game_client.h"
#include "game_ui.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <cstring>

void DoScreenMainMenu(GameLocal & gameLocal) {
    BeginDrawing();
    ClearBackground(Fade(BLUE, 0.5f));

    UIStateReset();
    if (UIDrawButtonCenter(GetScreenWidth() / 2, GetScreenHeight() / 2 - 100, "Single Player")) {
        MapCreate(gameLocal.map, true);
        MapStart(gameLocal.map);
        gameLocal.playerNumber = 1;
        gameLocal.screen = SCREEN_TYPE_GAME;
    }
    if (UIDrawButtonCenter(GetScreenWidth() / 2, GetScreenHeight() / 2, "Multiplayer")) {
        if (NetworkConnectToServer(gameLocal.gameSettings.serverIp, gameLocal.gameSettings.serverPort)) {
            printf("Connected to server.\n");
            MapCreate(gameLocal.map, false);
        }
    }
    if (UIDrawButtonCenter(GetScreenWidth() / 2, GetScreenHeight() / 2 + 100, "Quit(How dare you !)")) {
        gameLocal.running = false;
    }

    if (NetworkIsConnected()) {
        GamePacket packet = {};
        while (NetworkPoll(packet)) {
            if (packet.type == GAME_PACKET_TYPE_MAP_START) {
                printf("Received map start packet.\n");
                MapStart(gameLocal.map);
                gameLocal.playerNumber = packet.mapStart.localPlayerNumber;
                gameLocal.sync.Start();

                gameLocal.screen = SCREEN_TYPE_GAME;
            }
        }
    }

    UIStateDraw();

    EndDrawing();
}

static void DrawBounds(Bounds bounds) {
    switch (bounds.type) {
    case BOUNDS_TYPE_CIRCLE: {
        DrawCircleV({ bounds.circle.pos.x, bounds.circle.pos.y }, bounds.circle.radius, RED);
    } break;
    case BOUNDS_TYPE_RECT: {
        Rectangle r = { bounds.rect.min.x, bounds.rect.min.y, bounds.rect.max.x - bounds.rect.min.x, bounds.rect.max.y - bounds.rect.min.y };
        DrawRectangleLinesEx(r, 2, RED);
    } break;
    }
}

static void DrawGeneral(Entity & entity) {
    const i32 gruntCount = entity.general.grunts.GetCount();
    for (i32 i = 0; i < gruntCount; i++) {
        Grunt & grunt = entity.general.grunts[i];
        Vector2 pos = { grunt.visPos.x, grunt.visPos.y };
        Color c = entity.playerNumber == 1 ? SKYBLUE : SKYRED;
        if (entity.selected) {
            DrawCircleV(pos, 8, GREEN);
        }
        DrawCircleV(pos, 6, c);
    }

    Vector2 pos = { entity.general.visPos.x, entity.general.visPos.y };
    if (entity.selected) {
        DrawCircleV(pos, 10, GREEN);
    }

    if (entity.playerNumber == 1) {
        DrawCircleV(pos, 7, BLUE);
    }
    else {
        DrawCircleV(pos, 7, RED);
    }
}

static void DrawTownCenter(Entity & entity) {
    Rectangle r = { entity.townCenter.visPos.x, entity.townCenter.visPos.y, MAP_TILE_WIDTH * BUILDING_TOWN_CENTER_TILE_W_COUNT, MAP_TILE_HEIGHT * BUILDING_TOWN_CENTER_TILE_H_COUNT };
    Color c = entity.playerNumber == 1 ? BLUE : RED;
    DrawRectangleRec(r, c);
    DrawRectangleLinesEx(r, 2, entity.selected ? GREEN : BLACK);
    i32 cx = (i32)entity.townCenter.visPos.x + (MAP_TILE_WIDTH * BUILDING_TOWN_CENTER_TILE_W_COUNT) / 2;
    i32 cy = (i32)entity.townCenter.visPos.y + (MAP_TILE_HEIGHT * BUILDING_TOWN_CENTER_TILE_H_COUNT) / 2;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), "TC", 20, 1);
    cx -= (i32)textSize.x / 2;
    cy -= (i32)textSize.y / 2;
    DrawText("TC", cx, cy, 20, BLACK);
}

static void DrawResource1(Entity & entity) {
    Rectangle r = { entity.resourceNode.visPos.x, entity.resourceNode.visPos.y, MAP_TILE_WIDTH * RESOURCE_NODE_TILE_W_COUNT, MAP_TILE_HEIGHT * RESOURCE_NODE_TILE_H_COUNT };
    Color c = GREEN;
    DrawRectangleRec(r, c);
    DrawRectangleLinesEx(r, 2, entity.selected ? GREEN : BLACK);
    i32 cx = (i32)entity.resourceNode.visPos.x + (MAP_TILE_WIDTH * RESOURCE_NODE_TILE_W_COUNT) / 2;
    i32 cy = (i32)entity.resourceNode.visPos.y + (MAP_TILE_HEIGHT * RESOURCE_NODE_TILE_H_COUNT) / 2;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), "R1", 20, 1);
    cx -= (i32)textSize.x / 2;
    cy -= (i32)textSize.y / 2;
    DrawText("R1", cx, cy, 20, BLACK);
}

static void DrawResource2(Entity & entity) {
    Rectangle r = { entity.resourceNode.visPos.x, entity.resourceNode.visPos.y, MAP_TILE_WIDTH * RESOURCE_NODE_TILE_W_COUNT, MAP_TILE_HEIGHT * RESOURCE_NODE_TILE_H_COUNT };
    Color c = DARKGREEN;
    DrawRectangleRec(r, c);
    DrawRectangleLinesEx(r, 2, entity.selected ? GREEN : BLACK);
    i32 cx = (i32)entity.resourceNode.visPos.x + (MAP_TILE_WIDTH * RESOURCE_NODE_TILE_W_COUNT) / 2;
    i32 cy = (i32)entity.resourceNode.visPos.y + (MAP_TILE_HEIGHT * RESOURCE_NODE_TILE_H_COUNT) / 2;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), "R2", 20, 1);
    cx -= (i32)textSize.x / 2;
    cy -= (i32)textSize.y / 2;
    DrawText("R2", cx, cy, 20, BLACK);
}

Entity * GetEntityUnderMouse(Map & map, v2 worldMouse) {
    for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
        Entity * entity = &map.entities[i];
        if (entity->inUse) {
            Bounds b = EntityGetSelectionBounds(entity);
            if (IsPointInBounds(worldMouse, b)) {
                return entity;
            }
        }
    }
    return nullptr;
}

void DoScreenGame(GameLocal & gameLocal, i32 surfaceWidth, i32 surfaceHeight, RenderTexture2D surface) {
    Map & map = gameLocal.map;
    Camera2D & cam = gameLocal.cam;

    if (map.isSinglePlayer) {
        gameLocal.turnAccumulator += GetFrameTime();
        if (gameLocal.turnAccumulator >= gameLocal.sync.turnRateMS) {
            gameLocal.turnAccumulator = 0;
            static MapTurn dummyTurn = {};
            dummyTurn.turnNumber = map.turnNumber;
            gameLocal.mapTurn.turnNumber = map.turnNumber;
            MapDoTurn(map, gameLocal.mapTurn, dummyTurn);
            ZeroStruct(gameLocal.mapTurn);
            ZeroStruct(dummyTurn);
        }
    }
    else {
        GamePacket packet = {};
        while (NetworkPoll(packet)) {
            if (packet.type == GAME_PACKET_TYPE_MAP_TURN) {
                //printf("Received map turn packet.\n");
                gameLocal.sync.AddTurn(packet.playerNumber, packet.mapTurn);
            }
        }

        gameLocal.turnAccumulator += GetFrameTime();
        if (gameLocal.turnAccumulator >= gameLocal.sync.turnRateMS) {
            if (gameLocal.sync.CanTurn()) {
                gameLocal.turnAccumulator = 0;

                gameLocal.mapTurn.checkSum = MapMakeTurnCheckSum(map);
                gameLocal.mapTurn.turnNumber = map.turnNumber + gameLocal.sync.GetSlidingWindowWidth();
                gameLocal.sync.AddTurn(gameLocal.playerNumber, gameLocal.mapTurn);

                GamePacket packet = {};
                packet.type = GAME_PACKET_TYPE_MAP_TURN;
                packet.playerNumber = gameLocal.playerNumber;
                packet.mapTurn = gameLocal.mapTurn;
                NetworkSendPacket(packet, true);
                ZeroStruct(gameLocal.mapTurn);

                MapTurn * player1Turn = gameLocal.sync.GetNextTurn(1);
                MapTurn * player2Turn = gameLocal.sync.GetNextTurn(2);
                MapDoTurn(map, *player1Turn, *player2Turn);
                gameLocal.sync.FinishTurn();
            }
        }
    }

    UIStateReset();
    SmallString r1Text = StringFormat::Small("R1: %d", gameLocal.playerNumber == 1 ? map.player1Resource1Count : map.player2Resource1Count);
    SmallString r2Text = StringFormat::Small("R2: %d", gameLocal.playerNumber == 1 ? map.player1Resource2Count : map.player2Resource2Count);
    UIDrawButtonTopLeft(10, 10, r1Text.GetCStr());
    UIDrawButtonTopLeft(10, 40, r2Text.GetCStr());
    if (MapSelectionContainsType(map, ENTITY_TYPE_BUILDING_TOWN_CENTER)) {
        UIDrawBlockButton(V2(GetScreenWidth() - 60, GetScreenHeight() - 60), V2(100, 100), GRAY, "Build\nGeneral");
    }

    f32 scale = Min((f32)GetScreenWidth() / surfaceWidth, (f32)GetScreenHeight() / surfaceHeight);

    Vector2 mouse = GetMousePosition();
    Vector2 mouseDelta = GetMouseDelta();
    bool mouseMoved = true;
    if (mouseDelta.x == 0 && mouseDelta.y == 0) {
        mouseMoved = false;
    }

    v2 surfaceMouse = {};
    surfaceMouse.x = (mouse.x - (GetScreenWidth() - (surfaceWidth * scale)) * 0.5f) / scale;
    surfaceMouse.y = (mouse.y - (GetScreenHeight() - (surfaceHeight * scale)) * 0.5f) / scale;
    surfaceMouse = Clamp(surfaceMouse, { 0, 0 }, { (float)surfaceWidth, (float)surfaceHeight });

    DoCameraPanning(cam);
    DoCameraZooming(cam);

    Vector2 rayMouseWorld = GetScreenToWorld2D({ surfaceMouse.x, surfaceMouse.y }, cam);
    v2 mouseWorld = { rayMouseWorld.x, rayMouseWorld.y };

    static v2 startDrag = {};
    static v2 endDrag = {};
    static bool dragging = false;

    if (UIElementHover() == false) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            startDrag = mouseWorld;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && mouseMoved) {
            dragging = true;
        }

        if (dragging == true) {
            endDrag = mouseWorld;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (dragging == true) {
                dragging = false;

                Rect selectionRect = {};
                selectionRect.min = Min(startDrag, endDrag);
                selectionRect.max = Max(startDrag, endDrag);

                map.selection.Clear();

                startDrag = {};
                endDrag = {};
                for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
                    Entity * entity = &map.entities[i];
                    if (entity->inUse && entity->playerNumber == gameLocal.playerNumber) {
                        if (entity->type == ENTITY_TYPE_GENERAL) {
                            Bounds c = EntityGetSelectionBounds(entity);
                            if (c.type == BOUNDS_TYPE_CIRCLE && CircleVsRect(c.circle, selectionRect)) {
                                entity->selected = true;
                                map.selection.Add(entity->id);
                            }
                            else {
                                entity->selected = false;
                            }
                        }
                        else {
                            entity->selected = false;
                        }
                    }
                }
            }
            else {
                startDrag = {};
                endDrag = {};

                map.selection.Clear();

                for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
                    Entity * entity = &map.entities[i];
                    if (entity->inUse && entity->playerNumber == gameLocal.playerNumber) {
                        Bounds b = EntityGetSelectionBounds(entity);
                        if (IsPointInBounds(mouseWorld, b)) {
                            entity->selected = true;
                            map.selection.Add(entity->id);
                        }
                        else {
                            entity->selected = false;
                        }
                    }
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            Entity * entity = GetEntityUnderMouse(map, mouseWorld);
            if (entity == nullptr) {
                MapCommand & cmd = gameLocal.mapTurn.cmds.AddEmpty();
                MapCreateCommandMoveSelectedUnits(map, cmd, V2fp(mouseWorld));
            }
            else {
                if (MapSelectionContainsType(map, ENTITY_TYPE_GENERAL) && EntityIsResourceNode(entity)) {
                    MapCommand & cmd = gameLocal.mapTurn.cmds.AddEmpty();
                    MapCreateCommand(map, cmd, MAP_COMMAND_MOVE_UNITS);
                    Bounds bounds = EntityGetSelectionBounds(entity);
                    v2 tAvg = {};
                    for (i32 i = 0; i < map.selection.GetCount(); i++) {
                        Entity * selectedEntity = MapLookUpEntityFromId(map, map.selection[i]);
                        if (selectedEntity && selectedEntity->type == ENTITY_TYPE_GENERAL) {
                            v2 cp = ClosestPointOnBounds(selectedEntity->general.visPos, bounds);
                            v2 dir = Normalize(selectedEntity->general.visPos - cp);
                            v2 t = cp + dir * 30.0f;
                            tAvg = tAvg + t;
                            cmd.entities.Add(selectedEntity->id);
                        }
                    }

                    tAvg = tAvg / (f32)cmd.entities.GetCount();
                    cmd.target = V2fp(tAvg);
                }
            }
        }
    }

    BeginTextureMode(surface);
    BeginMode2D(gameLocal.cam);
    ClearBackground(RAYWHITE);

    for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
        Entity * entity = &map.entities[i];
        if (entity->inUse) {
            switch (entity->type) {
            case ENTITY_TYPE_GENERAL:               DrawGeneral(*entity); break;
            case ENTITY_TYPE_BUILDING_TOWN_CENTER:  DrawTownCenter(*entity); break;
            case ENTITY_TYPE_RESOURCE_NODE_R1:      DrawResource1(*entity); break;
            case ENTITY_TYPE_RESOURCE_NODE_R2:      DrawResource2(*entity); break;
            default:
                break;
            }
        }
    }

#if 0
    for (i32 i = 0; i < MAX_MAP_TILES; i++) {
        MapTile * tile = &map.tiles[i];
        Rectangle r = { tile->visPos.x + 2, tile->visPos.y + 2, MAP_TILE_WIDTH - 2, MAP_TILE_HEIGHT - 2 };
        Color c = tile->isWalkable ? GREEN : RED;
        DrawRectangleRec(r, Fade(c, 0.7f));
        DrawText(TextFormat("%d", tile->flatIndex), (i32)tile->visPos.x, (i32)tile->visPos.y, 10, BLACK);
    }
#endif
#if 0
    for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
        Entity * entity = &map.entities[i];
        if (entity->inUse) {
            Bounds b = EntityGetSelectionBounds(entity);
            DrawBounds(b);
        }
    }
#endif
    if (dragging == true) {
        v2 topLeft = {};
        topLeft.x = Min(startDrag.x, endDrag.x);
        topLeft.y = Min(startDrag.y, endDrag.y);
        v2 bottomRight = {};
        bottomRight.x = Max(startDrag.x, endDrag.x);
        bottomRight.y = Max(startDrag.y, endDrag.y);
        DrawRectangleRec({ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y }, Fade(GREEN, 0.5f));
    }

    //DrawTempCenterText(surfaceWidth, surfaceHeight);

    EndMode2D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    Rectangle r1 = { 0.0f, 0.0f, (float)surface.texture.width, (float)-surface.texture.height };
    Rectangle r2 = {};
    r2.x = (GetScreenWidth() - ((float)surfaceWidth * scale)) * 0.5f;
    r2.y = (GetScreenHeight() - ((float)surfaceHeight * scale)) * 0.5f;
    r2.width = (float)surfaceWidth * scale;
    r2.height = (float)surfaceHeight * scale;
    DrawTexturePro(surface.texture, r1, r2, {}, 0.0f, WHITE);

    UIStateDraw();

    EndDrawing();
}

void DoCameraPanning(Camera2D & cam) {
    f32 speed = 10.0f;
    if (IsKeyDown(KEY_W)) {
        cam.target.y -= speed;
    }
    if (IsKeyDown(KEY_S)) {
        cam.target.y += speed;
    }
    if (IsKeyDown(KEY_A)) {
        cam.target.x -= speed;
    }
    if (IsKeyDown(KEY_D)) {
        cam.target.x += speed;
    }
}

void DoCameraZooming(Camera2D & cam) {
    f32 speed = 0.05f;
    if (IsKeyDown(KEY_Q)) {
        cam.zoom -= speed;
    }
    if (IsKeyDown(KEY_E)) {
        cam.zoom += speed;
    }
    cam.zoom += ((float)GetMouseWheelMove() * speed);
    cam.zoom = Clamp(cam.zoom, 0.5f, 2.0f);
}

*/