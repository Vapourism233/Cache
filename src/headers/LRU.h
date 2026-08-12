#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>
#include <memory>

template<typename Key, typename Value> class LRUCache; // forward declaration

template<typename Key, typename Value>
class Node{
public:
    Key key;
    Value val;
    std::shared_ptr<Node<Key, Value>> next;
    std::weak_ptr<Node<Key, Value>> prev;

    Node(Key key, Value val){
        this->key = key;
        this->val = val;
        this->next = std::shared_ptr<Node<Key, Value>>();
        this->prev = std::weak_ptr<Node<Key, Value>>();
    }
};

template<typename Key, typename Value>
class doubleLinkedList {
    friend class LRUCache<Key, Value>;
    std::shared_ptr<Node<Key, Value>> head;
    std::shared_ptr<Node<Key, Value>> tail;

public:
    doubleLinkedList(){
        head = std::make_shared<Node<Key, Value>>(Key(), Value());
        tail = std::make_shared<Node<Key, Value>>(Key(), Value());
        head->next = tail;
        tail->prev = head;
    }
};

// Abstract cache policy interface.
// Declares the core operations every cache replacement policy
// (LRU, LRU-K, LFU, ARC, ...) must provide, without any concrete implementation.
template<typename Key, typename Value>
class CachePolicy {
public:
    virtual ~CachePolicy() = default;

    // Insert a new key/value pair, or update the value if the key already exists.
    virtual void put(Key key, Value value) = 0;

    // Look up a key. On a hit, write the stored value into `value` and return true;
    // on a miss, return false.
    virtual bool get(Key key, Value& value) = 0;

    // Remove a key from the cache if it is present.
    virtual void remove(Key key) = 0;
};

template<typename Key, typename Value>
class LRUCache : public CachePolicy<Key, Value>{
public:

    // methods for LRU-K
    size_t getAccessCount()const {
        return accessCount;
    }
    void increamentAccessCount(){
        accessCount++;
    }

private:
    int capacity;
    doubleLinkedList<Key, Value> cacheList;
    std::unordered_map<Key, std::shared_ptr<Node<Key, Value>>> cache;
    // access count
    size_t accessCount;

    void moveToEnd(std::shared_ptr<Node<Key, Value>> node){
        // disconnect the node from the list
        auto prevNode = node->prev.lock();
        auto nextNode = node->next;
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
        // connect the node to the end of the list
        addToEnd(node);
    }

    void addToEnd(std::shared_ptr<Node<Key, Value>> node){
        auto prevNode = cacheList.tail->prev.lock();
        node->prev = prevNode;
        prevNode->next = node;
        node->next = cacheList.tail;
        cacheList.tail->prev = node;
    }

    void removeNode(std::shared_ptr<Node<Key, Value>> node){
        auto prevNode = node->prev.lock();
        auto nextNode = node->next;
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }

    void evictLeastRecentlyUsed(){
        std::shared_ptr<Node<Key, Value>> node = cacheList.head->next;
        removeNode(node);
        cache.erase(node->key);
    }

    public:
    void remove(Key key) override {
        if(cache.find(key) != cache.end()){
            std::shared_ptr<Node<Key, Value>> node = cache[key];
            removeNode(node);
            cache.erase(key);
        }
    }

public:
    LRUCache(int capacity){
        this->capacity = capacity;
    }
    
    bool get(Key key, Value& value) override {
        // corner case
        if(cache.find(key) == cache.end()){
            return false;
        }
        std::shared_ptr<Node<Key, Value>> node = cache[key];
        // move the node to the end of the list
        moveToEnd(node);
        value = node->val;
        return true;
    }

    // put method has done the following things:
    // 1. If the key already exists, update the value and move the node to the end of the list.
    // 2. If the key does not exist, insert a new node to the end of the list. If the cache is full, remove the least recently used item before inserting the new node.
    void put(Key key, Value val) override {
        if(capacity <= 0){
            return;
        }
        // corner case
        if(cache.find(key) != cache.end()){
            std::shared_ptr<Node<Key, Value>> node = cache[key];
            node->val = val;
            moveToEnd(node);
            return;
        }
        if(cache.size() >= capacity){
            // remove the least recently used item in the cache
            std::shared_ptr<Node<Key, Value>> node = cacheList.head->next;
            // remove the node from the list
            removeNode(node);
            cache.erase(node->key);
        }
        // insert from the back of the list
        std::shared_ptr<Node<Key, Value>> node = std::make_shared<Node<Key, Value>>(key, val);
        cache[key] = node;
        addToEnd(node);
    }
};

#endif // LRU_CACHE_H
