#include <iostream>
#include <iomanip>
#include <regex>
#include <chrono>
#include "Socket.hpp"

using namespace std;

bool checkIp(const std::string& ip)
{
    std::regex ipAddres("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(ip, ipAddres);
}

std::string getInputIp(int argc, char* argv[])
{
    if (argc < 2)
        throw std::invalid_argument("No argument was given");
    
    else if (!checkIp(argv[1]))
        throw std::invalid_argument("Argument should be IPv4 address");

    return std::string(argv[1]);
}

EchoPacket getNewPacket(uint16_t ttl)
{
    static size_t sequence = 0;
    return EchoPacket(ttl, ++sequence);
}

void send3Packets(uint16_t ttl, Socket& s)
{
    for (int i = 0; i < 3; i++)
        s.send(getNewPacket(ttl));
}

// returns pair {ip address, time in milliseconds}
std::array<std::pair<std::string, double>, 3> waitAndReceive(Socket& socket, uint16_t expectedTtl)
{
    auto beginTime = std::chrono::high_resolution_clock::now();
    int received = 0;
    timeval timeVal = {1, 0};
    auto returnVal = std::array<std::pair<std::string, double>, 3>{{std::make_pair(std::string(""), -1), 
                                                                    std::make_pair(std::string(""), -1), 
                                                                    std::make_pair(std::string(""), -1)}};

    while(received < 3)
    {
        int recivedNow = socket.wait(timeVal);
        if (recivedNow == 0)
            break;
        
        auto timeOfReceiving = std::chrono::high_resolution_clock::now();
        //returns in milliseconds
        auto delataTime = timeOfReceiving - beginTime;
        while (recivedNow > 0)
        {
            auto ret = socket.receiveFrom();
            if (socket.checkPacket(ret.first, expectedTtl))
            {
                returnVal[received] = std::make_pair(ret.second, delataTime.count() / 1000000);
                received++;
            }
            recivedNow--;
        }
    }
            
    return returnVal;
}

// returns if destination has been achieved
bool showOnePath(const std::array<std::pair<std::string, double>, 3>& path, const std::string& destination)
{
    double avrage = 0;
    int recived = 0;
    bool ret = false;

    // checks if the given ip address appeared before
    auto isRepeat = [path](size_t index) { for (size_t i = 0; i < index; i++) if (path[i].first == path[index].first) return true; return false; };
    while (recived < 3)
    {
        if (path[recived].second < 0)
            break;
        
        if (path[recived].first == destination)
            ret = true;

        if (!isRepeat(recived))
            std::cout<<path[recived].first<<" ";

        avrage += path[recived].second;
        recived++;
    }

    if (recived == 0)
        std::cout<<'*'<<std::endl;
    else if (recived < 3)
        std::cout<<"???"<<std::endl;
    else if (recived == 3)
        std::cout<<fixed<<setprecision(1)<<avrage/3<<"ms"<<std::endl;

    return ret;
}

void tryBlock(const std::string& ip)
{
    Socket s;
    s.connect(ip);
    
    for (int i = 1; i <= 30; i++)
    {
        send3Packets(i, s);
        std::cout<<i<<". ";
        if (showOnePath(waitAndReceive(s, i), ip))
            break;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        const std::string ip = getInputIp(argc, argv);
        tryBlock(ip);
    }
    catch(const std::invalid_argument& e)
    {
        std::cerr<<"Exception: "<<e.what()<<std::endl;
        if (strcmp(strerror(errno), "Success"))
            std::cerr<<"errno: "<<strerror(errno)<<std::endl;
        return -1;
    }
    return 0;
}