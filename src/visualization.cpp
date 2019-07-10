#include "visualization.h"

#include "globals.h"
#include "job.h"

#include <dirent.h>
#include <iostream>
#include <stdlib.h>

std::vector<std::string> visualization::files()
{
    std::vector<std::string> results;

    std::string directory = g_job->visualizationDirectory();

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(directory.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) {
                std::string filename(ent->d_name);
                results.push_back(filename);
            }
        }

        closedir(dir);
    } else {
        printf("Could not read directory: %s\n", directory.c_str());
        exit(1);
    }

    return results;
}
