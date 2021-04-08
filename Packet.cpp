#include "Packet.hpp"

//function that calculates a checksum for a header 
static uint16_t compute_icmp_checksum (const void *buff, int length)
{
	u_int32_t sum;
	const u_int16_t* ptr = (const u_int16_t*) buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}


EchoPacket::EchoPacket(uint16_t ttl, uint16_t sequence)
    :_ttl(ttl)
{
    _header.type = ICMP_ECHO;
	_header.code = 0;
	_header.un.echo.id = getpid() % UINT16_MAX;
	_header.un.echo.sequence = sequence;
	_header.checksum = 0;
	_header.checksum = compute_icmp_checksum ((u_int16_t*)&_header, sizeof(_header));
}

EchoPacket::EchoPacket(uint16_t ttl, const icmphdr& header)
    :_ttl(ttl), _header(header) {}

uint16_t EchoPacket::ttl() const
{
    return _ttl;
}

uint16_t EchoPacket::packetPid() const
{
    return _header.un.echo.id;
}

icmphdr EchoPacket::header() const
{
    return _header;
}