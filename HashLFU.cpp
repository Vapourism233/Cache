#include "LFUK.cpp"
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>

template<typename Key, typename Value>
class LFUHashCache {
public:
    LFUHashCache(size_t capacity, int sliceNum, int maxAverageNum = 10)
    : capacity_(capacity), sliceNum_(sliceNum), maxAverageNum_(maxAverageNum) {
        sliceNum_ = sliceNum_ > 0 ? sliceNum_ : std::thread::hardware_concurrency();
        size_t sliceSize = std::ceil(capacity_ / static_cast<double>(sliceNum_));
        for(int i = 0; i < sliceNum_; ++i) {
            lfuSliceCaches_.emplace_back(new LFUKCache<Key, Value>(sliceSize, maxAverageNum_));
            sliceMutexes_.emplace_back(new std::mutex());
        }

    }

    size_t Hash(Key key) {
        std::hash<Key> hashFunc;
        return hashFunc(key);
    }

    void put(Key key, Value value) {
        size_t sliceIndex = Hash(key) % sliceNum_;
                std::lock_guard<std::mutex> lock(*sliceMutexes_[sliceIndex]);
        lfuSliceCaches_[sliceIndex]->put(key, value);
    }       

    bool get(Key key, Value& value) {
        size_t sliceIndex = Hash(key) % sliceNum_;
        std::lock_guard<std::mutex> lock(*sliceMutexes_[sliceIndex]);
        return lfuSliceCaches_[sliceIndex]->get(key, value);
    }

private:
    size_t capacity_;
    int sliceNum_;
    int maxAverageNum_;

    std::vector<std::unique_ptr<LFUKCache<Key, Value>>> lfuSliceCaches_;
    std::vector<std::unique_ptr<std::mutex>> sliceMutexes_;
};