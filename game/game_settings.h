#pragma once

#include "game_ini.h"

struct GameSettings {
    int width;
    int height;
    int posX;
    int posY;
    bool fullscreen;
    char serverIp[64];
    int serverPort;
};

GameSettings ParseConfigFileForGameSettings(Config & config);
GameSettings CreateDefaultGameSettings();