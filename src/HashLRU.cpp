#include <memory>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include "headers/LRU.h"
#include "headers/LRUK.h"

template <typename Key, typename Value>
class HashLRUKCache {
public:
    HashLRUKCache(size_t capacity, int sliceNum)
        : capacity(capacity)
        , sliceNum(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency())
        {
            size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum));
            for (int i = 0; i < sliceNum; i++){
                lruSliceCaches.emplace_back(new LRUKCache<Key, Value>(sliceSize, sliceSize, 2));
                sliceMutexes.emplace_back(new std::mutex());
            }
        }
    // get method 1:
    bool get(Key key, Value& value) {
        size_t sliceIndex = Hash(key) % sliceNum;
        std::lock_guard<std::mutex> lock(*sliceMutexes[sliceIndex]);
        return lruSliceCaches[sliceIndex]->get(key, value);
    }

    void put(Key key, Value value) {
        size_t sliceIndex = Hash(key) % sliceNum;
        std::lock_guard<std::mutex> lock(*sliceMutexes[sliceIndex]);
        lruSliceCaches[sliceIndex]->put(key, value);
    }

private:
    size_t Hash(Key key) {
        std::hash<Key> hashFunc;
        return hashFunc(key);
    }

private:
    size_t capacity;
    int sliceNum;
    std::vector<std::unique_ptr<LRUKCache<Key, Value>>> lruSliceCaches;
    std::vector<std::unique_ptr<std::mutex>> sliceMutexes;
};