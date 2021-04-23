#include "socket.h"

using namespace std;

enum command{CONNECTION,DISCONNECT,PUBLISH,SUBSCRIBE,QUIT,NONE};

pthread_mutex_t mutex_lock;

Socket *socket_helper = new Socket();

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

void *thread_function(void *input){
  pthread_detach(pthread_self());
  int socket = *((int *)input);

  int listenfd = socket_helper->set_up_socket(socket);
  if(listenfd < 0){
    cout << "Invalid socket\n";
    return input;
  }
  
  char recvBuff[100];
  memset(recvBuff, '0',strlen(recvBuff));
  int n;

  while(1){
    socket = accept(listenfd, (struct sockaddr*)NULL, NULL); 
    while((n = read(socket, recvBuff, sizeof(recvBuff)-1)) > 0){	  
      pthread_mutex_lock(&mutex_lock);
      std::cout << "New message: " << recvBuff << "\n";
      pthread_mutex_unlock(&mutex_lock);
    }
  }
  return input;
}

pthread_t publish_mqtt(int sockfd,bool just_topic=false){
  char commandBuff[200];
  memset(commandBuff, '0',strlen(commandBuff));
  
  char topic[10];
  int loop = 1;
  int topicNum;
  pthread_t tid;
  int port;
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
  port = 5000 + topicNum;

  if(!just_topic){
    char msg[1200],buffer[50];
    std::cout << "What message would you like to publish(max 1000 characters)?\nPublication(Type 'exit' to finish publication): ";
    int size_check = 0;
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
    write(sockfd,commandBuff,strlen(commandBuff));
    memset(msg, '\0',strlen(msg));
    printf("Your sent package: %s\n",commandBuff);
  }else{
    strcpy(commandBuff,"SUB,");
    strcat(commandBuff,topic);
    write(sockfd,commandBuff,strlen(commandBuff));
    printf("Your sent package: %s\n",commandBuff);

    memset(commandBuff, '0',strlen(commandBuff));
    cout << "New observer for topic " << topic << " on socket " << port << "\n\n";
    pthread_create(&tid,NULL,thread_function,(void *)&port);

    return tid;

  }
  return (pthread_t)0;
}

void disconnect_mqtt(int sockfd){
  char commandBuff[20];
  memset(commandBuff, '0',strlen(commandBuff));
  strcpy(commandBuff,"DISC");
  pthread_mutex_lock(&mutex_lock);

  write(sockfd,commandBuff,strlen(commandBuff));

  memset(commandBuff, '0',strlen(commandBuff));
  
  int n;
  while((n = read(sockfd, commandBuff, sizeof(commandBuff)-1)) > 0){
    if(strstr(commandBuff,"DISC_ACK")!=NULL)
      break;
  }
  pthread_mutex_unlock(&mutex_lock);
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
  string command;
  int sockfd = -1;
  pthread_t tid[4];
  int whereTid=0;
  
  while(cont){
    std::cout << "Please write a command: ";
    cin >> command;
    
    switch(hashit(command)){
    case CONNECTION:
      if(sockfd != -1)
	std::cout << "Already connected!\n";
      else{
	sockfd = socket_helper->make_connection();
        std::cout << "Successful Connection\n";
      }
      break;
    case DISCONNECT:
      disconnect_mqtt(sockfd);
      std::cout << "Successful Disconnection\n";
      sockfd = -1;
      break;
    case PUBLISH:
      publish_mqtt(sockfd);
      break;
    case SUBSCRIBE:
      tid[whereTid] = publish_mqtt(sockfd,true);
      whereTid++;
      break;
    case QUIT:
      disconnect_mqtt(sockfd);
      cont = 0;
      for(int i=0;i<whereTid;i++)
	pthread_cancel(tid[i]);
      break;
    default:
      std::cout << "Invalid command, please try again:\n";
      break;	
    }
  }
  
  return 0;
}
