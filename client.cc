#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

enum command{CONNECTION,DISCONNECT,PUBLISH,SUBSCRIBE,QUIT,NONE};

command hashit(string& command){
  for(int i=0;i<(int)command.length();i++)
    command.at(i) = toupper(command.at(i));

  if("CONNECT" == command)
    return CONNECTION;
  if("DISCONNECT" == command)
    return DISCONNECT;
  if("PUBLISH" == command)
    return PUBLISH;
  if("SUBSCRIBE" == command)
    return SUBSCRIBE;
  if("QUIT" == command)
    return QUIT;
  return NONE;
}

int make_connection(){
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
  serv_addr.sin_port = htons(5000); 

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

int publish_mqtt(int sockfd){
  char commandBuff[200];
  memset(commandBuff, '0',strlen(commandBuff));
  
  string topicNumber;
  char topic[10];
  int loop = 1;
  int topicNum = 0;
  while(loop){
    std::cout << "Please choose a topic(type a number):\n[1] Weather\n[2] News\n[3] Health\n[4] Security\n";
    
    scanf("%d",&topicNum);
    std::cout << "echo: " << topicNumber << '\n';
    if(topicNum == 1){
      strcpy(topic,"weather");
      loop=0;
    }
    else if(topicNum == 2){
      strcpy(topic,"news");
      loop=0;
    }
    else if(topicNum == 3){
      strcpy(topic,"health");
      loop=0;
    }
    else if(topicNum == 4){
      strcpy(topic,"security");
      loop=0;
    }
    else{
      std::cout << "Invalid response, please try again\n";
    }
  }

  char msg[1200],buffer[50];
  std::cout << "What message would you like to publish?\nPublication(max 100 characters): ";
  while(1){
    scanf("%s",buffer);
    if(strcmp(buffer,"exit")==0)
      break;
    strcat(msg,buffer);
    strcat(msg," ");
  }
  
  strcpy(commandBuff,"PUB,");
  strcat(commandBuff,topic);
  strcat(commandBuff,",");
  //need to change message to a char*
  strcat(commandBuff,msg);
  write(sockfd,commandBuff,strlen(commandBuff));
  memset(msg, '\0',strlen(msg));
  printf("Your sent package: %s\n",commandBuff);
  
  return 0;
}

/****************************************************** 
    MQTT Project
 ******************************************************/
int main(int argc, char *argv[]){
  std::cout << "Welcome to the Project 1 Publishing service!\n";

  int cont = 1;
  string command;
  int sockfd = -1;
  
  while(cont){
    std::cout << "Please write a command: ";
    cin >> command;
    
    switch(hashit(command)){
    case CONNECTION:
      if(sockfd != -1)
	std::cout << "Already connected!\n";
      else{
	sockfd = make_connection();
	std::cout << "Successful Connection\n";
      }
      break;
    case DISCONNECT:
      break;
    case PUBLISH:
      publish_mqtt(sockfd);
      break;
    case SUBSCRIBE:
      break;
    case QUIT:
      cont = 0;
      break;
    default:
      std::cout << "Invalid command, please try again:\n";
      break;	
    }
  }
  
  return 0;
}
