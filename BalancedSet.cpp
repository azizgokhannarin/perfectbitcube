#include "BalancedSet.h"

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
