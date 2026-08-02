#include <iostream>
#include "ARC/ARCCache.cpp"

int main() {
    std::cout << "Test 1: Create ARC cache\n";
    ARCCache<int, int> cache(20, 5);
    std::cout << "OK\n";
    
    std::cout << "Test 2: Put key 0\n";
    cache.put(0, 0);
    std::cout << "OK\n";
    
    std::cout << "Test 3: Get key 0\n";
    int value;
    bool hit = cache.get(0, value);
    std::cout << "Hit: " << hit << ", Value: " << value << "\n";
    
    std::cout << "Test 4: Put key 1-10\n";
    for(int i = 1; i <= 10; ++i) {
        cache.put(i, i);
    }
    std::cout << "OK\n";
    
    std::cout << "Test 5: Try to get key 15 (should miss, trigger checkGhost)\n";
    bool hit15 = cache.get(15, value);
    std::cout << "Hit: " << hit15 << "\n";
    
    std::cout << "Test 6: Now put key 15\n";
    cache.put(15, 15);
    std::cout << "OK\n";
    
    std::cout << "All tests passed!\n";
    return 0;
}
