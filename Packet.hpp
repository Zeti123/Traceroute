#ifndef PACKET_HPP
#define PACKET_HPP

#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <cassert>

class EchoPacket
{
public:
    EchoPacket(uint16_t ttl, uint16_t sequence);
    EchoPacket(uint16_t ttl, const icmphdr& header);
    uint16_t ttl() const;
    uint16_t packetPid() const;
    icmphdr header() const;

private:
    uint16_t _ttl;
    icmphdr _header;
};

#endif