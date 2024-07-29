#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return {};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  // Read all the data of the input stream
  if (windows == 0) {
    transmit(make_empty_message());
    return;
  }

  // Send as much as senderMessage
  while (windows > 0 && !input_.reader().is_finished()) {
    //Get the max amount of data we can send
    uint64_t bufSize = min(windows, TCPConfig::MAX_PAYLOAD_SIZE);
    TCPSenderMessage send;
    send.FIN = absSeqno == 0;
    send.seqno = Wrap32::wrap(absSeqno, isn_);
    string_view data = input_.reader().peek();

    //Avoid exception caused by out of range substring.
    if (data.size() < bufSize) {
      send.payload = data;
    } else
      send.payload = data.substr(0, bufSize);

    //Pop out the bytes, set the FIN flag ans update the absSeqno
    input_.reader().pop( send.payload.size());
    send.FIN = input_.reader().is_finished();
    absSeqno += send.payload.size();

    unAck.push(send);
    transmit(send);
    windows -= send.payload.size();
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  //Based on the absSeqno, calculate the new absSeqno
  absSeqno = msg.ackno->unwrap(isn_, absSeqno);
  windows = msg.window_size;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  (void)ms_since_last_tick;
  (void)transmit;
  (void)initial_RTO_ms_;
}
