#ifndef HASH_LRU_CACHE_H
#define HASH_LRU_CACHE_H

#include <vector>
#include <memory>
#include <cmath>
#include <thread>
#include <functional>
#include <mutex>
#include "LRUK.h"

// 分片 LRU (HashLRU) 优化：对 LRU-K进行分片，缩小锁的粒度，大幅提高高并发场景下的性能
template<typename Key, typename Value>
class HashLRUKCache
{
public:
    HashLRUKCache(size_t capacity, int historyCapacity, size_t k, int sliceNum)
        : capacity_(capacity)
        , sliceNum_(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency())
    {
        size_t sliceSize = std::ceil(capacity_ / static_cast<double>(sliceNum_));
        size_t historySliceSize = std::ceil(historyCapacity / static_cast<double>(sliceNum_));
        
        for (int i = 0; i < sliceNum_; ++i)
        {
            lruSliceCaches_.emplace_back(new LRUKCache<Key, Value>(sliceSize, historySliceSize, k)); 
            sliceMutexes_.emplace_back(new std::mutex());
        }
    }

    void put(Key key, Value value)
    {
        size_t sliceIndex = Hash(key) % sliceNum_;
        std::lock_guard<std::mutex> lock(*sliceMutexes_[sliceIndex]);
        lruSliceCaches_[sliceIndex]->put(key, value);
    }

    bool get(Key key, Value& value)
    {
        size_t sliceIndex = Hash(key) % sliceNum_;
        std::lock_guard<std::mutex> lock(*sliceMutexes_[sliceIndex]);
        return lruSliceCaches_[sliceIndex]->get(key, value);
    }

private:
    size_t Hash(Key key)
    {
        std::hash<Key> hashFunc;
        return hashFunc(key);
    }

private:
    size_t capacity_;
    int    sliceNum_;
    
    std::vector<std::unique_ptr<LRUKCache<Key, Value>>> lruSliceCaches_; 
    std::vector<std::unique_ptr<std::mutex>> sliceMutexes_; 
};

#endif // HASH_LRU_CACHE_H
