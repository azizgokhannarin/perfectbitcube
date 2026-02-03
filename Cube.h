#ifndef CUBE_H
#define CUBE_H

#include <cstdint>
#include <vector>

struct Cube {
    uint8_t data[8][8];
    std::vector<uint8_t> usedNumbers;
};

#endif
