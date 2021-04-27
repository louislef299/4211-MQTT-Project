#include "socket.h"

using namespace std;

std::ofstream output_log;

enum command{CONNECTION,DISCONNECT,PUBLISH,SUBSCRIBE,QUIT,NONE};

Socket *socket_helper = new Socket();

command hashit(string& command){
  for(int i=0;i<(int)command.length()-1;i++)
    command.at(i) = toupper(command.at(i));

  if("CONNECT\n" == command)
    return CONNECTION;
  if("DISCONNECT\n" == command)
    return DISCONNECT;
  if("PUBLISH\n" == command)
    return PUBLISH;
  if("SUBSCRIBE\n" == command)
    return SUBSCRIBE;
  if("QUIT\n" == command)
    return QUIT;
  return NONE;
}

void publish_mqtt(int sockfd,bool just_topic=false){
  char topic[10];
  int loop = 1;
  int topicNum;
  while(loop){
    std::cout << "Please choose a topic(type a number):\n[1] Weather\n[2] News\n[3] Health\n[4] Security\n";

    int e;
    while((e = scanf("%d",&topicNum)) < 1){
      std::cout << "Invalid response, please try again\n";
      scanf("%*s");
    }
    
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
      scanf("%*s");
    }
  }

  if(!just_topic){   
    char msg[1200],buffer[50],commandBuff[2000];
    memset(commandBuff, '\0',strlen(commandBuff));  
    std::cout << "What message would you like to publish(max 1000 characters)?\nPublication(Type 'exit' to finish publication): ";
    int size_check = 0;
    memset(msg, '\0',strlen(msg));
    while(1){
      scanf("%s",buffer);
      if(buffer[strlen(buffer)] == '\n' || strcmp("exit",buffer) == 0)
	break;
      size_check += strlen(buffer)+1;
      if(size_check > 1000){
	std::cout << "Publication is too large!\n";
	break;
      }
      strcat(msg,buffer);
      strcat(msg," ");
    }
  
    strcpy(commandBuff,"PUB,");
    strcat(commandBuff,topic);
    strcat(commandBuff,",");
    //need to change message to a char*
    strcat(commandBuff,msg);
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";
    memset(msg, '\0',strlen(msg));
    printf("Your sent package: %s\n",commandBuff);

  }else{
    char commandBuff[50];
    memset(commandBuff, '\0',strlen(commandBuff));  
    strcpy(commandBuff,"SUB,");
    strcat(commandBuff,topic);
    
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";

    printf("Your sent package: %s\n",commandBuff);
  
    return;

  }
  return;
}

void disconnect_mqtt(int sockfd){
  char commandBuff[200];
  memset(commandBuff, '0',strlen(commandBuff));
  strcpy(commandBuff,"DISC");

  while(write(sockfd,commandBuff,strlen(commandBuff)) < 0)
    std::cout << "Write failure, trying again...\n";

  memset(commandBuff, '0',strlen(commandBuff));
  
  int n;
  while((n = read(sockfd, commandBuff, sizeof(commandBuff)-1)) > 0){
    if(strstr(commandBuff,"DISC_ACK")!=NULL)
      break;
  }

  output_log << "Successful Disconnect on socket " << sockfd << "\n";
  output_log.flush();
  
  close(sockfd);
  if(n < 0){
    printf("\n Read error \n");
  }
}

/****************************************************** 
    MQTT Project
 ******************************************************/
int main(int argc, char *argv[]){
  std::cout << "Welcome to the Project 1 Publishing service!\n";
  
  int cont = 1;
  int sockfd,timeout,nfds = 1,on = 1,rc;
  char commandBuff[200];
  struct pollfd fds;
  
  sockfd = -1;
  while(cont){    
    std::cout << "Please write a command: ";
    fgets(commandBuff, 100, stdin);
    string command(commandBuff);
    
    switch(hashit(command)){
    case CONNECTION:
      if(sockfd != -1)
	std::cout << "Already connected!\n";
      else{
	sockfd = socket_helper->make_connection(8080);
	memset(commandBuff, '\0',strlen(commandBuff));
	sprintf(commandBuff,"client_connections_log%d.txt",sockfd);
	output_log.open(commandBuff);
	if(sockfd == -1)
	  std::cout << "Unsuccessful Connection\n";
	else{
	  std::cout << "Successful Connection on sockect " << sockfd << "\n";
	  output_log << "Successful Connection on sockect " << sockfd << "\n";
	  output_log.flush();
	  memset(&fds,'0',sizeof(fds));
	  fds.fd = sockfd;
	  fds.events = POLLIN;
	  timeout = 100;
	  rc = ioctl(sockfd, FIONBIO, (char *)&on);
	}
      }
      break;
    case DISCONNECT:
      if(sockfd != -1){
	disconnect_mqtt(sockfd);
	std::cout << "Successful Disconnection, all channels successfully unsubscribed\n";
      }else{
	std::cout << "No prior connection was made\n";
      }
      sockfd = -1;
      break;
    case PUBLISH:
      publish_mqtt(sockfd);
      break;
    case SUBSCRIBE:
      publish_mqtt(sockfd,true);
      break;
    case QUIT:
      if(sockfd != -1)
	disconnect_mqtt(sockfd);
      cont = 0;
      output_log.close();
      sockfd = -1;
      break;
    default:
      std::cout << "Invalid command, please try again\n";
      break;	
    }

    int n;
    char recvBuff[200];
    memset(recvBuff, '\0',strlen(recvBuff));
    if(((rc = poll(&fds,nfds,timeout)) > 0) && sockfd != -1){
      while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
	if(strstr(recvBuff,"Message") != NULL){
	  std::cout << recvBuff;
	  std::cout << "\n";
	  break;
	}
      }
    }
    
  }
  
  return 0;
}
