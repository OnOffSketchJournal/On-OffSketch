cmake_minimum_required(VERSION 3.14)
project(bench)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -mbmi -O3 -mcmodel=medium")

include_directories(.)
include_directories(common)
include_directories(FPI)
include_directories(PE)
include_directories(Botnet)

add_executable(bench
        common/Abstract.h
        common/bitset.h
        common/hash.h
        FPI/OPI_OPT.h
        FPI/OPI_BSC.h
        FPI/OO_SIMD.h
        FPI/OO_SIMD_8.h
        FPI/OO_SIMD_32.h
        FPI/PIE.h
        FPI/SS.h
        PE/CM_BF.h
        PE/CM_HT.h
        PE/OPE_BSC.h
        PE/OPE_OPT.h
        benchmark.h
        Botnet_Detection.h
        Botnet/Botnet_Bitmap.h
        main.cpp)
