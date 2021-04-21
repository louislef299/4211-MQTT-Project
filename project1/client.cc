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

using namespace std;

enum command{
  connection,
  disconnect,
  publish,
  subscribe,
  quit,
  none
};

command hashit(string& command){
  for(int i=0;i<command.length();i++)
    command.at(i) = toupper(command.at(i));

  if("CONNECT" == command)
    return connection;
  if("DISCONNECT" == command)
    return disconnect;
  if("PUBLISH" == command)
    return publish;
  if("SUBSCRIBE" == command)
    return subscribe;
  if("QUIT" == command)
    return quit;
  return none;
}

int make_connection(){
  int sockfd = 0;
  struct sockaddr_in serv_addr; 

  char recvBuff[1024];
  memset(recvBuff, '0',std::strlen(recvBuff));
  
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

int pub(int sockfd){
  char commandBuff[200];
  memset(commandBuff, '0',std::strlen(commandBuff));
  
  string topicNumber;
  char topic[10];
  while(1){
    std::cout << "Please choose a topic(type a number):\n[1] Weather\n[2] News\n[3] Health\n[4] Security\n";
    
    cin >> topicNumber;
    int topicNum = stoi(topicNumber);
    if(topicNum == 1){
      strcpy(topic,"weather");
      break;
    }
    else if(topicNum == 2){
      strcpy(topic,"news");
      break;
    }
    else if(topicNum == 3){
      strcpy(topic,"health");
      break;
    }
    else if(topicNum == 4){
      strcpy(topic,"security");
      break;
    }
    else
      std::cout << "Invalid response, please try again\n";
  }

  string msg;
  std::cout << "What message would you like to publish?\nPublication(max 100 characters): ";
  std::getline(std::cin,msg);
  strcpy(commandBuff,"PUB,");
  strcat(commandBuff,topic);
  strcat(commandBuff,",");
  //need to change message to a char*
  strcat(commandBuff,msg.c_str());
  write(sockfd,commandBuff,strlen(commandBuff));
  printf("Your sent package: %s\n",commandBuff);
  
  return 0;

}

int main(int argc, char *argv[]){
  std::cout << "Welcome to the Project 1 Publishing service!\n";

  int cont = 1;
  string command;
  int sockfd = -1;
  
  while(cont){
    std::cout << "Please write a command: ";
    cin >> command;
    
    switch(hashit(command)){
    case connection:
      if(sockfd != -1)
	std::cout << "Already connected!\n";
      else{
	sockfd = make_connection();
	std::cout << "Successful Connection\n";
      }
      break;
    case disconnect:
      break;
    case publish:
      pub(sockfd);
      break;
    case subscribe:
      break;
    case quit:
      cont = 0;
      break;
    default:
      std::cout << "Invalid command, please try again:\n";
      break;	
    }
  }
  
  return 0;
}
