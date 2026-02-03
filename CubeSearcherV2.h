#ifndef CUBESEARCHERV2_H
#define CUBESEARCHERV2_H

#include "BalancedSet.h"
#include <vector>
#include <cstdint>
#include <array>
#include <atomic>
#include <mutex>
#include <fstream>
#include <chrono>

class CubeSearcherV2 {
public:
    CubeSearcherV2(const BalancedSet& bSet);
    
    // Main search method
    void search(int nThreads, bool findOnlyFirst = true);
    
    // Get results
    int getCubeCount() const { return foundCubeCount; }
    long getTotalPathsChecked() const { return totalPathsChecked; }
    long getTotalPermutations() const { return totalPermutations; }
    
    // Get the first found cube (if any)
    const std::array<ShiftSet, 8>* getFirstCube() const { return firstCubeFound ? &firstCubeData : nullptr; }
    
private:
    const BalancedSet& balancedSet;
    std::atomic<int> foundCubeCount{0};
    std::atomic<long> totalPathsChecked{0};
    std::atomic<bool> shouldStop{false};  // Signal to stop search after first found
    long totalPermutations{0};  // Total possible combinations
    
    // First cube data
    bool firstCubeFound{false};
    std::array<ShiftSet, 8> firstCubeData;
    
    std::mutex mtx;
    std::ofstream resultFile;
    
    // Filter rule check: 4 values >=128, 4 values <128, with parity rules
    bool passesFilterRule(const std::array<uint8_t, 8>& row) const;
    
    // Check if row passes parity constraint
    bool checkParityConstraint(const std::array<uint8_t, 8>& row) const;
    
    // Count values in range [128, 255]
    int countUpperHalf(const std::array<uint8_t, 8>& row) const;
    
    // Count even/odd in range [128, 255]
    void countParityUpper(const std::array<uint8_t, 8>& row, 
                         int& evenCount, int& oddCount) const;
    
    // Count even/odd in range [0, 127]
    void countParityLower(const std::array<uint8_t, 8>& row,
                         int& evenCount, int& oddCount) const;
    
    // Recursive search with permutation branching
    void searchRecursive(const ShiftSet& fixedSet,
                        int setIdx,
                        std::vector<uint8_t>& usedNumbers,
                        std::array<ShiftSet, 8>& currentCube,
                        long& localChecked);
    
    // Try to place a shift set at position setIdx
    void tryPlaceSet(const ShiftSet& fixedSet,
                    int setIdx,
                    std::vector<uint8_t>& usedNumbers,
                    std::array<ShiftSet, 8>& currentCube,
                    long& localChecked);
    
    // Calculate total possible permutations
    long calculateTotalPermutations() const;
    
    // Save result to file
    void saveResult(const std::array<ShiftSet, 8>& cube, int resultId);
    
    // Open result file
    void openResultFile();
    void closeResultFile();
    
    // Validate Z-axis constraint (each column must have 4 ones across 8 layers)
    bool validateZAxis(const std::array<ShiftSet, 8>& cube) const;
};

#endif
