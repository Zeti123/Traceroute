#include "Socket.hpp"
#include "Packet.hpp"


Socket::Socket()
    : _socketFileDescriptor(socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)), _socketAddress({})
{
    if (_socketFileDescriptor == -1) 
        throw std::invalid_argument("Cannot open socket");
}

Socket::~Socket()
{
    close(_socketFileDescriptor);
}

void Socket::connect(const std::string& ip)
{
    // resetting the structure
    _socketAddress = {};
    
    _socketAddress.sin_family = usedAddressFamily;

    // changing the address from text to bit form and saving in the appropriate structure
    int ret = inet_pton(usedAddressFamily, ip.data(), &_socketAddress.sin_addr);

    if (ret == -1)
        throw std::invalid_argument("Address family is not valid");
    if (ret == 0)
        throw std::invalid_argument("Given address is not a valid network address in the specified address family");
}

void Socket::send(const EchoPacket& packet) const
{
    int ttl = packet.ttl();

    // setting the appropriate socket options
    int ret = setsockopt (_socketFileDescriptor, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    if (ret == -1)
        throw std::invalid_argument("Error while setting options for socket");
    
    icmphdr header = packet.header();

    // sending echo request to address in sockaddr
    ssize_t sendBytes = sendto(_socketFileDescriptor, &header, sizeof(header), 0, (sockaddr*)&_socketAddress, sizeof(_socketAddress));

    if (sendBytes == -1)
        throw std::invalid_argument("Sending packet failed");
}

int Socket::wait(timeval& timeVal) const
{
    fd_set descriptorsSet;
	FD_ZERO(&descriptorsSet);
	FD_SET(_socketFileDescriptor, &descriptorsSet);

	int received = select(_socketFileDescriptor + 1, &descriptorsSet, NULL, NULL, &timeVal);
    if (received == -1)
        throw std::invalid_argument("Error while waiting for packets");

    return received;
}

// it returns a pair of the received packet and the ip address it came from
std::pair<EchoPacket, std::string> Socket::receiveFrom() const
{
	uint8_t buff[IP_MAXPACKET];
    struct sockaddr_in 	sender;	
	socklen_t senderLength = sizeof(sender);

    // receiving the package
	ssize_t packetLength = recvfrom (_socketFileDescriptor, buff, IP_MAXPACKET, MSG_DONTWAIT, (struct sockaddr*)&sender, &senderLength);
	if (packetLength == -1) 
		throw std::invalid_argument("Error while reciving packet");

    // changing the ip address from bitwise to string
    char ipAddress[50]; 
	const char* ptr = inet_ntop(AF_INET, &(sender.sin_addr), ipAddress, sizeof(ipAddress));
    if (ptr == nullptr)
        throw std::invalid_argument("Error while conversion of ip address");

    // writing the values ​​from the received packet to the appropriate structures
    iphdr* ipHeader = (iphdr*)buff;
	icmphdr* icmpHeader = (icmphdr*) (buff + 4 * ipHeader->ihl); // ihl holds the number of 32 bit machine words that the ip header has
    
    switch (icmpHeader->type)
    {
    case ICMP_ECHOREPLY:
        return std::make_pair(EchoPacket((icmpHeader->un.echo.sequence - 1) / 3 + 1, *icmpHeader), std::string(ipAddress));
    break;

    case ICMP_TIME_EXCEEDED:
    {
        char* i = ((char*)icmpHeader) + 8;
        icmp* ic = (icmp*)(i + 4 * ((struct ip*) i)->ip_hl);
        return std::make_pair(EchoPacket((ic->icmp_seq - 1) / 3 + 1, *(icmphdr*)ic), std::string(ipAddress));
    }
    break;

    default:
        /* so we return something that
         * the rest of the code will be considered an invalid package*/ 
        return std::make_pair(EchoPacket(-1, *icmpHeader), "");
    break;
    }
}


//checks to see if this is the package we want
bool Socket::checkPacket(const EchoPacket& packet, uint16_t expectedTtl) const
{
    return (packet.packetPid() == getpid() % UINT16_MAX) && (packet.ttl() == expectedTtl);
}