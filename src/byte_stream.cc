#include "byte_stream.hh"
#include <iostream>
#include <stdexcept>

using namespace std;

ByteStream::ByteStream(uint64_t capacity)
    : capacity_(capacity), is_close_{false}, popped_total_(0), pushed_total_(0),
      buffer_(capacity, 0) {}

void Writer::push(string data) {

  // Your code here.
  if (has_error() || !data.size()) {
    return;
  }
  if (is_closed()) {
    set_error();
    return;
  }

  uint64_t data_size = static_cast<uint64_t>(data.size());
  if (data_size > popped_total_ + capacity_ - pushed_total_) { //
    data_size = popped_total_ + capacity_ - pushed_total_;
  }
  size_t idx = 0;
  while (data_size--) {
    buffer_[pushed_total_ % capacity_] = data[idx];
    pushed_total_++;
    idx++;
  }
}

void Writer::close() {
  // Your code here.
  is_close_ = true;
}

//
bool Writer::is_closed() const {
  // Your code here.
  return is_close_;
}

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const {
  // Your code here.
  return capacity_ - (pushed_total_ - popped_total_);
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const {
  // Your code here.
  return pushed_total_;
}

//  --------------------------------------------------------
//  --------------------------------------------------------

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const {
  // Your code here.
  return is_close_ && pushed_total_ == popped_total_;
}

// Number of bytes have been popped
uint64_t Reader::bytes_popped() const {
  // Your code here.
  return popped_total_;
}

// Peek at the next bytes in the buffer
string_view Reader::peek() const {
  const uint64_t buffered = bytes_buffered();
  if (buffered == 0) {
    return {}; // 没有数据，返回空 view
  }

  const uint64_t read_pos = popped_total_ % capacity_;
  const uint64_t contiguous_size = std::min(buffered, capacity_ - read_pos);

  if (contiguous_size > 0) {
    // 正常情况：当前位置到缓冲区末尾有一段连续数据
    return std::string_view(&buffer_[read_pos],
                            static_cast<size_t>(contiguous_size));
  } else {
    // read_pos == capacity_ 的情况（刚好回环到头）
    // 下一段数据从 buffer_[0] 开始
    return std::string_view(&buffer_[0],
                            static_cast<size_t>(std::min(buffered, capacity_)));
  }
}

// Remove `len` bytes from the buffer
void Reader::pop(uint64_t len) {
  popped_total_ += len;
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const {
  // Your code here.
  return pushed_total_ - popped_total_;
}
