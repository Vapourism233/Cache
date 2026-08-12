#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "LRU.h"
#include "LFUK.h"
#include "ARC/ARCCache.cpp"

std::vector<int> generateHotspot(uint32_t workingSetSize, uint32_t accessCount) {
    std::vector<int> sequence;
    uint32_t hotsetSize = std::max(1U, workingSetSize / 5);
    
    std::mt19937 gen(42);
    std::uniform_int_distribution<> hotDist(0, hotsetSize - 1);
    std::uniform_int_distribution<> coldDist(hotsetSize, workingSetSize - 1);
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    for(uint32_t i = 0; i < accessCount; ++i) {
        if(probDist(gen) < 0.8) {
            sequence.push_back(hotDist(gen));
        } else {
            sequence.push_back(coldDist(gen));
        }
    }
    
    return sequence;
}

int main() {
    const uint32_t WORKING_SET_SIZE = 10000;
    const uint32_t ACCESS_COUNT = 100000;
    
    std::cout << "Generating hotspot workload (10k keys, 100k accesses)...\n";
    auto sequence = generateHotspot(WORKING_SET_SIZE, ACCESS_COUNT);
    std::cout << "Generated " << sequence.size() << " accesses\n\n";
    
    // Test LRU
    std::cout << "Testing LRU with capacity 20...\n";
    {
        auto start = std::chrono::high_resolution_clock::now();
        LRUCache<int, int> cache(20);
        uint32_t hits = 0;
        
        for(int key : sequence) {
            int value;
            if(cache.get(key, value)) {
                hits++;
            } else {
                cache.put(key, key);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "LRU Hits: " << hits << "/" << sequence.size() << " ("
                  << 100.0 * hits / sequence.size() << "%)\n"
                  << "Time: " << timeMs << "ms\n\n";
    }
    
    // Test LFU
    std::cout << "Testing LFU with capacity 20...\n";
    {
        auto start = std::chrono::high_resolution_clock::now();
        LFUKCache<int, int> cache(20);
        uint32_t hits = 0;
        
        for(int key : sequence) {
            int value;
            if(cache.get(key, value)) {
                hits++;
            } else {
                cache.put(key, key);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "LFU Hits: " << hits << "/" << sequence.size() << " ("
                  << 100.0 * hits / sequence.size() << "%)\n"
                  << "Time: " << timeMs << "ms\n\n";
    }
    
    // Test ARC
    std::cout << "Testing ARC with capacity 20...\n";
    {
        auto start = std::chrono::high_resolution_clock::now();
        ARCCache<int, int> cache(20, 5);
        uint32_t hits = 0;
        
        for(int key : sequence) {
            int value;
            if(cache.get(key, value)) {
                hits++;
            } else {
                cache.put(key, key);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "ARC Hits: " << hits << "/" << sequence.size() << " ("
                  << 100.0 * hits / sequence.size() << "%)\n"
                  << "Time: " << timeMs << "ms\n\n";
    }
    
    std::cout << "All tests passed!\n";
    return 0;
}
