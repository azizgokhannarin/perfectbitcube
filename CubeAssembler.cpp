#include "CubeAssembler.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstring>

CubeAssembler::CubeAssembler(const BalancedSet &bSet)
    : balancedSet(bSet), checkedPaths(0), foundCount(0) {}

void CubeAssembler::assembleParallel(const std::vector<Layer> &layers, int nThreads)
{
    int n = layers.size();
    if (n == 0) {
        std::cout << "[CubeAssembler] ERROR: No layers to assemble!" << std::endl;
        return;
    }

    std::cout << "[CubeAssembler] Building lookup index..." << std::endl;

    // Build lookup table by first row value
    std::vector<std::vector<int>> lookup(256);
    for (int i = 0; i < n; ++i) {
        lookup[layers[i].rows[0]].push_back(i);
    }

    // Count non-empty buckets for statistics
    int nonEmptyBuckets = 0;
    for (int i = 0; i < 256; ++i) {
        if (!lookup[i].empty()) nonEmptyBuckets++;
    }
    std::cout << "[CubeAssembler] Lookup ready: " << nonEmptyBuckets << " buckets with data" << std::endl;

    std::atomic<int> completedRoots{0};
    auto startTime = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;

    // Distribute work evenly across threads
    int chunk = (n + nThreads - 1) / nThreads;

    std::cout << "[CubeAssembler] Launching " << nThreads << " worker threads..." << std::endl;
    std::cout << "[CubeAssembler] Searching " << n << " root layers (chunk size: " << chunk << ")\n" << std::endl;

    for (int t = 0; t < nThreads; ++t) {
        threads.emplace_back([this, &layers, &lookup, t, chunk, n, &completedRoots, startTime]() {
            int start = t * chunk;
            int end = std::min(start + chunk, n);

            // Thread-local state
            uint64_t localZCounts[3];
            uint64_t localMask[4];
            uint8_t localRows[4][8];
            long localChecked = 0;

            for (int i = start; i < end; ++i) {
                const Layer &L1 = layers[i];

                // Initialize state with first layer
                localZCounts[0] = L1.bitMatrix;
                localZCounts[1] = 0;
                localZCounts[2] = 0;

                for (int m = 0; m < 4; m++) {
                    localMask[m] = L1.numMask[m];
                }
                std::memcpy(localRows[0], L1.rows, 8);

                // Search for remaining 3 layers (Z=1,2,3)
                searchWithLookup(layers, lookup, i + 1, 1, localZCounts, localMask, localRows, localChecked);

                completedRoots++;

                // Update progress every 10 roots
                if (completedRoots % 10 == 0) {
                    this->checkedPaths += localChecked;
                    localChecked = 0;

                    auto currentTime = std::chrono::steady_clock::now();
                    double elapsed = std::chrono::duration<double>(currentTime - startTime).count();
                    double progress = (double)completedRoots / n * 100.0;
                    double speed = (elapsed > 0.1) ? (checkedPaths / 1000000.0) / elapsed : 0;

                    // Estimate remaining time
                    int eta_seconds = 0;
                    if (speed > 0 && progress > 0.5) {
                        double totalPaths = checkedPaths / (progress / 100.0);
                        double remaining = totalPaths - checkedPaths;
                        eta_seconds = (int)(remaining / (speed * 1000000.0));
                    }

                    std::lock_guard<std::mutex> lock(mtx);
                    std::cout << "\r[PROGRESS] " << std::fixed << std::setprecision(2) << progress
                              << "% | Roots: " << completedRoots << "/" << n
                              << " | Speed: " << speed << "M/s"
                              << " | Checked: " << (checkedPaths / 1000000.0) << "M"
                              << " | Found: " << foundCount;
                    if (eta_seconds > 0) {
                        std::cout << " | ETA: " << (eta_seconds / 60) << "m " << (eta_seconds % 60) << "s";
                    }
                    std::cout << " | Elapsed: " << (int)elapsed << "s" << std::flush;
                }
            }

            // Add remaining local checked count
            this->checkedPaths += localChecked;
        });
    }

    for (auto &th : threads) th.join();

    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

    std::cout << "\n\n[CubeAssembler] Search complete!" << std::endl;
    std::cout << "[CubeAssembler] Time: " << elapsed << "s (" << (elapsed / 60) << "m " <<
              (elapsed % 60) << "s)" << std::endl;
    std::cout << "[CubeAssembler] Total paths checked: " << checkedPaths.load() << std::endl;
    std::cout << "[CubeAssembler] Perfect cubes found: " << foundCount.load() << std::endl;
}

void CubeAssembler::searchWithLookup(const std::vector<Layer> &layers,
                                     const std::vector<std::vector<int>> &lookup,
                                     int layerStartIdx,
                                     int currentZ,
                                     uint64_t zCounts[3],
                                     uint64_t currentCubeMask[4],
                                     uint8_t currentCubeRows[4][8],
                                     long &localChecked)
{
    if (currentZ == 3) {
        // Z=3: Calculate target matrix (columns need exactly 4 ones)
        uint64_t targetMatrix = ~(zCounts[0] | zCounts[1]);

        uint8_t targetFirstRow = (uint8_t)(targetMatrix & 0xFF);

        // Use lookup to find candidates quickly
        for (int idx : lookup[targetFirstRow]) {
            if (idx < layerStartIdx) continue;

            localChecked++;
            const Layer &cand = layers[idx];

            // Fast check: exact matrix match
            if (cand.bitMatrix != targetMatrix) continue;

            // Fast check: number collision
            if ((cand.numMask[0] & currentCubeMask[0]) ||
                (cand.numMask[1] & currentCubeMask[1]) ||
                (cand.numMask[2] & currentCubeMask[2]) ||
                (cand.numMask[3] & currentCubeMask[3])) continue;

            // Found a valid 4-layer combination!
            Cube c;

            // Copy first 3 layers
            for (int z = 0; z < 3; z++) {
                std::memcpy(c.data[z], currentCubeRows[z], 8);
            }

            // Add 4th layer
            std::memcpy(c.data[3], cand.rows, 8);

            // Complete cube with central symmetry (layers 4-7 are complements)
            for (int z = 0; z < 4; z++) {
                for (int y = 0; y < 8; y++) {
                    c.data[7 - z][y] = balancedSet.getComplement(c.data[z][y]);
                }
            }

            // Save to disk
            int cubeId = ++foundCount;
            saveToDisk(c, cubeId);
        }
        return;
    }

    // Recursive case: try adding more layers (Z=1 or Z=2)
    for (int i = layerStartIdx; i < (int)layers.size(); i++) {
        const Layer &cand = layers[i];

        // Pruning 1: Z-axis constraint (no column can exceed 4 ones)
        if (zCounts[2] & cand.bitMatrix) continue;

        // Pruning 2: Number collision (each number 0-255 can appear at most once)
        if ((cand.numMask[0] & currentCubeMask[0]) ||
            (cand.numMask[1] & currentCubeMask[1]) ||
            (cand.numMask[2] & currentCubeMask[2]) ||
            (cand.numMask[3] & currentCubeMask[3])) continue;

        localChecked++;

        // Bit-slice addition (parallel 64-bit addition for Z-counts)
        uint64_t nextCounts[3];
        uint64_t carry0 = zCounts[0] & cand.bitMatrix;
        nextCounts[0] = zCounts[0] ^ cand.bitMatrix;

        uint64_t carry1 = zCounts[1] & carry0;
        nextCounts[1] = zCounts[1] ^ carry0;

        nextCounts[2] = zCounts[2] | carry1;

        // Update number mask
        uint64_t nextMask[4];
        for (int m = 0; m < 4; m++) {
            nextMask[m] = currentCubeMask[m] | cand.numMask[m];
        }

        // Update rows (no copy, just add new layer)
        std::memcpy(currentCubeRows[currentZ], cand.rows, 8);

        // Recurse
        searchWithLookup(layers, lookup, i + 1, currentZ + 1, nextCounts, nextMask, currentCubeRows, localChecked);
    }
}

void CubeAssembler::saveToDisk(const Cube &cube, int id)
{
    std::lock_guard<std::mutex> lock(mtx);

    // === COMPLETE VERIFICATION ===
    int totalOnes = 0;
    bool allAxesValid = true;
    std::string errorMsg = "";

    // Count total bits
    for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
            totalOnes += __builtin_popcount(cube.data[z][y]);
        }
    }

    // Verify X-axis (64 lines: each row in each layer)
    for (int z = 0; z < 8; z++) {
        for (int y = 0; y < 8; y++) {
            if (!balancedSet.isBalanced(cube.data[z][y])) {
                allAxesValid = false;
                errorMsg += "X-axis FAIL at z=" + std::to_string(z) + " y=" + std::to_string(y) + "\n";
            }
        }
    }

    // Verify Y-axis (64 lines: bit positions within each layer)
    for (int z = 0; z < 8; z++) {
        for (int bitPos = 0; bitPos < 8; bitPos++) {
            uint8_t yLine = 0;
            for (int y = 0; y < 8; y++) {
                if ((cube.data[z][y] >> bitPos) & 1) {
                    yLine |= (1 << y);
                }
            }
            if (!balancedSet.isBalanced(yLine)) {
                allAxesValid = false;
                errorMsg += "Y-axis FAIL at z=" + std::to_string(z) + " bitPos=" + std::to_string(bitPos) + "\n";
            }
        }
    }

    // Verify Z-axis (64 lines: columns through layers)
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            uint8_t zLine = 0;
            for (int z = 0; z < 8; z++) {
                if ((cube.data[z][y] >> (7 - x)) & 1) {
                    zLine |= (1 << z);
                }
            }
            if (!balancedSet.isBalanced(zLine)) {
                allAxesValid = false;
                errorMsg += "Z-axis FAIL at y=" + std::to_string(y) + " x=" + std::to_string(x) + "\n";
            }
        }
    }

    // Save to file
    std::ofstream out("PerfectCube_" + std::to_string(id) + ".txt");
    out << "=== PERFECT BIT CUBE #" << id << " ===\n\n";

    out << "VERIFICATION RESULTS:\n";
    out << "====================\n";
    out << "Total 1s: " << totalOnes << " (should be 256) " << (totalOnes == 256 ? "✓" : "✗") << "\n";
    out << "Total 0s: " << (512 - totalOnes) << " (should be 256) " << ((512 - totalOnes) == 256 ? "✓" : "✗") << "\n";
    out << "All X-axis lines balanced: " << (allAxesValid ? "✓ YES" : "✗ NO") << "\n";
    out << "All Y-axis lines balanced: " << (allAxesValid ? "✓ YES" : "✗ NO") << "\n";
    out << "All Z-axis lines balanced: " << (allAxesValid ? "✓ YES" : "✗ NO") << "\n";
    out << "VERDICT: " << (allAxesValid && totalOnes == 256 ? "✓✓✓ PERFECT CUBE ✓✓✓" : "✗ INVALID") << "\n\n";

    if (!errorMsg.empty()) {
        out << "ERRORS:\n" << errorMsg << "\n";
    }

    // Print each layer
    for (int z = 0; z < 8; z++) {
        out << "Layer Z=" << z << ":\n";
        for (int y = 0; y < 8; y++) {
            out << std::bitset<8>(cube.data[z][y]) << " (" << std::setw(3) << (int)cube.data[z][y] << ")\n";
        }
        out << "\n";
    }

    out.close();

    if (allAxesValid && totalOnes == 256) {
        std::cout << "\n🎉✓✓✓ PERFECT CUBE #" << id << " VERIFIED AND SAVED! ✓✓✓" << std::endl;
    } else {
        std::cout << "\n⚠️  WARNING: Cube #" << id << " saved but FAILED verification!" << std::endl;
    }
}
