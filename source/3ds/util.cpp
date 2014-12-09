#include <stdio.h>
#include <string.h>
#include <sys/dirent.h>
#include <algorithm>

#include "util.h"

struct alphabetize {
    inline bool operator() (char* a, char* b) {
        return strcasecmp(a, b) < 0;
    }
};

bool isDirectory(char* path) {
    DIR *dir = opendir(path);
    if(!dir) {
        return false;
    }

    closedir(dir);
    return true;
}

std::vector<char*>* getDirectoryContents(const char* directory, const char* extensionFilter) {
    std::vector<char*>* contents = new std::vector<char*>();
    char slash[strlen(directory) + 2];
    snprintf(slash, sizeof(slash), "%s/", directory);
    DIR *dir = opendir(slash);
    if(dir != NULL) {
        for(int i = 0; i < 256; i++) {
            struct dirent *ent = readdir(dir);
            if(ent == NULL) {
                break;
            }

            char path[strlen(directory) + strlen(ent->d_name) + 2];
            snprintf(path, strlen(directory) + strlen(ent->d_name) + 2, "%s/%s", directory, ent->d_name);
            if(isDirectory(path)) {
                contents->push_back(strdup(ent->d_name));
            } else {
                const char *dot = strrchr(path, '.');
                if(dot && dot != path && strcmp(dot + 1, extensionFilter) == 0) {
                    contents->push_back(strdup(ent->d_name));
                }
            }
        }

        closedir(dir);
        if(std::find(contents->begin(), contents->end(), "..") == contents->end()) {
            contents->push_back((char*) ".");
            contents->push_back((char*) "..");
        }

        std::sort(contents->begin(), contents->end(), alphabetize());
    } else {
        return NULL;
    }

    return contents;
}