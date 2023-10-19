#include "game_client.h"
#include "game_local.h"
#include "game_ini.h"
#include "game_settings.h"

#include "rendering/game_rendering.h"

#include "../vendor/glm/glm/glm.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/glm/gtc/type_ptr.hpp"

#include "../vendor/glad/glad.h"
#include "../vendor/glfw/include/glfw/glfw3.h"

static GLFWmonitor * monitor = nullptr;
static LargeString                 monitorName = LargeString::FromLiteral("");
static f64                         monitorRefreshRate = 0;
static GLFWwindow * window = nullptr;
static i32                         windowWidth = 0;
static i32                         windowHeight = 0;
static f32                         windowAspect = 0;
static SmallString                 windowTitle = SmallString::FromLiteral("Game");
static bool                        windowFullscreen = false;
static bool                        shouldClose = false;

int main(int argc, char * argv[]) {

    const char * iniPath = nullptr;
    if (argc == 2) {
        iniPath = argv[1];
    }

    GameSettings gameSettings = CreateDefaultGameSettings();
    if (iniPath != nullptr) {
        PlatformPrint("Using ini file: %s\n", iniPath);
        static Config cfg = {};
        ReadEntireConfigFile(iniPath, cfg);
        gameSettings = ParseConfigFileForGameSettings(cfg);
    }

    PlatformPrint("width: %d\n", gameSettings.width);
    PlatformPrint("height: %d\n", gameSettings.height);
    PlatformPrint("posX: %d\n", gameSettings.posX);
    PlatformPrint("posY: %d\n", gameSettings.posY);
    PlatformPrint("using server: %s\n", gameSettings.serverIp);

    if (glfwInit() == GLFW_FALSE) {
        PlatformPrint("Failed to init glfw\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

#if ATTO_DEBUG_RENDERING
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    monitor = glfwGetPrimaryMonitor();
    if (monitor != nullptr) {
        const GLFWvidmode * videoMode = glfwGetVideoMode(monitor);
        monitorRefreshRate = videoMode->refreshRate;
        monitorName = glfwGetMonitorName(monitor);
    }

    windowWidth = gameSettings.width;
    windowHeight = gameSettings.height;

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.GetCStr(), windowFullscreen ? monitor : nullptr, 0);

    if (window == nullptr) {
        PlatformPrint("Failed to create window\n");
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // glfwSetCursorPosCallback(window, MousePositionCallback);
    // glfwSetKeyCallback(window, KeyCallback);
    // glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // glfwSetScrollCallback(window, ScrollCallback);
    // glfwSetFramebufferSizeCallback(window, FramebufferCallback);

    if (gameSettings.posX != -1) {
        glfwSetWindowPos(window, gameSettings.posX, gameSettings.posY);
    }

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    PlatformPrint("OpenGL %s, GLSL %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    i32 w = 0;
    i32 h = 0;
    glfwGetFramebufferSize(window, &w, &h);

    InitRendering((f32)w, (f32)h);

    static DrawCommands drawCommands = {};

    f64 currentTime = glfwGetTime();
    f64 deltaTime = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.4f, 1);

        FontRenderSomeText((f32)w, (f32)h);

        drawCommands.commands.Clear();
        DrawRect(drawCommands, glm::vec2(0, 0), glm::vec2(100, 100), 0, glm::vec4(1, 0, 0, 1));

        RenderCommit(drawCommands);

        glfwSwapBuffers(window);
        f64 endTime = glfwGetTime();
        deltaTime = endTime - currentTime;
        currentTime = endTime;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}

/*
#include "../shared/shared_game.h"

#include "game_client.h"
#include "game_local.h"
#include "game_ini.h"
#include "game_ui.h"
#include "game_settings.h"

#include "../vendor/raylib/include/raylib.h"

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <cstring>

static f32 tempTextTimer = 0.0f;
static const char * tempCeterText = "";
static void SetTempCenterText(const char * text, f32 time) {
    tempTextTimer = time;
    tempCeterText = text;
}

static void DrawTempCenterText(i32 w, i32 h) {
    if (tempTextTimer > 0.0f) {
        tempTextTimer -= GetFrameTime();
        DrawText(tempCeterText, w / 2 - MeasureText(tempCeterText, 20) / 2, h / 2 - 20 / 2, 20, BLACK);
    }
    else {
        tempCeterText = "";
    }
}

struct UIToolBar {
    bool vertical;
    i32 currentP;
};

static bool ToolBarButton(UIToolBar & tb, const char * text) {
    bool res = false;
    if (tb.vertical) {
        res = UIDrawButtonTopLeft(0, tb.currentP, text);
        tb.currentP += 50;
        return res;
    }
    else {
        Rectangle bb = {};
        res = UIDrawButtonTopLeft(tb.currentP, 0, text, &bb);
        tb.currentP += (i32)bb.width;
    }

    return res;
}

enum LevelEditorToolMode {
    LEVEL_EDITOR_TOOL_MODE_INVALID = 0,
    LEVEL_EDITOR_TOOL_MODE_TILE,
    LEVEL_EDITOR_TOOL_MODE_P1,
    LEVEL_EDITOR_TOOL_MODE_P2,
    LEVEL_EDITOR_TOOL_MODE_BROWN_ENEMY,
    LEVEL_EDITOR_TOOL_MODE_COUNT,
};

struct LevelEditor {
    std::string mapName;
    LevelEditorToolMode toolMode;
    UIToolBar menuTB;
};

static void ToolBarButtonEditorMode(LevelEditor & editor, LevelEditorToolMode mode, const char * text) {
    UIColorsPush(UI_COLOR_SLOT_BACKGROUND, Fade(ORANGE, 0.5f));
    if (editor.toolMode == mode) {
        UIColorsPush(UI_COLOR_SLOT_BACKGROUND, Fade(GREEN, 0.5f));
        if (ToolBarButton(editor.menuTB, text)) {
            editor.toolMode = mode;
        }
        UIColorsPop(UI_COLOR_SLOT_BACKGROUND);
    }
    else {
        if (ToolBarButton(editor.menuTB, text)) {
            editor.toolMode = mode;
        }
    }
    UIColorsPop(UI_COLOR_SLOT_BACKGROUND);
}

#include "../vendor/fixed/Fixed64.h"

int main(int argc, char * argv[]) {

    const char * iniPath = nullptr;
    if (argc == 2) {
        iniPath = argv[1];
    }

    GameSettings gameSettings = CreateDefaultGameSettings();
    if (iniPath != nullptr) {
        printf("Using ini file: %s\n", iniPath);
        static Config cfg = {};
        ReadEntireConfigFile(iniPath, cfg);
        gameSettings = ParseConfigFileForGameSettings(cfg);
    }

    printf("width: %d\n", gameSettings.width);
    printf("height: %d\n", gameSettings.height);
    printf("posX: %d\n", gameSettings.posX);
    printf("posY: %d\n", gameSettings.posY);
    printf("using server: %s\n", gameSettings.serverIp);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(gameSettings.width, gameSettings.height, "Yes. This is a game you slut!");

    SetWindowPosition(gameSettings.posX, gameSettings.posY);

    bool recreateSufrace = false;
    i32 surfaceWidth = 1280;
    i32 surfaceHeight = 720;

    RenderTexture2D surface = LoadRenderTexture(surfaceWidth, surfaceHeight);
    SetTextureFilter(surface.texture, TEXTURE_FILTER_BILINEAR);

    UIColorsCreate();

    static GameLocal gameLocal = {};
    gameLocal.gameSettings = gameSettings;
    gameLocal.cam.zoom = 1.0f;
    gameLocal.cam.offset = { surfaceWidth / 2.0f, surfaceHeight / 2.0f };

    SetTargetFPS(60);
    while (!WindowShouldClose() && gameLocal.running) {
        switch (gameLocal.screen) {
        case SCREEN_TYPE_MAIN_MENU: DoScreenMainMenu(gameLocal); break;
        case SCREEN_TYPE_GAME:      DoScreenGame(gameLocal, surfaceWidth, surfaceHeight, surface); break;
        default: Assert(false); break;
        }
    }

    NetoworkDisconnectFromServer();

    CloseWindow();

    return 0;
}
*/