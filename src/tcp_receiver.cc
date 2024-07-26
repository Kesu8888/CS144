#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if (message.RST || reader().has_error()) {
    reader().set_error();
    return;
  }
  if (message.SYN) {
    SYN = true;
    zeropoint = Wrap32(message.seqno);
    message.seqno = message.seqno+1;
  }
  if (SYN) {
    uint64_t streamIndex = message.seqno.unwrap(zeropoint, writer().bytes_pushed())-1;
    reassembler_.insert(streamIndex, message.payload, message.FIN);
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage msg;
  if (SYN) {
    if (writer().is_closed()) {
      msg.ackno = Wrap32::wrap(writer().bytes_pushed()+2, zeropoint);
    } else {
      msg.ackno = Wrap32::wrap(writer().bytes_pushed()+1, zeropoint);
    }
  }

  msg.window_size = min(writer().available_capacity(), (uint64_t)UINT16_MAX);
  msg.RST = reader().has_error();
  return msg;
}
