#include <memory>
#include <list>
#include <unordered_map>
#include <stdexcept>
#include <mutex>
#include <thread>

template<typename Key, typename Value> class LFUKCache; // forward declaration, for friend class declaration

template<typename Key, typename Value>
class FreqList {
private:
    struct Node {
        Key key;
        int freq;
        Value value;
        std::shared_ptr<Node> next;
        std::weak_ptr<Node> prev;

        Node(Key k, Value v) : key(k), freq(1), value(v), next(nullptr) {}
    };

    using NodePtr = std::shared_ptr<Node>;
    int freq_;
    NodePtr head_; // Dummy head node
    NodePtr tail_; // Dummy tail node

public:
    explicit FreqList(int n) : freq_(n) {
        head_ = std::make_shared<Node> (Key(), Value()); // Dummy head node
        tail_ = std::make_shared<Node> (Key(), Value()); // Dummy tail node
        head_->next = tail_;
        tail_->prev = head_;
    }

    bool isEmpty() const { // const for thread safety
        return head_->next == tail_;
    }

    void addToTail(std::shared_ptr<Node> node) {
        auto prevNode = tail_->prev.lock();
        node->prev = prevNode;
        prevNode->next = node;
        node->next = tail_;
        tail_->prev = node;
    }

    void removeNode(std::shared_ptr<Node> node) {
        auto prevNode = node->prev.lock();
        auto nextNode = node->next;
        node->next = nullptr;
        node->prev.reset();
        prevNode->next = nextNode;
        nextNode->prev = prevNode;
    }    


    NodePtr getFirstNode() {
        if (isEmpty()) {
            return head_->next;
        }
        return head_->next;
    }
    
    friend class LFUKCache<Key, Value>;

};

template<typename Key, typename Value>
class LFUKCache {
public:
    using Node = typename FreqList<Key, Value>::Node;
    using NodePtr = std::shared_ptr<Node>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

    LFUKCache(int capacity, int maxAverageNum = 10)
    : capacity(capacity), minFreq_(INT8_MAX), maxAverageNum_(maxAverageNum)
    , curAverageNum_(0), curTotalNum_(0) 
    {}

    ~LFUKCache() = default;

    void put(Key key, Value value) {
        if(capacity <= 0) {
            return;
        }
        auto it = nodeMap_.find(key);
        if(it != nodeMap_.end()) {
            it->second->value = value;
            getInternal(it->second, value);
            return;
        }
        // Insert new key-value pair logic goes here
        putInternal(key, value);
    }

    bool get(Key key, Value& value) {
        auto it = nodeMap_.find(key);
        if(it == nodeMap_.end()) {
            return false;
        }
        else {
            value = it->second->value;
            getInternal(it->second, value);
            return true;
        }
    }

private: // available functions
    void putInternal(Key key, Value value);
    void getInternal(NodePtr node, Value& value);

    void kickOut();

    void removeFromFreqList(NodePtr node);
    void addToFreqList(NodePtr node);

    void addFreqNum();
    void decreaseFreqNum(int f);
    void handleOverMaxAverageNum();
    void updateMinFreq();

private:
    int capacity;
    int minFreq_;

    int maxAverageNum_;
    int curTotalNum_;
    int curAverageNum_;

    NodeMap nodeMap_;
    std::unordered_map<int, std::unique_ptr<FreqList<Key, Value>>> freqToFreqList_;
};

template<typename Key, typename Value>
void LFUKCache<Key, Value>::getInternal(NodePtr node, Value& value) {
    removeFromFreqList(node);
    node->freq++;
    addToFreqList(node);
    minFreq_ = std::min(minFreq_, node->freq);
    addFreqNum();
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::putInternal(Key key, Value value) {
    if(nodeMap_.size() >= capacity){
        kickOut();
    }

    NodePtr newNode = std::make_shared<Node>(key, value);
    nodeMap_[key] = newNode;
    addToFreqList(newNode);
    addFreqNum();
    minFreq_ = std::min(minFreq_, newNode->freq);
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::addToFreqList(NodePtr node) {
    int freq = node->freq;
    if(freqToFreqList_.find(freq) == freqToFreqList_.end()) {
        freqToFreqList_[freq] = std::make_unique<FreqList<Key, Value>>(freq);
    }
    freqToFreqList_[freq]->addToTail(node);
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::removeFromFreqList(NodePtr node) {
    int freq = node->freq;
    if(freqToFreqList_.find(freq) != freqToFreqList_.end()) {
        freqToFreqList_[freq]->removeNode(node);
        if(freqToFreqList_[freq]->isEmpty()) {
            freqToFreqList_.erase(freq);
            if(freq == minFreq_) {
                updateMinFreq();
            }
        }
    }
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::kickOut() {
    NodePtr nodeToRemove = freqToFreqList_[minFreq_]->getFirstNode();
    nodeMap_.erase(nodeToRemove->key);
    removeFromFreqList(nodeToRemove);
    // Decrease the total frequency count
    decreaseFreqNum(nodeToRemove->freq);
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::addFreqNum() {
    curTotalNum_++;
    curAverageNum_ = curTotalNum_ / nodeMap_.size();
    if(curAverageNum_ > maxAverageNum_) {
        handleOverMaxAverageNum();
    }
    updateMinFreq();
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::decreaseFreqNum(int f) {
    curTotalNum_ -= f;
    if(nodeMap_.size() > 0) {
        curAverageNum_ = curTotalNum_ / nodeMap_.size();
    } else {
        curAverageNum_ = 0;
    }
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::handleOverMaxAverageNum() {
    if(nodeMap_.empty()) {
        return;
    }
    for(auto it = nodeMap_.begin(); it != nodeMap_.end();) {
        NodePtr node = it->second;
        if(node->freq > maxAverageNum_) {
            removeFromFreqList(node);
            node->freq -= maxAverageNum_ / 2;
            if(node->freq < 1){
                node->freq = 1;
            }
            addToFreqList(node);
            ++it;
        }
        else {
            ++it;
        }
    }
}

template<typename Key, typename Value>
void LFUKCache<Key, Value>::updateMinFreq() {
    if(freqToFreqList_.empty()) {
        minFreq_ = INT8_MAX;
        return;
    }
    minFreq_ = INT8_MAX;

    for(auto it = freqToFreqList_.begin(); it != freqToFreqList_.end(); it++) {
        if(it->second && !it->second->isEmpty()) {
            minFreq_ = std::min(minFreq_, it->first);
        }

    }
    if(minFreq_ == INT8_MAX) {
        minFreq_ = 1;
    }
}