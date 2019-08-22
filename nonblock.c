
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>


#ifdef USE_SOCKET_IO

#include <sys/socket.h>
#include <arpa/inet.h>

#endif


const size_t dummy_buffer_size_ = 1024 * 1200;

#ifndef USE_SOCKET_IO
const char *test_filename = "./testfile";
#endif


char *CreateDummyBuffer(size_t size) {
  char sym = 'A';
  char *buf = (char *)malloc(size);
  if (buf == NULL) {
    return NULL;
  }
  for (; size; --size) {
    buf[size] = sym;
  }
  return buf;
}

#ifdef USE_SOCKET_IO
int connect_to9997() {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9997);
  // 127.0.0.1
  addr.sin_addr.s_addr = (in_addr_t)htonl(0x7f000001);
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (connect(s, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    shutdown(s, SHUT_RDWR);
    close(s);
    s = -1;
  }
  return s;
}
#endif


int main(
#ifndef USE_SOCKET_IO
int argc, char *argv[]
#endif
) {
#ifndef USE_SOCKET_IO
  int target_fd = open(argc > 1 ? argv[1] : test_filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
  if (target_fd == -1) {
    perror("cant create testfile");
    return errno;
  }
  lseek(target_fd, 0, SEEK_SET);
#else
  int target_fd = connect_to9997();
  if (target_fd == -1) {
    perror("socket");
    return errno;
  }
#endif

  int flags = 0;
  flags = fcntl(target_fd, F_GETFL, 0);
  if (flags == -1)
    perror("fcntl: getfl");
  printf("fcntl flags: 0x%X\n", flags);
   if (fcntl(target_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    perror("fcntl: setfl");
  else
    printf("fcntl flags (set): 0x%X\n", flags | O_NONBLOCK);
  char *buffer = CreateDummyBuffer(dummy_buffer_size_);
  if (!buffer) {
    fprintf(stderr, "failed to allocate memory\n");
    return 3;
  }
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 700000;
  fd_set write_fds;
  FD_ZERO(&write_fds);
  FD_SET(target_fd, &write_fds);
  int res = select(target_fd + 1, NULL, &write_fds, NULL, &tv);
  if (res == 0) {
    printf("note: writeable timeout reached\n");
  } else if (res == -1) {
    perror("select failed");
  } else {
    printf("select result is %i, usec = %lu\n", res, tv.tv_usec);
  }
  ssize_t n = -1;
  unsigned int triescount = 4;
  do {
    n = write(target_fd, buffer, dummy_buffer_size_);
    printf("write result is %li\n", n);
    if (n == -1) {
      perror("write failed");
      printf("errno is %i\n", errno);
    }
    tv.tv_usec = 700000;
    select(target_fd + 1, NULL, &write_fds, NULL, &tv);
    triescount -= 1;
  } while (n == -1 && triescount > 0);
  free(buffer);
  fprintf(stderr, "Done");
  close(target_fd);
  return 0;
}

