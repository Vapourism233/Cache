Cache/
├── LRU.h                    # Basic LRU cache implementation
├── LFUK.cpp                 # LFU-K cache implementation
├── LRUK.h                   # LRU-K cache implementation
├── HashLRU.cpp              # LRU with sharded concurrency (thread-safe)
├── HashLFU.cpp              # LFU with sharded concurrency (thread-safe)
├── ARC/                     # ARC (Adaptive Replacement Cache) coordinator design
│   ├── ARCCache.cpp         # ARC main coordinator
│   ├── ARCLFUPart.cpp       # LFU portion in ARC
│   ├── ARCLRUPart.cpp       # LRU portion in ARC
│   └── ARCKCacheNode.h      # ARC node definition
├── Makefile                 # Compilation configuration
├── test_lru.cpp             # LRU unit test
├── test_lruk_run.cpp        # LRU-K unit test
├── benchmark_hitrate.cpp    # Multi-scenario hit rate benchmark
└── README.md
```[cite: 1]

## 🎯 Implemented Caching Algorithms[cite: 1]

### 1. **LRU (Least Recently Used)**[cite: 1]
- File: `LRU.h`[cite: 1]
- Features: Based on access timestamps; the least recently unaccessed item is evicted[cite: 1].
- Time Complexity: O(1) get/put[cite: 1]
- Space Complexity: O(capacity)[cite: 1]
- Applicable Scenarios: Workloads with strong temporal locality[cite: 1]

### 2. **LFU (Least Frequently Used)**[cite: 1]
- File: `LFUK.cpp`[cite: 1]
- Features: Based on access frequency; the least frequently accessed item is evicted[cite: 1].
- Data Structures: Frequency buckets + intra-frequency LRU sorting[cite: 1]
- Time Complexity: O(1) get/put[cite: 1]
- Space Complexity: O(capacity)[cite: 1]
- Applicable Scenarios: Workloads with concentrated hotspots[cite: 1]

### 3. **LRU-K**[cite: 1]
- File: `LRUK.h`[cite: 1]
- Features: Tracks the last $K$ accesses; items enter the main cache only after being accessed $K$ times[cite: 1].
- Advantages: Better discrimination between hot spots and cold data[cite: 1]
- Applicable Scenarios: Scenarios with distinct hot/cold data separation[cite: 1]

### 4. **HashLRU/HashLFU (Concurrent Versions)**[cite: 1]
- File: `HashLRU.cpp`, `HashLFU.cpp`[cite: 1]
- Features: Partitions the cache into multiple smaller caches, with each partition independently locked (sharded lock scheme)[cite: 1].
- Concurrency Level: Configurable number of shards (default is the number of CPU cores)[cite: 1]
- Applicable Scenarios: Multi-threaded, high-concurrency environments[cite: 1]

### 5. **ARC (Adaptive Replacement Cache)**[cite: 1]
- File: `ARC/ARCCache.cpp`[cite: 1]
- Features: Adaptively combines LRU and LFU, automatically adjusting the capacities of both parts via a Ghost Cache[cite: 1].
- Architecture[cite: 1]:
  - Coordinator (`ARCCache`) manages total capacity[cite: 1].
  - LFU portion (`ARCLFUPart`) manages hot spots using frequency buckets[cite: 1].
  - LRU portion (`ARCLRUPart`) manages recent accesses using a linked list[cite: 1].
  - Push-and-pull mechanism: When a hit occurs in a part's ghost cache, that part's capacity is increased by 1, and the other part's capacity is decreased by 1[cite: 1].
- Status: ✅ Basic architecture complete, ⚠️ Bug present in small-capacity edge cases[cite: 1].

## 🧪 Testing & Benchmarking[cite: 1]

### Unit Tests[cite: 1]
```bash
# LRU test
make test_lru

# LRU-K test  
make test_lruk_run

# ARC basic test
g++ -std=c++14 -Wall -g ARC/test_arc_lfu.cpp -o test_arc_lfu && ./test_arc_lfu
```[cite: 1]

### Performance Benchmarking[cite: 1]
```bash
# Compile
g++ -std=c++14 -Wall -O2 -o benchmark_hitrate benchmark_hitrate.cpp

# Run
./benchmark_hitrate
```[cite: 1]

**Test Scale**[cite: 1]:
- Working set size: 100,000 distinct keys[cite: 1]
- Total accesses: 1,000,000 times[cite: 1]
- Capacity range: 20, 100, 1000[cite: 1]

**Test Scenarios**[cite: 1]:
1. **Hotspot (80-20)**: 20% of keys account for 80% of accesses[cite: 1]
2. **Time Locality**: Recently accessed keys are more likely to be accessed again[cite: 1]
3. **Periodic**: Periodic access patterns[cite: 1]
4. **Uniform Random**: Completely random access[cite: 1]

## 📊 Benchmark Results[cite: 1]

### Average Hit Rate Rankings[cite: 1]

| Workload | Best | Runner-up | Difference |
|----------|------|-----------|------------|
| **Hotspot** | LFU | LRU | LFU +21%[cite: 1] |
| **Time Locality** | Tie | - | No difference (~65%)[cite: 1] |
| **Periodic** | LFU | LRU | LFU +7%[cite: 1] |
| **Random** | Tie | - | No difference (~0.4%)[cite: 1] |

### Performance Comparison[cite: 1]

| Cache | Relative Speed | Memory Overhead | Applicable Scenarios |
|-------|----------------|-----------------|----------------------|
| LRU | Baseline (1x)[cite: 1] | Low[cite: 1] | General purpose, strong temporal locality[cite: 1] |
| LFU | Slow (3-5x)[cite: 1] | Medium[cite: 1] | Concentrated hotspots[cite: 1] |
| LRU-K | Slower (2-3x)[cite: 1] | Medium[cite: 1] | Clear hot/cold distinction[cite: 1] |
| HashLRU | Baseline (1x)[cite: 1] | Low[cite: 1] | Multi-threaded high concurrency[cite: 1] |
| HashLFU | Slow (3-5x)[cite: 1] | Medium[cite: 1] | Concurrent hotspot scenarios[cite: 1] |
| ARC | Medium (1.5-2x)*[cite: 1] | Medium[cite: 1] | Adaptive general-purpose**[cite: 1] |

*\* ARC currently has a bug under small capacities; performance data is incomplete[cite: 1].*  
*\*\* Pending fixes for integrity issues[cite: 1].*

## 🔧 Compilation & Usage[cite: 1]

### Compilation Requirements[cite: 1]
- C++14 or higher[cite: 1]
- macOS/Linux/Windows (POSIX compatible)[cite: 1]
- g++ or clang++[cite: 1]

### Basic Compilation[cite: 1]
```bash
# Using Makefile
make

# Or manual compilation
g++ -std=c++14 -Wall -O2 -o your_program your_file.cpp
```[cite: 1]

### Basic Usage Examples[cite: 1]

**LRU Cache**[cite: 1]:
```cpp
#include "LRU.h"

int main() {
    LRUCache<int, int> cache(100);  // Capacity 100
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```[cite: 1]

**LFU Cache**[cite: 1]:
```cpp
#include "LFUK.cpp"

int main() {
    LFUKCache<int, int> cache(100);  // Capacity 100
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```[cite: 1]

**Concurrent LRU**[cite: 1]:
```cpp
#include "HashLRU.cpp"

int main() {
    HashLRUKCache<int, int> cache(100, 8);  // Capacity 100, 8 shards
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```[cite: 1]

**ARC Cache**[cite: 1]:
```cpp
#include "ARC/ARCCache.cpp"

int main() {
    ARCCache<int, int> cache(100, 5);  // Capacity 100, transformThreshold=5
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```[cite: 1]

## 🐛 Known Issues[cite: 1]

### ARC Edge Case Bug[cite: 1]
- **Symptoms**: Segmentation fault under specific hotspot workloads when capacity is small ($\le 20$)[cite: 1].
- **Trigger Conditions**: Hotspot access pattern, capacity of 5-20, large working set (10k+ keys)[cite: 1].
- **Impact**: Works normally when capacity $\ge 20$; reliability uncertain when capacity $< 5$[cite: 1].
- **Root Cause**: Not yet localized; likely in ghost cache coordination or capacity decay logic[cite: 1].
- **Status**: Pending fix[cite: 1].

### HashLRU/HashLFU Compilation Issues[cite: 1]  
- **Symptoms**: Multiple inclusions of `LFUK.cpp` lead to class redefinition errors[cite: 1].
- **Solution**: Convert `LFUK.cpp` into a header file or use include guards[cite: 1].
- **Status**: Pending improvement[cite: 1].

## 📈 Next Steps & Improvements[cite: 1]

### High Priority[cite: 1]
1. [ ] Fix the ARC small-capacity segmentation fault[cite: 1]
2. [ ] Fix HashLFU compilation issues and integrate it into benchmarks[cite: 1]
3. [ ] Implement W-TinyLFU (hybrid scheme combining LRU + probabilistic filtering)[cite: 1]
4. [ ] Add multi-threaded concurrency testing[cite: 1]

### Medium Priority[cite: 1]
5. [ ] Implement the CLOCK replacement algorithm (LRU approximation)[cite: 1]
6. [ ] Add TTL (Time To Live) support[cite: 1]
7. [ ] Implement Bloom Filter optimization for miss detection[cite: 1]
8. [ ] Performance profiling (cache line alignment, SIMD optimization)[cite: 1]

### Low Priority[cite: 1]  
9. [ ] Support larger data types (std::string, custom objects)[cite: 1]
10. [ ] Distributed cache design (consistent hashing, network protocols)[cite: 1]

## 📚 References[cite: 1]

- **LRU**: Classic cache replacement policy algorithm[cite: 1]
- **LFU**: O(1) time complexity implementation using frequency buckets[cite: 1]
- **LRU-K**: IBM Research paper *"A Comparative Study of LRU and Clock Algorithms for Page Replacement"*[cite: 1]
- **ARC**: IBM Nimble system paper *"ARC: A Self-Tuning, Low Overhead Replacement Cache"*[cite: 1]
- **Concurrent Hash Table**: Google Abseil's sharded lock scheme[cite: 1]

## 📝 License[cite: 1]

MIT License[cite: 1]

## 👤 Author[cite: 1]

Jiang Sunny. 
Created as a learning project in cache algorithm design and implementation[cite: 1].

---

**Last Updated**: 2026-08-02[cite: 1]  
**Project Version**: v0.5 (ARC architecture complete, performance benchmarking complete, pending edge case fixes)[cite: 1]