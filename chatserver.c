/*
 * chatserver.c - A simple chat server
 * Communication protocol
 *     JOIN name
 *     WHO
 *     LEAVE
 *
 *     reference: http://www.csc.villanova.edu/~mdamian/classes/csc2405sp18/sockets/chat
 */

#include "nethelp.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_CLIENTS (20)
#define MAX_NAME_LENGTH (20)
#define DEBUG_ON (1)
#define DEBUG if(DEBUG_ON)
#define MAX_MESSAGE_SIZE (512)
#define MAX_HOSTNAME_SIZE (50)
#define VERSION "Chat Server v0.1\n"

typedef struct
{
    int fd;
    char* name;
} client_info;

client_info clients[MAX_CLIENTS]; // slot i is empty, is clients[i]= NULL
sem_t mutex; // semaphore to protect critical sections


// prototypes:

// thread function
void* HandleClient(void* arg);

// add client name in position index in the array 'clients'
void HandleJOIN(char* buf, int index);

// write the names of all active clients over the connection fd
void HandleWHO(int fd);

// remove the client from position index in the array 'clients'
void HandleLEAVE(int index);

// broadcast message from client in position index to all active clients
void HandleBroadcast(char* msg, int index);

// send version of the server
void HandleVERSION(int fd);

// check WHO command
bool checkWHO(char* message);

// check JOIN command
bool checkJOIN(char* message);

// check LEAVE command
bool checkLEAVE(char* message);

// check VERSION command
bool checkVERSION(char* message);

int main(int argc, char** argv) {
  int listenfd, port;
  struct sockaddr clientaddr;
  int clientlen = sizeof(struct sockaddr);
  int connfd; // connfd: connection file descriptor
  int tid;    // thread id

  if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      // argv[0] is the name of the program by convention
      exit(EXIT_FAILURE);
  }

  // get the port number on which to lsiten for incomming request from clients
  port = atoi(argv[1]);
  // TODO: atoi is not the safest function to parse integers, as it doesn't handle errors. 'strol' may be better.

  // initialize array of clients
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
      clients[i].name = NULL;
      clients[i].fd = -1;
  }

  printf("%s\n", VERSION);

  // initialize semaphore to protect critical sections.
  sem_init(&mutex, 0, 1);

  // create a listening socket
  listenfd = open_listenfd(port);
  if (listenfd < 0) {
      printf("Failed to open listening socket\n");
      exit(EXIT_FAILURE);
  }

  char hostname[MAX_HOSTNAME_SIZE];
  gethostname(hostname, MAX_HOSTNAME_SIZE);
  fprintf(stdout, "Server ready! host: %s, port: %d\n", hostname, port);

  while (1) {
      // Accept an incomming request from a client
      connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen); // connfd : connection file descriptor
      if (connfd < 0) { continue; } // connection failed

      // Allocate a new entry in the array of clients for the new client
      // create a new thread to handle the client
      // 'clients' may be modified by other threads, so it has to be protected with a semaphore

      sem_wait(&mutex);
      fprintf(stdout, "accepted new connection\n");
      for (int i = 0; i < MAX_CLIENTS; i++)
      {
          if (clients[i].fd < 0) // find empty slot
          {
              clients[i].fd = connfd;
              pthread_create(&tid,         // pointer to pthread descriptor
                             NULL,         // use default atributes
                             HandleClient, // thread function entry point 
                             (void*)i      // parameters to passs to the thread function 
                            );
               break;
          }
      }
      printf("thread launched with id: %d\n", tid);  // TODO: clean?
      sem_post(&mutex);

  } // end while(1)

  return EXIT_SUCCESS;
}

/*
* HandleClient - handle chat client
* arg is the client index in the client's array
*/
void* HandleClient(void* arg)
{
  size_t n = 1;
  char buf[MAXLINE];

  int index = (int)arg;
  int connfd = clients[index].fd;

  // Detach the thread to free memory resources upon termination
  pthread_detach(pthread_self());

  while(n > 0) {
	  // process commands JOIN, WHO, LEAVE
	  // broadcast anything else

	  // TODO: add your code here

	  // read the next line from the client
	  n = readline(connfd, buf, MAXLINE);

	  fprintf(stdout, "[%s] %s\n",(clients[index].name? clients[index].name : "?"), buf);

	  if (clients[index].name == NULL) {
	      if (checkJOIN(buf)) {
		  HandleJOIN(buf, index);
	      }
	  } else if (checkWHO(buf)) {
	     HandleWHO(connfd);
	  } else if (checkLEAVE(buf)) {
	     HandleLEAVE(index);
	  } else if (checkVERSION(buf)) {
             HandleVERSION(connfd);
	  } else {
	     HandleBroadcast(buf, index);
	  }
  }

  // close connection with the client
  close(connfd);
  return NULL;
}

/* HandleWHO: send out client names in response to the WHO command
*/
void HandleWHO(int fd)
{
  // TODO: add your code here
  for (int i = 0; i < MAX_CLIENTS; i++)  {

      if ((clients[i].name == NULL) || (clients[i].fd == -1))
          continue;

      errno = 0;
      ssize_t message_length = 0;

      message_length = write(fd, clients[i].name, (strlen(clients[i].name /*+ 1*/)));
      write(fd, "\n", sizeof(char));
      fprintf(stdout, "%s\n", clients[i].name);

      if (message_length == -1) {
          // errno is set to error
          fprintf(stderr, "Error when sending the WHO message\n");
          fprintf(stdout, "Error when sending the WHO message\n");
          fprintf(stderr, "client: %s, with file descriptor: %d\n", clients[i].name, clients[i].fd);
          fprintf(stdout, "client: %s, with file descriptor: %d\n", clients[i].name, clients[i].fd);
          reportErrno();
      }
  }
  printf("\n");
}

/*
* HandleBroadcast: Broadcast msg from client in position index
* to all the active chat clients
*/
void HandleBroadcast(char* msg, int index)
{
    // TODO: add your code here
    if (index >= MAX_CLIENTS)
	return;

    if (clients[index].name == NULL)  // TODO: check this in a better place!
        return;

    if (*msg == NULL)
	return;

    char message[MAX_MESSAGE_SIZE];
    memset(message,0x00,sizeof(char)*MAX_MESSAGE_SIZE);

    for (int i = 0; i < MAX_CLIENTS; i++) // go through all the clients
    {
        if ((i != index) && (clients[i].name != NULL) && (clients[i].fd != -1)) {
            errno = 0;
            ssize_t message_lenght = 0;

            memset(message, 0x00, sizeof(char)*MAX_MESSAGE_SIZE);
	    strcat(message, "[");
	    strcat(message, clients[index].name);
	    strcat(message, "] ");
	    strcat(message, msg);

	    ssize_t rv = send(clients[i].fd, message, strlen(message),0);

	    if (rv == -1) {
		// errno is set to error
		fprintf(stderr, "Error when sending broadcast message\n");
		fprintf(stderr, "client: %s, with file descriptor: %d\n", clients[i].name, clients[i].fd);
		reportErrno();
	    }
        }
    }
}

/* HandleJOIN: Add client name in position index in the
*  array of clients
*/
void HandleJOIN(char* buf, int index)
{
    if (index >= MAX_CLIENTS) return;
    if (*buf == NULL) return;

    char output[MAXLINE];

    // Make sure that client did not already join
    if (clients[index].name != NULL) {
        sprintf(buf, "Already joined as %s\n", clients[index].name);
	fprintf(stdout, "you %s have already joined\n", clients[index].name);
       	send(clients[index].fd, output, strlen(buf),0);
        return;
    }

    char* p_name=buf+strlen("JOIN"); //point after JOIN command

    // skip all leading whitespaces
    while((*p_name == ' ') || (*p_name == '\t')) { p_name++; }

    // TODO: check the max lenght of the name

    // replace '\n' by '\0' in user name if any
    if ('\n' == p_name[strlen(p_name)-1]) {
        p_name[strlen(p_name)-1] = '\0';
    }

    sem_wait(&mutex); // we have to block here, in case several clients want to subscribe at the same time

    clients[index].name = malloc(strlen(p_name)+1); // +1 to account for '\0'
    strcpy(clients[index].name, p_name); // is better to use, strncpy. it is safer. It could be possible to also use strdup().

    sem_post(&mutex); // release the lock

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].name == NULL) { continue; }
	if (i == index) {
	    sprintf(output, "Welcome to the chat room, %s!\n", p_name);
	    fprintf(stdout, "Welcome to the chat room, %s!\n\n", p_name);
	} else {
	    sprintf(output, "%s has joined the chat room\n", p_name);
	    //fprintf(stdout, "%s has joined the chat room\n", p_name);
       	}

        ssize_t rv = send(clients[i].fd, output, strlen(output), 0);
	if (rv == -1)
            printf("Error when sending a message to the client\n");
    }
}

/* HandleLEAVE: Remove client form position index in array clients
 * and close the connection to the client
 */
void HandleLEAVE(int index)
{
    if (index >= MAX_CLIENTS)
	return;
    
    if (clients[index].name == NULL) 
        return;
    
    // prepare the message that someone leaves the chat.   
    char buf[MAXLINE];
    sprintf(buf, "%s just leaved the chat room\n", clients[index].name);

    // broadcast that someone leaves the chat.   
    HandleBroadcast(buf, index);

    sem_wait(&mutex); 
    if (clients[index].name != NULL) {
	printf("%s just leaved the chat room.\n\n", clients[index].name);
        free(clients[index].name);  // free memory
       	clients[index].name = NULL; // clean up the name
    }
    close(clients[index].fd);
    clients[index].fd = -1;         // clean up the file descriptor
    sem_post(&mutex);

    // Terminate the thread
    pthread_exit(NULL);
}

void HandleVERSION(int fd) {
   printf("%s", VERSION);
   send(fd, VERSION, strlen(VERSION),0);
}

static char* removeLeadingSpaces(char* message) {
    if (*message == NULL) return NULL;

    char* pstring = message;
    // skip all leading whitespaces
    while((*pstring == ' ') || (*pstring == '\t')) { pstring++; }
    return *pstring;
}

static bool checkCommand(const char* command, char* message) {
    if (*command == NULL) return false;
    if (*message == NULL) return false; 

    if (0 == strncmp(message, command, strlen(command)) ) {
	return true;
    }
    return false;
}

bool checkWHO(char* message) {
    return checkCommand("WHO", message);
}

bool checkJOIN(char* message) {
    return checkCommand("JOIN ", message);
}

bool checkLEAVE(char* message) {
    return checkCommand("LEAVE", message);
}

bool checkVERSION(char* message) {
    return checkCommand("VERSION", message);
}
