#include "socket.h"

std::ofstream output_log;

std::vector<int> weather;
std::vector<int> news;
std::vector<int> health;
std::vector<int> security;

Socket *socket_helper = new Socket();

void subscription_handler(int client,char* recvBuff){
  char *topic,*rest;
  rest = recvBuff;
  int iteration = 0;
  std::vector<int> *temp;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      output_log << "Client"<< client << ": <SUB,TOPIC>\nTopic: " << topic << "\n\n";
      output_log.flush();

      if(strstr(topic,"weather") != NULL)
	temp = &weather;
      else if(strstr(topic,"news") != NULL)
	temp = &news;
      else if(strstr(topic,"health") != NULL)
	temp = &health;
      else if(strstr(topic,"security") != NULL)
	temp = &security;
      temp->push_back(client);
    }  
    iteration++;
  }
}

int disconnect_handler(int client){
  char recvBuff[1000];
  memset(recvBuff, '0',strlen(recvBuff));
  memset(recvBuff, '0',strlen(recvBuff));
  strcpy(recvBuff,"DISC_ACK");
  write(client,recvBuff,strlen(recvBuff));
  output_log << "Client" << client << ": <DISC>\nServer: <DISC_ACK>\n\n";
  output_log.flush();
  close(client);
  return 0;
}

void publish_handler(int client,char* recvBuff){
  char *topic,*rest,commandBuff[1200];
  memset(commandBuff,'\0',sizeof(commandBuff));
  rest = recvBuff;
  int iteration = 0;
  std::vector<int> *temp;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      output_log << "Client" << client << ":<PUB,TOPIC,MSG>\nTopic: " << topic << "\n\tmessage: ";
      //output_log.flush();

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
      
    }
    else if(iteration == 2){
      output_log << topic << "\n\n";
      output_log.flush();
      strcpy(commandBuff,"Message received: ");
      strcat(commandBuff,topic);
      for(int i=0;i<(int)temp->size();i++){
	int n;
	if((n = write(temp->at(i),commandBuff,sizeof(commandBuff))) < 0)
	  output_log << "Write Failure\n";
	output_log<<"Writing to Client" << temp->at(i) << " with message " << topic << "\n\n";
	output_log.flush();
      }
    }
    iteration++;  
  }
}

int main(int argc, char *argv[]){
  int master_socket,max_sd,sd,max_clients = 100,activity,new_socket,check,port=8080,i,client_socket[100],addrlen;
  
  fd_set readfds;

  output_log.open("connections_log.txt");

  struct sockaddr_in serv_addr; 

  master_socket = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port); 

  socket_helper->enable_keepalive(master_socket);
    
  if((check = bind(master_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    std::cout << "Bind error\n";

  listen(master_socket, 10);
  
  char sendBuff[1025];
  memset(sendBuff, '0', sizeof(sendBuff)); 
  char recvBuff[100];
  memset(recvBuff, '0',sizeof(recvBuff));
  
  for(int i=0;i<max_clients;i++)
    client_socket[i] = 0;

  output_log << "Master socket: " << master_socket << "\n\n";
  output_log.flush();
  
  while(1){
    FD_ZERO(&readfds);

    FD_SET(master_socket,&readfds);
    max_sd = master_socket;

    for(i=0;i<max_clients;i++){
      sd = client_socket[i];

      if(sd > 0)
	FD_SET(sd,&readfds);

      if(sd > max_sd)
	max_sd = sd;
    }

    activity = select(max_sd+1,&readfds,NULL,NULL,NULL);
    addrlen = sizeof(serv_addr);
    
    if(FD_ISSET(master_socket,&readfds)){
      if((new_socket = accept(master_socket, (struct sockaddr*)&serv_addr, ((socklen_t *)&addrlen)))<0)
	std::cout << "Acceptance error\n";

      memset(recvBuff, '0',strlen(recvBuff));
      int n,successful_connection=0;
      while((n = read(new_socket, recvBuff, sizeof(recvBuff)-1)) > 0){	  
	if(strstr(recvBuff,"CONN")!=NULL){
	  output_log << "Client" << new_socket << ": <CONN>\nServer: <CONN_ACK>\n\n";
	  output_log.flush();
	  strcpy(sendBuff,"CONN_ACK");
	  write(new_socket, sendBuff, strlen(sendBuff)); 
	  memset(sendBuff, '0', sizeof(sendBuff));
	  successful_connection = 1;
	  break;
	}
	fputs(recvBuff,stdout);
      }
      if(successful_connection){
	for(i=0;i<max_clients;i++){
	  if(client_socket[i] == 0){
	    client_socket[i] = new_socket;
	    break;
	  }
	}
      }

      if(i == max_clients){
	output_log << "Could not add new Client" << new_socket << "\n\n";
	output_log.flush();
      }

    }

    for(i=0;i<max_clients;i++){
      sd = client_socket[i];

      if(FD_ISSET(sd,&readfds)){
	memset(recvBuff, '\0',strlen(recvBuff));
	int n;
	while((n=read(sd,recvBuff,sizeof(recvBuff)))>0){	  
	  if(strstr(recvBuff,"PUB")!=NULL){
	    publish_handler(client_socket[i],recvBuff);
	  }
	  else if(strstr(recvBuff,"DISC")!=NULL){
	    disconnect_handler(client_socket[i]);
	  }
	  else if(strstr(recvBuff,"SUB")!=NULL){
	    subscription_handler(client_socket[i],recvBuff);
	  }
	  memset(recvBuff, '\0',strlen(recvBuff));
	}
      }
    }
  
  }
  output_log.close();
 
  
}
