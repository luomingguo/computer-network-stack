#include "reassembler.hh"

#include <algorithm>

using namespace std;


void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring ) {
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

  // 容量为空就不需要处理
  uint64_t capacity = output_.writer().available_capacity();
  if (capacity == 0) {
      return;
  }
  auto first_unpoped_idx = bytes_pending();
  uint64_t last_index = first_index + data.size();  // 计算数据段的结束位置
  uint64_t first_unassembled = output_.writer().bytes_pushed();

  // 如果当前插入的数据的末尾位置小于等于已接收的数据的总长度，丢弃这段数据

  if (last_index <= first_unassembled) {
      return;  // 如果数据完全在已组装的范围内，直接返回
  }
  // 如果数据部分在已组装范围内，截取未组装的部分
  if (first_index < first_unassembled) {
      uint64_t trim_start = first_unpoped_idx - first_index;
      data = data.substr(trim_start);
      first_index = first_unpoped_idx;
  }

  // 如果数据超出容量，截取合适的部分
  uint64_t first_unacceptable = first_unassembled + capacity;
  if (last_index > first_unacceptable) {
      data = data.substr(0, first_unacceptable - first_index);
      last_index = first_index + data.size(); // 重新计算last_index
      is_last_substring = false; // TODO 截断后不能作为最后一个子字符串
  }

    // 处理重叠和合并
    auto it = slots_.begin();
    while (it != slots_.end()) {
        uint64_t seg_start = it->start_idx();
        uint64_t seg_end = it->end_idx();
        
        // 当前段在新数据之后，插入到这里
        if (seg_start >= last_index) {
            break;
        }
        
        // 有重叠或连续，需要合并
        if (seg_end >= first_index && seg_start <= last_index) {
            // 计算合并后的范围
            uint64_t new_start = std::min(first_index, seg_start);
            uint64_t new_end = std::max(last_index, seg_end);
            
            // 合并数据
            if (first_index < seg_start) {
                // 新数据在前
                data += it->data_.substr(seg_start - first_index);
            } else if (first_index > seg_start) {
                // 已有段在前
                data = it->data_.substr(0, first_index - seg_start) + data;
            } else {
                // 起始相同，选择较长的数据
                if (data.size() < it->data_.size()) {
                    data = it->data_;
                }
            }
            
            first_index = new_start;
            last_index = new_end;
            
            // 删除已合并的段
            it = slots_.erase(it);
        } else {
            ++it;
        }
    }
    // 插入新的段（按起始位置排序）
    segment new_segment(first_index, std::move(data));
    auto insert_pos = std::lower_bound(slots_.begin(), slots_.end(), new_segment);
    slots_.insert(insert_pos, std::move(new_segment));
    
    // 尝试将连续的段写入输出流
    while (!slots_.empty() && slots_.front().start_idx() == first_unassembled) {
        const segment& front_seg = slots_.front();
        uint64_t seg_size = front_seg.data_.size();
        
        // 写入到输出流
        output_.writer().push(front_seg.data_);
        first_unassembled += seg_size;
        
        // 移除已处理的段
        slots_.pop_front();
    }
    
    // 如果是最后一个子字符串且所有数据都已处理，关闭流
    if (is_last_substring && bytes_pending() == 0) {
        output_.writer().close();
    }
}

void Reassembler::segment::merge(uint64_t first_index, std::string& new_data) {
  uint64_t new_start = std::min(start_, first_index);
  uint64_t new_end = std::max(end_, first_index + new_data.size());
  
  if (first_index < start_) {
      // 新数据在前
      uint64_t overlap = start_ - first_index;
      data_ = new_data + data_.substr(overlap);
  } else if (first_index > start_) {
      // 已有段在前
      uint64_t overlap = first_index - start_;
      data_ = data_.substr(0, overlap) + new_data;
  } else {
      // 起始相同，选择较长的数据
      if (new_data.size() > data_.size()) {
          data_ = new_data;
      }
  }
  start_ = new_start;
  end_ = new_end;
}



uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t total = 0;
  for (const auto& seg : slots_) {
      total += seg.data_.size();
  }
  return total;
}
