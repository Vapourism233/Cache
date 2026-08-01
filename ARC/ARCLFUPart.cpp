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
    size_t minFreq_;
    size_t transformThreshold_;
    
    std::map<size_t, std::pair<NodePtr, NodePtr>> freqToList_;
    NodeMap mainCache_, ghostCache_;

public:
explicit ARCLFUPart(size_t capacity_)
    : capacity_(capacity)
    , ghost_capacity_(capacity / 2)
    , mainHead_(std::make_shared<NodeType>())
    , mainTail_(std::make_shared<NodeType>())
    , ghostHead_(std::make_shared<NodeType>())
    , ghostTail_(std::make_shared<NodeType>())
    {initializeLists();}

private:
    void initializeLists() {
        mainHead_->next = mainTail_;
        mainTail_->prev = mainHead_;
        ghostHead_->next = ghostTail_;
        ghostTail_->prev = ghostHead_;
    }

    void addToFreqList(NodePtr node, size_t freq) {
        freqToList_[freq].push_front(node);
        node->setFreq(freq);
    }
    
    void increaseFreq(NodePtr node) {
        size_t freq = node->getFreq();
        // 1. remove from the current freq list
        removeFromCurrentFreqList(node);
        // 2. add to new fre list, to the front
        addToFreqList(node, freq + 1);
    }

    void evictLowestFreq(){
        NodePtr nodeToEvict = freqToFreqList_.end()->second.back();
        freqToFreqList_.end()-second.pop_back();
        mainCache_.erase(nodeToEvict->getKey());
        addToGhost(nodeToEvict);
        if(ghostCache_.size() > ghost_capacity_) {
            removeOldestFromGhost();
        }
    }

    void removeOldestFromGhost() {
        NodePtr nodeToRemove = ghostTail_->prev.lock();
        if(nodeToRemove && nodeToRemove != ghostHead_) {
            removeFromGhost(nodeToRemove);
            ghostCache_.erase(nodeToRemove->getKey());
        }
    }

    void addToGhost(NodePtr node) {
        node->next = ghostHead_->next;
        node->prev = ghostHead_;
        ghostHead_->next->prev = node;
        ghostHead_->next = node;
    }

    void removeFromCurrentFreqList(NodePtr node) {
        size_t freq = node->getFreq();
        auto& pair = freqToFreqList_[freq];
        // if the list is empty, remove the freq entry
        if(pair.first == node && pair.second == node) {

        // Remove node from the list
        node->prev.lock()->next = node->next;
        node->next->prev = node->prev;
        node->next = nullptr;
        node->prev.reset();
    }

public:
    bool get(Key& key) {
        NodePtr node = mainCache_[key];
        size_t oldFreq = node->getFreq();
        removeNodeFroCurrentFreqList(node);
        if
    }

    bool put(Key& key, Value& value) {

    }

};