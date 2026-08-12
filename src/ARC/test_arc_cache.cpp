#include "ARCCache.cpp"
#include <iostream>

int main() {
    ARCCache<int, int> cache(20, 5);  // 更大的容量
    
    // Put items
    for (int i = 1; i <= 10; i++) {
        cache.put(i, i * 100);
    }
    
    // Get them back
    int hits = 0;
    for (int i = 1; i <= 10; i++) {
        int v = 0;
        if (cache.get(i, v)) {
            hits++;
        }
    }
    
    std::cout << "Hits: " << hits << " / 10\n";
    
    if (hits == 10) {
        std::cout << "Basic test passed.\n";
        return 0;
    }
    return 1;
}
