reference: http://www.csc.villanova.edu/~mprobson/courses/sp21-csc2405/chat.html Chat Client and Multi-Threaded Chat Server

## Introduction

The goal of this activity is to implement a simplified chat server that can support multiple clients over the Internet. There are a seemingly infinite number of chat programs out there using various protocols, with IRC (Internet Relay Chat) being one of the earliest and most popular chat protocols. We will implement a very simplified version of IRC.

You may use the echo client and server as starting point.

### Starting Point: Code Template

1. In your ~/systems/sockets directory create another directory called chat, then download the following files in your ~/systems/sockets/chat directory:

- chatserver.c
- chatclient.c
- nethelp.c
- nethelp.h
- Makefile

2. Build the executables xchatserver and xchatclient by typing in at the shell prompt

```
    make
```

3. Your server should listen on a port number of your choice but you should avoid well-known ports (0-1023).


4. Start the server with your designated port number N as an argument:
```
     ./chatserver N
```
Open a separate terminal window and start the echo client using
```
     ./chatclient localhost N
```
No communication will take place, because the code is incomplete. Let's fill in the missing code.


5. Take some time to understand what each the existing code in the client and server code does.


## Next Step: Complete the Chat Client/Server Application

Extend the code in the chat client and server template to implement a chat application. Users should be able to join the chat server after entering their names, broadcast messages to all other users, and leave the chat room anytime.

Specifically, the client and server should implement the following communication protocol (the client reads in commands from the user and forwards them to the server):


### JOIN name (Example: JOIN Melissa)
The chat client forwards the request to join to the server. When the server receives this request from the client, it adds that client to a list of clients involved in the chat session.


### LEAVE
The chat client forwards the request to leave to the server. When the server receives the request to leave the chat session from the client, it removes that client from its list of clients involved in the chat session.
The client should not be able to invoke the LEAVE command before joining the chat session.


### WHO
The chat client forwards this request to the server. The server responds back with a list of names of those who have joined the chat session, one per line. Once the client receives this list, it displays it on the screen.


### HELP
The client prints out a list of available commands.


All other messages sent by the client and received by the server are then redistributed to all clients involved in the current chat session.

When displayed, the message is of the form: Name: Message (Example: Melissa: Hello there)

Work incrementally, one step at a time, one command at a time. Make sure to thoroughly test and debug one command before moving on to the next.

1. Start with chatclient.c and add code to the main function to do the following:
  - In a loop, read lines of text from the user and sends them to the server
  - If the command is LEAVE, close the connection to the server
  - Since the chat client and the chat server operate asynchronously (the server can send messages to the client at any time), the client needs to spawn a thread to handle all messages received from the server.
  - Implement the HandleFeedback thread function that reads messages from the server and prints them onto the screen. The function loops indefinitely, until EOF is received (in response to the server closing the connection to the client for whatever reason).
  - Compile your code and remove all errors
  
2. The chatserver.c program uses an array clients that stores information abotu the chat clients (name and connection file descriptor). Add code to the main function to allocate a new entry in this array each time a new connection request is received from a client. Make sure you protect critical sections with semaphores.

3. The chat server is multi-threaded: each time a new connection request is received from a chat client, the server creates a thread to handle the new chat client. The thread should begin executing the HandleClient code.

4. Add code to HandleClient to interpret the line of text received from the chat client (whether it is a JOIN, WHO, LEAVE command, or anything else) and call the appropriate function to handle each command (HandleJOIN, HandleWHO, HandleLEAVE and HandleBroadcast).

5. The function HandleJOIN handles the JOIN request (already implemented). Take some time to understand what this function does. It takes as arguments the name of the chat client (extracted from the client message) and the index of the entry in the array clients allocated for this client. The function stores the name of the chat client into the array and sends a welcome message to the new chat client. It also checks to make sure that the client hasn't already joined the chat room.

6. At this point you should be able to test the code for your server and client(s). Compile the code and remove all errors. In a terminal window invoke the server with your chosen port number as a command line argument. In a separate terminal window invoke the client with host name localhost and the same port number as command line arguments. If the user types in something like
   JOIN Bob
in the client window, it should receive the message "Welcome to the chat room, Bob!" from the server.

7. Make sure that the JOIN command works as expected, then proceed to implementing HandleWHO. This function simply scans the array of clients, and if the entry is valid (i.e, the name field is not NULL), it simply sends to the client the name stored in that array entry, followed by "\n" (so that each name appears on a separate line). Make sure you test the function thoroughly (with multiple clients running in separate terminal windows) before moving on.

8. Write code for the function HandleBroadcast that takes as argument a message and the index (in the array clients) of the client that sent that message, and broadcasts the message to all other clients in the chat room (along with the name of the sender). Make sure you test the function thoroughly (with multiple clients running in separate terminal windows) before moving on.

9. Extend the chat protocol to include a HELP command that prints out a list of available commands to the clients.

10. Be creative. Extend the chat protocol to include any commands you find interesting and useful.

HAVE FUN!
