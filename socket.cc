#include "socket.h"

Socket::Socket(){}

void Socket::enable_keepalive(int sock){
  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));

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

  memset(&serv_addr, '0', sizeof(serv_addr)); 

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port); 

  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){
    printf("\n inet_pton error occured\n");
    return -1;
  } 
  
  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    printf("\n Error : Connect Failed \n");
    return -1;
  } 

  int n;
  char commandBuff[100];
  
  strcpy(commandBuff,"CONN");
  write(sockfd,commandBuff,strlen(commandBuff));
  
  while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
    if(strcmp(recvBuff,"CONN_ACK")==0)
      break;
  } 
  if(n < 0){
    printf("\n Read error \n");
  } 

  
  return sockfd;
}

int Socket::set_up_socket(int port){
  int listenfd;
  struct sockaddr_in serv_addr; 

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port); 

  enable_keepalive(listenfd);
    
  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10);
  return listenfd;
}

