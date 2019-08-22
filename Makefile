CC = g++
CXXFLAGS = -O2 -Wall -Wextra
DEFINES =

all: server_stuff nonblock

nonblock:
	gcc $(CXXFLAGS) $(DEFINES) nonblock.c -o nonblock

server_stuff: servicesocket.o
	$(CC) $(CXXFLAGS) server_stuff.cc servicesocket.o -o server_stuff

servicesocket.o:
	$(CC) $(CXXFLAGS) -c servicesocket.cc

clean:
	rm server_stuff nonblock *.o
