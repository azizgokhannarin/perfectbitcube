#include "LayerGenerator.h"
#include <iostream>
#include <iomanip>
#include <bitset>

LayerGenerator::LayerGenerator(const BalancedSet &bSet) : balancedSet(bSet)
{
    for (int i = 0; i < 8; ++i) {
        colCounts[i] = 0;  // Z-axis bit counts
        yAxisCounts[i] = 0; // Y-axis bit counts
    }
}

void LayerGenerator::generate()
{
    std::cout << "[LayerGen] Starting backtrack search for valid 8x8 layers..." << std::endl;
    std::cout << "[LayerGen] Constraint: X-axis (rows), Y-axis (bit positions), Z-axis (columns) all balanced" << std::endl;

    uint8_t rows[8];
    uint64_t usedMask = 0;
    totalAttempts = 0;

    auto startTime = std::chrono::steady_clock::now();
    backtrack(0, rows, usedMask);
    auto endTime = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    std::cout << "\n[LayerGen] Complete! Found " << validLayers.size() << " valid layers" << std::endl;
    std::cout << "[LayerGen] Time: " << elapsed << "s | Attempts: " << totalAttempts << std::endl;
}

bool LayerGenerator::canAddRow(uint8_t row, int rowIdx)
{
    // Check Z-axis constraint (column bits)
    for (int col = 0; col < 8; ++col) {
        int bit = (row >> (7 - col)) & 1;
        int newCount = colCounts[col] + bit;

        // Too many 1s in this column
        if (newCount > 4) return false;

        // Not enough rows left to reach 4 ones
        int remainingRows = 7 - rowIdx;
        if (newCount + remainingRows < 4) return false;
    }

    // Check Y-axis constraint (bit positions across rows)
    // For each bit position in the byte, count how many 1s we'd have across all rows so far
    for (int bitPos = 0; bitPos < 8; ++bitPos) {
        int bit = (row >> bitPos) & 1;
        int newCount = yAxisCounts[bitPos] + bit;

        // Too many 1s in this Y-axis line
        if (newCount > 4) return false;

        // Not enough rows left to reach 4 ones
        int remainingRows = 7 - rowIdx;
        if (newCount + remainingRows < 4) return false;
    }

    return true;
}

void LayerGenerator::updateCounts(uint8_t row, int delta)
{
    // Update Z-axis (column) counts
    for (int col = 0; col < 8; ++col) {
        int bit = (row >> (7 - col)) & 1;
        colCounts[col] += (bit * delta);
    }

    // Update Y-axis (bit position) counts
    for (int bitPos = 0; bitPos < 8; ++bitPos) {
        int bit = (row >> bitPos) & 1;
        yAxisCounts[bitPos] += (bit * delta);
    }
}

void LayerGenerator::backtrack(int rowIdx, uint8_t currentRows[8], uint64_t usedMask)
{
    totalAttempts++;

    // Progress reporting every 100K attempts
    if (totalAttempts % 100000 == 0) {
        std::cout << "\r[LayerGen] Attempts: " << (totalAttempts / 1000000.0) << "M | Found: " << validLayers.size() <<
                  std::flush;
    }

    // Base case: first 4 rows done, add complements for last 4
    if (rowIdx == 4) {
        // Add complement rows
        for (int i = 0; i < 4; ++i) {
            currentRows[i + 4] = balancedSet.getComplement(currentRows[i]);
        }

        // Final verification: check all axes sum to exactly 4

        // Z-axis verification (columns)
        int finalColCounts[8] = {0};
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                finalColCounts[c] += (currentRows[r] >> (7 - c)) & 1;
            }
        }

        bool validZ = true;
        for (int c = 0; c < 8; ++c) {
            if (finalColCounts[c] != 4) {
                validZ = false;
                break;
            }
        }

        if (!validZ) return;

        // Y-axis verification (bit positions across rows)
        int finalYCounts[8] = {0};
        for (int r = 0; r < 8; ++r) {
            for (int bitPos = 0; bitPos < 8; ++bitPos) {
                finalYCounts[bitPos] += (currentRows[r] >> bitPos) & 1;
            }
        }

        bool validY = true;
        for (int bitPos = 0; bitPos < 8; ++bitPos) {
            if (finalYCounts[bitPos] != 4) {
                validY = false;
                break;
            }
        }

        if (!validY) return;

        // X-axis is automatically valid (each row is a balanced number)

        // All three axes validated! Create layer
        Layer L;
        L.bitMatrix = 0;
        for (int i = 0; i < 4; ++i) L.numMask[i] = 0;

        for (int i = 0; i < 8; ++i) {
            L.rows[i] = currentRows[i];
            L.uniqueNumbers.push_back(currentRows[i]);

            // Build 64-bit matrix (each row is 8 bits)
            L.bitMatrix |= (static_cast<uint64_t>(currentRows[i]) << (i * 8));

            // Build 256-bit number mask
            int bucket = currentRows[i] / 64;
            int bit = currentRows[i] % 64;
            L.numMask[bucket] |= (1ULL << bit);
        }

        validLayers.push_back(L);
        return;
    }

    // Recursive case: try each unused value from up set
    const auto &upSet = balancedSet.getUpSet();

    for (size_t i = 0; i < upSet.size(); ++i) {
        // Skip if already used
        if (usedMask & (1ULL << i)) continue;

        uint8_t candidate = upSet[i];

        // Check if this row can be added (both Z and Y axis constraints)
        if (!canAddRow(candidate, rowIdx)) continue;

        // Add this row
        currentRows[rowIdx] = candidate;
        updateCounts(candidate, 1);

        // Recurse
        backtrack(rowIdx + 1, currentRows, usedMask | (1ULL << i));

        // Backtrack
        updateCounts(candidate, -1);
    }
}
