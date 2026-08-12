#include <iostream>
#include <cassert>
#include <string>
#include "LRU.h"

void test_basic_put_get() {
    std::cout << "Running test: Basic Put/Get..." << std::endl;
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(2, "two");

    std::string val;
    assert(cache.get(1, val) && val == "one");
    assert(cache.get(2, val) && val == "two");
    std::cout << "The elements in the cache are: " << std::endl;
    // print the elements in the cache
    std::cout << "Key: 1, Value: " << val << std::endl;
    
    std::cout << "PASS" << std::endl;
}

void test_update_value() {
    std::cout << "Running test: Update Value..." << std::endl;
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");
    cache.put(1, "new_one");

    std::string val;
    assert(cache.get(1, val) && val == "new_one");
    std::cout << "PASS" << std::endl;
}

void test_eviction() {
    std::cout << "Running test: Eviction..." << std::endl;
    LRUCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30); // Should evict key 1

    int val;
    assert(!cache.get(1, val)); // Key 1 should be gone
    assert(cache.get(2, val) && val == 20);
    assert(cache.get(3, val) && val == 30);
    std::cout << "PASS" << std::endl;
}

void test_get_updates_recency() {
    std::cout << "Running test: Get Updates Recency..." << std::endl;
    LRUCache<int, int> cache(2);
    cache.put(1, 10);
    cache.put(2, 20);
    
    int val;
    cache.get(1, val); // Access key 1, making it most recently used
    
    cache.put(3, 30); // Should evict key 2, because 1 was just used

    assert(!cache.get(2, val)); // Key 2 should be gone
    assert(cache.get(1, val) && val == 10);
    assert(cache.get(3, val) && val == 30);
    std::cout << "PASS" << std::endl;
}

void test_zero_or_negative_capacity() {
    std::cout << "Running test: Zero or Negative Capacity..." << std::endl;
    LRUCache<int, int> cache(0);
    cache.put(1, 10);
    
    int val;
    assert(!cache.get(1, val));

    LRUCache<int, int> cache_neg(-1);
    cache_neg.put(1, 10);
    assert(!cache_neg.get(1, val));
    std::cout << "PASS" << std::endl;
}


int main() {
    test_basic_put_get();
    test_update_value();
    test_eviction();
    test_get_updates_recency();
    test_zero_or_negative_capacity();

    std::cout << "\nAll tests passed successfully!" << std::endl;
    return 0;
}

#include "LRUK.cpp"

void test_lruk_basic() {
    std::cout << "Running test: LRU-K Basic..." << std::endl;
    LRUKCache<int, std::string> cache(2, 5, 2); // capacity=2, history=5, k=2
    
    cache.put(1, "one");
    cache.put(2, "two");
    
    std::string val;
    // Keys currently accessed once. They shouldn't be in main cache yet.
    // get returns false because they haven't reached k=2
    assert(!cache.get(1, val));
    // Getting them above also increases access count to 2. So now they are in main cache!
    // Wait, getting them increases historyCount? Let's check logic:
    // When get() is called, historyCount++. If >= k_, we put in main cache.
    // The previous get(1) got history=2. It should have been added. Let's assert it's in main.
    assert(cache.get(1, val) && val == "one");
    std::cout << "PASS" << std::endl;
}
