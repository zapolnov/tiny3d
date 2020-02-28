#pragma once

#include <string>
#include <sstream>

bool loadBinaryFile(const std::string& fileName, std::stringstream& output);
bool writeTextFile(const std::string& fileName, std::stringstream&& contents);
