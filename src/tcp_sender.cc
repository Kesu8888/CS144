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
  if (FINSent)
    return;

  if (windows == 0) {
    TCPSenderMessage msg;
    //If the msg contains SYN or FIN, add it to "unAck".
    msg.seqno = Wrap32::wrap(absSeqno, isn_);
    if (absSeqno == 0) {
      msg.SYN = true;
    } else if (input_.reader().is_finished() && !FINSent) {
      msg.FIN = true;
      FINSent = true;
    }
    if (msg.sequence_length() > 0) {
      absSeqno++;
      unAck.push(msg);
      transmit(msg);
      startTimer();
    }
    return;
  }

  // Send as much as senderMessage
  uint64_t maxSeqno = recvAck + windows;
  while (absSeqno < maxSeqno && !FINSent) {
    //Get the max buffer space
    uint64_t bufSize = min(maxSeqno-absSeqno, TCPConfig::MAX_PAYLOAD_SIZE);

    string_view data;
    if ((data = input_.reader().peek()).empty() && !input_.reader().is_finished())
      return;
    TCPSenderMessage send;
    send.seqno = Wrap32::wrap(absSeqno, isn_);


    //Avoid exception caused by out of range substring.
    if (data.size() <= bufSize) {
      send.payload = data;
    } else
      send.payload = data.substr(0, bufSize);

    //Pop out the bytes, set the FIN flag and update the absSeqno
    input_.reader().pop( send.payload.size());
    send.SYN = absSeqno == 0;
    send.RST = RST;
    //Adding FIN flag if the stream is closed, the FIN flag is not limited by max payload size.
    if (input_.reader().is_finished() && (recvAck+windows-absSeqno) > data.size()) {
      send.FIN = input_.reader().is_finished();
      FINSent = true;
    }
    absSeqno += send.SYN+send.payload.size()+send.FIN;

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
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  //Calculate the msg
  if (msg.RST) {
    RST = true;
    return;
  }

  uint64_t msgSeqno = msg.ackno == nullopt ? 0 : msg.ackno->unwrap(isn_, absSeqno);
  //Receive an impossible seqno, we should ignore it
  if (msgSeqno > absSeqno)
    return;

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
    stopTimer();
  }
  if (!unAck.empty())
    startTimer();
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // If the timer is not started, return
  if (timer >= RTO)
    return;

  //If no msg in the retransmission list, stop the timer and return
  if (unAck.empty()) {
    stopTimer();
    return;
  }

  //timer plus the past tick. If it is above the RTO_ms limit, transmit the function
  timer += ms_since_last_tick;
  if (timer >= RTO) {
    transmit(unAck.front());
    retrans ++;
    timer = 0;
    RTO *= 2;
  }
}
