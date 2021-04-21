#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <netinet/tcp.h>

void enable_keepalive(int sock){
  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));

  //int idle = 1;
  //setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));

  int interval = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

  int maxpkt = 10;
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
}


int main(int argc, char *argv[]){
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    int sockets[1000];
    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    //enable_keepalive(listenfd);
    
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    char recvBuff[1024];
    memset(recvBuff, '0',strlen(recvBuff));
    int n;
    
    while(1){
        sockets[connfd] = accept(listenfd, (struct sockaddr*)NULL, NULL); 
	
        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(sockets[connfd], sendBuff, strlen(sendBuff)); 
	memset(sendBuff, '0', sizeof(sendBuff));
	int cont = 1;
	while((n = read(sockets[connfd], recvBuff, sizeof(recvBuff)-1)) > 0 && cont){	  
	  strcat(sendBuff,recvBuff);
	  sputs(sendBuff,stdout);
	  if(strcmp(sendBuff,"Close\n")==0)
	    cont = 0;
	  if(fputs(recvBuff, stdout) == EOF){
	    printf("\n Error : Fputs error\n");
	  }
	}
	memset(sendBuff, '0', sizeof(sendBuff));
	sprintf(sendBuff,"Close\n");
	write(sockets[connfd],sendBuff,strlen(sendBuff));
	
        //close(sockets[connfd]);
        sleep(1);
     }
}
