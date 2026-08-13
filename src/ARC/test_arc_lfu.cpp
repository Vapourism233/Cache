#include "ARCLFUPart.h"
#include <iostream>
#include <string>

static int g_failures = 0;

#define CHECK(cond, msg)                                                     \
    do {                                                                     \
        if (cond) {                                                          \
            std::cout << "  [PASS] " << msg << "\n";                         \
        } else {                                                             \
            std::cout << "  [FAIL] " << msg << " (line " << __LINE__ << ")\n"; \
            ++g_failures;                                                    \
        }                                                                    \
    } while (0)

// Helpers so the bool& out-param does not clutter every call.
template <typename Cache>
static void putKV(Cache& c, int k, int v) {
    bool t = false;
    c.put(k, v, t);
}

template <typename Cache>
static bool getKV(Cache& c, int k, int& out) {
    bool t = false;
    return c.get(k, out, t);
}

// 1. Basic hit / miss.
static void testBasicHitMiss() {
    std::cout << "Test 1: basic hit/miss\n";
    ARCLFUPart<int, int> c(3);
    putKV(c, 1, 100);

    int v = 0;
    CHECK(getKV(c, 1, v), "get on existing key returns true");
    CHECK(v == 100, "value returned is 100");
    CHECK(!getKV(c, 999, v), "get on missing key returns false");
}

// 2. Frequency-based eviction: lowest-frequency node is evicted.
static void testFrequencyEviction() {
    std::cout << "Test 2: frequency eviction\n";
    ARCLFUPart<int, int> c(3);
    putKV(c, 1, 10);
    putKV(c, 2, 20);
    putKV(c, 3, 30);

    // Raise frequency of 1 and 2, leave 3 at the lowest frequency.
    int v = 0;
    getKV(c, 1, v);
    getKV(c, 2, v);

    putKV(c, 4, 40); // triggers eviction of the lowest-frequency key (3)

    CHECK(!c.inMainCache(3), "lowest-freq key 3 evicted from main");
    CHECK(c.inGhostCache(3), "evicted key 3 moved to ghost");
    CHECK(c.inMainCache(1), "high-freq key 1 stays");
    CHECK(c.inMainCache(2), "high-freq key 2 stays");
    CHECK(c.inMainCache(4), "new key 4 inserted");
}

// 3. Same frequency -> evict least-recently-inserted (LRU within a freq bucket).
static void testSameFreqLRU() {
    std::cout << "Test 3: same-frequency LRU order\n";
    ARCLFUPart<int, int> c(3);
    putKV(c, 1, 10); // oldest in freq-1 bucket
    putKV(c, 2, 20);
    putKV(c, 3, 30);

    putKV(c, 4, 40); // all freq 1 -> evict the oldest (key 1)

    CHECK(!c.inMainCache(1), "oldest same-freq key 1 evicted");
    CHECK(c.inMainCache(2), "key 2 stays");
    CHECK(c.inMainCache(3), "key 3 stays");
    CHECK(c.inMainCache(4), "new key 4 inserted");
}

// 4. Insert a brand-new key into a full cache (the evict-then-insert branch).
static void testFullInsertNewKey() {
    std::cout << "Test 4: full cache + new key\n";
    ARCLFUPart<int, int> c(2);
    putKV(c, 1, 100);
    putKV(c, 2, 200);

    putKV(c, 3, 300); // full -> must evict then insert 3

    int v = 0;
    CHECK(getKV(c, 3, v), "new key 3 present after insert into full cache");
    CHECK(v == 300, "value for key 3 is 300");
    CHECK(!c.inMainCache(1), "oldest key 1 evicted to make room");
}

// 5. Ghost hit revives the node and grows capacity (ARC adaptivity).
static void testGhostHitGrowsCapacity() {
    std::cout << "Test 5: ghost hit revival + capacity growth\n";
    ARCLFUPart<int, int> c(2);
    putKV(c, 1, 100);
    putKV(c, 2, 200);
    putKV(c, 3, 300); // evicts key 1 to ghost

    CHECK(c.inGhostCache(1), "key 1 is in ghost before revival");
    size_t capBefore = c.getCapacity();

    int v = 0;
    bool hit = getKV(c, 1, v); // ghost hit -> revive + increaseCapacity

    CHECK(hit, "ghost hit returns true");
    CHECK(v == 100, "revived value is original 100");
    CHECK(c.inMainCache(1), "key 1 revived into main cache");
    CHECK(!c.inGhostCache(1), "key 1 removed from ghost after revival");
    CHECK(c.getCapacity() == capBefore + 1, "capacity grew by 1 on ghost hit");
}

// 6. Revive oldest ghost node when ghost has multiple nodes (edge case for unlink order).
static void testGhostMultipleRevive() {
    std::cout << "Test 6: revive oldest from multi-node ghost\n";
    ARCLFUPart<int, int> c(4);  // capacity 4 -> ghost_capacity 2
    putKV(c, 1, 100); // oldest
    putKV(c, 2, 200);
    putKV(c, 3, 300);
    putKV(c, 4, 400); // main is full
    putKV(c, 5, 500); // evicts 1 to ghost (main=[2,3,4,5], ghost=[1])
    putKV(c, 6, 600); // evicts 2 to ghost (main=[3,4,5,6], ghost=[1,2])
    putKV(c, 7, 700); // evicts 3 to ghost, ghost full -> removeOldest deletes 1

    CHECK(c.inGhostCache(2), "key 2 in ghost");
    CHECK(c.inGhostCache(3), "key 3 in ghost");
    CHECK(!c.inGhostCache(1), "key 1 evicted from ghost (oldest)");
    CHECK(c.mainSize() == 4, "main cache has 4 items");

    int v = 0;
    bool hit = getKV(c, 3, v); // revive key 3 (which is oldest remaining ghost node)

    CHECK(hit, "revive ghost returns true");
    CHECK(v == 300, "revived value is 300");
    CHECK(c.inMainCache(3), "key 3 revived to main");
    CHECK(!c.inGhostCache(3), "key 3 removed from ghost");
    CHECK(c.inGhostCache(2), "key 2 still in ghost (not affected)");
}

int main() {
    testBasicHitMiss();
    testFrequencyEviction();
    testSameFreqLRU();
    testFullInsertNewKey();
    testGhostHitGrowsCapacity();
    testGhostMultipleRevive();

    std::cout << "\n";
    if (g_failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }
    std::cout << g_failures << " check(s) failed.\n";
    return 1;
}
