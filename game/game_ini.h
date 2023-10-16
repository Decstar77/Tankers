#pragma once

struct ConfigEntry {
    char key[256];
    char value[256];
};

struct Config {
    ConfigEntry entries[256];
    int entryCount;
};

bool ReadEntireConfigFile(const char * filePath, Config &cfg);



