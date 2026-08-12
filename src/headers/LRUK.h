#ifndef LRU_K_CACHE_H
#define LRU_K_CACHE_H  

#include <unordered_map>
#include <memory>
#include "LRU.h"

template<typename Key, typename Value>
class LRUKCache : public LRUCache<Key, Value> {
public:
    LRUKCache(int capacity, int historyCapacity, size_t k) 
        : LRUCache<Key, Value>(capacity)
        , historyList_(new LRUCache<Key, size_t>(historyCapacity))
        , k_(k) 
    {}

    bool get(Key key, Value& value) 
    {
        bool inMainCache = LRUCache<Key, Value>::get(key, value);

        size_t historyCount = 0;
        historyList_->get(key, historyCount);
        historyCount++;
        historyList_->put(key, historyCount);

        if (inMainCache) return true;

        if (historyCount >= k_) 
        {
            auto it = historyValueMap_.find(key);
            if (it != historyValueMap_.end()) 
            {
                value = it->second;
                historyList_->remove(key);
                historyValueMap_.erase(it);
                LRUCache<Key, Value>::put(key, value);
                return true;
            }
        }
        return false;
    }

    void put(Key key, Value value) 
    {
        Value existingValue{};
        bool inMainCache = LRUCache<Key, Value>::get(key, existingValue);
        
        if (inMainCache) 
        {
            LRUCache<Key, Value>::put(key, value);
            return;
        }
        
        size_t historyCount = 0;
        historyList_->get(key, historyCount);
        historyCount++;
        historyList_->put(key, historyCount);
        
        historyValueMap_[key] = value;
        
        if (historyCount >= k_) 
        {
            historyList_->remove(key);
            historyValueMap_.erase(key);
            LRUCache<Key, Value>::put(key, value);
        }
    }

private:
    size_t k_;
    std::unique_ptr<LRUCache<Key, size_t>> historyList_; 
    std::unordered_map<Key, Value> historyValueMap_; 
};

#endif // LRU_K_CACHE_H
