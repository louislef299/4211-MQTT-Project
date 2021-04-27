#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <netinet/tcp.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <vector>
#include <string>
#include <cstring>
#include <sys/poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>

class Socket{
public:
  Socket();

  void enable_keepalive(int sock);

  int make_connection(int port = 5000);

  bool canReadFromPipe();
};

#endif // SOCKET_H
