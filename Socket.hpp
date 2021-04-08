#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <array>
#include <arpa/inet.h>
#include "Packet.hpp"

class Socket
{
public:
    constexpr static int usedAddressFamily = AF_INET;

public:
    Socket();
    ~Socket();
    void connect(const std::string& ip);
    void disconnect();
    void send(const EchoPacket& packet) const;
    int wait(timeval& timeVal) const;
    std::pair<EchoPacket, std::string> receiveFrom() const;
    bool checkPacket(const EchoPacket& packet, uint16_t expectedTtl) const;


private:
    int _socketFileDescriptor;
    sockaddr_in _socketAddress;
};

#endif