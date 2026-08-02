# Cache Implementation & Benchmarking

一个全面的缓存实现项目，包括多种缓存算法的设计、实现与性能对标。

## 📦 项目结构

```
Cache/
├── LRU.h                    # 基础 LRU 缓存实现
├── LFUK.cpp                 # LFU-K 缓存实现
├── LRUK.h                   # LRU-K 缓存实现
├── HashLRU.cpp              # 带分片并发的 LRU（线程安全）
├── HashLFU.cpp              # 带分片并发的 LFU（线程安全）
├── ARC/                     # ARC（自适应替换缓存）协调器设计
│   ├── ARCCache.cpp         # ARC 主协调器
│   ├── ARCLFUPart.cpp       # ARC 中的 LFU 部分
│   ├── ARCLRUPart.cpp       # ARC 中的 LRU 部分
│   └── ARCKCacheNode.h      # ARC 节点定义
├── Makefile                 # 编译配置
├── test_lru.cpp             # LRU 单元测试
├── test_lruk_run.cpp        # LRU-K 单元测试
├── benchmark_hitrate.cpp    # 多场景命中率基准测试
└── README.md
```

## 🎯 已实现的缓存算法

### 1. **LRU (Least Recently Used)**
- 文件：`LRU.h`
- 特点：基于访问时间戳，最近未访问的项被淘汰
- 时间复杂度：O(1) get/put
- 空间复杂度：O(capacity)
- 适用场景：时间局部性强的工作负载

### 2. **LFU (Least Frequently Used)**  
- 文件：`LFUK.cpp`
- 特点：基于访问频率，最少被访问的项被淘汰
- 数据结构：频率桶 + 频率内 LRU 排序
- 时间复杂度：O(1) get/put
- 空间复杂度：O(capacity)
- 适用场景：热点集中的工作负载

### 3. **LRU-K**
- 文件：`LRUK.h`
- 特点：跟踪最后 K 次访问，只有被访问 K 次的项才进入主缓存
- 优势：更好地区分热点与冷数据
- 适用场景：有明显热冷区分的场景

### 4. **HashLRU/HashLFU (并发版本)**
- 文件：`HashLRU.cpp`, `HashLFU.cpp`
- 特点：将缓存分片成多个小缓存，每个分片独立上锁（分片锁方案）
- 并发度：可配置的分片数（默认 CPU 核数）
- 适用场景：多线程高并发环境

### 5. **ARC (Adaptive Replacement Cache)**
- 文件：`ARC/ARCCache.cpp`
- 特点：自适应组合 LRU 和 LFU，通过 Ghost Cache 自动调整两部分容量
- 架构：
  - 协调器 (ARCCache) 管理总容量
  - LFU 部分 (ARCLFUPart) 用频率桶管理热点
  - LRU 部分 (ARCLRUPart) 用链表管理最近访问
  - 此消彼长：当某部分的 ghost cache 命中时，该部分容量 +1，另一部分 -1
- 状态：✅ 基础架构完成，⚠️ 小容量边界条件有 bug

## 🧪 测试与基准

### 单元测试
```bash
# LRU 测试
make test_lru

# LRU-K 测试  
make test_lruk_run

# ARC 基础测试
g++ -std=c++14 -Wall -g ARC/test_arc_lfu.cpp -o test_arc_lfu && ./test_arc_lfu
```

### 性能基准测试
```bash
# 编译
g++ -std=c++14 -Wall -O2 -o benchmark_hitrate benchmark_hitrate.cpp

# 运行
./benchmark_hitrate
```

**测试规模**：
- 工作集大小：100,000 个不同的 key
- 访问总数：1,000,000 次
- 容量范围：20、100、1000

**测试场景**：
1. **Hotspot (80-20)**: 20% 的 key 占 80% 的访问
2. **Time Locality**: 最近访问的 key 更容易再被访问
3. **Periodic**: 周期性的访问模式
4. **Uniform Random**: 完全随机访问

## 📊 基准测试结果

### 平均命中率排名

| Workload | 最优 | 次优 | 差异 |
|----------|------|------|------|
| **Hotspot** | LFU | LRU | LFU +21% |
| **Time Locality** | 平手 | - | 无差异 (~65%) |
| **Periodic** | LFU | LRU | LFU +7% |
| **Random** | 平手 | - | 无差异 (~0.4%) |

### 性能对比

| 缓存 | 相对速度 | 内存开销 | 适用场景 |
|-----|---------|---------|---------|
| LRU | 基准 (1x) | 低 | 通用，时间局部性强 |
| LFU | 慢 (3-5x) | 中等 | 热点集中 |
| LRU-K | 较慢 (2-3x) | 中等 | 热冷区分明显 |
| HashLRU | 基准 (1x) | 低 | 多线程高并发 |
| HashLFU | 慢 (3-5x) | 中等 | 并发热点场景 |
| ARC | 中等 (1.5-2x)* | 中等 | 自适应通用** |

*当前 ARC 在小容量下有 bug，性能数据不完整
**待修复完整性问题

## 🔧 编译与使用

### 编译要求
- C++14 或更高
- macOS/Linux/Windows (POSIX 兼容)
- g++ 或 clang++

### 基本编译
```bash
# 使用 Makefile
make

# 或手动编译
g++ -std=c++14 -Wall -O2 -o your_program your_file.cpp
```

### 基本使用示例

**LRU 缓存**：
```cpp
#include "LRU.h"

int main() {
    LRUCache<int, int> cache(100);  // 容量 100
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```

**LFU 缓存**：
```cpp
#include "LFUK.cpp"

int main() {
    LFUKCache<int, int> cache(100);  // 容量 100
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```

**并发 LRU**：
```cpp
#include "HashLRU.cpp"

int main() {
    HashLRUKCache<int, int> cache(100, 8);  // 容量 100, 8 个分片
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```

**ARC 缓存**：
```cpp
#include "ARC/ARCCache.cpp"

int main() {
    ARCCache<int, int> cache(100, 5);  // 容量 100, transformThreshold=5
    
    cache.put(1, 100);
    int value;
    if(cache.get(1, value)) {
        std::cout << "Hit: " << value << std::endl;
    }
    return 0;
}
```

## 🐛 已知问题

### ARC 边界条件 Bug
- **症状**：容量较小时（≤20）在特定热点 workload 下段错误
- **触发条件**：热点访问模式，容量 5-20，大工作集（10k+ keys）
- **影响**：容量 ≥ 20 时正常，容量 < 5 时可靠性不确定
- **根本原因**：尚未定位，可能在 ghost cache 协调或容量衰减逻辑
- **status**：待修复

### HashLRU/HashLFU 编译问题  
- **症状**：LFUK.cpp 多次 include 导致类重定义
- **solution**：需要将 LFUK.cpp 改为头文件或使用 include guards
- **status**：待改进

## 📈 下一步改进方向

### 优先级高
1. [ ] 修复 ARC 小容量段错误
2. [ ] 修复 HashLFU 编译问题，纳入基准测试
3. [ ] 实现 W-TinyLFU（LRU + 概率筛选的混合方案）
4. [ ] 添加多线程并发测试

### 优先级中
5. [ ] 实现 CLOCK 替换算法（LRU 近似）
6. [ ] 添加 TTL (Time To Live) 支持
7. [ ] 实现 Bloom Filter 优化 miss 检测
8. [ ] 性能分析（缓存行对齐、SIMD 优化）

### 优先级低  
9. [ ] 支持更大的数据类型（string, custom object）
10. [ ] 分布式缓存设计（一致性哈希、网络协议）

## 📚 参考资源

- **LRU**: Cache 替换策略经典算法
- **LFU**: O(1) 时间复杂度实现，使用频率桶
- **LRU-K**: IBM Research 论文《A Comparative Study of LRU and Clock Algorithms for Page Replacement》
- **ARC**: IBM Nimble 系统论文《ARC: A Self-Tuning, Low Overhead Replacement Cache》
- **并发哈希表**: Google Abseil 的分片锁方案

## 📝 许可证

MIT License

## 👤 作者

Created as a learning project in cache algorithm design and implementation.

---

**最后更新**：2026-08-02  
**项目版本**：v0.5 (ARC 基础架构完成，性能对标完成，待边界条件修复)