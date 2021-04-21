#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int sock;

/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port,char *addrBuff) {

  //initialize socket with IPv4 connection
  //with error handling
  if((sock = socket(PF_INET,SOCK_STREAM,0))==-1)
    perror("socket initialization failure");

  //set up socket address
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  //addr.sin_addr.s_addr = htonl(INADDR_ANY);
  inet_pton(AF_INET,addrBuff,&addr.sin_addr);
  addr.sin_port = htons(port);

  int enable = 1;
  if((setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(int)))==-1){
    perror("Can't set socket option");
    exit(0);
  }
  
  //bind socket to create connection and listen with size 50 queue of backlogs
  if((bind(sock,(struct sockaddr*)&addr,sizeof(addr))==-1)){
    perror("Failure to bind");
    //if failure, want to close all sockets
    printf("Exiting program...\n");
    exit(0);
  }
  
  if(listen(sock,20) == -1){
    perror("Failure to listen");
    //if failure, want to close all sockets
    printf("Exiting program...\n");
    exit(0);
  }

  
  printf("socket address: %u\n",addr.sin_addr.s_addr);

  return;
  
}

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the request should be ignored.
***********************************************/
int accept_connection(void) {
  int retval;

  //set up sockaddr_in for socket
  struct sockaddr_in client;
  socklen_t len = (socklen_t)sizeof(struct sockaddr);

  //make sure socket was accepted successfully
  while(((retval = accept(sock,(struct sockaddr*)&client,&len)) == -1) && (errno == EINTR));

  //just extra security, not needed honestly
  if(retval == -1)
    perror("Failed to accept connection");

  return retval;
}

/**********************************************
 * get_request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        from where you wish to get a request
      - filename is the location of a character buffer in which
        this function should store the requested filename. (Buffer
        should be of size 1024 bytes.)
   - returns 0 on success, nonzero on failure. You must account
     for failures because some connections might send faulty
     requests. This is a recoverable error - you must not exit
     inside the thread that called get_request. After an error, you
     must NOT use a return_request or return_error function for that
     specific 'connection'.
************************************************/
int get_request(int fd, char *filename) {
  //recieve request and save size of request into readsz
  int readsz;
  char tmpStr[1024];
  if((readsz = read(fd,tmpStr,1023)) >=0)
    filename[readsz] = '\0';
  else{
    perror("server read problem");
    return -1;
  }
  int cl = 1;
  char *closer;
  closer = strstr(tmpStr,"Close");
  if(closer == NULL)
    cl = 0;   
  
  //*closedConnection = cl;
  
  //if any of the checks fail, will set the valid variable
  //appropriately in order to close port once
  int valid = 0; 
  
  //make sure file is correct size
  if(strlen(tmpStr) > 1023)
    valid = 1;
  
  for(int i=0;i<strlen(tmpStr);i++){
    char ch = tmpStr[i];
     //make sure that "//" and ".." aren't in the filename to avoid hack
    if(ch == '/' && tmpStr[i+1] == '/'){
      valid = 1;
      break;
    }

    if(ch == '.' && tmpStr[i+1] == '.'){
      valid = 1;
      break;
    }
    
  }
  
  char *get,*path;
  char *saveptr;
  //use thread safe strtok to strings deliminated by " "
  get = strtok_r(tmpStr," ",&saveptr);
  path = strtok_r(NULL," ",&saveptr);
  
  //make sure first request is "GET" in filename
  if(strcmp(get,"GET")){
    fprintf(stderr,"Doesn't start with GET\n");
    valid = 1;
  }
  
  strcpy(filename,path);
  
  return valid;
}

/**********************************************
 * return_result
   - returns the contents of a file to the requesting client
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the result of a request
      - content_type is a pointer to a string that indicates the
        type of content being returned. possible types include
        "text/html", "text/plain", "image/gif", "image/jpeg" cor-
        responding to .html, .txt, .gif, .jpg files.
      - buf is a pointer to a memory location where the requested
        file has been read into memory (the heap). return_result
        will use this memory location to return the result to the
        user. (remember to use -D_REENTRANT for CFLAGS.) you may
        safely deallocate the memory after the call to
        return_result (if it will not be cached).
      - numbytes is the number of bytes the file takes up in buf
   - returns 0 on success, nonzero on failure.
************************************************/
int return_result(int fd, char *content_type, char *buf, int numbytes) {
  char retres[2048+numbytes];
  char connectionstr[12];

  //if(closedConnection)
  //strcpy(connectionstr,"Closed");
  //else
    strcpy(connectionstr,"Keep-Alive");

  sprintf(retres, "HTTP/1.1 200 OK\nContent-Type: %s\nContent-Length: %d\nConnection: %s\n\n%s",content_type,numbytes,connectionstr,buf);

  //retres[strlen(retres)]='\0';
  int write_check;
  
  if((write_check = write(fd,retres,numbytes+2048))<1)
    return 1;

  //close connection if closed connection is activated
  //if(closedConnection)
  //close(fd);
  return 0;
}

/**********************************************
 * return_error
   - returns an error message in response to a bad request
   - parameters:
      - fd is the file descriptor obtained by accept_connection()
        to where you wish to return the error
      - buf is a pointer to the location of the error text
   - returns 0 on success, nonzero on failure.
************************************************/
int return_error(int fd, char *buf) {
  char retres[2048+strlen(buf)];

  sprintf(retres, "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: %ld\nConnection: Close\n\n%s",strlen(buf),buf);

  //retres[strlen(retres)]='\0';
  
  if(write(fd,retres,strlen(retres))<1)
    return 1;

  int readsz;
  char tmpStr[1024];
  if(!((readsz = read(fd,tmpStr,1023)) >= 0))
    perror("server read problem");

  
  char *closeoption;
  closeoption = strstr(tmpStr,"Close");

  int cl = 0;
  if(strcmp(closeoption,"Close"))
    cl = 1;
  
  //close if Close option
  if(cl)
    close(fd);
  
  return 0;
}
