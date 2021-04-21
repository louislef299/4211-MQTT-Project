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
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <vector>

struct client{
  pthread_t tid;
  int socket;
  /****************************************************** 
     this is to store current subscriptions
     [0001 - weather]
     [0010 - news]
     [0100 - health]
     [1000 - security] 
  ******************************************************/
  unsigned short subscriptions;
  /****************************************************** 
     this is to store current spot in subscriptions
     [0 - weather]
     [1 - news]
     [2 - health]
     [3 - security] 
  ******************************************************/
  int current[4];
};

pthread_mutex_t lock;

std::ofstream output_log;

std::vector<client> weather;
std::vector<client> news;
std::vector<client> health;
std::vector<client> security;

void enable_keepalive(int sock){
  int yes = 1;
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));

  int interval = 1;
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));

  int maxpkt = 10;
  setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int));
}

int check_subs(client _client){
  pthread_mutex_lock(&lock);
  int retval = 0;
  if(_client.subscriptions & 1){
    if((int)(weather.size()-1) > _client.current[0])
      retval |= 1;
  }
  if(_client.subscriptions & 2){
    if((int)(news.size()-1) > _client.current[1])
      retval |= 2;
  }
  if(_client.subscriptions & 4){
    if((int)(health.size()-1) > _client.current[2])
      retval |= 4;
  }
  if(_client.subscriptions & 8){
    if((int)(security.size()-1) > _client.current[3])
      retval |= 8;
  }
  pthread_mutex_unlock(&lock);
  return retval;
}

void *thread_function(void* command){
  client _client = *((client *)command);

  char recvBuff[100];
  memset(recvBuff, '0',strlen(recvBuff));
  int n,m,loop_break;

  for(int i=0;i<4;i++)
    _client.subscriptions = 0b0000;

  loop_break = 1;
  while(loop_break){
    while((n=read(_client.socket,recvBuff,sizeof(recvBuff)-1))>0 || (m=check_subs(_client))>0){
      pthread_mutex_lock(&lock);
      if(n){
	if(strstr(recvBuff,"PUB")!=NULL){
	  char *topic,*rest;
	  rest = recvBuff;
	  int iteration = 0;
	  while((topic = strtok_r(rest,",",&rest))){
	    if(iteration == 1){
		output_log << "PUB: " << topic << "\n\tmessage: ";
		output_log.flush();
	    }
	    else if(iteration == 2){
		output_log << topic << "\n";
		output_log.flush();
	    }
	    else{
	      output_log << topic << "\n";
	      output_log.flush();
	    }
	    iteration++;  
	  }
	}
	if(strstr(recvBuff,"DISC")!=NULL){
	  loop_break = 0;
	}
      }
      if(m){
	
      }
      pthread_mutex_unlock(&lock);
    }
  }
  close(_client.socket);
  return NULL;
}

int main(int argc, char *argv[]){
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    output_log.open("connections_log.txt");
    
    client clients[100];
    char sendBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    enable_keepalive(listenfd);
    
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    while(1){
      clients[connfd].socket = accept(listenfd, (struct sockaddr*)NULL, NULL); 

      char recvBuff[100];
      memset(recvBuff, '0',strlen(recvBuff));
      int n;

      while((n = read(clients[connfd].socket, recvBuff, sizeof(recvBuff)-1)) > 0){	  
	if(strcmp(recvBuff,"CONN")==0){
	  output_log << "Client: <CONN>\nServer: <CONN_ACK>\n";
	  output_log.flush();
	  strcpy(sendBuff,"CONN_ACK");
	  write(clients[connfd].socket, sendBuff, strlen(sendBuff)); 
	  memset(sendBuff, '0', sizeof(sendBuff));
	  pthread_create(&clients[connfd].tid,NULL,thread_function,(void *)&clients[connfd]);
	  break;
	}

        fputs(recvBuff,stdout);
      }
      	
      sleep(1);
    }

    output_log.close();
}
