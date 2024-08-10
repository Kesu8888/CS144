#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  uint32_t ipAddr = next_hop.ipv4_numeric();
  EthernetFrame send;
  if (cacheList.contains(ipAddr)) {
    EthernetAddress eth = cacheList.at(ipAddr);
    send.header.type = EthernetHeader::TYPE_IPv4;
    send.payload = serialize(dgram);
    send.header.src = ethernet_address_;
    send.header.dst = eth;
    transmit(send);
  } else {
    if (requestList.contains(ipAddr)) {
      requestList.at(ipAddr).push_back(dgram);
      return;
    }
    send.header.type = EthernetHeader::TYPE_ARP;
    ARPMessage arpSend;
    arpSend.opcode = ARPMessage::OPCODE_REQUEST;
    arpSend.sender_ethernet_address = ethernet_address_;
    arpSend.sender_ip_address = ip_address_.ipv4_numeric();
    arpSend.target_ip_address = ipAddr;
    send.payload = serialize(arpSend);
    send.header.src = ethernet_address_;
    send.header.dst = ETHERNET_BROADCAST;
    transmit(send);
    std::deque<InternetDatagram> ID;
    ID.push_back(dgram);
    requestList.insert({ipAddr, ID});
    requestClock.emplace_back(rClock+5000, ipAddr);
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.

  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
    if (frame.header.dst != ethernet_address_)
      return;
    IPv4Datagram recv;
    if (parse(recv, frame.payload)) {
      datagrams_received_.push(recv);
    }
  } else {
    ARPMessage recv;
    if (parse(recv, frame.payload)) {
      uint32_t ipAddr = recv.sender_ip_address;
      if (!cacheList.contains(ipAddr)) {
        cacheList.insert({ipAddr, recv.sender_ethernet_address});
        cacheClock.emplace_back(cClock+30000, ipAddr);
        if (requestList.contains(ipAddr)) {
          for (const InternetDatagram& ID : requestList.at(ipAddr)) {
            send_datagram(ID, Address::from_ipv4_numeric(ipAddr));
          }
          requestList.erase(ipAddr);
        }
      }
      if (recv.opcode == ARPMessage::OPCODE_REQUEST && recv.target_ip_address == ip_address_.ipv4_numeric()) {
        reply(recv);
      }
    }
  }
}

void NetworkInterface::reply(ARPMessage& msg) {
  msg.opcode = ARPMessage::OPCODE_REPLY;
  msg.target_ip_address = msg.sender_ip_address;
  msg.target_ethernet_address = msg.sender_ethernet_address;
  msg.sender_ip_address = ip_address_.ipv4_numeric();
  msg.sender_ethernet_address = ethernet_address_;
  EthernetFrame reply;
  reply.payload = serialize(msg);
  reply.header.type = EthernetHeader::TYPE_ARP;
  reply.header.src = ethernet_address_;
  reply.header.dst = msg.target_ethernet_address;
  transmit(reply);
}
//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  cClock += ms_since_last_tick;
  rClock += ms_since_last_tick;
  while (!cacheClock.empty() && cacheClock.front().first <= cClock) {
    cacheList.erase(cacheClock.front().second);
    cacheClock.pop_front();
  }
  if (cacheClock.empty())
    cClock = 0;

  while (!requestClock.empty() && requestClock.front().first <= rClock) {
    requestList.erase(requestClock.front().second);
    requestClock.pop_front();
  }
  if (requestClock.empty())
    rClock = 0;
}
