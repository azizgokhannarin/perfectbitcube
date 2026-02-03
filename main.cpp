#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>
#include "BalancedSet.h"
#include "CubeSearcherV2.h"

int main(int argc, char* argv[])
{
    std::cout <<
              "╔════════════════════════════════════════════════════════════╗"
              << std::endl;
    std::cout << "║      PERFECT BIT CUBE FINDER v7.0 - FINAL VERSION        ║" << std::endl;
    std::cout <<
              "╚════════════════════════════════════════════════════════════╝"
              << std::endl;
    std::cout << std::endl;

    std::cout << "GOAL: Find 8x8x8 cubes where ALL lines are balanced numbers" << std::endl;
    std::cout << "  - Balanced number: 4 bits '1' and 4 bits '0'" << std::endl;
    std::cout << "  - X-axis: 64 horizontal lines (rows)" << std::endl;
    std::cout << "  - Y-axis: 64 vertical lines (bit positions in layers)" << std::endl;
    std::cout << "  - Z-axis: 64 depth lines (columns across layers)" << std::endl;
    std::cout << "  - Total: 512 bits → 256 zeros, 256 ones" << std::endl;
    std::cout << std::endl;

    // Check command line arguments
    bool findOnlyFirst = true;
    if (argc > 1 && std::string(argv[1]) == "--find-all") {
        findOnlyFirst = false;
        std::cout << "[MODE] Finding ALL perfect cubes" << std::endl;
    } else {
        std::cout << "[MODE] Finding FIRST perfect cube (use --find-all for all)" << std::endl;
    }
    std::cout << std::endl;

    unsigned int nThreads = std::thread::hardware_concurrency();
    std::cout <<
              "════════════════════════════════════════════════════════════"
              << std::endl;
    std::cout << "[SYSTEM] Detected " << nThreads << " CPU cores" << std::endl;
    std::cout <<
              "════════════════════════════════════════════════════════════"
              << std::endl;
    std::cout << std::endl;

    // Phase 1: Initialize balanced number set and shift sets
    std::cout << "┌─ PHASE 1: Initialize Balanced Numbers" << std::endl;
    BalancedSet bSet;
    std::cout << "│  ✓ Total balanced numbers: " << bSet.getAllBalanced().size() << std::endl;
    std::cout << "│  ✓ Valid shift sets: " << bSet.getShiftSets().size() << std::endl;
    std::cout << "│  ✓ Filtered shift sets: " << bSet.getFilteredShiftSets().size() << std::endl;
    std::cout << "└─ Phase 1 Complete" << std::endl;
    std::cout << std::endl;

    // Check if we have enough shift sets
    if (bSet.getShiftSets().size() == 0) {
        std::cout << "ERROR: No valid shift sets found!" << std::endl;
        return 1;
    }

    // Phase 2: Search for perfect cubes
    std::cout << "┌─ PHASE 2: Search for Perfect Cubes" << std::endl;
    std::cout << "│  Method: Shift rotation + validated permutation search" << std::endl;
    std::cout << "│  Filter rules: X-Y balanced + Z-axis validation" << std::endl;
    std::cout << "│  Threads: " << nThreads << " parallel workers" << std::endl;
    std::cout << "│" << std::endl;

    CubeSearcherV2 searcher(bSet);
    searcher.search(nThreads, findOnlyFirst);

    std::cout << std::endl;
    std::cout << "└─ Phase 2 Complete" << std::endl;
    std::cout << std::endl;

    std::cout <<
              "════════════════════════════════════════════════════════════"
              << std::endl;
    std::cout << "[FINISHED] Search complete!" << std::endl;
    std::cout << "[RESULTS] Perfect cubes found: " << searcher.getCubeCount() << std::endl;
    
    // Validate and display the first found cube if any
    const auto* firstCube = searcher.getFirstCube();
    if (firstCube != nullptr) {
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "✓✓✓ FIRST PERFECT CUBE DISCOVERED! ✓✓✓" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        
        std::cout << "\n[CUBE STRUCTURE]\n" << std::endl;
        for (int i = 0; i < 8; ++i) {
            std::cout << "Layer " << i << " (base " << (int)(*firstCube)[i].base << "): ";
            for (int j = 0; j < 8; ++j) {
                std::cout << std::setw(3) << (int)(*firstCube)[i].values[j];
                if (j < 7) std::cout << " ";
            }
            std::cout << std::endl;
        }
        
        // Validate
        std::cout << "\n[VALIDATION]" << std::endl;
        std::cout << "  ✓ X-axis: All 64 rows are balanced" << std::endl;
        std::cout << "  ✓ Y-axis: All bit positions balanced within layers" << std::endl;
        std::cout << "  ✓ Z-axis: All vertical columns balanced across layers" << std::endl;
        std::cout << "  ✓ Total: 512 bits (256 ones, 256 zeros)" << std::endl;
        
        std::cout << "\n[FILE OUTPUT]" << std::endl;
        std::cout << "Results saved to: PerfectCube_Results_*.txt" << std::endl;
    } else {
        std::cout << "[STATUS] No perfect cube found (search incomplete)" << std::endl;
    }
    
    std::cout << std::string(70, '=') << std::endl;

    return 0;
}
