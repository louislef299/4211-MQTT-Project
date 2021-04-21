#include "main.h"

using namespace std;

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

int currentPoint = 0; //keeps track of current first element to be serviced
int currentAdd = 0; //keeps track of current placement of queue addition
int numspots=MAX_queue_len;

FILE *logfile;

static volatile sig_atomic_t doneflag = 1;//signal handling flag for clean exit

// structs:
typedef struct request_queue {
   int fd;
   char request[BUFF_SIZE];
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

//request_queue queue of size MAX_quque_len
struct request_queue reqQ[MAX_queue_len];//queue representation
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;//lock/unlock

static int dactive=0;
static int wactive=0;

//saving the given value from terminal
int numworkers;
int numdispatchers;

pthread_t *dispatchTurn;
pthread_t *workerTurn;

static int error;

//will probably have to use this
static pthread_cond_t barrier_work = PTHREAD_COND_INITIALIZER;
static pthread_cond_t barrier_dis = PTHREAD_COND_INITIALIZER;

/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
  return 1;
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
  return;
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
  return;
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
  return;
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
  char retval[20];
  char *ret;
  if((ret=strstr(mybuf,".htm")))//strstr checks to see if the instance of
    //the string is in mybuf
    strcpy(retval,"text/html");
  else if((ret=strstr(mybuf,".jpg")))
    strcpy(retval,"image/jpeg");
  else if((ret=strstr(mybuf,".gif")))
    strcpy(retval,"image/gif");
  else
    strcpy(retval,"text/plain");

  memset(mybuf,'\0',strlen(mybuf));
  strcpy(mybuf,retval);
  return mybuf;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
    // Open and read the contents of file given the request
  return 0;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  
  if((error = pthread_detach(pthread_self())))
    fprintf(stderr,"Failed to detach thread: %s\n",strerror(error));

  int activate = 0;
  
  while (doneflag) {
    
    pthread_mutex_lock(&mtx);
    if(!dactive){
      dactive = 1;
      activate = 1;
    }
    
    while(!activate)
      pthread_cond_wait(&barrier_dis,&mtx);
    pthread_mutex_unlock(&mtx);

    activate = 1;
    //request queue

    // Accept client connection
    int a_fd = accept_connection();//save accepted connection into fd
    pthread_mutex_lock(&mtx);
    if(!(a_fd<0)){
      
      request_t tempQ;//basically casts arg into

      // Get request from the client
      char buffer[BUFF_SIZE];// = malloc(sizeof(char)*BUFF_SIZE);//buffer for get_request
    
      if(get_request(a_fd,buffer))
	fprintf(stderr,"get_request failure\n");
      
      strcpy(tempQ.request,buffer);//place buffer into request struct

      // Add the request into the queue
      tempQ.fd = a_fd;
      //critical section
    
      //struct request_queue *temp = arg;
      reqQ[currentAdd] = tempQ; 
      
      currentAdd=(currentAdd+1)%MAX_queue_len;
      numspots--;
      dactive=(dactive+1)%numdispatchers;
      pthread_mutex_unlock(&mtx);
      pthread_cond_signal(&barrier_dis);
      pthread_cond_signal(&barrier_work);
      activate = 0;
      
    } else {
      pthread_mutex_unlock(&mtx);
    }
  }

  return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  if((error = pthread_detach(pthread_self())))
    fprintf(stderr,"Failed to detach thread: %s\n",strerror(error));
  int numberReq = 0;

  while (doneflag) {
    
    pthread_mutex_lock(&mtx);
    while(currentPoint == currentAdd || !(pthread_equal(pthread_self(),workerTurn[wactive])))
      pthread_cond_wait(&barrier_work,&mtx);
    pthread_mutex_unlock(&mtx);

    //if there is something in the queue
    printf("my turn");
    char mycwd[255];//current working directory buffer
    // Get the request from the queue
    //critical section
    if((error = pthread_mutex_lock(&mtx)))
      fprintf(stderr,"Locking error: %s\n",strerror(error));
    //struct request_queue *temp = arg;//reqQ[currentPoint];
    //reqQ[currentPoint]=NULL;
    struct request_queue tempQ = reqQ[currentPoint];
    currentPoint=(currentPoint+1)%MAX_queue_len;
      
    // Get the data from the disk or the cache (extra credit B)
       
    // return the result
     
    if(getcwd(mycwd,255)==NULL)
      perror("Failed to get cwd");
       
    strcat(mycwd,tempQ.request);
    char buf[BUFF_SIZE];
    wactive = (wactive+1)%numworkers;
     
    //set up stat struct to get file size
    struct stat st;
     
    if(stat(mycwd,&st))
      fprintf(stderr,"Stat failure\n");
       
    //return result back to user and print error if failure
    if(!(return_result(tempQ.fd,getContentType(tempQ.request),buf,st.st_size))){
      if(!(error = return_error(tempQ.fd,buf))){
	char logbuf[BUFF_SIZE];
	sprintf(logbuf,"[%d][%d][%s][%d]\n",wactive,numberReq,tempQ.request,error);
	fputs(logbuf,logfile);//place request into logfile
      }
      else{
	numberReq++;
	char logbuf[BUFF_SIZE];
	sprintf(logbuf,"[%d][%d][%s][%ld]\n",wactive,numberReq,tempQ.request,(long)st.st_size);
	fputs(logbuf,logfile);//place request into logfile
      }
    }

    numspots++;
    if((error = pthread_mutex_unlock(&mtx)))
      fprintf(stderr,"Locking error: %s\n",strerror(error));
       
    // Log the request into the file and terminal
    
    free(tempQ.request);
    free(buf);
     
  }
  return NULL;
}

void setdoneflag (int signo){
  doneflag = 0;
}

void setupHandler(){
  // Change SIGINT action for grace termination
  struct sigaction intmask;
  intmask.sa_flags = 0;
  intmask.sa_handler = setdoneflag;
  if((sigemptyset(&intmask.sa_mask)==-1) || (sigaction(SIGINT,&intmask,NULL)==-1))
    perror("Signal set initialization failure");
  //this works for graceful term
}

int getIPAddr(char *addrBuff){

  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  void * tmpAddrPtr=NULL;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
      // is a valid IP4 Address
      tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      printf("%s IPv4 Address %s\n", ifa->ifa_name, addressBuffer); 
      if(!strcmp(ifa->ifa_name,"en0"))
	strcpy(addrBuff,addressBuffer);
    } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
      // is a valid IP6 Address
      tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
      char addressBuffer[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
      printf("%s IPv6 Address %s\n", ifa->ifa_name, addressBuffer); 
    } 
  }
  if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);

  return 0;
}

int main(int argc, char **argv) {

  // Error check on number of arguments
  /*if(argc != 8){
    printf("usage: %s path queue_length cache_size\n", argv[0]);
    return -1;
    }*/

  // Get the input args

  //port = arg[1]
  //path = arg[2]
  //num_dispatcher = arg[3]
  //num_workers = arg[4]
  //dynamic_flag = arg[5]
  //qlen = arg[6]
  //cache_entries = arg[7]
  numdispatchers = 100;
  numworkers = 100;
  // Perform error checks on the input arguments

  setupHandler();
  
  // Open log file
  logfile=fopen("web_server_log","w+");

  // Change the current working directory to server root directory

  if(chdir("/Users/louis/Documents/4211/project1")==-1)
    perror("Directory change failed");
  // Initialize cache (extra credit B)

  // Create dispatcher and worker threads (all threads should be detachable)

  char addrBuff[INET_ADDRSTRLEN];
  getIPAddr(addrBuff);
  printf("IPv4 Address: %s\n",addrBuff);

  // Start the server
  init(6615,addrBuff);
  
  if((error=pthread_cond_init(&barrier_work,NULL)))
    fprintf(stderr,"Failed to initialize barrier:%s\n",strerror(error));
  if((error=pthread_cond_init(&barrier_dis,NULL)))
    fprintf(stderr,"Failed to initialize barrier:%s\n",strerror(error));
  
  dispatchTurn = (pthread_t *)calloc(numdispatchers,sizeof(pthread_t));
  workerTurn = (pthread_t *)calloc(numworkers,sizeof(pthread_t));
  
  //create the threads and an error check
  int i = 0;
  while(i<numdispatchers){
    //strcpy(reqS.request,argv[2]);
    if((error = pthread_create (dispatchTurn + i,NULL,dispatch,(void *)(reqQ))))
      fprintf(stderr,"Failed to create thread: %s\n",strerror(error));
    i++;
  }
  
  i=0;
  while(i<numworkers){
    if((error = pthread_create (workerTurn + i,NULL,worker,(void *)(reqQ))))    
      fprintf(stderr,"Failed to create thread: %s\n",strerror(error));  
    i++;
  }
  
  // Create dynamic pool manager thread (extra credit A)
  
  // Terminate server gracefully
    // Print the number of pending requests in the request queue
    // close log file
    // Remove cache (extra credit B)
  
  while(doneflag)
    sleep(1);
  
  int numreq;
  if(currentAdd>currentPoint || currentAdd == currentPoint)
    numreq=currentAdd-currentPoint;
  else
    numreq=100-(currentAdd-currentPoint);
  printf("\nNumber of pending requests in the request queue: %d\n",numreq);
  fclose(logfile);
  for(int i = 0;i<numdispatchers;i++)
    if(pthread_kill(*(dispatchTurn + i),SIGKILL))
      fprintf(stderr,"Failed to commit suicide\n");
  for(int i = 0;i<numworkers;i++)
    if(pthread_kill(*(workerTurn + i),SIGKILL))
      fprintf(stderr,"Failed to commit suicide\n");
  if((error = pthread_cond_destroy(&barrier_dis)))
    fprintf(stderr,"Failed to destroy barrier_dis:%s\n",strerror(error));
  if((error = pthread_cond_destroy(&barrier_work)))
    fprintf(stderr,"Failed to destroy barrier_work:%s\n",strerror(error));
  free(dispatchTurn);
  free(workerTurn);
    
  return 0;
}
