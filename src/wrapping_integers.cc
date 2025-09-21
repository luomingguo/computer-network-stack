#include "wrapping_integers.hh"
#include <cstdlib>
#include <algorithm>
using namespace std;

// 优化后的 wrap 方法 - 保持不变，已经很高效
Wrap32 Wrap32::wrap(uint64_t n, Wrap32 zero_point) {
    return Wrap32{static_cast<uint32_t>(n + zero_point.raw_value_)};
}

uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const {
    uint32_t offset = raw_value_ - zero_point.raw_value_;
    
    // 计算三个可能的候选值
    uint64_t base = (checkpoint >> 32) << 32;
    uint64_t candidate0 = base + offset;                    // 当前区间
    uint64_t candidate1 = candidate0 + (1ULL << 32);       // 下一区间
    uint64_t candidate2 = (base >= (1ULL << 32)) ?         // 上一区间
                          candidate0 - (1ULL << 32) : candidate0;
    
    // 计算到checkpoint的距离
    auto distance = [checkpoint](uint64_t val) -> uint64_t {
        return (val >= checkpoint) ? val - checkpoint : checkpoint - val;
    };
    
    uint64_t dist0 = distance(candidate0);
    uint64_t dist1 = distance(candidate1);
    uint64_t dist2 = (candidate2 != candidate0) ? distance(candidate2) : UINT64_MAX;
    
    // 选择距离最小的候选值
    if (dist0 <= dist1 && dist0 <= dist2) {
        return candidate0;
    } else if (dist1 <= dist2) {
        return candidate1;
    } else {
        return candidate2;
    }
}