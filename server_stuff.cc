
#include "servicesocket.h"


#include <sys/time.h>
#include <unistd.h>


#include <iostream>
#include <vector>
#include <string>


class FdSelect {
public:
  FdSelect();
  ~FdSelect() { };
  void AddFd(int fd);
  void SetTimeout(__time_t seconds, __useconds_t microseconds);
  int Select();
  std::vector<int> GetReadyFds();

private:
  struct timeval timeout;
  fd_set read_fds;
  std::vector<int> v_read_fds;
  int highest_fd;
};


FdSelect::FdSelect() {
  FD_ZERO(&read_fds);
  highest_fd = -1;
}


void FdSelect::AddFd(int fd) {
  if (highest_fd < fd) 
    highest_fd = fd;
  FD_SET(fd, &read_fds);
  v_read_fds.push_back(fd);
}


void FdSelect::SetTimeout(__time_t seconds, __useconds_t microseconds) {
  timeout.tv_sec = seconds;
  timeout.tv_usec = microseconds;
}


int FdSelect::Select() {
  auto sec = timeout.tv_sec;
  auto msec = timeout.tv_usec;
  int res = select(highest_fd + 1, &read_fds, NULL, NULL, &timeout);
  timeout.tv_sec = sec;
  timeout.tv_usec = msec;
  return res;
}


std::vector<int> FdSelect::GetReadyFds() {
  std::vector<int> ready_fds;
  for (auto fd : v_read_fds) {
    if (FD_ISSET(fd, &read_fds)) {
      ready_fds.push_back(fd);
    }
  }
  return ready_fds;
}


int main() {
  ServiceSocket serv_socket{ 9997 };

  if (serv_socket.Bind() == -1) {
    perror("[-] bind");
    return errno;
  }

  serv_socket.Listen();
  FdSelect sel;
  sel.AddFd(serv_socket.ServerFd());
  sel.SetTimeout(4, 440000);

  int n = -1;
  for (int i = 0; i < 3; ++i) {
    n = sel.Select();

    if (n == 0) {
      std::cerr << "Connection timeout reached\n";
    }

    if (n == -1) {
      perror("[-] select");
      return errno;
    }
  }

  if (n == 0) {
    return ETIMEDOUT;
  } else {
    std::string msgready("");
    for (auto desc : sel.GetReadyFds()) {
      msgready.append(" ") += std::to_string(desc);
    }
    std::cerr << "These descriptors are ready:" << msgready << "\n";
  }

  int client_socket = serv_socket.Accept();

  if (client_socket == -1) {
    perror("[-] accept");
    return errno;
  }

  std::string message{ "My pid is" };
  pid_t process_pid = getpid();
  message += " ";
  message += std::to_string(process_pid);
  message += "\n";

  write(client_socket, message.c_str(), message.size());
  close(client_socket);

  return 0;
}
