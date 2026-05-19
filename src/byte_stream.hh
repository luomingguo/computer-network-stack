#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>

class Reader;
class Writer;

/*
ByteStream 可靠字节流抽象对象
- 数据从输入端写入，从输出端按照相同顺序读取
- writer 可以close输入并结束，reader读到EOF才能结束
- 单线程运行
*/
class ByteStream {
public:
  explicit ByteStream(uint64_t capacity);

  // Helper functions (provided) to access the ByteStream's Reader and Writer
  // interfaces
  Reader &reader();
  const Reader &reader() const;
  Writer &writer();
  const Writer &writer() const;

  void set_error() {
    error_ = true;
  }; // Signal that the stream suffered an error.
  bool has_error() const { return error_; }; // Has the stream had an error?

protected:
  // Writer and Reader interfaces.
  uint64_t capacity_;
  bool error_{};
  bool is_close_;
  uint64_t popped_total_;
  uint64_t pushed_total_;
  std::deque<std::string> container_;
};

class Writer : public ByteStream {
public:
  void push(std::string data); // Push data to stream, but only as much as
                               // available capacity allows.
  void close(); // Signal that the stream has reached its ending. Nothing more
                // will be written.

  bool is_closed() const; // Has the stream been closed?
  uint64_t available_capacity()
      const; // How many bytes can be pushed to the stream right now?
  uint64_t bytes_pushed()
      const; // Total number of bytes cumulatively pushed to the stream
};

class Reader : public ByteStream {
public:
  std::string_view peek() const; // Peek at the next bytes in the buffer
  void pop(uint64_t len);        // Remove `len` bytes from the buffer

  bool is_finished() const; // Is the stream finished (closed and fully popped)?
  uint64_t bytes_buffered()
      const; // Number of bytes currently buffered (pushed and not popped)
  uint64_t
  bytes_popped() const; // Total number of bytes cumulatively popped from stream
};

/*
 * read: A (provided) helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read(Reader &reader, uint64_t len, std::string &out);
