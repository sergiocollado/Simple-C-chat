/* chatclient.c - a simple chat client
 * communication protocol:
 *   JOIN name
 *   WHO
 *   LEAVE
 * based on: http://www.csc.villanova.edu/~mdamian/classes/csc2405sp18/sockets/chat/chat.html
 * reference: http://www.csc.villanova.edu/~mprobson/courses/sp21-csc2405/
 */

/* JOIN name
 * The chat forwards the request to join the server.
 * When the server receives this request from the client
 * it adds that client to a list of clients involved in 
 * that chat session
 */

/* LEAVE
 * The chat client forwards the request to leave the server.
 * When the server receives the request to leave the chat
 * session from the client, it removes that client form its list
 * of clients involved in the chat session.
 * The client should not be able to invoque LEAVE command 
 * before joining the chat session
 */

/* WHO
 * The chat client forwards this request to the server. 
 * The server responds back with a list of names of those
 * who have joined the chat session, one per line.  * Once the client reveives this list, it displays it * on the screen.
 */

/* HELP
 * The client prints out a list of available commands
 */

/* All the messages sen by the client and reveived by
 * the server are then redistributed to all clients
 * involved in teh current session
 * 
 * When displayed the message is of the form: Name: Message
 */

#include "nethelp.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>

#define MAX_MESSAGE_SIZE (512)
#define VERSION "Chat Client v0.1"

// function prototypes
void* HandleFeedback(int fd);
size_t readInput(char* buf, int buf_size);
void* ReadMessagesFromServerLoop(void* fileDescriptor);
void readTextAndSendToServerLoop(int clientfd);
void printCommands(void);
void printWelcomeMessage(void);
void printVERSION(void);

typedef struct {
  // thread parameters to pass in
  int fileDescriptor;
} threadParams_t;

// global variables
pthread_t thread_id;		// this thread will be used to handle the responses from the server
threadParams_t parameter;	// this parameter will be used to pass parameters to the previous thread.

int main(int argc, char* argv[])
{
    int clientfd, port, tid;
    char* host, buff[MAXLINE];

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);	
        // argv[0] is the name of the program by convention
        exit(EXIT_FAILURE);
    }

   // get the name of the machine on which the server is running
   host = argv[1];

   // get the port number on which server is listening for the client requests
   port = atoi(argv[2]);
   // atoi is not the best option, as it doesn't report errors
   // it would be better to use strol function
   // ref: http://www.microhowto.info/howto/safely_parse_an_integer_using_the_standard_c_library.html

   printWelcomeMessage();

   // open a connection to the server
   clientfd = open_clientfd(host, port);
   if (clientfd < 0) {
       printf("Connection to <%s, %d> failed\n", host, port);
       exit(EXIT_FAILURE);
   } else {
       printf("Connection to the server opened...\n");
   }
    

   // TODO: create a thread to read messages from the server
   // and print them on the screen

   HandleFeedback(clientfd);

   // TODO: In a loop, read string from user and send them to
   // the server & terminate when the user writes LEAVE
   
   readTextAndSendToServerLoop(clientfd);

   // close sockets
   close(clientfd);
   return(EXIT_SUCCESS); 
}

void printCommands(void) {
    printf("You can use the commands:\n"
	  " - JOIN <name> : join to the chat with alias <name>\n"
	  " - WHO : enumerate the chat participands\n"
	  " - LEAVE : leave the chat\n"
	  " - HELP : list the possible commands\n\n");
}

void printWelcomeMessage(void) {
    printf("Welcome to the chat client.\n"
          "Please use \'JOIN <name>\' to join the chat\n\n");
    printCommands();
}

void printVERSION(void) {
    printf("%s\n", VERSION);
}

void *ReadMessagesFromServerLoop(void* parameter) {

   // retrieve the socket file descriptor passed as a paremeter of the thread.
   threadParams_t *param = (threadParams_t*)parameter;
   int fileDescriptor = param->fileDescriptor;

   // Detach the thread to free memory resources upon termination
   pthread_detach(pthread_self());

   // reference: https://man7.org/linux/man-pages/man2/read.2.html
   // reference: https://man7.org/linux/man-pages/man2/recv.2.html
   // The only difference between recv() and read(2) is the presence of
   // flags. With a zero flags argument, recv() is generally
   // equivalent to read(2) (but see NOTES)

   char message[MAX_MESSAGE_SIZE];

   while(1) {
       memset(message,0x00,sizeof(char)*MAX_MESSAGE_SIZE);
       ssize_t readedBytes = 0;
       readedBytes = recv(fileDescriptor, message, (MAX_MESSAGE_SIZE-1), 0);
       message[MAX_MESSAGE_SIZE -1] = '\0'; // just in case to protect the message size
       printf("%s\n", message);
   }
}

// Thread function that reads messages from server and
// writes them onto the screen
void* HandleFeedback(int fileDescriptor)
{
    // Since the chat client and the chat server operate asynchronously
    // (the server can send messages to the client at any time), the
    // client needs to spawn a thread to handle messages received 
    // from the server.
    //
    // Implement the HandleFeedback tread function that reads messages
    // from the server and prints them onto the screen. The function 
    // loops indefinitely, until EOF (end-of-file) is received (in 
    // response of the server closing the connection to the client 
    // for whatever reason).  
    //
    // exit when the connection is terminated

    // TODO: add code here
    
    if (fileDescriptor == NULL) {
	printf("Error fileDescriptor for the server is NULL!!\n");
	exit(EXIT_FAILURE);
    }

    parameter.fileDescriptor = fileDescriptor;

    // create thread
    int rv =     \
    pthread_create(&thread_id,			// pointer to thread descriptor
		   (void*)0,			// use default attributes
		   ReadMessagesFromServerLoop,	// thread function entry point
		   (void*)&parameter		// parameters to pass in
                  );

    if (rv != 0) {
	printf("Error creating thread: %d\n", rv);
	exit(EXIT_FAILURE);
    }

    // this thread must be terminated when the command LEAVE is used!
}

size_t readInput(char* buf, int buf_size) {
    // never use `gets()`! instead use: `fgets()`.
    /*
    gets() is dangerous because it is possible for the user to crash
    the program by typing too much into the prompt. It can't detect the
    end of available memory, so if you allocate an amount of memory 
    too small for the purpose, it can cause a seg fault and crash.
    ... so we use fgets() because allows you to specify how many characters 
    are taken out of the standard input buffer, so they don't overrun the variable.
    */

    // use this: `fgets()`: "file get string", which reads until either EOF is
    // reached, OR a newline (`\n`) is found, keeping the newline char in
    // `buf`.
    // For `feof()` and `ferror()`, see:
    // 1. https://en.cppreference.com/w/c/io/feof
    // 1. https://en.cppreference.com/w/c/io/ferror
    
    errno = 0; 
    char* retval = fgets(buf, buf_size, stdin);
    /* fgets() reads in at most one less than size characters from stream and 
     * stores them into the buffer pointed to by s.  Reading stops after an EOF 
     * or a newline.  If a newline is read, it is stored into the buffer. A 
     * terminating null byte ('\0') is stored after the last character in the buffer. */

    if (feof(stdin))
    {
	// Check for `EOF`, which means "End of File was reached".
	// - This doesn't really make sense on `stdin`, but it is a good
	//   check to have when reading from a regular file with `fgets
	//   ()`. Keep it here regardless, just in case.
	perror("EOF (End of File) reached.\n");
    }
    if (ferror(stdin))
    {
	perror("Error indicator set. IO error when reading from file "
	       "`stdin`.\n");
    }
    if (retval == NULL)
    {
	fprintf(stderr, "ERROR in %s(): fgets() failed; errno = %i: %s\n",
	    __func__, errno, strerror(errno));
	exit(EXIT_FAILURE);
    } else if (retval == buf) {
	size_t num_chars_written = strlen(buf) + 1; // + 1 for null terminator
	// TODO: do I have to check if we got a newline character??
	if (num_chars_written >= buf_size)
	{
	    printf("Warning: user input may have been truncated! All %zu chars "
	      "were written into buffer.\n", num_chars_written);
	}
	return strlen(buf);
    }

    return -1;
}

void readTextAndSendToServerLoop(int clientfd) {
    char message[MAX_MESSAGE_SIZE];
    size_t message_length = 0;
    
    //memset(message,0x00,sizeof(char)*MAX_MESSAGE_SIZE);

    // get string from the user
    message_length = readInput(message, MAX_MESSAGE_SIZE);

    while(strncmp(message, "LEAVE", strlen("LEAVE"))) { // check if the LEAVE command has been used

       if (!strncmp(message, "VERSION", strlen("VERSION"))) {
           printVERSION();
       }

       if (!strncmp(message, "HELP", strlen("HELP"))) {
           printCommands();
       }

       // send string to the server
       send(clientfd, message, message_length, 0);
       
       // get string from the user
       message_length = readInput(message, MAX_MESSAGE_SIZE-1);
       message[message_length+1] = '\0'; // end of string
       printf("\n");
    }
    
    // TODO send LEAVE command  message to server 
    send(clientfd, "LEAVE\n", strlen("LEAVE\n"), 0);

    // TODO kill the reading server thread! 
    // the thead is detached, so it will delete itself when finishing the program.
}

