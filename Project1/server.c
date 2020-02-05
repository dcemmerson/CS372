/*
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


#define BACKLOG 10
#define MESSAGE_SIZE 1000
#define HOST_NAME_LENGTH 10
#define SERVERPORT "12001"
#define MESSAGE_SIZE_RECV 1012
/*
struct addrinfo {
  int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
  int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
  int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
  int              ai_protocol;  // use 0 for "any"
  size_t           ai_addrlen;   // size of ai_addr in bytes
  struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
  char            *ai_canonname; // full canonical hostname    
   struct addrinfo *ai_next;      // linked list, next node
   };

*/

void send_message(int, const char*, int, const char*, int, int);
void set_host_name(char**);
void receive_message(int, char**);
int check_connection_closed(char*, int, int*);

int main(int argc, char** argv){
  char *host_name, *buffer_send, *buffer_receive;
  size_t send_length = MESSAGE_SIZE;
  int status;
  int open = 1;
  int socketfd, new_fd;
  socklen_t addr_size;
  struct addrinfo hints, *serverinfo, *p;
  struct sockaddr_storage their_addr;
  char ip4[INET_ADDRSTRLEN];
  int yes = 1;
  
  set_host_name(&host_name);
  
  buffer_send = malloc((MESSAGE_SIZE + 1) * sizeof(char));
  memset(buffer_send, '\0', MESSAGE_SIZE + 1);
  buffer_receive = malloc((MESSAGE_SIZE_RECV + 1) * sizeof(char));
  memset(buffer_receive, '\0', MESSAGE_SIZE_RECV + 1);
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if((status = getaddrinfo(NULL, argv[1], &hints, &serverinfo)) != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }
  
  if((socketfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol)) == -1){
    fprintf(stderr, "socket error: %s\n", gai_strerror(socketfd));
    exit(1);
  }
  
  for(p = serverinfo; p != NULL; p = p->ai_next) {
    if((socketfd = socket(p->ai_family, p->ai_socktype,
			  p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    
    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		  sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    
    if(bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(socketfd);
      perror("server: bind");
      continue;
    }
    
    break;
  }
  freeaddrinfo(serverinfo);


  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if(listen(socketfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
  while(1){
    printf("listening and ready to accept\n");
    addr_size = sizeof their_addr;
    new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
    if(new_fd == -1){
      perror("accept");
      continue;
    }
    inet_ntop(AF_INET, &(serverinfo->ai_addr), ip4, INET_ADDRSTRLEN);
    
    printf("server got a connection from %s\n",ip4);
    
    /*
      if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
      perror("send");
      close(new_fd);
      exit(0);
      }
    */
    /*    send(new_fd, " enter your user handle:", 30, 0); */
    send_message(new_fd, host_name, strlen(host_name), "Connected!", strlen("Connected!"), 0);
    open = 1;
    while(open){
      memset(buffer_receive, '\0', MESSAGE_SIZE_RECV + 1);
      receive_message(new_fd, &buffer_receive);
      if(!check_connection_closed(buffer_receive, 0, &open)){
	printf("%s> ", host_name);
	memset(buffer_send, '\0', MESSAGE_SIZE + 1);
	getline(&buffer_send, &send_length, stdin);
	if(check_connection_closed(buffer_send, 1, &open)){
	  send_message(new_fd, host_name, strlen(host_name), buffer_send, strlen(buffer_send), 1);
	  close(new_fd);
	}
	else
	  send_message(new_fd, host_name, strlen(host_name), buffer_send, strlen(buffer_send), 0);
      }
      else close(new_fd);
    }
  }
}

/*
  name: check_connection_closed
  arguments: str - string to check contents of
             sending: equals one is message is being sent to host, else 0 if receiving from host
	     open: pointer to variable indicating if connection needs to remain open
  returns: 0 if connection needs to remain open, else 1
  description: checks str, which is message to send to host or message being received from host,
               if it is equal to \quit and performs actions necessary to indicate to calling
	       location whether or not to close connections.
 */
int check_connection_closed(char* str, int sending, int* open){
  if(strcmp(str,"\\quit\n") == 0 || strcmp(str,"\\quit") == 0){
    *open = 0;
    if(sending)
      printf("Connection to client closed.\n");
    else
      printf("Connection closed by client.\n");
    return 1;
  }
  return 0;
}
/*
  name: send_message
  arguments: fd - filedescriptor where we are sending message
             handle - name of host handle to prepend before sending message
	     handle_length - length of handle not including null byte
	     msg - message to send
	     msg_length - length of message to send not including null byte
  description: Concatanate host handle + "> " + msg, then send this message to
             host previously bound to file descriptor fd.
 */
void send_message(int fd, const char* handle, int handle_length, const char* msg, int msg_length, int terminate){
  char* temp = malloc((handle_length + msg_length + 6) * sizeof(char));
  char* mid = "> ";
  char* end = "\n.\n";

  memset(temp, '\0', (handle_length + 6) * sizeof(char));
  if(!terminate){
    strcpy(temp, handle);
    strcat(temp, mid);
  }
  strcat(temp, msg);
  strcat(temp, end);
  temp[handle_length + msg_length + 5] = '\0';
  send(fd, temp, strlen(temp), 0);

  free(temp);
}

/*
  name: receive_message
  arguments: fd - filedescriptor where we are sending message
             buffer - char** address of pointer where max buffer size has alraedy been allocated	     
  description: Make sure buffer is filled with null bytes, then receive message.
 */
void receive_message(int fd, char** buffer){
  char* temp = "\n.\n";
  int num_bytes = 0;

  memset(*buffer, '\0', MESSAGE_SIZE_RECV + 1);
  do{
    num_bytes += recv(fd, *buffer, MESSAGE_SIZE_RECV, 0);
  }while(strstr(*buffer, temp) == NULL && num_bytes < MESSAGE_SIZE_RECV);

  /* ensure last byte is still nullbyte, then strip off "\n.\n" delimiter */
  (*buffer)[MESSAGE_SIZE_RECV] = '\0';
  (*buffer)[num_bytes] = (*buffer)[num_bytes - 1] = (*buffer)[num_bytes - 2] 
    = (*buffer)[num_bytes - 3] = (*buffer)[num_bytes - 4] = '\0';
  printf("%s\n", *buffer);
}

/*
  name: set_host_name
  arguments: host_name is the address the cstring location to store user input host name
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
