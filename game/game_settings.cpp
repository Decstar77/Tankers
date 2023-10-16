#include "game_settings.h"

#include <stdio.h>
#include <string>

GameSettings ParseConfigFileForGameSettings(Config & config) {
    GameSettings settings = {};
    for (int i = 0; i < config.entryCount; i++) {
        if (strcmp(config.entries[i].key, "window_x") == 0) {
            settings.posX = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_y") == 0) {
            settings.posY = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_width") == 0) {
            settings.width = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_height") == 0) {
            settings.height = atoi(config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "window_fullscreen") == 0) {
            settings.fullscreen = atoi(config.entries[i].value) != 0;
        }
        else if (strcmp(config.entries[i].key, "server_ip") == 0) {
            strcpy_s(settings.serverIp, config.entries[i].value);
        }
        else if (strcmp(config.entries[i].key, "server_port") == 0) {
            settings.serverPort = atoi(config.entries[i].value);
        }
    }

    return settings;
}

GameSettings CreateDefaultGameSettings() {
    GameSettings settings = {};
    settings.width = 1280;
    settings.height = 720;
    settings.posX = 200;
    settings.posY = 200;
    settings.fullscreen = false;
    settings.serverPort = 27164;
    strcpy_s(settings.serverIp, "127.0.0.1");

    return settings;
};
