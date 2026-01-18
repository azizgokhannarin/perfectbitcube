#ifndef BALANCEDSET_H
#define BALANCEDSET_H

#include <vector>
#include <cstdint>
#include <algorithm>

class BalancedSet {
public:
    BalancedSet();
    
    const std::vector<uint8_t>& getUpSet() const { return upSet; }
    const std::vector<uint8_t>& getAllBalanced() const { return allBalanced; }
    uint8_t getComplement(uint8_t val) const;
    bool isBalanced(uint8_t val) const;

private:
    std::vector<uint8_t> allBalanced;
    std::vector<uint8_t> upSet;
    uint8_t complementMap[256];
    int countSetBits(uint8_t n) const;
};

#endif
