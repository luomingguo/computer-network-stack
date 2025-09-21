#include "tcp_receiver.hh"
#include <algorithm>
using namespace std;

void TCPReceiver::receive(TCPSenderMessage message) {
  // 首先检查RST标志
  if (message.RST) {
    reassembler_.reader().set_error();
    return;
  }

  // 检查SYN标志来初始化zero_point
  if (message.SYN) {
    zero_point_ = message.seqno;
    initialized_zero_point_ = true;
  }

  // 如果没有初始化zero_point，无法处理数据
  if (!initialized_zero_point_) {
    return;
  }

  // 优化：避免重复计算，直接计算正确的zero_point
  // SYN包：zero_point已经是message.seqno，无需调整
  // 非SYN包：需要zero_point + 1
  Wrap32 effective_zero_point = message.SYN ? zero_point_ : zero_point_ + 1;

  // 计算ByteStream序列号
  uint64_t insert_idx = message.seqno.unwrap(
      effective_zero_point, reassembler_.writer().bytes_pushed());

  reassembler_.insert(insert_idx, message.payload, message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const {
  // Your code here.
  const auto &writer = reassembler_.writer();
  const auto &reader = reassembler_.reader();
  std::optional<Wrap32> ackno = std::nullopt;
  uint32_t increment = 0;
  if (initialized_zero_point_) {
    increment++; // 表示下一个要接受的序列号
    if (writer.is_closed()) {
      increment++;
    }
    ackno =
        zero_point_ + static_cast<uint32_t>(writer.bytes_pushed() + increment);
  }
  uint16_t window_size = static_cast<uint16_t>(
      std::min(writer.available_capacity(), static_cast<uint64_t>(UINT16_MAX)));

  return TCPReceiverMessage{
      ackno,
      window_size,
      reader.has_error(),
  };
}
