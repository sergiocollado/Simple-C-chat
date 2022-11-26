// reference: http://www.csc.villanova.edu/~mprobson/courses/sp21-csc2405/chat.html
// help: https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html
#include "nethelp.h"
#include <errno.h>

void reportErrno(void) {
    if (errno ==0) return;  // there is no error
    fprintf(stderr, "Value of errno: %d\n", errno);
    fprintf(stderr, "Error string: %s\n\n", strerror( errno ));
}

// left trim
char* ltrim(char* str) {
    if (!str) {
        return '\0';
    }
    if (!*str) {
        return str;
    }
    while (*str != '\0' && isspace(*str)) {
        str++;
    }
    return str;
}

// right trim
char* rtrim(char* str) {
    if (!str) {
        return '\0';
    }
    if (!*str) {
        return str;
    }
    char* end = str + strlen(str) - 1;
    while (end >= str && isspace(*end)) { end--;
    }
    *(end + 1) = '\0';
    return str;
}

/* 
 * open_listenfd - open and return a listening socket on port
 * return -1 in case of failure
 */
int open_listenfd(int port)
{
    int listenfd;
    int optval = 1;
    struct sockaddr_in serveraddr;

    // Create a socket descriptor
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { return -1; }
    
    // Eliminates "Address already in use" error from bind
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) < 0)
    { return -1; }

    // listenfd will be an endpoint for all request
    // to port on any IP address for this host
    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    { return -1; }

    // Make it a listening socket ready to accept connection requests
    if (listen(listenfd, LISTENQ) < 0)
    { return -1; }   
  
    return listenfd;
}

/*
 * open_clientfd - open connection to server 
 * and return a socket descriptor for reading and writing
 * return < 0 in case of failure
 */
int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    errno = 0;
    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) <0) {
        fprintf(stderr, "socket creation failed\n");	   
        reportErrno();
        return -1;
    }

    // fill in the server's IP address and port
    if ((hp = gethostbyname(hostname)) == NULL) { 
	fprintf(stderr, "unknown host name %s\n", hp->h_name);
        return -2;
    }
    fprintf(stdout, "connecting to host name %s\n", hp->h_name);

    bzero((char*) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    bcopy((char*) hp->h_addr, (char*) &serveraddr.sin_addr.s_addr, hp->h_length);

    // establish a connection with the server
    errno = 0; 
    if (connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {    printf("Error connecting with the server\n");
         reportErrno();
	 return -1;
    }

   return clientfd;
}


/*
 * readline -read a line of text ending with '\0'
 * return the number of characters read;
 * return -1 if error
 */
int readline(int fd, char* buf, int maxlen)
{
    int nc = 0; 
    int n = 0;

    for (n = 0; n < maxlen -1; n++)
    {
        nc = read(fd, &buf[n], 1); 
	if ((0 == n) && (buf[n] == '\0')) { n--; /*buf[n] = '-';*/}
	
	if (nc <= 0) { 
	    //printf("read error\n"); 
	    reportErrno(); 
	    return nc; }

	if (buf[n] == '\n') break;
    }

    buf[n+1] = '\0';
    return n+1;
}


int sendall(int s, char *buff, int *len, int flags)
{
    int total = 0;        // how many bytes already sent
    int bytesleft = *len; // how many bytes left to send
    int n = 0;

    while (total < *len) {
       n = send(s, buff+total, bytesleft, flags);
       if (-1 == n) { break; }
       total += n;
       bytesleft -= n;
    }

    *len = total;                 // return number actually sent here
    return ((n == -1) ? -1 : 0);  // return -1 on failure, 0 on success.
}

