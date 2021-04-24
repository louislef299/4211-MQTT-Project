#include "socket.h"

using namespace std;

std::ofstream output_log;

enum command{CONNECTION,DISCONNECT,PUBLISH,SUBSCRIBE,QUIT,NONE};

pthread_mutex_t mutex_lock;

Socket *socket_helper = new Socket();

int custom_pipe[2];

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

bool canReadFromPipe(){
  struct pollfd fds;
  fds.fd = custom_pipe[0];
  fds.events = POLLIN;
  int res = poll(&fds, 1, 0);

  if(res < 0||fds.revents&(POLLERR|POLLNVAL)){
      //an error occurred, check errno
    }
  return fds.revents&POLLIN;
}

void *thread_function(void *input){
  pthread_detach(pthread_self());
  int listenfd = *((int *)input);

  output_log << "Listener thread " << listenfd << " up and running\n\n";
  output_log.flush();
  
  if(listenfd < 0){
    cout << "Invalid socket\n";
    return input;
  }
  
  char recvBuff[200];
  memset(recvBuff, '\0',strlen(recvBuff));
  int n = 0;

  while(1){
    while((n = read(listenfd, recvBuff, strlen(recvBuff))) > 0){	  
      write(custom_pipe[1],recvBuff,strlen(recvBuff));
      output_log << "Message received: " << recvBuff << "\n";
      output_log.flush();
    }
  }
  return input;
}

void publish_mqtt(int sockfd,bool just_topic=false){
  char commandBuff[200];
  memset(commandBuff, '0',strlen(commandBuff));
  
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
    char msg[1200],buffer[50];
    std::cout << "What message would you like to publish(max 1000 characters)?\nPublication(Type 'exit' to finish publication): ";
    int size_check = 0;
    memset(msg, '\0',strlen(msg));
    while(1){
      scanf("%s",buffer);
      if(strcmp(buffer,"exit")==0)
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
    while(write(sockfd,commandBuff,strlen(commandBuff)) < 0)
      std::cout << "Write failure, trying again...\n";
    memset(msg, '\0',strlen(msg));
    printf("Your sent package: %s\n",commandBuff);
  }else{
    strcpy(commandBuff,"SUB,");
    strcat(commandBuff,topic);
    
    while(write(sockfd,commandBuff,strlen(commandBuff)) < 0)
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
  
  int n,fd;
  if(!canReadFromPipe()){
    fd = sockfd;
    output_log << "Can't read from pipe!\n";
    output_log.flush();
  }
  else
    fd = custom_pipe[0];
  
  while((n = read(fd, commandBuff, sizeof(commandBuff)-1)) > 0){
    if(strstr(commandBuff,"DISC_ACK")!=NULL)
      break;
  }
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
  int sockfd = -1;
  pthread_t tid;
  char commandBuff[200];
  if(pipe(custom_pipe) == -1)
    std::cout << "Couldn't create pipe\n";
    
  while(cont){    
    std::cout << "Please write a command: ";
    fgets(commandBuff, 100, stdin);
    string command(commandBuff);
    
    switch(hashit(command)){
    case CONNECTION:
      if(sockfd != -1)
	std::cout << "Already connected!\n";
      else{
	sockfd = socket_helper->make_connection();
	memset(commandBuff, '\0',strlen(commandBuff));
	sprintf(commandBuff,"client_connections_log%d.txt",sockfd);
	output_log.open(commandBuff);
	if(sockfd == -1)
	  std::cout << "Unsuccessful Connection\n";
	else{
	  std::cout << "Successful Connection on sockect " << sockfd << "\n";
	  pthread_create(&tid,NULL,thread_function,(void *)&sockfd);
	}
      }
      break;
    case DISCONNECT:
      if(sockfd != -1){
	disconnect_mqtt(sockfd);
	std::cout << "Successful Disconnection, all channels successfully unsubscribed\n";
        pthread_cancel(tid);
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
      pthread_cancel(tid);
      close(custom_pipe[0]);
      close(custom_pipe[1]);
      break;
    default:
      std::cout << "Invalid command, please try again\n";
      break;	
    }

    if(canReadFromPipe() && cont != 0){
      memset(commandBuff, '\0',strlen(commandBuff));
      read(custom_pipe[0], commandBuff, sizeof(commandBuff)-1);
      std::cout << "\n\nNew message: " << commandBuff << '\n';
    }
    
  }
  
  return 0;
}
