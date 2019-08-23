
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


const size_t dummy_buffer_size_ = 1024 * 1200 * 256;

#ifndef USE_SOCKET_IO
const char *test_filename = "./testfile";
#endif


typedef enum {
  kSelectReads,
  kSelectWrites,
  kSelectExceptions
} SelectMode;


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


int SetNonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
    return 0;
  if ( fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1 ) {
    return 0;
  }
  return flags | O_NONBLOCK;
}


int SelectSingle(int fd, SelectMode mode, __time_t secs, __useconds_t usecs) {
  fd_set writes, reads, excepts;
  fd_set *pfds_w = &writes;
  fd_set *pfds_r = NULL;
  fd_set *pfds_e = NULL;
  
  FD_ZERO(&writes);
  FD_ZERO(&reads);
  FD_ZERO(&excepts);

  FD_SET(fd, pfds_w);

  if (mode == kSelectReads) {
    pfds_w = NULL;
    pfds_r = &reads;
    FD_SET(fd, pfds_r);
  }
  else if (mode == kSelectExceptions) {
    pfds_w = NULL;
    pfds_e = &excepts;
    FD_SET(fd, pfds_e);
  }

  struct timeval t;
  t.tv_sec = secs;
  t.tv_usec = usecs;

  return select(fd + 1, pfds_r, pfds_w, pfds_e, &t);
}


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


  if (!SetNonblocking(target_fd)) {
    perror("fcntl failure");
    return errno;
  }

  char *buffer = CreateDummyBuffer(dummy_buffer_size_);
  if (!buffer) {
    fprintf(stderr, "failed to allocate memory\n");
    return 3;
  }
  
  int res = SelectSingle(target_fd, kSelectWrites, 0, 700000);

  if (res == 0) {
    printf("note: writeable timeout reached\n");
  } else if (res == -1) {
    perror("select failed");
  } else {
    printf("select result is %i\n", res);
  }

  ssize_t n = -1;
  size_t bytes_remaining = dummy_buffer_size_;

  do {
#ifdef CLRSCR
    printf("\033c");
#endif
    n = write(target_fd, buffer, bytes_remaining);
    printf("write result is %li\n", n);
    if (n == -1) {
      perror("write failed");
      printf("errno is %i\n", errno);
      free(buffer);
      return errno;
    }
    bytes_remaining -= n;

    if (bytes_remaining <= 0) {
      break;
    }

    printf("%li byte(s) remains\n", bytes_remaining);

    while (1) {
      res = SelectSingle(target_fd, kSelectWrites, 0, 200000);
      printf("selected %i fds\n", res);
      if (res == -1) {
        free(buffer);
        perror("select failed");
        return errno;
      }
      if (res > 0) {
        break;
      }
  }

  } while (n > 0);

  free(buffer);

  fprintf(stderr, "Done\n");

  close(target_fd);
  return 0;
}

