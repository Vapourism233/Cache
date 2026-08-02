#pragma once
#include "ARCKCacheNode.h"
#include <mutex>
#include <list>
#include <memory>
#include <unordered_map>
#include <map>

template<typename Key, typename Value>
class ARCLFUPart {
    using NodeType = ARCNode<Key, Value>;
    using NodePtr = std::shared_ptr<NodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

private:
    size_t capacity_;
    size_t ghost_capacity_;
    size_t transformThreshold_;
    size_t minFreq_;

    NodeMap mainCache_;
    NodeMap ghostCache_;
    
    NodePtr ghostHead_;
    NodePtr ghostTail_;

    std::unordered_map<size_t, std::list<NodePtr>> freqToFreqList_;

public:
explicit ARCLFUPart(size_t capacity_)
    : capacity_(capacity_)
    , ghost_capacity_(capacity_ / 2)
    , transformThreshold_(capacity_ / 4) {initializeGhostList(); minFreq_ = 1;}
    
    void initializeGhostList() {
        ghostHead_ = std::make_shared<NodeType>();
        ghostTail_ = std::make_shared<NodeType>();
        ghostHead_->next = ghostTail_;
        ghostTail_->prev = ghostHead_;
    }

private:

    void addToFreqList(NodePtr node, size_t freq) {
        freqToFreqList_[freq].push_front(node);
        node->setFreq(freq);
    }
    
    void increaseFreq(NodePtr node) {
        size_t freq = node->getFreq();
        // 1. remove from the current freq list
        removeFromFreqList(node);
        // 2. add to new fre list, to the front
        addToFreqList(node, freq + 1);
    }

    void evictLowestFreq(){
        NodePtr nodeToRemove = freqToFreqList_[minFreq_].back();
        removeFromFreqList(nodeToRemove);
        mainCache_.erase(nodeToRemove->getKey());
        if(ghostCache_.size() >= ghost_capacity_) {
            removeOldestFromGhost();
        }
        addToGhost(nodeToRemove);
    }

    void removeOldestFromGhost() {
        NodePtr nodeToRemove = ghostTail_->prev.lock();
        if(nodeToRemove && nodeToRemove != ghostHead_) {
            removeFromGhost(nodeToRemove);
            ghostCache_.erase(nodeToRemove->getKey());
        }
    }

    void addToGhost(NodePtr node) {
        ghostCache_[node->getKey()] = node;
        node->next = ghostHead_->next;
        node->prev = ghostHead_;
        ghostHead_->next->prev = node;
        ghostHead_->next = node;
    }

    void removeFromFreqList(NodePtr node) {
        size_t freq = node->getFreq();
        auto& freqList = freqToFreqList_[freq];
        freqList.remove(node);
        if(freqList.empty()) {
            freqToFreqList_.erase(freq);
            if(freq == minFreq_) {
                updateMinFreq();
            }
        }
    }

    void addToFreqList(NodePtr node) {
        size_t freq = node->getFreq();
        freqToFreqList_[freq].push_front(node);
    }

    void updateMinFreq() {
        if(freqToFreqList_.empty()) {
            minFreq_ = INT8_MAX;
            return;
        }
        minFreq_ = INT8_MAX;
        for(auto it = freqToFreqList_.begin(); it != freqToFreqList_.end(); it++) {
            if(!it->second.empty() ) {
                minFreq_ = std::min(minFreq_, it->first);
            }
        }
    }

    void removeFromGhost(NodePtr node) {
        node->next->prev = node->prev;
        node->prev.lock()->next = node->next;
        node->next = nullptr;
        node->prev.reset();
    }

public:
    bool increaseCapacity() {
        capacity_ = capacity_ + 1;
        return true;
    }

    bool decreaseCapacity() {
        if(capacity_ > 0) {
            capacity_ -= 1;
            return true;
        }
        else {
            capacity_ = 0;
            return false;
        }
    }
    bool checkGhost(const Key& key) {
        auto it = ghostCache_.find(key);
        if(it != ghostCache_.end()) {
            return true;
        }
        return false;
    }

    bool get(const Key& key, Value& value, bool& shouldTransform) {
        auto it = mainCache_.find(key);
        if(it != mainCache_.end()) {
            NodePtr node = it->second;
            node->incrementAccessCount();
            shouldTransform = (node->getAccessCount() >= transformThreshold_);
            removeFromFreqList(node);
            node->freq_++;
            addToFreqList(node);
            value = node->getValue();
            return true;
        }
        else {
            auto it = ghostCache_.find(key);
            if (it != ghostCache_.end()) {
                NodePtr node = it->second;
                node->incrementAccessCount();
                shouldTransform = (node->getAccessCount() >= transformThreshold_);
                if(mainCache_.size() < capacity_) {
                    mainCache_[key] = node;
                    ghostCache_.erase(key);
                    removeFromGhost(node);
                    addToFreqList(node);
                    // here we should increase capacity since we are reviving from ghost
                    value = node->getValue();
                    return true;
                }
                else {
                    // here we should increase capacity since we are reviving from ghost
                    mainCache_[key] = node;
                    ghostCache_.erase(key);
                    removeFromGhost(node);
                    evictLowestFreq();
                    addToFreqList(node);
                    value = node->getValue();
                    return true;
                }
            }
            else {
                return false;
            }
        }
    }

    void put(const Key& key, const Value& value, bool& shouldTransform) {
        auto it = mainCache_.find(key);
        if(it != mainCache_.end()) {
            it->second->setValue(value);
            it->second->incrementAccessCount();
            shouldTransform = (it->second->getAccessCount() >= transformThreshold_);
            increaseFreq(it->second);
            return;
        }
        else {
            if(ghostCache_.find(key) != ghostCache_.end()) {
                it = ghostCache_.find(key);
                it->second->setValue(value);
                it->second->incrementAccessCount();
                shouldTransform = (it->second->getAccessCount() >= transformThreshold_);
                if(mainCache_.size() < capacity_) {
                    NodePtr node = it->second;
                    mainCache_[key] = node;
                    ghostCache_.erase(key);
                    removeFromGhost(node);
                    addToFreqList(node);
                    // here we should increase capacity since we are reviving from ghost
                    return;
                }
                else {
                    NodePtr node = it->second;
                    mainCache_[key] = node;
                    ghostCache_.erase(key);
                    removeFromGhost(node);
                    evictLowestFreq();
                    addToFreqList(node);
                    // here we should increase capacity since we are reviving from ghost
                    return;
                }
            }
            else {
                NodePtr newNode = std::make_shared<NodeType>(key, value);
                if(mainCache_.size() < capacity_) {
                    mainCache_[key] = newNode;
                    addToFreqList(newNode);
                    minFreq_ = 1;
                    newNode->incrementAccessCount();
                    shouldTransform = (newNode->getAccessCount() >= transformThreshold_);
                    return;
                }
                else {
                    evictLowestFreq();
                    mainCache_[key] = newNode;
                    addToFreqList(newNode);
                    minFreq_ = 1;
                    newNode->incrementAccessCount();
                    shouldTransform = (newNode->getAccessCount() >= transformThreshold_);
                    return;
                }
            }
        }
    }

    // Test-only introspection helpers.
    size_t getCapacity() const { return capacity_; }
    bool inMainCache(const Key& key) const { return mainCache_.find(key) != mainCache_.end(); }
    bool inGhostCache(const Key& key) const { return ghostCache_.find(key) != ghostCache_.end(); }
    size_t mainSize() const { return mainCache_.size(); }
};