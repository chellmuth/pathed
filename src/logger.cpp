#include "logger.h"

#include "globals.h"
#include "job.h"

#include <sstream>

void Logger::line(const std::string &line)
{
    std::string directory = g_job->outputDirectory();

    std::ostringstream lineStream;
    lineStream << "[" << directory << "] " << line;

    std::cout << lineStream.str() << std::endl;
}
