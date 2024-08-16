#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  for (uint8_t i = prefix_length; i <= 32; i++)
  hops.at(prefix_length).insert({route_prefix & masks.at(i), hopInfo(next_hop, interface_num)});
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for (std::shared_ptr<NetworkInterface>& NI : _interfaces) {
    std::queue<InternetDatagram>& q = NI->datagrams_received();
    while (!q.empty()) {
      InternetDatagram ID = q.front();
      q.pop();
      if (ID.header.ttl <= 1) {
        continue;
      }
      ID.header.ttl --;
      ID.header.compute_checksum();
      optional<hopInfo> hp;
      for (int i = 32; i >= 0; i--) {
        uint32_t dst = ID.header.dst & masks.at(i);
        if (hops[i].contains(dst)) {
          hp = hops[i].at(dst);
          break;
        }
      }
      if (hp->ipAddr != nullopt) {
        interface(hp->interface)->send_datagram(ID, hp->ipAddr.value());
      } else {
        interface(hp->interface)->send_datagram(ID, Address::from_ipv4_numeric(ID.header.dst));
      }
    }
  }
}
std::vector<uint32_t> Router::mask_init()
{
  std::vector<uint32_t> mask(33);
  mask[0] = 0;
  mask[1] = 1 << 31;
  for (size_t i = 2; i < mask.size(); i++) {
    mask.at(i) = mask.at(i-1) | (mask.at(i-1)>>1);
  }
  return mask;
}
