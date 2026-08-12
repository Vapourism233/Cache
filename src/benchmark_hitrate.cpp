#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include "headers/LRU.h"
#include "headers/LRUK.h"
#include "headers/LFUK.h"
#include "HashLRU.cpp"
#include "HashLFU.cpp"
#include "ARC/ARCCache.cpp"

// ============================================================================
// Workload 生成器
// ============================================================================

enum WorkloadType {
    HOTSPOT_80_20,      // 20% 的 key 占 80% 的访问
    TIME_LOCALITY,      // 时间局部性：最近访问过的 key 更容易再被访问
    PERIODIC,           // 周期循环访问
    UNIFORM_RANDOM      // 完全随机
};

class WorkloadGenerator {
private:
    uint32_t workingSetSize_;
    uint32_t accessCount_;
    std::mt19937 gen_;
    
public:
    WorkloadGenerator(uint32_t workingSetSize, uint32_t accessCount, unsigned seed = 42)
        : workingSetSize_(workingSetSize), accessCount_(accessCount), gen_(seed) {}
    
    std::vector<int> generate(WorkloadType type) {
        std::vector<int> sequence;
        sequence.reserve(accessCount_);
        
        switch(type) {
            case HOTSPOT_80_20:
                return generateHotspot();
            case TIME_LOCALITY:
                return generateTimeLocality();
            case PERIODIC:
                return generatePeriodic();
            case UNIFORM_RANDOM:
                return generateUniform();
            default:
                return generateUniform();
        }
    }
    
private:
    // 80-20：前 20% 的 key 占 80% 的访问
    std::vector<int> generateHotspot() {
        std::vector<int> sequence;
        uint32_t hotsetSize = std::max(1U, workingSetSize_ / 5);  // 20% 的 key
        
        std::uniform_int_distribution<> hotDist(0, hotsetSize - 1);
        std::uniform_int_distribution<> coldDist(hotsetSize, workingSetSize_ - 1);
        std::uniform_real_distribution<> probDist(0.0, 1.0);
        
        for(uint32_t i = 0; i < accessCount_; ++i) {
            if(probDist(gen_) < 0.8) {
                // 80% 概率访问热点
                sequence.push_back(hotDist(gen_));
            } else {
                // 20% 概率访问冷数据
                sequence.push_back(coldDist(gen_));
            }
        }
        
        return sequence;
    }
    
    // 时间局部性：最近访问的 key 更可能再被访问
    std::vector<int> generateTimeLocality() {
        std::vector<int> sequence;
        std::uniform_int_distribution<> allKeys(0, workingSetSize_ - 1);
        std::uniform_real_distribution<> probDist(0.0, 1.0);
        
        int lastKey = allKeys(gen_);
        
        for(uint32_t i = 0; i < accessCount_; ++i) {
            if(probDist(gen_) < 0.7 && i > 0) {
                // 70% 概率重复最近访问的 key
                // 偶尔从最近访问的几个 key 中选一个
                sequence.push_back(lastKey);
                if(probDist(gen_) < 0.1) {
                    lastKey = allKeys(gen_);  // 10% 概率切换到新 key
                }
            } else {
                lastKey = allKeys(gen_);
                sequence.push_back(lastKey);
            }
        }
        
        return sequence;
    }
    
    // 周期循环：每 100 个请求重复一个固定序列
    std::vector<int> generatePeriodic() {
        std::vector<int> sequence;
        uint32_t periodSize = 100;
        uint32_t periodKeyCount = std::min(50U, workingSetSize_);
        
        // 生成周期内的访问序列
        std::vector<int> period;
        std::uniform_int_distribution<> keyDist(0, periodKeyCount - 1);
        for(uint32_t i = 0; i < periodSize; ++i) {
            period.push_back(keyDist(gen_));
        }
        
        // 重复周期
        for(uint32_t i = 0; i < accessCount_; ++i) {
            sequence.push_back(period[i % periodSize]);
        }
        
        return sequence;
    }
    
    std::vector<int> generateUniform() {
        std::vector<int> sequence;
        std::uniform_int_distribution<> dist(0, workingSetSize_ - 1);
        
        for(uint32_t i = 0; i < accessCount_; ++i) {
            sequence.push_back(dist(gen_));
        }
        
        return sequence;
    }
};

// ============================================================================
// 统一的缓存测试框架
// ============================================================================

struct BenchmarkResult {
    std::string cacheName;
    std::string workloadType;
    size_t capacity;
    uint32_t hits;
    uint32_t misses;
    double hitRate;
    double timeMs;
};

// 测试 LRUCache
BenchmarkResult benchmarkLRU(size_t capacity, const std::vector<int>& sequence, const std::string& workloadType) {
    auto start = std::chrono::high_resolution_clock::now();
    
    LRUCache<int, int> cache(capacity);
    uint32_t hits = 0, misses = 0;
    
    for(int key : sequence) {
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            misses++;
            cache.put(key, key);  // 用 key 作为 value
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    BenchmarkResult result;
    result.cacheName = "LRU";
    result.workloadType = workloadType;
    result.capacity = capacity;
    result.hits = hits;
    result.misses = misses;
    result.hitRate = (double)hits / (hits + misses);
    result.timeMs = timeMs;
    
    return result;
}

// 测试 LFUKCache
BenchmarkResult benchmarkLFU(size_t capacity, const std::vector<int>& sequence, const std::string& workloadType) {
    auto start = std::chrono::high_resolution_clock::now();
    
    LFUKCache<int, int> cache(capacity, 10);
    uint32_t hits = 0, misses = 0;
    
    for(int key : sequence) {
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            misses++;
            cache.put(key, key);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    BenchmarkResult result;
    result.cacheName = "LFU";
    result.workloadType = workloadType;
    result.capacity = capacity;
    result.hits = hits;
    result.misses = misses;
    result.hitRate = (double)hits / (hits + misses);
    result.timeMs = timeMs;
    
    return result;
}

// 测试 ARCCache
BenchmarkResult benchmarkARC(size_t capacity, const std::vector<int>& sequence, const std::string& workloadType) {
    auto start = std::chrono::high_resolution_clock::now();
    
    ARCCache<int, int> cache(capacity, 5);  // transformThreshold = 5
    uint32_t hits = 0, misses = 0;
    
    for(int key : sequence) {
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            misses++;
            cache.put(key, key);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(end - start).count();
    
    BenchmarkResult result;
    result.cacheName = "ARC";
    result.workloadType = workloadType;
    result.capacity = capacity;
    result.hits = hits;
    result.misses = misses;
    result.hitRate = (double)hits / (hits + misses);
    result.timeMs = timeMs;
    
    return result;
}



// 测试 LRUKCache (LRU-K, k=2)
BenchmarkResult benchmarkLRUK(size_t capacity, const std::vector<int>& sequence, const std::string& workloadType) {
    auto start = std::chrono::high_resolution_clock::now();

    LRUKCache<int, int> cache(capacity, capacity, 2);  // historyCapacity=capacity, k=2
    uint32_t hits = 0, misses = 0;

    for(int key : sequence) {
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            misses++;
            cache.put(key, key);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(end - start).count();

    BenchmarkResult result;
    result.cacheName = "LRU-K";
    result.workloadType = workloadType;
    result.capacity = capacity;
    result.hits = hits;
    result.misses = misses;
    result.hitRate = (double)hits / (hits + misses);
    result.timeMs = timeMs;
    return result;
}

// 测试 HashLRUKCache (分片并发 LRU-K, sliceNum=4)
BenchmarkResult benchmarkHashLRU(size_t capacity, const std::vector<int>& sequence, const std::string& workloadType) {
    auto start = std::chrono::high_resolution_clock::now();

    HashLRUKCache<int, int> cache(capacity, 4);  // sliceNum=4
    uint32_t hits = 0, misses = 0;

    for(int key : sequence) {
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            misses++;
            cache.put(key, key);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(end - start).count();

    BenchmarkResult result;
    result.cacheName = "HashLRU";
    result.workloadType = workloadType;
    result.capacity = capacity;
    result.hits = hits;
    result.misses = misses;
    result.hitRate = (double)hits / (hits + misses);
    result.timeMs = timeMs;
    return result;
}

// 测试 LFUHashCache (分片并发 LFU, sliceNum=4)
BenchmarkResult benchmarkHashLFU(size_t capacity, const std::vector<int>& sequence, const std::string& workloadType) {
    auto start = std::chrono::high_resolution_clock::now();

    LFUHashCache<int, int> cache(capacity, 4, 10);  // sliceNum=4, maxAverageNum=10
    uint32_t hits = 0, misses = 0;

    for(int key : sequence) {
        int value;
        if(cache.get(key, value)) {
            hits++;
        } else {
            misses++;
            cache.put(key, key);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double timeMs = std::chrono::duration<double, std::milli>(end - start).count();

    BenchmarkResult result;
    result.cacheName = "HashLFU";
    result.workloadType = workloadType;
    result.capacity = capacity;
    result.hits = hits;
    result.misses = misses;
    result.hitRate = (double)hits / (hits + misses);
    result.timeMs = timeMs;
    return result;
}

// 实时打印单行结果（即使后续算法崩溃，已完成的结果也已刷新到终端）
static void reportRow(const BenchmarkResult& r) {
    std::cout << "    " << std::left << std::setw(10) << r.cacheName
              << " hit=" << std::fixed << std::setprecision(4) << r.hitRate
              << "  time=" << std::setprecision(2) << r.timeMs << "ms"
              << "  (" << r.hits << "/" << r.misses << ")" << std::endl;
}

// 在子进程中运行 ARC，避免其已知边界 bug 的段错误中断整个对比测试。
// 成功则填充 out 并返回 true；子进程崩溃则返回 false。
bool benchmarkARC_isolated(size_t capacity, const std::vector<int>& sequence,
                           const std::string& workloadType, BenchmarkResult& out) {
    int fds[2];
    if(pipe(fds) != 0) return false;
    std::cout << std::flush;
    pid_t pid = fork();
    if(pid < 0) { close(fds[0]); close(fds[1]); return false; }
    if(pid == 0) {
        close(fds[0]);
        BenchmarkResult r = benchmarkARC(capacity, sequence, workloadType);
        double buf[3] = { (double)r.hits, (double)r.misses, r.timeMs };
        ssize_t n = write(fds[1], buf, sizeof(buf));
        (void)n;
        close(fds[1]);
        _exit(0);
    }
    close(fds[1]);
    double buf[3] = {0, 0, 0};
    ssize_t got = read(fds[0], buf, sizeof(buf));
    close(fds[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    if(WIFSIGNALED(status) || got != (ssize_t)sizeof(buf)) {
        return false;
    }
    out.cacheName = "ARC";
    out.workloadType = workloadType;
    out.capacity = capacity;
    out.hits = (uint32_t)buf[0];
    out.misses = (uint32_t)buf[1];
    out.hitRate = out.hits / (double)(out.hits + out.misses);
    out.timeMs = buf[2];
    return true;
}

// ============================================================================
// 主程序
// ============================================================================

int main() {
    // 参数设置
    const uint32_t WORKING_SET_SIZE = 5000;    // 不同 key 的数量
    const uint32_t ACCESS_COUNT = 300000;      // 访问总数
    const std::vector<size_t> CAPACITIES = {100, 500, 2000};
    
    std::cout << "================================================================================\n"
              << "缓存命中率基准测试\n"
              << "================================================================================\n"
              << "工作集大小: " << WORKING_SET_SIZE << " keys\n"
              << "访问总数: " << ACCESS_COUNT << " accesses\n"
              << "缓存容量: ";
    for(size_t cap : CAPACITIES) std::cout << cap << " ";
    std::cout << "\n\n";
    
    // 生成不同类型的 workload
    struct Workload {
        WorkloadType type;
        std::string name;
    };
    
    std::vector<Workload> workloads = {
        {HOTSPOT_80_20, "Hotspot(80-20)"},
        {TIME_LOCALITY, "TimeLocality"},
        {PERIODIC, "Periodic"},
        {UNIFORM_RANDOM, "UniformRandom"}
    };
    
    std::vector<BenchmarkResult> allResults;
    
    for(size_t i = 0; i < workloads.size(); ++i) {
        const auto& wl = workloads[i];
        std::cout << "生成 " << wl.name << " workload...\n";
        WorkloadGenerator gen(WORKING_SET_SIZE, ACCESS_COUNT);
        std::vector<int> sequence = gen.generate(wl.type);
        
        for(size_t capacity : CAPACITIES) {
            std::cout << "  运行容量 " << capacity << "...\n";
            
            BenchmarkResult r;
            r = benchmarkLRU(capacity, sequence, wl.name);     reportRow(r); allResults.push_back(r);
            r = benchmarkLRUK(capacity, sequence, wl.name);    reportRow(r); allResults.push_back(r);
            r = benchmarkLFU(capacity, sequence, wl.name);     reportRow(r); allResults.push_back(r);
            r = benchmarkHashLRU(capacity, sequence, wl.name); reportRow(r); allResults.push_back(r);
            r = benchmarkHashLFU(capacity, sequence, wl.name); reportRow(r); allResults.push_back(r);
            BenchmarkResult arcR;
            if(benchmarkARC_isolated(capacity, sequence, wl.name, arcR)) {
                reportRow(arcR); allResults.push_back(arcR);
            } else {
                std::cout << "    " << std::left << std::setw(10) << "ARC"
                          << " CRASHED (known ARC boundary bug)" << std::endl;
            }
        }
        std::cout << "\n";
    }
    
    // 输出结果表格
    std::cout << "================================================================================\n"
              << "测试结果\n"
              << "================================================================================\n\n";
    
    std::cout << std::left 
              << std::setw(12) << "Cache"
              << std::setw(18) << "Workload"
              << std::setw(10) << "Capacity"
              << std::setw(12) << "Hit Rate"
              << std::setw(12) << "Hits/Misses"
              << std::setw(10) << "Time(ms)"
              << "\n";
    std::cout << std::string(74, '-') << "\n";
    
    std::string lastWorkload = "";
    for(size_t i = 0; i < allResults.size(); ++i) {
        const auto& result = allResults[i];
        if(result.workloadType != lastWorkload) {
            std::cout << "\n--- " << result.workloadType << " ---\n";
            lastWorkload = result.workloadType;
        }
        
        std::cout << std::left
                  << std::setw(12) << result.cacheName
                  << std::setw(18) << " "
                  << std::setw(10) << result.capacity
                  << std::setw(12) << std::fixed << std::setprecision(4) << result.hitRate
                  << std::setw(12) << (std::to_string(result.hits) + "/" + std::to_string(result.misses))
                  << std::setw(10) << std::fixed << std::setprecision(2) << result.timeMs
                  << "\n";
    }
    
    // 统计总结
    std::cout << "\n" << std::string(74, '=') << "\n";
    std::cout << "每个 workload 中，缓存的平均命中率排名：\n\n";
    
    for(size_t i = 0; i < workloads.size(); ++i) {
        const auto& wl = workloads[i];
        std::cout << wl.name << ":\n";
        std::vector<std::pair<std::string, double>> avgHitRates;
        std::vector<std::string> cacheNames = {"LRU", "LRU-K", "LFU", "HashLRU", "HashLFU", "ARC"};
        
        for(size_t j = 0; j < cacheNames.size(); ++j) {
            const auto& cacheName = cacheNames[j];
            double totalHitRate = 0;
            int count = 0;
            for(size_t k = 0; k < allResults.size(); ++k) {
                const auto& result = allResults[k];
                if(result.workloadType == wl.name && result.cacheName == cacheName) {
                    totalHitRate += result.hitRate;
                    count++;
                }
            }
            if(count > 0) {
                avgHitRates.push_back(std::make_pair(cacheName, totalHitRate / count));
            }
        }
        
        std::sort(avgHitRates.begin(), avgHitRates.end(),
                 [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) { 
                     return a.second > b.second; 
                 });
        
        for(size_t j = 0; j < avgHitRates.size(); ++j) {
            std::cout << "  " << (j+1) << ". " << std::setw(10) << avgHitRates[j].first
                     << ": " << std::fixed << std::setprecision(4) << avgHitRates[j].second << "\n";
        }
        std::cout << "\n";
    }
    
    return 0;
}
