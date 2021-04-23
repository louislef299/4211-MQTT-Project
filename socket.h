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
#include <poll.h>

class Socket{
public:
  Socket();

  void enable_keepalive(int sock);

  int set_up_socket(int port=5000);

  int make_connection(int port = 5000);
};

#endif // SOCKET_H
