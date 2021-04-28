#include "socket.h"

using namespace std;

std::ofstream output_log;

enum command{CONNECTION,DISCONNECT,PUBLISH,SUBSCRIBE,QUIT,NONE,UNSUBSCRIBE,LIST};

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
  if("UNSUBSCRIBE\n" == command)
    return UNSUBSCRIBE;
  if("LIST\n" == command)
    return LIST;
  if("QUIT\n" == command)
    return QUIT;
  return NONE;
}

void publish_mqtt(int sockfd,bool just_topic=false,bool unsub=false){
  char topic[50],msg[1200],buffer[50],commandBuff[2000];
  int retain;
  
  std::cout << "Please type your desired topic: ";
  memset(topic,'\0',strlen(topic));
  fgets(topic, 50, stdin);
  for(int i=0;i<(int)strlen(topic);i++)
    if(topic[i] == '\n')
      topic[i] = '\0';
  
  std::cout << "\n";
  if(!just_topic){   
    memset(commandBuff, '\0',strlen(commandBuff));  
    std::cout << "Would you like this message to be retained?(yes or no): ";
    fgets(buffer, 50, stdin);
    if(strstr(buffer,"y") != NULL)
      retain = 1;
    else
      retain = 0;
    memset(buffer,'\0',strlen(buffer));
    std::cout << "\n";
    std::cout << "What message would you like to publish(max 1000 characters)?\nPublication: ";

    memset(msg, '\0',strlen(msg));
    fgets(msg,1200,stdin);
    for(int i=0;i<(int)strlen(msg);i++)
      if(msg[i] == '\n')
        msg[i] = '\0';
  
    std::cout << "\n";

    strcpy(commandBuff,"PUB,");
    strcat(commandBuff,topic);
    strcat(commandBuff,",");
    if(retain)
      strcat(commandBuff,"RETAIN,");
    else
      strcat(commandBuff,"NORET,");
    //need to change message to a char*
    strcat(commandBuff,msg);
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";
    memset(msg, '\0',strlen(msg));
    printf("Your sent package: %s\n",commandBuff);

  }else if(!unsub){
    memset(commandBuff, '\0',strlen(commandBuff));  
    strcpy(commandBuff,"SUB,");
    strcat(commandBuff,topic);
    
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";

    printf("Your sent package: %s\n",commandBuff);
    return;
  }else{
    memset(commandBuff, '\0',strlen(commandBuff));  
    strcpy(commandBuff,"UNSUB,");
    strcat(commandBuff,topic);
    
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";

    printf("Your sent package: %s\n",commandBuff);  
    return;
  }
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
	while(!output_log.is_open())
	  output_log.open(commandBuff);
	if(sockfd == -1)
	  std::cout << "Unsuccessful Connection\n";
	else{
	  std::cout << "Successful Connection on sockect " << sockfd << "\n";
	  memset(&fds,'0',sizeof(fds));
	  fds.fd = sockfd;
	  fds.events = POLLIN;
	  timeout = 100;
	  rc = ioctl(sockfd, FIONBIO, (char *)&on);
	  output_log << "Successful Connection on sockect " << sockfd << "\n";
	  output_log.flush();
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
    case UNSUBSCRIBE:
      publish_mqtt(sockfd,true,true);
      break;
    case LIST:
      char commandBuff[12];
      memset(commandBuff, '\0',strlen(commandBuff));  
      strcpy(commandBuff,"LIST");
      while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
	std::cout << "Write failure, trying again...\n";

      printf("Your sent package: %s\n",commandBuff);
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
	  output_log << "received message :" << recvBuff << "\n\n";
	  output_log.flush();
	  break;
	  
	}
	if(strstr(commandBuff,"SUCCESS")!=NULL){
	  output_log << "Successful subscription\n\n";
	  output_log.flush();
	  break;
	}
	if(strstr(commandBuff,"ERROR")!=NULL){
	  output_log << "Error with subscription\n\n";
	  output_log.flush();
	  break;
	}
      }
    }
    
  }
  
  return 0;
}
