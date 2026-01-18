# Perfect Bit Cube

A high-performance C++ exploration of **3D binary symmetry** and **combinatorial balance**.

## 🌌 The Grand Challenge: Finding the "Perfect" Cube

The **Perfect Bit Cube** is a theoretical 8x8x8 bit structure (512 bits in total) where every orthogonal axis—X, Y, and Z—maintains a perfect **Balanced** state. A value is considered balanced if its **Hamming Weight is exactly 4** (four `1`s and four `0`s).

Despite highly optimized algorithms reaching search speeds of **~15 Million paths/second**, a confirmed "Perfect" configuration remains undiscovered due to the sheer mathematical scarcity of these states and limited computational resources.

> **📢 Call for Contributions:** If your hardware successfully identifies a Perfect Cube using this codebase, please share the resulting `PerfectCube_*.txt` file! This project is a journey to find a needle in a digital haystack, and every discovery is a contribution to combinatorial mathematics.

---

## 📐 Mathematical Foundation

- **Combinatorics:** We operate within the set of 70 balanced 8-bit numbers ($\binom{8}{4} = 70$).
- **Symmetry:** The search space is optimized using an "Up-Set" (values 128-255) and their bitwise complements, arranged with **Point Reflection Symmetry**.
- **Z-Axis Targeting:** The current engine uses **Bit-Slice Addition** to verify Z-axis balance across 64 parallel "veins" simultaneously within 64-bit registers.



---

## 🛠 Architecture

The project is built with a modular, performance-first C++17 design:

- **`BalancedSet`**: Manages the 70 seeds, their dualities, and complement mappings.
- **`LayerGenerator`**: Generates 8x8 matrices balanced in both X and Y axes. (Current optimized run identified **1,256,640 valid layers** in <1s).
- **`CubeAssembler`**: Performs a parallel recursive search using a lookup index and mathematical targeting to solve for the Z-axis.
- **`Cube`**: The core 512-bit lattice structure.

---

## 🚀 Building the Explorer

### Prerequisites
- A C++17 compliant compiler (GCC 7+, Clang 5+, or MSVC 2017+).
- CMake 3.10 or higher.

### Build and Run
```bash
# Clone the repository
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./perfect_bit_cube
