#include <iostream>
#include <vector>
#include <random>
#include "LRU.h"
#include "LFUK.h"
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Testing LRU...\n";
    {
        LRUCache<int, int> cache(20);
        for(int i = 0; i < 100; ++i) {
            int val;
            if(!cache.get(i, val)) {
                cache.put(i, i);
            }
        }
        std::cout << "LRU: OK\n";
    }
    
    std::cout << "Testing LFU...\n";
    {
        LFUKCache<int, int> cache(20);
        for(int i = 0; i < 100; ++i) {
            int val;
            if(!cache.get(i, val)) {
                cache.put(i, i);
            }
        }
        std::cout << "LFU: OK\n";
    }
    
    std::cout << "Testing ARC...\n";
    {
        ARCCache<int, int> cache(20, 5);
        for(int i = 0; i < 100; ++i) {
            int val;
            if(!cache.get(i, val)) {
                cache.put(i, i);
            }
        }
        std::cout << "ARC: OK\n";
    }
    
    std::cout << "All tests passed!\n";
    return 0;
}
