CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

traceroute: main.o Packet.o Socket.o
	$(CXX) $(CXXFLAGS) -o traceroute main.o Packet.o Socket.o

main.o: main.cpp

Packet.o: Packet.cpp Packet.hpp

Socket.o: Socket.cpp Socket.hpp

clean:
	$(RM) *.o

distclean:
	$(RM) traceroute *.o

.PHONY: clean
