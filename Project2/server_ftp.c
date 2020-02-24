/*
  author: Dane Emmerson
  Due Date: 3/1/2020
  Description: Server side part of program to implement basic ftp
               for transfer of text file to client program. See
	       README for more details.
  sources: https://beej.us/guide/bgnet/html/#system-calls-or-bust
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "server_ftp.h"


/* name: send_user_request
   preconditions: fd is open socket to client
                  parsed_buffer contains c string received from client tokenized into arr of cstrings
		  argv command line argv. Needed to determine which port to connect to client
		  ip4 ip address of host in "0.0.0.0" cstring format
   postconditions: Client either sent requested information or error message
   description: send_user_request looks at received string from client, does on of the following:
                "-l" sends client list of items in cwd on fd sock conn
		"-g <filename>" if filename exists in cwd, send client filename on new sock conn.
		                else send user error message on current fd sock conn
		else send user error message.
 */
void send_user_request(int fd, char** parsed_buffer, char** argv, char* ip4){
  if(strcmp(parsed_buffer[0], "-l") == 0){
    send_message(fd, "ACK");
    send_dir_contents(fd, argv, ip4);
  }
  else if(strcmp(parsed_buffer[0], "-g") == 0){
    send_message(fd, "ACK");
  }
  else{ /*we did not receive a valid command from client. notify client */
    send_message(fd, "Error: invalid request\n");
  }
}

/* name: send_dir_contents
   preconditions: fd is open socket to client 
                  argv command line argv. Needed to determine which port to connect to client
		  ip4 ip address of host in "0.0.0.0" cstring format
   postconditions: cwd directory content list has been sent to client
   description: This function is called when server receives "-l" request from client.
                Send user list of content in cwd.
 */
void send_dir_contents(int fd, char** argv, char* ip4){
  char pwd[DIRECTORY_LENGTH];
  struct addrinfo *serverinfo, *p;
  int fd_send;

  memset(pwd, '\0', sizeof(char) * DIRECTORY_LENGTH);
  
  getcwd(pwd, sizeof(char) * (DIRECTORY_LENGTH - 1));
  open_sock(argv, ip4, &fd_send, &serverinfo);
  /* try connecting to each ai_next in linked list until one works or returns invalid */  
  for(p = serverinfo; p != NULL; p = p->ai_next) {
    if ((fd_send = socket(p->ai_family, p->ai_socktype,
			   p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
    
    if (connect(fd_send, p->ai_addr, p->ai_addrlen) == -1) {
      close(fd_send);
      perror("client: connect");
      continue;
    }
    
    break;
  }

  send_message(fd_send, pwd);
  close(fd_send);
}
/*
  name: send_message
  arguments: fd - filedescriptor where we are sending message
	     msg - message to send
  preconditions: arguments meet definitions in arguments above. Currently open
                 socket connection with other host
  postconditions: Message has been sent to other host
  description: Concatanate host handle + "> " + msg, then send this message to
             host previously bound to file descriptor fd.
 */
void send_message(int fd, const char* msg){
  char* temp = malloc((strlen(msg) + strlen(END) + 1) * sizeof(char));
  int num_bytes_sent = 0;
  memset(temp, '\0', (strlen(msg) + strlen(END) + 1) * sizeof(char));

  strcpy(temp, msg);
  strcat(temp, END);

  while(num_bytes_sent < (strlen(msg) + strlen(END))){
    num_bytes_sent += send(fd, temp, strlen(temp), 0);
  }
  
  free(temp);
}

/*
  name: receive_message
  arguments: fd - filedescriptor where we are sending message
                  *buffer may or may not contain a cstring allocated/recvd from previous
		         call to receive_message
  description: Make sure buffer is filled with null bytes, then receive message.
 */
void receive_message(int fd, char** buffer){
  char* temp = "\n.\n";
  int num_bytes = 0, buffer_size = 0;

  /* enter loop to recv from sender, but only allocate memory on an as needed basis */
  do{
    if(buffer_size <= (num_bytes + MESSAGE_SIZE_RECV)){
      buffer_size += MESSAGE_SIZE_RECV;
      *buffer = realloc(*buffer, 
		       sizeof(char) * (buffer_size + 1));
      memset(*(buffer + num_bytes), '\0', 
	     (buffer_size - num_bytes + 1) * sizeof(char));
    }
    num_bytes += recv(fd, *buffer, MESSAGE_SIZE_RECV, 0);
  }while(strstr(*buffer, temp) == NULL);

  /* ensure last byte is still nullbyte, then strip off "\n.\n" delimiter */
  (*buffer)[num_bytes] = (*buffer)[num_bytes - 1] = (*buffer)[num_bytes - 2] 
    = (*buffer)[num_bytes - 3] = '\0';
  printf("%s\n", *buffer);
}

/* name: open_sock
   preconditions:
   postconditions:
   description:

 */
void open_sock(char** argv, char* host, int* sockfd, struct addrinfo** serverinfo){
  struct addrinfo hints, *p;
  int status;
  int yes = 1;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if(host == NULL){ /* then we need to open connectionon localhost */
    if((status = getaddrinfo(NULL, argv[1], &hints, serverinfo)) != 0){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
  }
  else /* then we are opening connection with client who is listening */
    if((status = getaddrinfo(host, argv[1] + 1, &hints, serverinfo)) != 0){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
  
  if((*sockfd = socket((*serverinfo)->ai_family, (*serverinfo)->ai_socktype, (*serverinfo)->ai_protocol)) == -1){
    fprintf(stderr, "socket error: %s\n", gai_strerror(*sockfd));
    exit(1);
  }
  
  for(p = *serverinfo; p != NULL; p = p->ai_next) {
    if((*sockfd = socket(p->ai_family, p->ai_socktype,
			  p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if(setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		  sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    if(bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("server: bind");
      continue;
    }
    
    break;
  }
  freeaddrinfo(*serverinfo);


  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

}
/*
  name: server_listen
  preconditions: sockfd contains valid file descriptor to open socket
  postconditions: server is now listening on port specified by file descriptor
  description: Calls listen system call on provided file descriptor. If fails,
               prints error to std error and exits program.
 */
void server_listen(int sockfd, char* port){
  if(listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  printf("Listening on port %s\n", port);
}

/*
  name: accept_connection
  preconditions: sockfd contains valid file descriptor to open socket
                 serverinfo struct is completed by getaddrinfo()
		 ip4 contains ipv4 address as cstring
  postconditions: *newfd now contains valid file descriptor to socket connected to client
  description: accepts a connection from client. Print message to console indicating
               connection has been accepted and ip address of client.
 */
int accept_connection(int sockfd, int* newfd, struct addrinfo* serverinfo,char* ip4){
  socklen_t addr_size;
  struct sockaddr_storage their_addr;
  *newfd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
  if(*newfd == -1){
    perror("accept");
    return -1;
  }
  inet_ntop(AF_INET, &(serverinfo->ai_addr), ip4, INET_ADDRSTRLEN);
  printf("server got a connection from %s\n",ip4);
  return 0;
}

/*
  name: set_host_name
  preconditions: host_name is the address the cstring location to store user input host name
  postconditions: host_name has been obtained from user, memory has been allocated.
  description: Take address of location to store host name, prompt user for input
 */
void set_host_name(char** host_name){
  size_t name_length = HOST_NAME_LENGTH;
  
  *host_name = malloc((HOST_NAME_LENGTH + 1) * sizeof(char));
  memset(*host_name, '\0', HOST_NAME_LENGTH + 1);
  /* set a name for the server host */
  printf("Enter a name to be assigned to this host: ");
  getline(host_name, &name_length, stdin);
  (*host_name)[strlen(*host_name) - 1] = '\0';
  (*host_name)[HOST_NAME_LENGTH] = '\0';  
}
