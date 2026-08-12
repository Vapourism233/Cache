#include <iostream>
#include <cassert>
#include <string>
#include "LRU.h"
#include "LRUK.cpp"

int main() {
    std::cout << "========== 运行 LRU-K 测试 ==========" << std::endl;
    // 初始化：主缓存容量=2，历史记录容量=5，K阈值=2
    LRUKCache<int, std::string> cache(2, 5, 2); 
    
    std::string val;

    std::cout << "[操作] put(1, \"Apple\")" << std::endl;
    cache.put(1, "Apple"); // 第一次访问（存入历史暂存区）
    
    std::cout << "[操作] get(1)" << std::endl;
    bool found = cache.get(1, val); // 第二次访问：触发晋升机制，但 get 刚开始因为还在历史区所以返回时将其提升
    std::cout << "  -> 结果: " << (found ? "拿到数据(此时已到达K次)" : "未拿到数据") << std::endl;
    
    std::cout << "[操作] get(1) 再次获取" << std::endl;
    found = cache.get(1, val); // 已经在主缓存，直接返回
    std::cout << "  -> 结果: " << (found ? "拿到数据: " + val : "未拿到数据") << std::endl;
    
    std::cout << "[操作] put(1, \"Apple Updated\")" << std::endl;
    cache.put(1, "Apple Updated"); // 主缓存更新
    
    std::cout << "[操作] get(1) 获取更新后的数据" << std::endl;
    found = cache.get(1, val);
    std::cout << "  -> 结果: " << (found ? "拿到数据: " + val : "未拿到数据") << std::endl;

    std::cout << "========== 测试全部完成 ==========" << std::endl;
    return 0;
}
