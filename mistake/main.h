#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

extern "C" {
#include "socket.h"
  //#include "server.h"

}


#endif /* _MAIN_H */
