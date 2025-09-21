#include "reassembler.hh"

#include <algorithm>
#include <cstring>
using namespace std;

void Reassembler::insert(uint64_t first_index, string data,
                         bool is_last_substring) {
  // Your code here.
  // Reassembler uses byteStream of the writer side only
  // 首先unpoped 和 unaccepted 指针都由 bytestream限制着
  // 我们要的就是维护，中间 unassembled，及其多个零散段
  // 滑动窗口，先固定 last_index = index + data.size()
  // 在slots尾遍历，if last_index < item.start_
  // 扫描段区 段头 <= 数据尾，说明有交叉
  //   1. 完全装入数据，无需处理，break
  //   2. 段头没改，break
  //   3. 段头改了，需要删除分段，并插入新的分段（实际上第一次不需要重新）
  //      3.1 知道段头不改为止，遍历的分段都要删除
  // pending_idx 代表当前已接收的数据的索引
	if (is_last_substring) is_last_ = true;

  uint64_t last_index = first_index + data.size(); // 计算数据段的结束位置

  // 已被缓存ByteStream缓存的索引（未被接收端接受的序列号）
  uint64_t first_unassembled = output_.writer().bytes_pushed();
  uint64_t capacity = output_.writer().available_capacity();
  uint64_t first_unacceptable = first_unassembled + capacity;

  // 1. 丢掉完全在已写入范围之前的数据
  if (last_index <= first_unassembled) {
    if (is_last_ && slots_.empty() && !output_.reader().bytes_buffered()) {
      output_.writer().close();
    }
    return;
  }
  if (first_index >= first_unacceptable) {
    return; // RFC 793 明确：如果收到的数据段序号超出了 RCV.NXT +
            // RCV.WND（也就是接收窗口范围），该段必须被丢弃。
  }
	std::string_view sv{data};
  // 2. 裁掉前面已经写过的部分
	if (first_index < first_unassembled) {
			sv.remove_prefix(first_unassembled - first_index);
			first_index = first_unassembled;
	}
  // 3. 裁掉超出容量部分； 特殊情况，发送者不按照滑动发送，必须丢掉
	if (last_index > first_unacceptable) {
			sv.remove_suffix(last_index - first_unacceptable);
			last_index = first_index + sv.size();
			is_last_ = false;
	}

  // 4. 插入到 slots，合并重叠区间
	segment new_seg(first_index, std::string(sv));
  auto it = slots_.begin();
  while (it != slots_.end()) {
    if (new_seg.end_idx() < it->start_idx())
      break; // 没重叠，且在它前面
    if (new_seg.start_idx() > it->end_idx()) {
      ++it; // 没重叠，且在它后面
      continue;
    }
		
    // 有重叠，合并
    uint64_t new_start = std::min(new_seg.start_idx(), it->start_idx());
    uint64_t new_end = std::max(new_seg.end_idx(), it->end_idx());

    // 构造合并后的完整字符串
    string merged(new_end - new_start, 0);
    for (uint64_t i = new_start; i < new_end; i++) {
      char c = 0;
      if (i >= new_seg.start_idx() && i < new_seg.end_idx()) {
        c = new_seg.data_[i - new_seg.start_idx()];
      }
      if (i >= it->start_idx() && i < it->end_idx()) {
        c = it->data_[i - it->start_idx()];
      }
      merged[i - new_start] = c;
    }
    new_seg = segment(new_start, std::move(merged));
    it = slots_.erase(it); // 返回指向下一个元素的迭代器， 行为安全
  }
  // 插入新的段（按起始位置排序）
  slots_.insert(it, std::move(new_seg));

  // 5. 尝试把最前面的段写入 ByteStream
  if (!slots_.empty() && slots_.front().start_idx() == first_unassembled) {
    auto &seg = slots_.front();
    output_.writer().push(seg.data_);
    first_unassembled += seg.data_.size();
    slots_.pop_front();
  }

  if (is_last_ && slots_.empty()) {
    output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const {
  // Your code here.
  uint64_t total = 0;
  for (const auto &seg : slots_) {
    total += seg.data_.size();
  }
  return total;
}
