cmake_minimum_required(VERSION 3.10)
project(PerfectBitCube)

# C++17 standardını zorunlu kılıyoruz (std::async, std::future ve bit işlemleri için)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Performans için en üst düzey optimizasyonu açıyoruz (-O3)
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-O3 -Wall -Wextra)
endif()

# Threading desteğini bul ve projeye bağla
find_package(Threads REQUIRED)

# Kaynak dosyalarımızı listeliyoruz
set(SOURCES
    main.cpp
    BalancedSet.cpp
    LayerGenerator.cpp
    CubeAssembler.cpp
)

# Çalıştırılabilir dosyayı oluştur
add_executable(perfect_bit_cube ${SOURCES})

# Thread kütüphanesini çalıştırılabilir dosyaya bağla
target_link_libraries(perfect_bit_cube PRIVATE Threads::Threads)
