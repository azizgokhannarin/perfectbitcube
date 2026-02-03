#ifndef LAYERGENERATOR_H
#define LAYERGENERATOR_H

#include "BalancedSet.h"
#include "Layer.h"
#include <chrono>

class LayerGenerator {
public:
    LayerGenerator(const BalancedSet& bSet);
    void generate();
    const std::vector<Layer>& getValidLayers() const { return validLayers; }

private:
    const BalancedSet& balancedSet;
    std::vector<Layer> validLayers;

    // Z-axis: Column bit counters (vertical through layer)
    int colCounts[8];
    
    // Y-axis: Bit position counters (same bit across all 8 rows)
    int yAxisCounts[8];
    
    // Statistics
    uint64_t totalAttempts;

    void backtrack(int rowIdx, uint8_t currentRows[8], uint64_t usedMask);
    bool canAddRow(uint8_t row, int rowIdx);
    void updateCounts(uint8_t row, int delta);
};

#endif
