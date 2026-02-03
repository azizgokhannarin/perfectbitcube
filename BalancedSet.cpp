#include "BalancedSet.h"
#include <iostream>

BalancedSet::BalancedSet()
{
    for (int i = 0; i < 256; ++i) {
        if (countSetBits(static_cast<uint8_t>(i)) == 4) {
            uint8_t val = static_cast<uint8_t>(i);
            allBalanced.push_back(val);
            if (val >= 128) upSet.push_back(val);
            complementMap[val] = ~val;
        }
    }
    std::sort(upSet.rbegin(), upSet.rend());
    
    // Generate shift sets
    generateShiftSets();
}

int BalancedSet::countSetBits(uint8_t n) const
{
    int count = 0;
    while (n > 0) { n &= (n - 1); count++; }
    return count;
}

uint8_t BalancedSet::getComplement(uint8_t val) const
{
    return complementMap[val];
}

bool BalancedSet::isBalanced(uint8_t val) const
{
    int count = 0;
    uint8_t temp = val;
    while (temp > 0) { temp &= (temp - 1); count++; }
    return count == 4;
}

uint8_t BalancedSet::rotateLeft(uint8_t val) const
{
    // Rotate left by 1 bit with wrap-around
    return static_cast<uint8_t>((val << 1) | (val >> 7));
}

bool BalancedSet::isValidShiftSet(uint8_t base) const
{
    // Check if 8 left rotations produce all unique balanced numbers
    std::array<uint8_t, 8> rotations;
    uint8_t current = base;
    
    for (int i = 0; i < 8; ++i) {
        rotations[i] = current;
        current = rotateLeft(current);
    }
    
    // Check uniqueness - all must be different
    for (int i = 0; i < 8; ++i) {
        for (int j = i + 1; j < 8; ++j) {
            if (rotations[i] == rotations[j]) return false;
        }
    }
    
    return true;
}

void BalancedSet::generateShiftSets()
{
    std::cout << "[BalancedSet] Generating shift sets..." << std::endl;
    
    for (uint8_t balanced : allBalanced) {
        if (isValidShiftSet(balanced)) {
            ShiftSet ss;
            ss.base = balanced;
            uint8_t current = balanced;
            
            for (int i = 0; i < 8; ++i) {
                ss.values[i] = current;
                current = rotateLeft(current);
            }
            
            shiftSets.push_back(ss);
            
            // Also add to filtered sets if it passes filter
            if (passesFilterRule(ss.values)) {
                filteredShiftSets.push_back(ss);
            }
        }
    }
    
    std::cout << "[BalancedSet] Found " << shiftSets.size() << " valid shift sets" << std::endl;
    std::cout << "[BalancedSet] Filtered to " << filteredShiftSets.size() << " passing filter rule" << std::endl;
}

bool BalancedSet::passesFilterRule(const std::array<uint8_t, 8>& values) const
{
    // Rule 1: 4 values >= 128, 4 values < 128
    int upperCount = 0;
    for (uint8_t val : values) {
        if (val >= 128) upperCount++;
    }
    if (upperCount != 4) return false;
    
    // Rule 2: Upper group (>=128): 2 even + 2 odd
    int upperEven = 0, upperOdd = 0;
    for (uint8_t val : values) {
        if (val >= 128) {
            if (val % 2 == 0) upperEven++;
            else upperOdd++;
        }
    }
    if (upperEven != 2 || upperOdd != 2) return false;
    
    // Rule 3: Lower group (<128): 2 even + 2 odd
    int lowerEven = 0, lowerOdd = 0;
    for (uint8_t val : values) {
        if (val < 128) {
            if (val % 2 == 0) lowerEven++;
            else lowerOdd++;
        }
    }
    if (lowerEven != 2 || lowerOdd != 2) return false;
    
    return true;
}

