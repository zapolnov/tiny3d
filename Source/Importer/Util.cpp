#include "Util.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

bool loadBinaryFile(const std::string& fileName, std::stringstream& output)
{
    FILE* f = fopen(fileName.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "Unable to open file \"%s\": %s\n", fileName.c_str(), strerror(errno));
        return false;
    }

    while (!feof(f)) {
        char buf[16384];
        size_t bytesRead = fread(buf, 1, sizeof(buf), f);
        if (ferror(f)) {
            fprintf(stderr, "Unable to read file \"%s\": %s\n", fileName.c_str(), strerror(errno));
            fclose(f);
            return false;
        }
        output.write(buf, bytesRead);
    }

    fclose(f);
    return true;
}

bool writeTextFile(const std::string& fileName, std::stringstream&& contents)
{
    FILE* f = fopen(fileName.c_str(), "w");
    if (!f) {
        fprintf(stderr, "Unable to write file \"%s\": %s\n", fileName.c_str(), strerror(errno));
        return false;
    }

    std::string s = contents.str();
    fwrite(s.data(), s.size(), 1, f);
    if (ferror(f)) {
        fprintf(stderr, "Unable to write file \"%s\": %s\n", fileName.c_str(), strerror(errno));
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}
