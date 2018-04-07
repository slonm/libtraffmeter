#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <dlfcn.h>

static int init_l = 0;
static int sent_bytes = 0;
static int recv_bytes = 0;

typedef int (*connect_t)(int, const struct sockaddr *, socklen_t);
connect_t true_connect;

typedef ssize_t(*send_t)(int, const void *, size_t, int);
send_t true_send;

typedef ssize_t(*sendto_t)(int, const void *, size_t, int, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);
sendto_t true_sendto;

typedef ssize_t(*recv_t)(int, const void *, size_t, int);
recv_t true_recv;

typedef ssize_t(*recvfrom_t)(int, const void *, size_t, int, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
recvfrom_t true_recvfrom;

void bye() {
  printf("Sent: %iB, Resv: %iB\n", sent_bytes, recv_bytes);
}

static void init_lib() {
  true_connect = (connect_t) dlsym(RTLD_NEXT, "connect");

  if (!true_connect) {
    fprintf(stderr, "Cannot load symbol 'connect' %s\n", dlerror());
    exit(1);
  }

  true_send = (send_t) dlsym(RTLD_NEXT, "send");

  if (!true_send) {
    fprintf(stderr, "Cannot load symbol 'send' %s\n", dlerror());
    exit(1);
  }

  true_sendto = (sendto_t) dlsym(RTLD_NEXT, "sendto");

  if (!true_sendto) {
    fprintf(stderr, "Cannot load symbol 'sendto' %s\n", dlerror());
    exit(1);
  }

  true_recv = (recv_t) dlsym(RTLD_NEXT, "recv");

  if (!true_recv) {
    fprintf(stderr, "Cannot load symbol 'recv' %s\n", dlerror());
    exit(1);
  }

  true_recvfrom = (recvfrom_t) dlsym(RTLD_NEXT, "recvfrom");

  if (!true_recvfrom) {
    fprintf(stderr, "Cannot load symbol 'recvfrom' %s\n", dlerror());
    exit(1);
  }

  if (atexit(bye) != 0) {
    fprintf(stderr, "cannot set exit function\n");
    exit(1);
  }
  init_l = 1;
}

int connect(int sock, const struct sockaddr *addr, unsigned int len) {
  if (!init_l)
    init_lib();
  return true_connect(sock, addr, len);
}

ssize_t send(int __fd, const void *__buf, size_t __n, int __flags) {
  if (!init_l)
    init_lib();
  ssize_t result = true_send(__fd, __buf, __n, __flags);
  if (result > -1) {
    sent_bytes += result;
  }

  return result;
}

ssize_t sendto(int __fd, const void *__buf, size_t __n,
        int __flags, __CONST_SOCKADDR_ARG __addr,
        socklen_t __addr_len) {
  if (!init_l)
    init_lib();
  ssize_t result = true_sendto(__fd, __buf, __n, __flags, __addr, __addr_len);
  if (result > -1) {
    sent_bytes += result;
  }

  return result;

}

ssize_t recv(int __fd, void *__buf, size_t __n, int __flags) {
  if (!init_l)
    init_lib();
  ssize_t result = true_recv(__fd, __buf, __n, __flags);
  if (result > -1) {
    recv_bytes += result;
  }

  return result;
}

ssize_t recvfrom(int __fd, void *__restrict __buf, size_t __n,
        int __flags, __SOCKADDR_ARG __addr,
        socklen_t *__restrict __addr_len) {
  if (!init_l)
    init_lib();
  ssize_t result = true_recvfrom(__fd, __buf, __n, __flags, __addr, __addr_len);
  if (result > -1) {
    recv_bytes += result;
  }

  return result;
}
