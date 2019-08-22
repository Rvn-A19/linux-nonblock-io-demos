
#ifndef SERVICESOCKET_H_
#define SERVICESOCKET_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <cstring>


class ServiceSocket {

public:
  ServiceSocket(in_port_t server_port);
  int Bind();
  void Listen();
  int Accept();
  int ServerFd();
  // TODO.
  ~ServiceSocket() { };

private:
  int server_socket;
  struct sockaddr_in service_address;

};


#endif // SERVICESOCKET_H_
