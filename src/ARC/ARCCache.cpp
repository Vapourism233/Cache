#pragma once
#include "ARCLFUPart.h"
#include "ARCLRUPart.h"
#include <memory>
#include <unordered_map>

template<typename Key, typename Value>
class ARCCache {
    using NodeType = ARCNode<Key, Value>;
    using NodePtr = std::shared_ptr<NodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

    size_t capacity_;
    size_t transformThreshold_;

    std::unique_ptr<ARCLFUPart<Key, Value>> LFUPart_;
    std::unique_ptr<ARCLRUPart<Key, Value>> LRUPart_;

public:
    explicit ARCCache(size_t capacity, size_t transformThreshold)
        : capacity_(capacity)
        , transformThreshold_(transformThreshold)
        , LFUPart_(std::make_unique<ARCLFUPart<Key, Value>>(capacity / 2))
        , LRUPart_(std::make_unique<ARCLRUPart<Key, Value>>(capacity / 2, transformThreshold)) {}

    bool checkGhostCaches(const Key& key) {
        if( LRUPart_->checkGhost(key)) {
            if(LFUPart_->decreaseCapacity()) {
                LRUPart_->increaseCapacity();
            }
            return true;
        }
        else if(LFUPart_->checkGhost(key)) {
            if(LRUPart_->decreaseCapacity()) {
                LFUPart_->increaseCapacity();
            }
            return true;
        }
        return false;
    }

    bool get(const Key& key, Value& value) {
        checkGhostCaches(key);
        bool shouldTransform = false;
        if(LRUPart_->get(key, value, shouldTransform)) {
            if(shouldTransform) {
                LFUPart_->put(key, value, shouldTransform);
            }
            return true;
        }
        return LFUPart_->get(key, value, shouldTransform);
    }

    bool put(const Key& key, const Value& value) {
        checkGhostCaches(key);
        bool shouldTransform = false;
        LRUPart_->put(key, value, shouldTransform);
        if(shouldTransform) {
            LFUPart_->put(key, value, shouldTransform);
        }
        return true;
    }
};