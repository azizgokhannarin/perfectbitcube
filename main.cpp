#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>
#include "BalancedSet.h"
#include "LayerGenerator.h"
#include "CubeAssembler.h"

int main()
{
    std::cout <<
              "╔════════════════════════════════════════════════════════════╗"
              << std::endl;
    std::cout << "║          PERFECT BIT CUBE FINDER v5.0 FINAL               ║" << std::endl;
    std::cout <<
              "╚════════════════════════════════════════════════════════════╝"
              << std::endl;
    std::cout << std::endl;

    std::cout << "GOAL: Find 8x8x8 cubes where ALL lines are balanced numbers" << std::endl;
    std::cout << "  - Balanced number: 4 bits '1' and 4 bits '0' (70 total)" << std::endl;
    std::cout << "  - X-axis: 64 lines (horizontal rows in each layer)" << std::endl;
    std::cout << "  - Y-axis: 64 lines (bit positions across rows)" << std::endl;
    std::cout << "  - Z-axis: 64 lines (vertical columns through layers)" << std::endl;
    std::cout << "  - Total: 512 bits → 256 zeros, 256 ones" << std::endl;
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

    // Phase 1: Initialize balanced number set
    std::cout << "┌─ PHASE 1: Initialize Balanced Numbers" << std::endl;
    BalancedSet bSet;
    std::cout << "│  ✓ Total balanced numbers: " << bSet.getAllBalanced().size() << std::endl;
    std::cout << "│  ✓ Up-set size (128-255): " << bSet.getUpSet().size() << std::endl;
    std::cout << "└─ Phase 1 Complete" << std::endl;
    std::cout << std::endl;

    // Phase 2: Generate valid 8x8 layers
    std::cout << "┌─ PHASE 2: Generate Valid 8x8 Layers" << std::endl;
    std::cout << "│  Constraints: X-axis, Y-axis, Z-axis all balanced" << std::endl;
    std::cout << "│  Method: Backtracking with aggressive pruning" << std::endl;
    std::cout << "│" << std::endl;

    LayerGenerator generator(bSet);
    generator.generate();
    const auto &layers = generator.getValidLayers();

    std::cout << "│" << std::endl;
    std::cout << "│  ✓ Valid layers found: " << layers.size() << std::endl;

    if (layers.size() == 0) {
        std::cout << "│  ✗ ERROR: No valid layers found!" << std::endl;
        std::cout << "│  Cannot proceed to cube assembly." << std::endl;
        std::cout << "└─ Phase 2 Failed" << std::endl;
        return 1;
    }

    std::cout << "└─ Phase 2 Complete" << std::endl;
    std::cout << std::endl;

    // Phase 3: Assemble perfect cubes
    std::cout << "┌─ PHASE 3: Assemble Perfect Bit Cubes" << std::endl;
    std::cout << "│  Method: Recursive search with Z-axis tracking" << std::endl;
    std::cout << "│  Symmetry: Central complement symmetry (layers 0-3 ↔ 4-7)" << std::endl;
    std::cout << "│  Threads: " << nThreads << " parallel workers" << std::endl;
    std::cout << "│" << std::endl;

    CubeAssembler assembler(bSet);
    assembler.assembleParallel(layers, nThreads);

    std::cout << std::endl;
    std::cout << "└─ Phase 3 Complete" << std::endl;
    std::cout << std::endl;

    std::cout <<
              "════════════════════════════════════════════════════════════"
              << std::endl;
    std::cout << "[FINISHED] All phases complete. Check PerfectCube_*.txt files" << std::endl;
    std::cout <<
              "════════════════════════════════════════════════════════════"
              << std::endl;

    return 0;
}
