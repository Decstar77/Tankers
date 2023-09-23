#include "game_local.h"
#include "game_client.h"


#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <cstring>

static UIState uiState = {};

void DoScreenMainMenu(GameLocal & gameLocal) {
    BeginDrawing();
    ClearBackground(Fade(BLUE, 0.5f));

    uiState.surfaceMouse = { GetMousePosition().x, GetMousePosition().y };
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

            map.selectionCount = 0;

            startDrag = {};
            endDrag = {};
            for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
                Entity * entity = &map.entities[i];
                if (entity->inUse && entity->playerNumber == gameLocal.playerNumber) {
                    if (entity->type == ENTITY_TYPE_GENERAL) {
                        Bounds c = EntityGetSelectionBounds(entity);
                        if (c.type == BOUNDS_TYPE_CIRCLE && CircleVsRect(c.circle, selectionRect)) {
                            entity->selected = true;
                            map.selection[map.selectionCount] = entity->id;
                            map.selectionCount++;
                        }
                        else {
                            entity->selected = false;
                        }
                    } else {
                        entity->selected = false;
                    }
                }
            }
        }
        else {
            map.selectionCount = 0;
            startDrag = {};
            endDrag = {};

            for (i32 i = 0; i < MAX_MAP_ENTITIES; i++) {
                Entity * entity = &map.entities[i];
                if (entity->inUse && entity->playerNumber == gameLocal.playerNumber) {
                    Bounds b = EntityGetSelectionBounds(entity);
                    if (b.type == BOUNDS_TYPE_CIRCLE && CircleVsCircle(b.circle, { mouseWorld, 10 })) {
                        entity->selected = true;
                        map.selection[map.selectionCount] = entity->id;
                        map.selectionCount++;
                    }
                    else if (b.type == BOUNDS_TYPE_RECT && IsPointInRect(mouseWorld, b.rect)) {
                        entity->selected = true;
                        map.selection[map.selectionCount] = entity->id;
                        map.selectionCount++;
                    }
                    else {
                        entity->selected = false;
                    }
                }
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
        MapCreateCommandMoveSelectedUnits(map, gameLocal.mapTurn.cmds[gameLocal.mapTurn.commandCount++], V2fp(mouseWorld));
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

    EndDrawing();
}

void UIColorsCreate() {
    uiState.uiColors.counts[UI_COLOR_SLOT_BACKGROUND] = 1;
    uiState.uiColors.stacks[UI_COLOR_SLOT_BACKGROUND][0] = LIGHTGRAY;
}

void UIColorsPush(UIColorSlot slot, Color c) {
    Assert(uiState.uiColors.counts[slot] < ArrayCount(uiState.uiColors.stacks[slot]));
    Assert(slot != UI_COLOR_SLOT_INVALID);
    uiState.uiColors.stacks[slot][uiState.uiColors.counts[slot]] = c;
    uiState.uiColors.counts[slot]++;
}

void UIColorsPop(UIColorSlot slot) {
    Assert(uiState.uiColors.counts[slot] > 0);
    Assert(slot != UI_COLOR_SLOT_INVALID);
    uiState.uiColors.counts[slot]--;
}

Color UIColorsGet(UIColorSlot slot) {
    Assert(uiState.uiColors.counts[slot] > 0);
    Assert(slot != UI_COLOR_SLOT_INVALID);
    return uiState.uiColors.stacks[slot][uiState.uiColors.counts[slot] - 1];
}

bool UIDrawButtonCenter(i32 centerX, i32 centerY, const char * text, Rectangle * bb) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
    Rectangle rect = { (float)centerX - rectSize.x / 2, (float)centerY - rectSize.y / 2, rectSize.x, rectSize.y };

    DrawRectangleRec(rect, UIColorsGet(UI_COLOR_SLOT_BACKGROUND));

    if (bb != nullptr) {
        *bb = rect;
    }

    Vector2 textPos = { rect.x + rect.width / 2 - textSize.x / 2, rect.y + rect.height / 2 - textSize.y / 2 };
    DrawTextEx(GetFontDefault(), text, textPos, 20, 1, BLACK);

    Vector2 mousePos = { uiState.surfaceMouse.x, uiState.surfaceMouse.y };
    if (CheckCollisionPointRec(mousePos, rect)) {
        DrawRectangleLinesEx(rect, 2, RED);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
}

bool UIDrawButtonTopLeft(i32 centerX, i32 centerY, const char * text, Rectangle * bb) {
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
    Vector2 rectSize = { textSize.x + 20, textSize.y + 20 };
    Rectangle rect = { (float)centerX, (float)centerY, rectSize.x, rectSize.y };

    DrawRectangleRec(rect, UIColorsGet(UI_COLOR_SLOT_BACKGROUND));

    if (bb != nullptr) {
        *bb = rect;
    }

    Vector2 textPos = { rect.x + rect.width / 2 - textSize.x / 2, rect.y + rect.height / 2 - textSize.y / 2 };
    DrawTextEx(GetFontDefault(), text, textPos, 20, 1, BLACK);

    Vector2 mousePos = { uiState.surfaceMouse.x, uiState.surfaceMouse.y };
    if (CheckCollisionPointRec(mousePos, rect)) {
        DrawRectangleLinesEx(rect, 2, RED);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    return false;
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
