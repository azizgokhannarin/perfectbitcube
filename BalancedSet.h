#ifndef BALANCEDSET_H
#define BALANCEDSET_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>

// Shift rotation set: 8 unique balanced numbers (each shifted by 1 bit)
struct ShiftSet {
    std::array<uint8_t, 8> values;
    uint8_t base;  // Original number before shift
    
    ShiftSet() : base(0) {
        for (int i = 0; i < 8; ++i) values[i] = 0;
    }
};

class BalancedSet {
public:
    BalancedSet();
    
    const std::vector<uint8_t>& getUpSet() const { return upSet; }
    const std::vector<uint8_t>& getAllBalanced() const { return allBalanced; }
    const std::vector<ShiftSet>& getShiftSets() const { return shiftSets; }
    
    uint8_t getComplement(uint8_t val) const;
    bool isBalanced(uint8_t val) const;
    
    // New method: rotate left by 1 bit
    uint8_t rotateLeft(uint8_t val) const;
    
    // Check if 8 rotations are all unique
    bool isValidShiftSet(uint8_t base) const;

private:
    std::vector<uint8_t> allBalanced;
    std::vector<uint8_t> upSet;
    std::vector<ShiftSet> shiftSets;  // Valid shift sets
    std::vector<ShiftSet> filteredShiftSets;  // Sets that pass filter rule
    uint8_t complementMap[256];
    
    int countSetBits(uint8_t n) const;
    void generateShiftSets();
    bool passesFilterRule(const std::array<uint8_t, 8>& values) const;

public:
    const std::vector<ShiftSet>& getFilteredShiftSets() const { return filteredShiftSets; }
};

#endif
