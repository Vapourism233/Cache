# Cache

A C++14 collection of cache eviction strategies, including LRU, LRU-K, LFU, ARC, and sharded concurrent variants, along with a hit-rate benchmarking framework.

## Table of Contents

- [Overview](#overview)
- [Implemented Cache Strategies](#implemented-cache-strategies)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Benchmarking](#benchmarking)
- [Known Issues](#known-issues)
- [Update Logs](#update-logs)

## Overview

This project implements several classic cache eviction algorithms from scratch, all built with a template design (`template<typename Key, typename Value>`) to support arbitrary key/value types. The underlying data structures use intrusive doubly-linked lists (`shared_ptr` forward + `weak_ptr` backward to avoid circular references) combined with hash tables, guaranteeing O(1) complexity for core operations.

## Implemented Cache Strategies

| Strategy | File | Description | Status |
|----------|------|-------------|--------|
| **LRU** | [LRU.h](src/headers/LRU.h) | Least Recently Used, doubly-linked list + hash table | ✅ Stable |
| **LRU-K** | [LRUK.h](src/headers/LRUK.h) | LRU variant, requires K accesses before entering main cache, resists burst traffic | ✅ Stable |
| **LFU** | [LFUK.h](src/headers/LFUK.h) | Least Frequently Used, frequency-bucket design with average-frequency decay | ✅ Stable |
| **ARC** | [ARC/](src/ARC/) | Adaptive Replacement Cache, LRU + LFU dual-part coordinator | ✅ Stable |
| **HashLRU** | [HashLRU.cpp](src/HashLRU.cpp) | Sharded concurrent LRU-K, reduces lock contention | ✅ Stable |
| **HashLFU** | [HashLFU.cpp](src/HashLFU.cpp) | Sharded concurrent LFU | ✅ Stable |

### LRU
Classic implementation: on a hit, move the node to the tail of the list (most recently used); on eviction, remove the head of the list (least recently used).

### LRU-K
Inherits from `LRUCache` and maintains a history access-count table. A key is only promoted to the main cache after it has been accessed K times, effectively preventing occasional one-off scans from polluting the cache.

### LFU
Based on a **frequency-bucket** design: `unordered_map<freq, list<Node>>`, where each frequency maps to a doubly-linked list. Within the same frequency, items are ordered by LRU. A `maxAverageNum` mechanism provides frequency decay, preventing historically high-frequency items from occupying the cache permanently.

### ARC (Adaptive Replacement Cache)
Managed by the `ARCCache` coordinator, which controls two parts:
- **ARCLRUPart**: captures the **recency** of accesses
- **ARCLFUPart**: captures the **frequency** of accesses

Each part takes half of the capacity and maintains its own ghost cache (recording evicted keys). When a part's ghost cache hits, the coordinator dynamically rebalances the capacity between the two parts (one grows as the other shrinks). Hot items are promoted from the LRU part to the LFU part.

### HashLRU / HashLFU
Sharded concurrent versions: keys are distributed across multiple independent shards via hashing, and each shard holds its own lock, reducing lock contention under multi-threaded access.

## Project Structure

```
Cache/
├── CMakeLists.txt
├── src/
│   ├── headers/             # Template definitions (#ifndef guards)
│   │   ├── LRU.h
│   │   ├── LRUK.h
│   │   ├── LFUK.h
│   │   └── HashLRU.h
│   ├── LRU.cpp / LRUK.cpp / LFUK.cpp   # Thin wrappers, #include the header
│   ├── HashLRU.cpp           # Sharded concurrent LRU-K (HashLRUKCache)
│   ├── HashLFU.cpp           # Sharded concurrent LFU (LFUHashCache)
│   ├── ARC/
│   │   ├── ARCKCacheNode.h   # ARC node definition
│   │   ├── ARCLRUPart.h      # LRU part of ARC
│   │   ├── ARCLFUPart.h      # LFU part of ARC
│   │   ├── ARCCache.cpp      # ARC coordinator
│   │   ├── test_arc_cache.cpp
│   │   └── test_arc_lfu.cpp
│   └── benchmark_hitrate.cpp # Hit-rate & latency benchmarking framework
└── test/                     # Unit / debug tests (built via CTest)
```

## Getting Started

### Requirements
- C++14 or higher (ARC uses `std::make_unique`)
- g++ / clang++
- CMake ≥ 3.10

### Build and Run Tests

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Usage Example

```cpp
#include "LRU.h"

LRUCache<int, std::string> cache(100);  // capacity 100

cache.put(1, "hello");

std::string value;
if (cache.get(1, value)) {
    // hit, value == "hello"
}
```

## Benchmarking

[benchmark_hitrate.cpp](src/benchmark_hitrate.cpp) provides a hit-rate benchmarking framework supporting four typical access patterns:

| Workload | Scenario Description |
|----------|----------------------|
| **Hotspot (80-20)** | 20% of keys account for 80% of accesses (Pareto principle) |
| **TimeLocality** | Temporal locality; recently accessed keys are more likely to be accessed again |
| **Periodic** | Periodic cyclic access |
| **UniformRandom** | Completely uniform random access |

### Running the Benchmark

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/benchmark_hitrate
```

### Test Parameters
- Working set size: 100,000 distinct keys
- Total accesses: 1,000,000
- Cache capacities: 20 / 100 / 1000

### Results

#### Summary of winners by hit rate

| Workload | Winner | Winner Hit Rate | Best Non-ARC |
|----------|--------|------------------|---------------|
| Hotspot (80-20) | ARC | 0.496 | 0.439 (LFU) |
| TimeLocality | ARC | 0.738 | 0.712 (LFU) |
| UniformRandom | ARC | 0.241 | 0.173 (LRU) |
| Periodic | tie | 1.00 | 1.00 |

#### Average hit rate per strategy

**Hotspot (80-20):**
1. ARC — 0.4959
2. LFU — 0.4392
3. HashLFU — 0.4392
4. LRU — 0.4061
5. LRU-K — 0.4061
6. HashLRU — 0.4060

**TimeLocality:**
1. ARC — 0.7375
2. HashLFU — 0.7118
3. LFU — 0.7118
4. HashLRU — 0.7110
5. LRU — 0.7110
6. LRU-K — 0.7110

**Periodic:**
1. LRU — 0.9999
2. LRU-K — 0.9999
3. LFU — 0.9999
4. HashLRU — 0.9999
5. HashLFU — 0.9999
6. ARC — 0.9999

**UniformRandom:**
1. ARC — 0.2412
2. LRU — 0.1732
3. LRU-K — 0.1732
4. HashLRU — 0.1731
5. HashLFU — 0.1731
6. LFU — 0.1730

#### Latency (300k ops, capacity 2000 — lower is better)

| Strategy | File | Time Consumption |
|----------|------|-------------------|
| **LRU** | [LRU.h](src/headers/LRU.h) | ~15 ms |
| **LRU-K** | [LRUK.h](src/headers/LRUK.h) | ~44 ms |
| **LFU** | [LFUK.h](src/headers/LFUK.h) | ~2124 ms |
| **ARC** | [ARC/](src/ARC/) | ~65 ms |
| **HashLRU** | [HashLRU.cpp](src/HashLRU.cpp) | ~47 ms |
| **HashLFU** | [HashLFU.cpp](src/HashLFU.cpp) | ~614 ms |

**Conclusion:** LRU is 3-5x faster than LFU, but LFU has a higher hit rate in hotspot workloads. The combination of LRU and LFU in ARC achieves the best hit rate across all workloads, with an acceptable time-consumption tradeoff.

### Key Findings (LRU vs LFU)

| Workload | Better Strategy | Notes |
|----------|-----------------|-------|
| Hotspot (80-20) | **LFU** | ~20% higher hit rate, better at identifying hot spots |
| TimeLocality | Comparable | Both leverage temporal locality well |
| Periodic | **LFU** | ~7% higher hit rate |
| UniformRandom | Comparable | No difference under uniform access |

> In terms of performance, LRU is 3-5x faster than LFU (LFU's frequency-bucket maintenance incurs extra overhead).

## Known Issues

- The benchmark currently only stably covers LRU and LFU; HashLRU / HashLFU were not included in the unified benchmark until the class-redefinition issue (caused by duplicate header includes) was resolved — see the 2026/08/12 update below.

## Update Logs

**2026/08/13** — Fixed the ARC unit test failure. The issue was due to a missing `increaseCapacity()` call in `ARCLFUPart` when reviving a node from the ghost cache, which caused the LFU part to exceed its capacity and trigger an assertion failure. The fix involved adding the `increaseCapacity()` call in both relevant code paths.

**2026/08/12** — Fixed the ARC boundary condition bug (segfault on hotspot workloads at small capacities, e.g. capacity=20). The issue was a `weak_ptr` in the ghost cache being destroyed before the corresponding `shared_ptr` in the main cache, leading to dangling references. The fix ensures ghost cache entries are only removed after confirming the main cache still holds a valid reference.

The build system was also replaced with CMake for better dependency management, and CTest was added for unit testing. The ARC unit test (`test_arc_lfu`) was found failing at this point — fixed the next day (see 2026/08/13).

**2026/08/10** — Added a unified benchmark for HashLRU and HashLFU, resolving class-redefinition issues by using forward declarations and separating template definitions into implementation files.

**2026/08/02** — Baseline done: LRU, LRU-K, LFU, and ARC implemented and benchmarked. ARC shows the best hit rate across all workloads, but with higher latency than LRU.

## License

Personal learning project.

