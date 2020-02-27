#include "Util.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

bool writeFile(std::string fileName, std::stringstream&& contents)
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
