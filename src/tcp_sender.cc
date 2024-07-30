#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return absSeqno-recvAck;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retrans;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  // Read all the data of the input stream
  if (windows == 0) {
    TCPSenderMessage msg = make_empty_message();
    //If the msg contains SYN or FIN, add it to "unAck".
    if (msg.sequence_length() > 0) {
      absSeqno += msg.sequence_length();
      unAck.push(msg);
      timer = (timer == 0) ? initial_RTO_ms_ : timer;
    }
    transmit(msg);
    return;
  }

  // Send as much as senderMessage
  uint64_t maxSeqno = recvAck + windows;
  while (absSeqno < maxSeqno && !input_.reader().peek().empty()) {
    //Get the max amount of data we can send
    uint64_t bufSize = min(maxSeqno-absSeqno, TCPConfig::MAX_PAYLOAD_SIZE);
    TCPSenderMessage send;
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

    //Save the msg to "unAck" and send the msg
    unAck.push(send);
    transmit(send);
    startTimer();
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap(absSeqno, isn_);
  msg.SYN = absSeqno == 0;

  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  //Calculate the msg
  uint64_t msgSeqno = msg.ackno->unwrap(isn_, absSeqno);
  //If the acknowledged seqno is below our recvAck, it means the msg is outdated. A newer msg was received previously
  //and we can ignore this msg.
  if (msgSeqno <= recvAck) {
    if (msgSeqno+msg.window_size > recvAck+windows)
      windows = msgSeqno+msg.window_size-recvAck;
    return;
  }

  recvAck = msgSeqno;
  windows = msg.window_size;

  //If the tcpsendermsg has been acknowledged, remove them from the list
  while (!unAck.empty()) {
    TCPSenderMessage top = unAck.front();
    uint64_t seqno = top.seqno.unwrap(isn_, absSeqno) + top.sequence_length();
    if (seqno > recvAck) {
      break;
    }
    unAck.pop();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // If the timer is not started, return
  if (timer == UINT64_MAX)
    return;

  //If no msg in the retransmission list, stop the timer and return
  if (unAck.empty()) {
    stopTimer();
    return;
  }

  //timer plus the past tick. If it is above the RTO_ms limit, transmit the function
  timer += ms_since_last_tick;
  if (timer >= initial_RTO_ms_) {
    transmit(unAck.front());
    retrans ++;
    timer = 0;
  }
}
