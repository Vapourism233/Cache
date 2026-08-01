#pragma once // means that the file will only be included once in a single compilation, preventing duplicate definitions.
#include <memory>

template<typename Key, typename Value>
class ARCNode {
private:
    Key key_;
    Value value_;
    size_t accessCount_;
    size_t freq_;  // Frequency for LFU part
    std::weak_ptr<ARCNode<Key, Value>> prev;
    std::shared_ptr<ARCNode<Key, Value>> next;

public:
    ARCNode() : accessCount_(1), freq_(1), next(nullptr) {}
    ARCNode(Key key, Value value) // Constructor to initialize key, value, accessCount, and freq
        : key_(key)
        , value_(value)
        , accessCount_(1)
        , freq_(1)
        , next(nullptr) 
    {}

    void incrementAccessCount(){
        ++accessCount_;
    }

    Key getKey() const {
        return key_;
    } // const getter for key

    Value getValue() const {
        return value_;
    } // const getter for value

    void setValue(const Value& value) {
        value_ = value;
    } // setter for value

    size_t getAccessCount() const {
        return accessCount_;
    } // const getter for accessCount

    size_t getFreq() const {
        return freq_;
    } // const getter for frequency

    void setFreq(size_t f) {
        freq_ = f;
    } // setter for frequency

    void incrementFreq() {
        ++freq_;
    } // increment frequency

    template<typename K, typename V> friend class ARCLRUPart;
    template<typename K, typename V> friend class ARCLFUPart;

};