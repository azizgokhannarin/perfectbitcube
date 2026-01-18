#ifndef CUBE_H
#define CUBE_H

#include <cstdint>
#include <vector>

struct Cube {
    // data[z][y] -> satırdaki 8 bit (x ekseni)
    uint8_t data[8][8];

    // Küpün içindeki tüm benzersiz sayıları (0-255) saklamak için (Analiz amaçlı)
    std::vector<uint8_t> usedNumbers;
};

#endif
