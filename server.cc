#include "socket.h"

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
  int client_pipe[2];
};

pthread_mutex_t lock;

int current_port = 5000;

std::ofstream output_log;

std::vector<client> weather;
std::vector<client> news;
std::vector<client> health;
std::vector<client> security;

Socket *socket_helper = new Socket();

void subscription_handler(client _client,char* recvBuff){
  char *topic,*rest;
  rest = recvBuff;
  int iteration = 0;
  std::vector<client> *temp;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      output_log << "Client"<< _client.socket << ": <SUB,TOPIC>\nTopic: " << topic << "\n\n";
      output_log.flush();

      if(strstr(topic,"weather") != NULL){
	temp = &weather;
      }
      else if(strstr(topic,"news") != NULL){
	temp = &news;
      }
      else if(strstr(topic,"health") != NULL){
	temp = &health;
      }
      else if(strstr(topic,"security") != NULL){
	temp = &security;
      }
      temp->push_back(_client);
    }
    iteration++;
  }
}

int disconnect_handler(client _client){
  char recvBuff[1000];
  memset(recvBuff, '0',strlen(recvBuff));
  memset(recvBuff, '0',strlen(recvBuff));
  strcpy(recvBuff,"DISC_ACK");
  write(_client.socket,recvBuff,strlen(recvBuff));
  output_log << "Client" << _client.socket << ": <DISC>\nServer: <DISC_ACK>\n\n";
  output_log.flush();
  close(_client.socket);
  return 0;
}

void publish_handler(client _client,char* recvBuff){
  char *topic,*rest;
  rest = recvBuff;
  int iteration = 0;
  std::vector<client> *temp;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      output_log << "Client" << _client.socket << ":<PUB,TOPIC,MSG>\nTopic: " << topic << "\n\tmessage: ";
      output_log.flush();

      if(strstr(topic,"weather") != NULL)
	temp = &weather;
      else if(strstr(topic,"news") != NULL)
	temp = &news;
      else if(strstr(topic,"health") != NULL)
	temp = &health;
      else if(strstr(topic,"security") != NULL)
	temp = &security;

    }
    else if(iteration == 2){
      output_log << topic << "\n\n";
      output_log.flush();

      for(int i=0;i<(int)(temp->size());i++){
	if(write(temp->at(i).socket,topic,sizeof(topic)+1) < 0)
	  output_log << "Write Failure\n";
	output_log<<"Writing to Client" << temp->at(i).socket << " with message " << topic << "\n\n";
	output_log.flush();
      }
    }
    iteration++;  
  }
}

void *thread_function(void* command){
  client _client = *((client *)command);

  if(pipe(_client.client_pipe) < 0)
    exit(1);

  char recvBuff[1000];
  memset(recvBuff, '\0',strlen(recvBuff));
  int n,loop_break;

  for(int i=0;i<4;i++)
    _client.subscriptions = 0b0000;

  loop_break = 1;
  while(loop_break){
    while((n=read(_client.socket,recvBuff,sizeof(recvBuff)-1))>0){
      pthread_mutex_lock(&lock);
      if(n){
	if(strstr(recvBuff,"PUB")!=NULL){
	  publish_handler(_client,recvBuff);
	}
	if(strstr(recvBuff,"DISC")!=NULL){
	  loop_break = disconnect_handler(_client);
	}
	if(strstr(recvBuff,"SUB")!=NULL){
	  subscription_handler(_client,recvBuff);
	}
      }
      pthread_mutex_unlock(&lock);
      memset(recvBuff, '\0',strlen(recvBuff));
    }
  }
  close(_client.socket);
  return NULL;
}

int main(int argc, char *argv[]){
  int listenfd,connfd = 0;

  output_log.open("connections_log.txt");

  listenfd = socket_helper->set_up_socket();
  
  client clients[100];
  char sendBuff[1025];
  memset(sendBuff, '0', sizeof(sendBuff)); 

  while(1){
    clients[connfd].socket = accept(listenfd, (struct sockaddr*)NULL, NULL); 

    char recvBuff[100];
    memset(recvBuff, '0',strlen(recvBuff));
    int n;
      
    while((n = read(clients[connfd].socket, recvBuff, sizeof(recvBuff)-1)) > 0){	  
      if(strstr(recvBuff,"CONN")!=NULL){
	output_log << "Client" << clients[connfd].socket << ": <CONN>\nServer: <CONN_ACK>\n\n";
	output_log.flush();
	strcpy(sendBuff,"CONN_ACK");
	write(clients[connfd].socket, sendBuff, strlen(sendBuff)); 
	memset(sendBuff, '0', sizeof(sendBuff));
	pthread_create(&clients[connfd].tid,NULL,thread_function,(void *)&clients[connfd]);
	connfd++;
	connfd%=100;
	break;
      }

      fputs(recvBuff,stdout);
    }
  }

  output_log.close();
}
