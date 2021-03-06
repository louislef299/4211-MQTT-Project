/**
 * @author Louis Lefebvre
 * LOOK AT README.md FOR MORE INFO
 */

#include "socket.h"

/**
 * All interactions are recorded and saved into
 * the connections_log.txt
 */
std::ofstream output_log;

/**
 * topic entries require a name to function
 * correctly. They can also save the client port 
 * numbers as well as retained messages
 */
struct topic_entry{
  std::string name;
  std::vector<int> clients;
  std::vector<std::string> msgs;
};

// saves all topics
std::vector<topic_entry*> topics;

// similar to helper in client.cc
Socket *socket_helper = new Socket();

/**
 * Sends ERROR to client
 */ 
void send_error(int client){
  char recvBuff[20];
  memset(recvBuff, '0',strlen(recvBuff));
  strcpy(recvBuff,"ERROR");
  write(client,recvBuff,strlen(recvBuff));
  output_log << "Server: <ERROR>\n\n";
  output_log.flush();
}

/**
 * Sends SUCCESS to client
 */
void send_success(int client){
  char recvBuff[20];
  memset(recvBuff, '0',strlen(recvBuff));
  strcpy(recvBuff,"SUCCESS");
  write(client,recvBuff,strlen(recvBuff));
  output_log << "Server: <SUCCESS>\n\n";
  output_log.flush();
}

/**
 * Handles the '#' in the subscription service
 */
void multi_level_wildcard_handler(int client,std::string topic){
  size_t placement = topic.rfind("#");
  topic.erase(topic.begin()+placement);
  int i;
  int found = 0;
  char commandBuff[1200];
  memset(commandBuff,'\0',sizeof(commandBuff));
  strcat(commandBuff,"Messages:\n");
	
  for(i=0;i<(int)topics.size();i++){
    if((int)topics.at(i)->name.find(topic) != -1){
      topics.at(i)->clients.push_back(client);
      if(!topics.at(i)->msgs.empty()){
	for(int j=0;j<(int)topics.at(i)->msgs.size();j++){
	  strcat(commandBuff,topics.at(i)->msgs.at(j).c_str());
	  strcat(commandBuff,"\n");
	}
	found = 1;
	output_log << "Wrote " << commandBuff << " to Client" << client << "\n\n" ;
	output_log.flush();
      }
    }
  }
  if(found)
    write(client,commandBuff,sizeof(commandBuff));	
  send_success(client);
}

/**
 * Handles the '+' wildcard in the subscription service
 */
void single_level_wildcard_handler(int client,std::string topic){
  size_t placement = topic.find("+");
  int i;
  std::string tempLeft,tempRight,legitLeft,legitRight;
  legitLeft = topic.substr(0,placement);
  legitRight = topic.substr(placement+1);
  int found = 0;
  char commandBuff[1200];
  memset(commandBuff,'\0',sizeof(commandBuff));
  strcat(commandBuff,"Messages:\n");

  output_log << legitLeft << " /+/ " << legitRight << "\n";
  output_log.flush();
  
  for(i=0;i<(int)topics.size();i++){
    if((int)topics.at(i)->name.find(legitLeft) != -1 && (int)topics.at(i)->name.find(legitRight) != -1 ){
      std::string takeover = topics.at(i)->name;
      placement = takeover.find("/");
      tempLeft = takeover.substr(0,placement+1);
      takeover.erase(0,placement+1);
      placement = takeover.find("/");
      tempRight = takeover.substr(placement);
      if(tempLeft == legitLeft && tempRight == legitRight){
	topics.at(i)->clients.push_back(client);
	if(!topics.at(i)->msgs.empty()){
	  for(int j=0;j<(int)topics.at(i)->msgs.size();j++){
	    strcat(commandBuff,topics.at(i)->msgs.at(j).c_str());
	    strcat(commandBuff,"\n");
	  }
	  found = 1;
	  output_log << "Wrote " << commandBuff << " to Client" << client << "\n\n" ;
	  output_log.flush();
	}
      }
    }
  }
  if(found)
    write(client,commandBuff,sizeof(commandBuff));
  send_success(client);
}

/**
 * The subscription handler is in charge of subscribing to
 * a topic of the client's choosing. If the topic does not
 * already exist, a topic of that name is created and the
 * client is subscribed to it. Can handle the wildcards
 */
void subscription_handler(int client,char* recvBuff){
  char *topic,*rest;
  rest = recvBuff;
  int iteration = 0,i,sendSucc=0;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      std::string tnode_topic(topic);
      output_log << "Client"<< client << ": <SUB,TOPIC>\nTopic: " << tnode_topic << "\n\n";
      output_log.flush();
      if((int)tnode_topic.find("#") != -1){
	multi_level_wildcard_handler(client,tnode_topic);
	return;
      }
      else if((int)tnode_topic.find("+") != -1){
	single_level_wildcard_handler(client,tnode_topic);
	return;
      }
      else{
	for(i=0;i<(int)topics.size();i++){
	  if(topics.at(i)->name == tnode_topic){
	    topics.at(i)->clients.push_back(client);
	    sendSucc = 1;
	    if(!topics.at(i)->msgs.empty()){
	      char commandBuff[1200];
	      memset(commandBuff,'\0',sizeof(commandBuff));
	      strcat(commandBuff,"Messages:\n");
	      for(int j=0;j<(int)topics.at(i)->msgs.size();j++)
		strcat(commandBuff,topics.at(i)->msgs.at(j).c_str());
	      write(client,commandBuff,sizeof(commandBuff));
	      output_log << "Wrote " << commandBuff << " to Client" << client << "\n\n" ;
	      output_log.flush();
	    }
	    break;
	  }
	}
      }
      if(i == (int)topics.size()){
	struct topic_entry *temp_topic = (topic_entry*)malloc(sizeof(topic_entry));
	temp_topic->name = tnode_topic;
	temp_topic->clients.push_back(client);
	topics.push_back(temp_topic);
      }
    }  
    iteration++;
  }
  if(sendSucc){
    send_success(client);
    return;
  } 
  send_error(client);
  
}

/**
 * The unsubscribed handler will unsubscribe a client from
 * a certain topic and respond with either SUCCESS or ERROR
 */
void unsubscribe_handler(int client,char* recvBuff){
  char *topic,*rest;
  rest = recvBuff;
  int iteration = 0,i,check;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      output_log << "Client"<< client << ": <UNSUB,TOPIC>\nTopic: " << topic << "\n\n";
      output_log.flush();
      std::string tnode_topic(topic);
      for(i=0;i<(int)topics.size();i++){
	if(topics.at(i)->name == tnode_topic){
	  for(int j=0;j<(int)topics.at(i)->clients.size();j++){
	    if(topics.at(i)->clients.at(j) == client){
	      topics.at(i)->clients.at(j) = -1;
	      check = 0;
	      break;
	    }
	  }
	}
      }
    }  
    iteration++;
  }

  if(check == 0)
    send_success(client);
  else
    send_error(client);
  
}

/**
 * The disconnect handler is very straight forward and simply
 * responds to a DISK with DISK_ACK
 */
int disconnect_handler(int client){
  char recvBuff[20];
  memset(recvBuff, '0',strlen(recvBuff));
  strcpy(recvBuff,"DISC_ACK");
  write(client,recvBuff,strlen(recvBuff));
  output_log << "Client" << client << ": <DISC>\nServer: <DISC_ACK>\n\n";
  output_log.flush();
  close(client);
  return 0;
}

/**
 * Handles the publish command by either adding a new message
 * to an already existing topic or creating a new topic with
 * a new message. The retain flag is functional and will save
 * a retained message to a topic. Either way, the message is 
 * sent to all clients that are subscribed to the server
 */
void publish_handler(int client,char* recvBuff){
  char *topic,*rest,commandBuff[1200];
  memset(commandBuff,'\0',sizeof(commandBuff));
  rest = recvBuff;
  int iteration = 0,retain,i;
  std::string tnode_topic;
  while((topic = strtok_r(rest,",",&rest))){
    if(iteration == 1){
      output_log << "Client" << client << ":<PUB,TOPIC,RETAIN,MSG>\nTopic: " << topic; 
      std::string tnode_temp(topic);
      tnode_topic = tnode_temp;
    }
    else if(iteration == 2){
      output_log << "\nmessage will ";
      if(strstr(topic,"RETAIN") != NULL){
	output_log << "be retained";
	retain = 1;
      }else{
	output_log << "not be retained";
	retain = 0;
      }
      output_log << "\n\tmessage: ";
      output_log.flush();
    }
    else if(iteration == 3){
      output_log << topic << "\n\n";
      output_log.flush();
      strcpy(commandBuff,"Message received: ");
      strcat(commandBuff,topic);
      
      for(i=0;i<(int)topics.size();i++){
	if(topics.at(i)->name == tnode_topic){
	  if(retain)
	    topics.at(i)->msgs.push_back(topic);
	  for(int j=0;j<(int)topics.at(i)->clients.size();j++)
	    if(topics.at(i)->clients.at(j) != -1){
	      write(topics.at(i)->clients.at(j),commandBuff,strlen(commandBuff));
	      output_log << "Wrote " << commandBuff << " to Client" << topics.at(i)->clients.at(j) << "\n\n" ;
	      output_log.flush();
	    }
	  
	  break;
	}
      }
      if(i == (int)topics.size()){
	struct topic_entry *temp_topic = (topic_entry*)malloc(sizeof(topic_entry));
	temp_topic->name = tnode_topic;
	if(retain)
	    temp_topic->msgs.push_back(topic);
	topics.push_back(temp_topic);
      }
    }
    iteration++;  
  }
}

/**
 * Handles the list command by sending all topics
 * to the client
 */
void list_handler(int client){
  char commandBuff[1200];
  memset(commandBuff,'\0',sizeof(commandBuff));

  sprintf(commandBuff,"%ld",topics.size());
  strcat(commandBuff,",Message");
  for(int i=0;i<(int)topics.size();i++){
    strcat(commandBuff,",");
    strcat(commandBuff,topics.at(i)->name.c_str());
  }
  write(client,commandBuff,strlen(commandBuff));
  output_log << "Wrote " << commandBuff << " to Client" << client << "\n\n";
  output_log.flush();
}

/**
 * The main function in server.cc initializes quite a lot in the beginning.
 * Glossing over details, I implemented the select() function in order to
 * save all clients accessing the server. Currently, this program only works
 * on local networks (127.0.0.1)
 */
int main(int argc, char *argv[]){
  int master_socket,max_sd,sd,max_clients = 100,new_socket,check,port=8080,i,client_socket[100],addrlen;
  
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

    select(max_sd+1,&readfds,NULL,NULL,NULL);
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
	  else if(strstr(recvBuff,"UNSUB")!=NULL){
	    unsubscribe_handler(client_socket[i],recvBuff);
	  }
	  else if(strstr(recvBuff,"SUB")!=NULL){
	    subscription_handler(client_socket[i],recvBuff);
	  }
	  else if(strstr(recvBuff,"LIST")!=NULL){
	    list_handler(client_socket[i]);
	  }
	  memset(recvBuff, '\0',strlen(recvBuff));
	}
      }
    }
  }
  output_log.close();
}
