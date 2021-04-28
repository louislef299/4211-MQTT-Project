/**
 * @author Louis Lefebvre
 */

#include "socket.h"

using namespace std;

std::ofstream output_log;

enum command{CONNECTION,DISCONNECT,PUBLISH,SUBSCRIBE,QUIT,NONE,UNSUBSCRIBE,LIST,HELP,ECHO};

/**
 * The helper is used to declutter the client.cc file
 * Functions that can be found in the socket class:
 *  enable_keepalive()
 *  make_connection()
 *  canReadFromPipe()
 * The last function is never used in this program
 */
Socket *socket_helper = new Socket();

int sockfd,echo;

/**
 * Used to simplify switch statement in main
 */
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
  if("HELP\n" == command)
    return HELP;
  if("ECHO\n" == command)
    return ECHO;
  return NONE;
}

/**
 * publish_mqtt is used for the publish command, subscribe command,
 * and unsubscribe command. If publish is being attempted, sockfd
 * is the only required variable. If subscribe, just_topic must
 * be set to true, and if unsubscribe, just_topic AND unsub must
 * be set to true
 */
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

    if(echo)
      printf("Your sent package: %s\n",commandBuff);

  }else if(!unsub){
    memset(commandBuff, '\0',strlen(commandBuff));  
    strcpy(commandBuff,"SUB,");
    strcat(commandBuff,topic);
    
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";

    if(echo)
      printf("Your sent package: %s\n",commandBuff);
    return;
  }else{
    memset(commandBuff, '\0',strlen(commandBuff));  
    strcpy(commandBuff,"UNSUB,");
    strcat(commandBuff,topic);
    
    while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";

    if(echo)
      printf("Your sent package: %s\n",commandBuff);  
    return;
  }
}

/**
 * disconnect_mqtt is for safe disconnection from the server and closing
 * of the used port
 */
void disconnect_mqtt(int sockfd){
  char commandBuff[200];
  memset(commandBuff, '0',strlen(commandBuff));
  strcpy(commandBuff,"DISC");

  while(write(sockfd,commandBuff,strlen(commandBuff)) < 0)
    std::cout << "Write failure, trying again...\n";

  if(echo)
    printf("Your sent package: %s\n",commandBuff);
  
  memset(commandBuff, '0',strlen(commandBuff));
  
  int n;
  while((n = read(sockfd, commandBuff, sizeof(commandBuff)-1)) > 0){
    if(strstr(commandBuff,"DISC_ACK")!=NULL)
      break;
  }

  output_log << "Successful Disconnect on socket " << sockfd << "\n";
  output_log.flush();
  
  close(sockfd);
  
}

/**
 * Used for safe exit when SIGINT is sent
 */
void sighandler(int signum){
  if(sockfd != -1){
    std::cout << "\nSafely disconnecting...\n";
    disconnect_mqtt(sockfd);
  }
  exit(signum);
}

/****************************************************** 
    MQTT Project
 ******************************************************/
int main(int argc, char *argv[]){
  std::cout << "Welcome to the Project 1 Publishing service! Type 'help' for commands\n";

  signal(SIGINT,sighandler);
  
  int cont = 1;
  int timeout,nfds = 1,on = 1,rc;
  char commandBuff[200];
  struct pollfd fds;
  
  sockfd = -1;
  echo = 0;
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
      if(sockfd == -1){
	std::cout << "Please connect to the server first\n";
	break;
      }
      publish_mqtt(sockfd);
      break;
    case SUBSCRIBE:
      if(sockfd == -1){
	std::cout << "Please connect to the server first\n";
	break;
      }
      publish_mqtt(sockfd,true);
      break;
    case UNSUBSCRIBE:
      if(sockfd == -1){
	std::cout << "Please connect to the server first\n";
	break;
      }
      publish_mqtt(sockfd,true,true);
      break;
    case LIST:
      if(sockfd == -1){
	std::cout << "Please connect to the server first\n";
	break;
      }
      char commandBuff[12];
      memset(commandBuff, '\0',strlen(commandBuff));  
      strcpy(commandBuff,"LIST");
      while(write(sockfd,commandBuff,sizeof(commandBuff)) < 0)
	std::cout << "Write failure, trying again...\n";

      printf("Your sent package: %s\n",commandBuff);
      break;
    case ECHO:
      if(echo)
	echo = 0;
      else{
	echo = 1;
	std::cout << "echo\n";
      }
      break;
    case QUIT:
      if(sockfd != -1)
	disconnect_mqtt(sockfd);
      cont = 0;
      output_log.close();
      sockfd = -1;
      std::cout << "Have a great day! :)\n";
      break;
    case HELP:
      std::cout << "Commands:\n <Connect> to connect to the server\n <Disconnect> to disconnect with the server\n <Subscribe> to subscribe to content on the server\n <Unsubscribe> to unsubscribe from content on the server\n <Publish> to publish content on the server\n <List> to list all the topics on the server\n <Echo> to echo the packages sent to the server\n <Quit> to end the session\n";
      break;
    default:
      std::cout << "Invalid command, please try again\n";
      break;	
    }

    /* At the end of every command call, we check the port to see if there
     * are any new messages. If they are a message, 'Message' will be in
     * the package sent from the server, and the whole message will be
     * displayed to the screen
     */
    if(sockfd != -1){
      int n;
      char recvBuff[200];
      memset(recvBuff, '\0',strlen(recvBuff));
      if(((rc = poll(&fds,nfds,timeout)) > 0) && sockfd != -1){
	while((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
	  if(strstr(recvBuff,"Message") != NULL){
	    std::cout << recvBuff;
	    std::cout << "\n";
	    output_log << recvBuff << "\n\n";
	    output_log.flush();
	    if(!((rc = poll(&fds,nfds,timeout)) > 0))
	      break;
	  
	  }
	  if(strstr(commandBuff,"SUCCESS")!=NULL){
	    output_log << "Successful (un)subscription\n\n";
	    output_log.flush();
	    break;
	  }
	  if(strstr(commandBuff,"ERROR")!=NULL){
	    output_log << "Error with (un)subscription\n\n";
	    output_log.flush();
	    break;
	  }
	}
      }
    }
    
  }
  
  return 0;
}
