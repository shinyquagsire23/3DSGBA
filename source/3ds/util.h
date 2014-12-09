#ifndef __UTIL_H__
#define __UTIL_H__

#include <vector>

bool isDirectory(char* path);
std::vector<char*>* getDirectoryContents(const char* directory, const char* extensionFilter);

#endif