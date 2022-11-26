//reference: http://www.csc.villanova.edu/~mdamian/classes/csc2405sp18/sockets/chat/chat.html
//reference: https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html#sendall

#ifndef __NET_HELP
#define __NET_HELP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>       // for fgets
#include <strings.h>      // for bzero, bcopy
#include <unistd.h>       // for read, write
#include <sys/socket.h>   // for using sockets
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>      // for using threads

#define MAXLINE (8192)      // max text line length
#define LISTENQ (1024)      // second argument to listen()

void reportErrno(void); // report errno

char* ltrim(char* str); // trim string of spaces from the left
char* rtrim(char* str); // trim string of spaces from the right 

/*
 *  open_listenfd - open and return a listeng socket on port
 *  return -1 in case of failure
 */
 int open_listenfd(int port);

/* 
 * open_clientfd - open connnetcion t server at
 * an return a socket descriptor ready for reading an writing
 * return < 0 in case of failure
 */
 int open_clientfd(char* hostname, int port);

/*
 * readline - read a line of text ending with '\0'
 * return the number of charcters read
 * return -1 in case of failure
 */
 int readline(int fd, char* buf, int maxlen);

/* 
 * 'send()' may not be able to send all the bytes reqeusted 
 * in one go, so to make sure everything is send use the
 * following function.
 * reference: https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html#sendall
 */
int sendall(int s, char *buff, int *len, int flags);

#endif //__NET_HELP
