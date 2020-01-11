#include "string_builder.h"
#include <fstream>

using namespace Beatmup;

void StringBuilder::dump(std::string filename) {
    std::ofstream file(filename, std::ios::out);
    file << str;
}