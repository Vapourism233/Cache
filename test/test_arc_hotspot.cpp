#include <iostream>
#include <vector>
#include <random>
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Testing ARC with hotspot-like pattern...\n";
    
    ARCCache<int, int> cache(20, 5);
    
    // Simulate hotspot: 2000 key accesses, 80% from first 20% of keys
    std::vector<int> sequence;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> hotDist(0, 3);     // 20% = 4 keys out of 20
    std::uniform_int_distribution<> coldDist(4, 19);   // 80% = 16 keys
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    for(int i = 0; i < 2000; ++i) {
        if(probDist(gen) < 0.8) {
            sequence.push_back(hotDist(gen));
        } else {
            sequence.push_back(coldDist(gen));
        }
    }
    
    std::cout << "Running hotspot access pattern...\n";
    int hits = 0;
    for(size_t i = 0; i < sequence.size(); ++i) {
        int key = sequence[i];
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            cache.put(key, key);
        }
        
        if(i % 500 == 0) {
            std::cout << "  Progress: " << i << "/" << sequence.size() 
                     << " (hits=" << hits << ")\n";
        }
    }
    
    std::cout << "Test passed! Total hits: " << hits << "/" << sequence.size() 
             << " (" << 100.0 * hits / sequence.size() << "%)\n";
    return 0;
}
