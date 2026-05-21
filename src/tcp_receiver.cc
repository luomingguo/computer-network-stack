#include "tcp_receiver.hh"
#include <algorithm>
using namespace std;

void TCPReceiver::receive(TCPSenderMessage message) {
  // 首先检查RST标志
  if (message.RST) {
    reassembler_.reader().set_error();
    return;
  }
  if (message.SYN) {
    isn_ = message.seqno;
  }
  // ISN 未初始化则忽略
  if (!isn_.has_value()) {
    return;
  }

  const uint64_t abs_seq =
      message.seqno.unwrap(*isn_, reassembler_.writer().bytes_pushed());
  const uint64_t stream_idx = abs_seq - 1 + static_cast<uint64_t>(message.SYN);

  reassembler_.insert(stream_idx, std::move(message.payload), message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const {
  TCPReceiverMessage msg;
  if (reassembler().reader().has_error()) {
    msg.RST = true;
    return msg;
  }
  if (isn_.has_value()) {
    // 绝对 ackno = 1(SYN) + 已推送字节数 + 1(FIN，仅流关闭后)
    const uint64_t abs_ackno =
        1 + reassembler_.writer().bytes_pushed() +
        static_cast<uint64_t>(reassembler_.writer().is_closed());

    msg.ackno = Wrap32::wrap(abs_ackno, *isn_);
  }
  msg.window_size = static_cast<uint16_t>(std::min<uint64_t>(
      reassembler_.writer().available_capacity(), UINT16_MAX));
  return msg;
}
