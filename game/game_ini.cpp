#include "game_ini.h"

#include <stdio.h>
#include <cstring>

// Ignore C4996
#pragma warning(disable : 4996)

static void RemoveWhiteSpace(char * str) {
    char * write = str;
    char * read = str;
    while (*read) {
        if (*read != ' ' && *read != '\t' && *read != '\r' && *read != '\n') {
            *write = *read;
            ++write;
        }
        ++read;
    }
    *write = 0;
}

bool ReadEntireConfigFile(const char * filePath, Config &cfg) {
    FILE * file = fopen(filePath, "r");
    if (file == nullptr) {
        printf("Failed to open config file: %s\n", filePath);
        return false;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (cfg.entryCount >= sizeof(cfg.entries) / sizeof(cfg.entries[0])) {
            break;
        }

        char * key = line;
        char * value = nullptr;
        for (char * c = line; *c; ++c) {
            if (*c == '=') {
                *c = 0;
                value = c + 1;
                break;
            }
        }

        if (value == nullptr) {
            continue;
        }

        while (*value && (*value == ' ' || *value == '\t')) {
            ++value;
        }

        char * end = value + strlen(value) - 1;
        while (end > value && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
            *end = 0;
            --end;
        }

        if (*value == 0) {
            continue;
        }

        ConfigEntry &entry = cfg.entries[cfg.entryCount++];
        strncpy(entry.key, key, sizeof(entry.key));
        strncpy(entry.value, value, sizeof(entry.value));

        RemoveWhiteSpace(entry.key);
        RemoveWhiteSpace(entry.value);
    }

    fclose(file);
    return true;
}

#pragma warning(default : 4996)