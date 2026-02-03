#ifndef CUBEASSEMBLER_H
#define CUBEASSEMBLER_H

#include "Layer.h"
#include "Cube.h"
#include "BalancedSet.h"
#include <vector>
#include <mutex>
#include <atomic>
#include <fstream>
#include <bitset>

class CubeAssembler
{
public:
    CubeAssembler(const BalancedSet &bSet);
    void assembleParallel(const std::vector<Layer> &layers, int nThreads);

private:
    const BalancedSet &balancedSet;
    std::mutex mtx;
    std::atomic<long> checkedPaths{0};
    std::atomic<int> foundCount{0};

    void searchWithLookup(const std::vector<Layer> &layers,
                          const std::vector<std::vector<int>> &lookup,
                          int layerStartIdx,
                          int currentZ,
                          uint64_t zCounts[3],
                          uint64_t currentCubeMask[4],
                          uint8_t currentCubeRows[4][8],
                          long &localChecked);
    void saveToDisk(const Cube &cube, int id);
};

#endif
