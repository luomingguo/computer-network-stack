#include "byte_stream.hh"
#include <iostream>
#include <stdexcept>

using namespace std;

ByteStream::ByteStream(uint64_t capacity)
    : capacity_(capacity), is_close_{false}, popped_total_(0), pushed_total_(0),
      container_() {}

void Writer::push(string data) {

  if (has_error() || !data.size()) {
    return;
  }
  if (is_closed()) {
    set_error();
    return;
  }
  uint64_t cap = available_capacity();
  if (cap == 0)
    return;
  if (data.size() > cap) {
    data.resize(cap);
  }
  uint64_t written = data.size();
  container_.push_back(std::move(data));
  pushed_total_ += written;
}

void Writer::close() { is_close_ = true; }

bool Writer::is_closed() const { return is_close_; }

// How many bytes can be pushed to the stream right now?
uint64_t Writer::available_capacity() const {

  return capacity_ - (pushed_total_ - popped_total_);
}

// Total number of bytes cumulatively pushed to the stream
uint64_t Writer::bytes_pushed() const { return pushed_total_; }

//  --------------------------------------------------------
//  --------------------------------------------------------

// Is the stream finished (closed and fully popped)?
bool Reader::is_finished() const {

  return is_close_ && pushed_total_ == popped_total_;
}

// Number of bytes have been popped
uint64_t Reader::bytes_popped() const { return popped_total_; }

// Peek at the next bytes in the buffer
string_view Reader::peek() const {
  if (container_.empty()) {
    return {};
  }

  const string &chunk = container_.front();

  if (chunk.empty()) {
    return {};
  }

  return string_view(chunk.data(), chunk.size());
}

// Remove `len` bytes from the buffer
void Reader::pop(uint64_t len) {
  while (len > 0 && !container_.empty()) {

    string &chunk = container_.front();

    if (len >= chunk.size()) {
      len -= chunk.size();
      popped_total_ += chunk.size();
      container_.pop_front();
    } else {
      chunk.erase(0, len);
      popped_total_ += len;
      len = 0;
    }
  }
}

// Number of bytes currently buffered (pushed and not popped)
uint64_t Reader::bytes_buffered() const {

  return pushed_total_ - popped_total_;
}
