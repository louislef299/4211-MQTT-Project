#include "socket.h"

Socket::Socket(){}

void Socket::enable_keepalive(int sock){
  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  int interval = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

  int maxpkt = 10;
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
}

int Socket::make_connection(int port){
  int sockfd = 0;
  struct sockaddr_in serv_addr; 

  char recvBuff[1024];
  memset(recvBuff, '0',strlen(recvBuff));
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("\n Error : Could not create socket \n");
    return -1;
  } 

  bzero(&serv_addr, sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port); 
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    printf("\n Error : Connect Failed \n");
    return -1;
  } 

  int n;
  char commandBuff[100];
  
  strcpy(commandBuff,"CONN");
  write(sockfd,commandBuff,strlen(commandBuff));
  
  while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
    if(strstr(recvBuff,"CONN_ACK")!=NULL)
      break;
  } 
  if(n < 0){
    printf("\n Read error \n");
  } 

  return sockfd;
}

bool canReadFromPipe(int *custom_pipe){
  struct pollfd fds;
  fds.fd = custom_pipe[0];
  fds.events = POLLIN;
  int res = poll(&fds, 1, 0);

  if(res < 0||fds.revents&(POLLERR|POLLNVAL)){
      //an error occurred, check errno
    }
  return fds.revents&POLLIN;
}

