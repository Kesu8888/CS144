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
  if (input_.reader().has_error()) {
    TCPSenderMessage msg = make_empty_message();
    transmit(msg);
    unAck.push(msg);
    startTimer(initial_RTO_ms_);
  }
  if (FINSent)
    return;
  if (windows == 0) {
    TCPSenderMessage msg = make_empty_message();
    if (absSeqno == 0) {
      msg.SYN = true;
    } else if (!input_.reader().peek().empty() && unAck.empty()) {
      msg.payload = input_.reader().peek().front();
      input_.reader().pop(1);
    } else {
      msg.FIN = input_.reader().is_finished();
    }
    if (msg.sequence_length() > 0) {
      if (!msg.SYN)
        stuckMsg = true;
      absSeqno += msg.sequence_length();
      unAck.push(msg);
      transmit(msg);
      startTimer(initial_RTO_ms_);
    }
    return;
  }

  // Send as much as senderMessage
  uint64_t maxSeqno = recvAck + windows;
  while (absSeqno < maxSeqno && !FINSent) {
    uint64_t bufSize = min(maxSeqno-absSeqno, TCPConfig::MAX_PAYLOAD_SIZE);

    string_view data;
    data = input_.reader().peek();
    TCPSenderMessage send = make_empty_message();
    if (data.size() <= bufSize) {
      send.payload = data;
    } else
      send.payload = data.substr(0, bufSize);

    input_.reader().pop( send.payload.size());
    send.SYN = absSeqno == 0;
    //Adding FIN flag if the stream is closed, the FIN flag is not limited by max payload size.
    if (input_.reader().is_finished() && (recvAck+windows-absSeqno) > data.size()) {
      send.FIN = FINSent = true;
    }
    if (send.sequence_length() == 0)
      return;
    absSeqno += send.sequence_length();
    //Save the msg to "unAck" and send the msg
    unAck.push(send);
    transmit(send);
    startTimer(initial_RTO_ms_);
  }
}
TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap(absSeqno, isn_);
  msg.RST = input_.has_error();
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if (msg.RST)
    input_.writer().set_error();
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
    retrans = 0;
    stopTimer();
  }
  if (!unAck.empty())
    startTimer(initial_RTO_ms_);
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // If the timer is not started, return
  if (timer >= RTO)
    return;
  //timer plus the past tick. If it is above the RTO_ms limit, transmit the function
  timer += ms_since_last_tick;
  if (timer >= RTO) {
    retrans ++;
    transmit(unAck.front());
    if (stuckMsg) {
      startTimer(RTO);
    } else
      startTimer(RTO*2);
  }
}
void TCPSender::startTimer(uint64_t RTO_) {
  //The timer is not started yet, start it by setting the timer to 0 and init the RTO
  if (timer >= RTO) {
    timer = 0;
    RTO = RTO_;
  }
}
void TCPSender::stopTimer() {
  RTO = initial_RTO_ms_;
  timer = RTO;
  stuckMsg = false;
}
