# Cache Simulator 

This model is built in C++ to simulate a simple L1 cache. 
Users can specify various attributes of the cache like, size, cache line size, set associativity, replacement policy, write allocation policy, etc. 
The model also supports synthetic traffic generation for randomized workload. 

```bash

Usage: CacheSimulator.exe [options]
Options:
  -h                           Show this help message
  -trace <trace_file>          Specify trace input file
  -stat <stats_file>           Specify stats output file
  -cs <cache_size_KB>          Specify the cache size in KB. eg. -s 2 creates 2KB cache
  -ls <cache_line_size_B>      Specify cache line size in bytes (default 0: 64B), else: Absolute fixed size
  -w <num_of_ways>             Specify the associativity of cache. (default: Fully Associative. 0: Direct Mapped,
                               else: specified sets).
  -rp <replacement_policy>     Specify the cache replacement policy. (default: 0: LRU, 1: FIFO)
  -wap <write_allocate_policy> Specify the write allocate policy. (default: 1: Write Allocate, 0: No Write Allocate)

Example: CacheSimulator.exe -cs 256 -ls 64 -w 4 -rp 0 -wap 0 -trace trace.txt -stat cache_stats.txt
This will run the simulation with a cache of size 256KB. Each cache line is 64B wide. Cache has 4-way set associativity.
LRU replacement policy. Write-no-allocate policy.

```

## Building the model
The model can be built using CMake build process. Users can open this project in Visual Studio Code and directly build it there using CMake. Make sure to change the paths as appropriate. 


## Running the model
The model can be run with all default values without any command line options. 

```bash

------------------------------------------
Cache Simulator
------------------------------------------
Cache size:             1048576 bytes
Cache line size:        64 bytes
Number of ways:         16
Replacement Policy:     LRU
Write Allocate Policy:  No Write Allocate
Trace file:             Not set. Synthetic random traffic
Stats file:             cache_stats.txt
Max addressable space:  0xfffff bytes
------------------------------------------

```

## Results
At the end of the run the model shows a bunch of useful stats like hit/miss ratios etc. The stats can also be stored in a file if specified.
Post run stats:

```bash

------------------------------------------
Cache Statistics:
------------------------------------------
  Cache Size:           1024 KB
  Line Size:            64 bytes
  Number of Ways:       16
  Replacement Policy:   LRU
  Write Allocate:       Yes
  Offset bits:          6
  Index bits:           20
  Tag bits:             38
  Total Accesses:       100000
  Total Hits:           83650
  Total Misses:         16350
  Hit Rate:             83.65%
  Miss Rate:            16.35%
------------------------------------------

```

## Work in progress

I am currently working on adding multi-level cache support in this model. Currently it supports only L1. I am also working on adding prefetcher support in the model. It would be interesting to see effect of prefetcher on a real world workload trace. Stay tuned for updates... 






