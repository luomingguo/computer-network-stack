#include "wrapping_integers.hh"
#include <algorithm>
#include <cstdlib>
using namespace std;

Wrap32 Wrap32::wrap(uint64_t n, Wrap32 zero_point) {
  return Wrap32{static_cast<uint32_t>(n + zero_point.raw_value_)};
}

// note: 
//  in TCP, checkpoint = first unassembled index
//  wrap/unwrap 操作应该保持偏移量一致性——如果两个 seqno 相差 17，那么它们对应的两个 absolute seqno 也应该相差 17
uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const {
  // 目标是找到里checkpoint最近的那个，核心技巧就是加基准数（2^{31}）再对齐，等价于四舍五入到基准数
  const uint32_t offset = raw_value_ - zero_point.raw_value_;
  const uint64_t sum    = checkpoint + (1ULL << 31);

  if (sum < static_cast<uint64_t>(offset)) return offset;

  return ((sum - offset) & ~uint64_t{0xFFFFFFFF}) + offset;

}