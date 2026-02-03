#include "CubeSearcherV2.h"
#include <iostream>
#include <thread>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <sstream>

CubeSearcherV2::CubeSearcherV2(const BalancedSet& bSet)
    : balancedSet(bSet), totalPathsChecked(0), totalPermutations(0) {
    openResultFile();
}

int CubeSearcherV2::countUpperHalf(const std::array<uint8_t, 8>& row) const
{
    int count = 0;
    for (uint8_t val : row) {
        if (val >= 128) count++;
    }
    return count;
}

void CubeSearcherV2::countParityUpper(const std::array<uint8_t, 8>& row,
                                      int& evenCount, int& oddCount) const
{
    evenCount = 0;
    oddCount = 0;
    for (uint8_t val : row) {
        if (val >= 128) {
            if (val % 2 == 0) evenCount++;
            else oddCount++;
        }
    }
}

void CubeSearcherV2::countParityLower(const std::array<uint8_t, 8>& row,
                                      int& evenCount, int& oddCount) const
{
    evenCount = 0;
    oddCount = 0;
    for (uint8_t val : row) {
        if (val < 128) {
            if (val % 2 == 0) evenCount++;
            else oddCount++;
        }
    }
}

bool CubeSearcherV2::checkParityConstraint(const std::array<uint8_t, 8>& row) const
{
    int upperEven, upperOdd, lowerEven, lowerOdd;
    countParityUpper(row, upperEven, upperOdd);
    countParityLower(row, lowerEven, lowerOdd);
    
    // >= 128 group: 2 even + 2 odd
    if (upperEven != 2 || upperOdd != 2) return false;
    
    // < 128 group: 2 odd + 2 even
    if (lowerOdd != 2 || lowerEven != 2) return false;
    
    return true;
}

bool CubeSearcherV2::passesFilterRule(const std::array<uint8_t, 8>& row) const
{
    // Rule 1: 4 values >= 128, 4 values < 128
    int upperCount = countUpperHalf(row);
    if (upperCount != 4) return false;
    
    // Rule 2: Parity constraints
    if (!checkParityConstraint(row)) return false;
    
    return true;
}

void CubeSearcherV2::search(int nThreads, bool findOnlyFirst)
{
    // Use filtered shift sets to reduce search space
    const auto& shiftSets = balancedSet.getFilteredShiftSets();
    int numSets = shiftSets.size();
    
    if (numSets == 0) {
        std::cout << "[CubeSearcherV2] ERROR: No filtered shift sets available!" << std::endl;
        return;
    }
    
    // Set the stop flag based on parameter
    if (findOnlyFirst) {
        shouldStop = true;
    }
    
    // Calculate total possible permutations
    totalPermutations = calculateTotalPermutations();
    
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "[PHASE 3] FILTERED PERMUTATION SEARCH" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "[INFO] Mode: " << (findOnlyFirst ? "FIND FIRST ONLY" : "FIND ALL") << std::endl;
    std::cout << "[INFO] Filtered shift sets to process: " << numSets << std::endl;
    std::cout << "[INFO] CPU threads available: " << nThreads << std::endl;
    std::cout << "[INFO] Permutation depth: 8 levels (Set 1 fixed, Sets 2-8 from filtered)" << std::endl;
    std::cout << "[INFO] Total permutations to check: " << totalPermutations << std::endl;
    std::cout << "[INFO] Memory usage: ~" << (numSets * 64 / 1024) << " MB estimated" << std::endl;
    std::cout << std::string(70, '=') << std::endl << std::endl;
    
    resultFile << "================================================\n";
    resultFile << "Perfect Bit Cube Search Results\n";
    resultFile << "Mode: " << (findOnlyFirst ? "FIND FIRST ONLY" : "FIND ALL") << "\n";
    resultFile << "Start time: " << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
    resultFile << "Total permutations: " << totalPermutations << "\n";
    resultFile << "Filtered shift sets: " << numSets << "\n";
    resultFile << "================================================\n\n";
    resultFile.flush();
    
    auto startTime = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    
    int chunkSize = (numSets + nThreads - 1) / nThreads;
    
    std::atomic<int> processedSets{0};
    std::atomic<long> totalLocalChecked{0};
    
    for (int t = 0; t < nThreads; ++t) {
        threads.emplace_back([this, &shiftSets, t, chunkSize, numSets, &processedSets, 
                             &totalLocalChecked, startTime]() {
            int start = t * chunkSize;
            int end = std::min(start + chunkSize, numSets);
            long localChecked = 0;
            
            for (int i = start; i < end; ++i) {
                // Exit early if we found the first cube and should stop
                if (firstCubeFound && shouldStop) {
                    break;
                }
                
                const ShiftSet& fixedSet = shiftSets[i];
                std::vector<uint8_t> usedNumbers;
                std::array<ShiftSet, 8> cube;
                
                cube[0] = fixedSet;
                usedNumbers.push_back(fixedSet.base);
                
                searchRecursive(fixedSet, 1, usedNumbers, cube, localChecked);
                
                processedSets++;
                totalLocalChecked += localChecked;
                localChecked = 0;
                
                if (processedSets % 1 == 0) {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        now - startTime).count();
                    
                    long checkedSoFar = totalLocalChecked.load();
                    double progress = totalPermutations > 0 ? 
                        (double)checkedSoFar / totalPermutations * 100.0 : 0;
                    double speed = (elapsed > 0) ? (checkedSoFar / (double)elapsed / 1000000.0) : 0;
                    
                    double eta_seconds = 0;
                    if (speed > 0 && progress > 0.1) {
                        double remaining = totalPermutations - checkedSoFar;
                        eta_seconds = remaining / (speed * 1000000.0);
                    }
                    
                    int eta_hours = (int)(eta_seconds / 3600);
                    int eta_mins = ((int)eta_seconds % 3600) / 60;
                    int eta_secs = (int)eta_seconds % 60;
                    
                    int elapsed_hours = (int)(elapsed / 3600);
                    int elapsed_mins = (elapsed % 3600) / 60;
                    int elapsed_secs = elapsed % 60;
                    
                    std::lock_guard<std::mutex> lock(mtx);
                    std::cout << "\r[PROGRESS] " << std::fixed << std::setprecision(2) 
                              << progress << "% | Checked: " << std::scientific << (double)checkedSoFar
                              << " / " << (double)totalPermutations
                              << std::fixed << " | Speed: " << std::setprecision(2) << speed << "M/s"
                              << " | Found: " << foundCubeCount.load()
                              << " | Elapsed: " << std::setw(2) << std::setfill('0') 
                              << elapsed_hours << ":" << elapsed_mins << ":" << elapsed_secs;
                    if (eta_seconds > 0 && eta_hours >= 0) {
                        std::cout << " | ETA: " << eta_hours << "h " << eta_mins << "m " << eta_secs << "s";
                    }
                    std::cout << std::setfill(' ') << std::flush;
                }
            }
        });
    }
    
    for (auto& th : threads) th.join();
    
    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    auto minutes = elapsed / 60;
    auto seconds = elapsed % 60;
    
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "[COMPLETE] Search finished!" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "[RESULT] Time: " << minutes << "m " << seconds << "s" << std::endl;
    std::cout << "[RESULT] Total permutations: " << totalPermutations << std::endl;
    std::cout << "[RESULT] Total permutations checked: " << totalLocalChecked.load() << std::endl;
    std::cout << "[RESULT] Progress: " << (totalPermutations > 0 ? 
        (double)totalLocalChecked.load() / totalPermutations * 100.0 : 0) << "%" << std::endl;
    std::cout << "[RESULT] Perfect cubes found: " << foundCubeCount.load() << std::endl;
    
    if (foundCubeCount.load() > 0) {
        std::cout << "[SUCCESS] ✓ Found " << foundCubeCount.load() << " solution(s)!" << std::endl;
    } else {
        std::cout << "[STATUS] No perfect cube found in this run." << std::endl;
    }
    std::cout << std::string(70, '=') << std::endl;
    
    // Save summary to file
    resultFile << "\n================================================\n";
    resultFile << "FINAL RESULTS\n";
    resultFile << "================================================\n";
    resultFile << "Total time: " << minutes << "m " << seconds << "s\n";
    resultFile << "Total permutations checked: " << totalLocalChecked.load() << " / " << totalPermutations << "\n";
    resultFile << "Progress: " << (totalPermutations > 0 ? 
        (double)totalLocalChecked.load() / totalPermutations * 100.0 : 0) << "%\n";
    resultFile << "Perfect cubes found: " << foundCubeCount.load() << "\n";
    resultFile << "================================================\n";
    resultFile.flush();
    closeResultFile();
}

bool CubeSearcherV2::validateZAxis(const std::array<ShiftSet, 8>& cube) const
{
    // Validate Z-axis: Each (row, bit_position) must have exactly 4 ones across 8 layers
    for (int row = 0; row < 8; ++row) {
        for (int bit_pos = 0; bit_pos < 8; ++bit_pos) {
            int bit_count = 0;
            for (int layer = 0; layer < 8; ++layer) {
                uint8_t val = cube[layer].values[row];
                int bit = (val >> bit_pos) & 1;
                bit_count += bit;
            }
            
            if (bit_count != 4) {
                return false;  // Z-axis constraint violated
            }
        }
    }
    return true;  // All Z-axis constraints satisfied
}

void CubeSearcherV2::searchRecursive(const ShiftSet& fixedSet,
                                     int setIdx,
                                     std::vector<uint8_t>& usedNumbers,
                                     std::array<ShiftSet, 8>& currentCube,
                                     long& localChecked)
{
    if (setIdx == 8) {
        // All 8 sets placed - validate Z-axis before accepting
        if (!validateZAxis(currentCube)) {
            return;  // Reject this cube - Z-axis not balanced
        }
        
        // Found a true perfect cube!
        int resultId = ++foundCubeCount;
        
        // Store first cube found
        if (resultId == 1) {
            firstCubeFound = true;
            firstCubeData = currentCube;
        }
        
        saveResult(currentCube, resultId);
        
        // Check if we should stop after finding the first cube
        if (firstCubeFound && shouldStop) {
            return;  // Signal to stop - but we can't really stop other threads
        }
        
        return;
    }
    
    // Use filtered shift sets only
    const auto& shiftSets = balancedSet.getFilteredShiftSets();
    
    // Use a fast lookup for used numbers
    uint32_t usedBitset = 0;
    for (uint8_t num : usedNumbers) {
        if (num < 64) {
            usedBitset |= (1U << num);
        }
    }
    
    for (const ShiftSet& candidate : shiftSets) {
        // Exit early if we found the first cube and should stop
        if (firstCubeFound && shouldStop) {
            return;
        }
        
        localChecked++;
        
        // Check if base number already used
        bool alreadyUsed = false;
        if (candidate.base < 64) {
            alreadyUsed = (usedBitset & (1U << candidate.base)) != 0;
        } else {
            alreadyUsed = std::find(usedNumbers.begin(), usedNumbers.end(), candidate.base) 
                         != usedNumbers.end();
        }
        
        if (alreadyUsed) {
            continue;
        }
        
        // Filter already done in BalancedSet, no need to recheck
        
        // Place this set and continue
        currentCube[setIdx] = candidate;
        usedNumbers.push_back(candidate.base);
        
        searchRecursive(fixedSet, setIdx + 1, usedNumbers, currentCube, localChecked);
        
        usedNumbers.pop_back();
    }
}

long CubeSearcherV2::calculateTotalPermutations() const
{
    // Simple approximation: filtered_sets^7 (Set 1 fixed, Sets 2-8 chosen from filtered)
    const auto& filteredSets = balancedSet.getFilteredShiftSets();
    int n = filteredSets.size();
    
    if (n == 0) return 0;
    
    // Rough estimate: n * (n-1) * (n-2) * ... * (n-6) for 7 nested levels
    // This is an underestimate since uniqueness check is not perfect
    long total = 1;
    for (int i = 0; i < 7 && i < n; ++i) {
        total *= (n - i);
    }
    
    return total;
}

void CubeSearcherV2::openResultFile()
{
    std::string filename = "PerfectCube_Results_";
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    filename += ss.str() + ".txt";
    
    resultFile.open(filename, std::ios::app);
    if (resultFile.is_open()) {
        std::cout << "[INFO] Results will be saved to: " << filename << std::endl;
    }
}

void CubeSearcherV2::closeResultFile()
{
    if (resultFile.is_open()) {
        resultFile.close();
    }
}

void CubeSearcherV2::saveResult(const std::array<ShiftSet, 8>& cube, int resultId)
{
    std::lock_guard<std::mutex> lock(mtx);
    
    resultFile << "\n================================================\n";
    resultFile << "SOLUTION #" << resultId << "\n";
    resultFile << "================================================\n";
    
    for (int i = 0; i < 8; ++i) {
        resultFile << "Set " << i << " (base: " << (int)cube[i].base << "): ";
        for (int j = 0; j < 8; ++j) {
            resultFile << (int)cube[i].values[j];
            if (j < 7) resultFile << " ";
        }
        resultFile << "\n";
    }
    
    resultFile << "================================================\n";
    resultFile.flush();
    
    std::cout << "\n[FOUND!] Perfect Cube #" << resultId << " discovered!\n";
}

