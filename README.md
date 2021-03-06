On-Off sketch
============

Introduction
--------
The On-Off sketch is an algorithm that can address two typical problems about persistence – persistence estimation and finding persistent items. For persistence estimation, utilizing the characteristic that the persistence of an item is increased periodically, we compress increments of multiple items mapped to the same counter, which significantly reduces the error. Compared with the Count-Min sketch, 1) in theory, we prove that the error of the On-Off sketch is always smaller; 2) in experiments, the On-Off sketch achieves around 6.17 times smaller error and 2.13 times higher throughput. For finding persistent items, based on our solution to persistence estimation, we separate persistent items and non-persistent items to further improve the accuracy. We show that the space complexity of our On-Off sketch is much better than the state-of-the-art (PIE), and it can reduce the error up to 4 orders of magnitude and achieves 3.58 times higher throughput than prior algorithms in experiments.

Repository structure
--------------------
*  `common/`: the hash function and bitset data structure used by many algorithms
*  `PE/`: the implementation of algorithms on persistence estimation in our experiments
*  `FPI/`: the implementation of algorithms on finding persistent items in our experiments
*  `Botnet/`: the implementation of existing algorithms on Botnet Detection
*  `benchmark.h`: C++ header of some benchmarks about AAE, F1 Score, and throughput
*  `Botnet_Detection.h`: C++ header of some benchmarks about Botnet

Requirements
-------
- cmake
- g++

We conduct the experiments on a server with a 18-core CPU (36 threads, Intel(R) Core(TM) i9-10980XE CPU @ 3.00GHz) and 128GB total DRAM memory. 
Each core has three levels of cache memory: 64KB L1 cache, 1MB L2 cache, and 24.75MB L3 cache shared by all cores. All the codes uses -O3 optimization.

How to run (make sure the path to dataset is right in main.cpp)
-------

```bash
$ cmake .
$ make
$ ./bench
```
