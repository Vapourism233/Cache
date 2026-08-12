#include <iostream>
#include <vector>
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Test with 10k working set, sequential access\n";
    
    ARCCache<int, int> cache(20, 5);
    
    // First, put many keys
    std::cout << "Putting keys 0-999...\n";
    for(int i = 0; i < 1000; ++i) {
        cache.put(i, i);
    }
    std::cout << "OK\n";
    
    // Now access with larger range
    std::cout << "Getting keys 5000-5019 (not in cache)...\n";
    int hits = 0;
    for(int i = 5000; i < 5020; ++i) {
        int value;
        if(cache.get(i, value)) {
            hits++;
        }
    }
    std::cout << "Hits: " << hits << "\n";
    
    std::cout << "Test passed!\n";
    return 0;
}
