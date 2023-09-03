#include <stdexcept>
#include <iostream>
#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ),
                                              is_close_(false),
                                              has_error_(false),
                                              popped_total_(0),
                                              pushed_total_(0),
                                              buffer_(std::deque<char>()) {}

void Writer::push( string data )
{
  // Your code here.
  if (this->is_closed()) {
    std::cerr << "writer is close" << std::endl;
    set_error();
    return;
  }
  uint64_t data_size = static_cast<uint64_t>(data.size());
  if (data_size == 0) return;
  if ( data_size > this->available_capacity() ) {
    data_size = this->available_capacity();
  }
  // std::copy(data.begin(), data.begin() + static_cast<size_t>(data_size), std::back_inserter(this->buffer_));
  for (uint64_t i = 0; i < data_size; i++) {
    this->buffer_.push_back(data[i]);
  }
  pushed_total_ += data_size;
}

void Writer::close()
{
  // Your code here.
  this->is_close_ = true;
}

void Writer::set_error()
{
  // Your code here.
  this->has_error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return this->is_close_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return this->capacity_ - static_cast<uint64_t>(this->buffer_.size());
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return this->pushed_total_;
}

string_view Reader::peek() const
{
  // Your code here.
  if (!this->buffer_.size()) {
    return {};
  }

  return std::string_view(&this->buffer_.front(), 1);
}

bool Reader::is_finished() const
{
  // Your code here.
  return this->is_close_ && !this->bytes_buffered();
}

bool Reader::has_error() const
{
  // Your code here.
  return this->has_error_;
}

void Reader::pop( uint64_t len )
{
  uint64_t pop_size = len;
  if (this->bytes_buffered() < len) {
    pop_size = this->bytes_buffered();
  }
  for (uint64_t i = 0; i < pop_size; i++) {
    this->buffer_.pop_front();
  }
  this->popped_total_ += pop_size;
}

uint64_t Reader::bytes_buffered() const
{
  return static_cast<uint64_t>(this->buffer_.size());
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return this->popped_total_;
}
