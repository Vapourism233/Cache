#include <iostream>
#include <vector>
#include <random>
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Reproducing exact crash sequence\n";
    
    ARCCache<int, int> cache(20, 5);
    
    // Generate exact sequence with seed 42
    std::vector<int> sequence;
    std::mt19937 gen(42);
    std::uniform_int_distribution<> hotDist(0, 4);
    std::uniform_int_distribution<> coldDist(5, 19);
    std::uniform_real_distribution<> probDist(0.0, 1.0);
    
    for(int i = 0; i < 100; ++i) {
        if(probDist(gen) < 0.8) {
            sequence.push_back(hotDist(gen));
        } else {
            sequence.push_back(coldDist(gen));
        }
    }
    
    std::cout << "Sequence generated. Accessing...\n";
    int hits = 0;
    for(size_t i = 0; i < 35; ++i) {  // Stop at 35 to pass crash point
        int key = sequence[i];
        std::cout << "  [" << i << "] Accessing key " << key << "...\n";
        std::cout.flush();
        
        int value;
        if(cache.get(key, value)) {
            std::cout << "    -> HIT\n";
            hits++;
        } else {
            std::cout << "    -> MISS (putting)\n";
            cache.put(key, key);
        }
    }
    
    std::cout << "\nReached access 35! Test passed!\n";
    return 0;
}
