
#include "servicesocket.h"


ServiceSocket::ServiceSocket(in_port_t server_port) {
  memset(&service_address, 0, sizeof(struct sockaddr_in));
  service_address.sin_family = AF_INET;
  service_address.sin_port = htons(server_port);
  service_address.sin_addr.s_addr = INADDR_ANY;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}


int ServiceSocket::Bind() {
  return bind(server_socket, (const sockaddr *)&service_address, sizeof(struct sockaddr_in));
}


void ServiceSocket::Listen() {
  listen(server_socket, 5);
}


int ServiceSocket::Accept() {
  return accept(server_socket, nullptr, 0);
}


int ServiceSocket::ServerFd() {
  return server_socket;
}
