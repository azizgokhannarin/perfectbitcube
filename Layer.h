#ifndef LAYER_H
#define LAYER_H

#include <cstdint>
#include <vector>

struct Layer {
    uint8_t rows[8];
    std::vector<uint8_t> uniqueNumbers; 
    uint64_t bitMatrix;
    uint64_t numMask[4];
    
    Layer() : bitMatrix(0) {
        for(int i=0; i<4; ++i) numMask[i] = 0;
    }
};

#endif
