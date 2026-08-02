#pragma once
#include "ARCKCacheNode.h"
#include <memory>
#include <list>
#include <unordered_map>
#include <mutex>

template<typename Key, typename Value>
class ARCLRUPart {
    using NodeType = ARCNode<Key, Value>;
    using NodePtr = std::shared_ptr<NodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

private:
    size_t capacity_;
    size_t ghost_capacity_;
    size_t transformThreshold_;
    std::mutex mutex_;

    NodeMap mainCache_;
    NodeMap ghostCache_;
    NodePtr mainHead_, mainTail_;
    NodePtr ghostHead_, ghostTail_;

public:
explicit ARCLRUPart(size_t capacity, size_t transformThreshold)
        : capacity_(capacity)
        , ghost_capacity_(capacity / 2)
        , transformThreshold_(transformThreshold)
        , mainHead_(std::make_shared<NodeType>())
        , mainTail_(std::make_shared<NodeType>())
        , ghostHead_(std::make_shared<NodeType>())
        , ghostTail_(std::make_shared<NodeType>())
    { initializeLists(); }

private:
    void initializeLists() {
        mainHead_->next = mainTail_;
        mainTail_->prev = mainHead_;
        ghostHead_->next = ghostTail_;
        ghostTail_->prev = ghostHead_;
    }

    void addToFront(NodePtr node) {
        node->next = mainHead_->next;
        node->prev = mainHead_;
        mainHead_->next->prev = node;
        mainHead_->next = node;
    }

    void moveToFront(NodePtr node) {
        node->prev.lock()->next = node->next;
        node->next->prev = node->prev;
        addToFront(node);
    }

    void addToGhost(NodePtr node) {
        ghostCache_[node->getKey()] = node;
        node->next = ghostHead_->next;
        node->prev = ghostHead_;
        ghostHead_->next->prev = node;
        ghostHead_->next = node;
    }   

    void removeOldestFromGhost() {
        NodePtr oldest = ghostTail_->prev.lock();
        if(oldest && oldest != ghostHead_){
            removeFromGhost(oldest);
            ghostCache_.erase(oldest->getKey());
        }
    }

    void removeFromGhost(NodePtr node){
        node->prev.lock()->next = node->next;
        node->next->prev = node->prev;
        node->next = nullptr;
        node->prev.reset();
    }

    void evictLeastRecent() {
        NodePtr lrulastNode = mainTail_->prev.lock();
        if(lrulastNode && lrulastNode != mainHead_) { // to avoid evicting the dummy head node
            removeFromMain(lrulastNode);
            mainCache_.erase(lrulastNode->getKey());
            addToGhost(lrulastNode);
            if(ghostCache_.size() > ghost_capacity_) {
                removeOldestFromGhost();
            }
        }
    }

    void removeFromMain(NodePtr node) {
        node->prev.lock()->next = node->next;
        node->next->prev = node->prev;
        node->next = nullptr;
        node->prev.reset();
    }

public:
    bool increaseCapacity(){
        capacity_++;
        return true;
    }

    bool decreaseCapacity(){
        if(capacity_ <= 0) return false;
        if(mainCache_.size() >= capacity_){
            evictLeastRecent();
        }
        capacity_--;
        return true;
    }
bool checkGhost(const Key& key) {
        auto it = ghostCache_.find(key);
        if(it != ghostCache_.end()){
            if(mainCache_.size() <= capacity_){
                mainCache_[key] = it->second;
                ghostCache_.erase(key);
                removeFromGhost(it->second);
                addToFront(it->second);
                return true;
            }
        }
        else {
            if(mainCache_.size() >= capacity_){
                evictLeastRecent();
            }
        }
        return false;
    }

bool get(const Key& key, Value& value, bool& shouldTransform) {
        auto it = mainCache_.find(key);
        if(it != mainCache_.end()) {
            NodePtr node = it->second;
            value = node->getValue();
            node->incrementAccessCount();
            shouldTransform = (node->getAccessCount() >= transformThreshold_);
            moveToFront(node);
            return true;
        }
        it = ghostCache_.find(key);
        if(it != ghostCache_.end()) {
            NodePtr node = it->second;
            value = node->getValue();
            node->incrementAccessCount();
            shouldTransform = (node->getAccessCount() >= transformThreshold_);
            mainCache_[key] = node;
            ghostCache_.erase(it);
            removeFromGhost(node);
            addToFront(node);
            return true;
        }
        return false;
    }

    void put(const Key& key, const Value& value, bool& shouldTransform) {
        auto it = mainCache_.find(key);
        if(it != mainCache_.end()) {
            NodePtr node = it->second;
            node->setValue(value);
            node->incrementAccessCount();
            shouldTransform = (node->getAccessCount() >= transformThreshold_);
            moveToFront(node);
        }
        else {
            if(mainCache_.size() >= capacity_) {
                evictLeastRecent();
                it = ghostCache_.find(key);
                if(it != ghostCache_.end()){
                    NodePtr node = it->second;
                    mainCache_[key] = node;
                    ghostCache_.erase(it);
                    node->setValue(value);
                    node->incrementAccessCount();
                    shouldTransform = (node->getAccessCount() >= transformThreshold_);
                    moveToFront(node);
                }
                else{
                    NodePtr newNode = std::make_shared<NodeType>(key, value);
                    mainCache_[key] = newNode;
                    addToFront(newNode);
                }
            }
            else{
                NodePtr newNode = std::make_shared<NodeType>(key, value);
                mainCache_[key] = newNode;
                addToFront(newNode);
            }
        }
    }

};