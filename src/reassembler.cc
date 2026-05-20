#include "reassembler.hh"

#include <algorithm>
#include <cstring>
using namespace std;

// Reassembler uses byteStream of the writer side only
void Reassembler::insert(uint64_t first_index, string data,
                         bool is_last_substring) {

  // 首先unpoped 和 unaccepted 指针都由 bytestream限制着
  // 我们要的就是维护，中间 unassembled，及其多个零散段
  // | unpoped | unaccepted | unassembled |
  //
  //                |<----- 总容量 ---------------------------->|
  //  已经正确popped | 被 ByteSteeam 缓存    |    重组器内部       |
  //  超出capacity不可接受
  // <-------------><--------------------->< --  ---- - --   -->-----
  // 0      1st_unpopped idx        1st_unassembled idx    1st_unacceptable idx
  //

  if (is_last_substring) {
    eof_index_ = first_index + data.size();
  }

  const uint64_t first_unassembled  = output_.writer().bytes_pushed();
  const uint64_t first_unacceptable = first_unassembled + output_.writer().available_capacity();
  uint64_t       last_index         = first_index + data.size();

  // 统一出口：推送连续段 + 判断 EOF
  auto flush = [&]() {
    while (!slots_.empty() && slots_.begin()->first == output_.writer().bytes_pushed()) {
      auto it = slots_.begin();
      output_.writer().push(std::move(it->second));
      slots_.erase(it);
    }
    if (eof_index_.has_value() && output_.writer().bytes_pushed() == *eof_index_)
      output_.writer().close();
  };

  if (data.empty() || last_index <= first_unassembled || first_index >= first_unacceptable)
    return flush();

  // ── 原地修剪（不申请新内存）──────────────────────────────────────────────
  if (first_index < first_unassembled) {
    data.erase(0, first_unassembled - first_index); // memmove，无 alloc
    first_index = first_unassembled;
  }
  if (last_index > first_unacceptable) {
    data.resize(first_unacceptable - first_index);  // O(1)
    last_index = first_index + data.size();
  }
  if (data.empty()) return flush();

  uint64_t seg_lo = first_index;
  uint64_t seg_hi = last_index;

  // ── 左邻居合并 ───────────────────────────────────────────────────────────
  auto it = slots_.lower_bound(seg_lo);
  if (it != slots_.begin()) {
    auto prev = std::prev(it);
    const uint64_t p_lo = prev->first;
    const uint64_t p_hi = p_lo + prev->second.size();
    if (p_hi > seg_lo) {
      if (p_hi >= seg_hi) {
        // prev 完全覆盖我们：直接丢弃 data，零拷贝
        return flush();
      }
      // prev 左延伸，data 右延伸：
      // 只把 data 中 prev 尚未覆盖的右尾 append 进去
      prev->second.append(data, p_hi - seg_lo, string::npos); // 只拷贝右尾
      data  = std::move(prev->second); // 窃取 prev 的 buffer，零拷贝
      seg_lo = p_lo;
      seg_hi = p_lo + data.size();
      slots_.erase(prev);
    }
  }

  // ── 右邻居合并 ───────────────────────────────────────────────────────────
  it = slots_.lower_bound(seg_lo);
  while (it != slots_.end() && it->first <= seg_hi) {
    const uint64_t r_hi = it->first + it->second.size();
    if (r_hi > seg_hi) {
      data.append(it->second, seg_hi - it->first, string::npos); // 只拷贝右尾
      seg_hi = r_hi;
    }
    it = slots_.erase(it);
  }

  slots_.emplace(seg_lo, std::move(data)); // move，零拷贝
  flush();

}

uint64_t Reassembler::bytes_pending() const {
  uint64_t total = 0;

  for (const auto &kv : slots_) {
    total += kv.second.size();
  }
  return total;
}