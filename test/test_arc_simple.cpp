#include <iostream>
#include <vector>
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Testing ARC with simple sequential puts (no workload)...\n";
    
    ARCCache<int, int> cache(20, 5);
    
    std::cout << "Putting 1000 sequential keys...\n";
    for(int i = 0; i < 1000; ++i) {
        cache.put(i, i);
        if(i % 100 == 0) std::cout << "  Put " << i << "\n";
    }
    
    std::cout << "Testing gets...\n";
    for(int i = 0; i < 100; ++i) {
        int val;
        cache.get(i, val);
    }
    
    std::cout << "Test passed!\n";
    return 0;
}
