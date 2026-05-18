#include "tcp_netstack_socket_impl.hh"

//! Specializations of TCPNetStackSocket for TCPOverIPv4OverTunFdAdapter and its
//! lossy version
template class TCPNetStackSocket<TCPOverIPv4OverTunFdAdapter>;
template class TCPNetStackSocket<LossyFdAdapter<TCPOverIPv4OverTunFdAdapter>>;
