#include <iostream>
#include <vector>
#include <random>
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Testing ARC with MINIMAL hotspot pattern\n";
    
    ARCCache<int, int> cache(20, 5);
    
    // Hotspot: 100 accesses, 20% from 5 hot keys, 80% from 15 cold keys
    std::vector<int> sequence;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> hotDist(0, 4);     // 5 hot keys
    std::uniform_int_distribution<> coldDist(5, 19);   // 15 cold keys
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    for(int i = 0; i < 100; ++i) {
        if(probDist(gen) < 0.8) {
            sequence.push_back(hotDist(gen));
        } else {
            sequence.push_back(coldDist(gen));
        }
    }
    
    std::cout << "Running minimal hotspot...\n";
    int hits = 0;
    for(size_t i = 0; i < sequence.size(); ++i) {
        int key = sequence[i];
        std::cout << "Access " << i << ": key=" << key;
        int value;
        if(cache.get(key, value)) {
            hits++;
            std::cout << " (HIT)\n";
        } else {
            std::cout << " (MISS, putting...";
            cache.put(key, key);
            std::cout << ")\n";
        }
    }
    
    std::cout << "Test passed! Hits: " << hits << "/100\n";
    return 0;
}
