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

#define HOST_NAME_LENGTH 10
#define MESSAGE_SIZE 1000
#define MESSAGE_SIZE_RECV 1012
#define PORT "12000"

void send_message(int, const char*, int, const char*, int, int);
void set_host_name(char**);
void receive_message(int, char**);
int check_connection_closed(char*, int, int*);

void *get_in_addr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char** argv){
  char* buffer_send, *buffer_receive, *host_name;
  size_t send_length = MESSAGE_SIZE;
  int status;
  int open = 1;
  int socketfd;
  struct addrinfo hints, *serverinfo, *p;
  char s[INET6_ADDRSTRLEN];

  set_host_name(&host_name);
  buffer_send = malloc((MESSAGE_SIZE + 1) * sizeof(char));
  memset(buffer_send, '\0', MESSAGE_SIZE + 1);
  buffer_receive = malloc((MESSAGE_SIZE_RECV + 1) * sizeof(char));
  memset(buffer_receive, '\0', MESSAGE_SIZE_RECV + 1);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if((status = getaddrinfo("flip1.engr.oregonstate.edu", PORT, &hints, &serverinfo) != 0)){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }
  
  
  for(p = serverinfo; p != NULL; p = p->ai_next) {
    if ((socketfd = socket(p->ai_family, p->ai_socktype,
			   p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
    
    if (connect(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(socketfd);
      perror("client: connect");
      continue;
    }
    
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  while(open){
    memset(buffer_receive, '\0', MESSAGE_SIZE_RECV + 1);
    receive_message(socketfd, &buffer_receive);
    if(!check_connection_closed(buffer_receive, 0, &open)){
      printf("%s> ", host_name);
      memset(buffer_send, '\0', MESSAGE_SIZE + 1);
      getline(&buffer_send, &send_length, stdin);
      if(check_connection_closed(buffer_send, 1, &open)){
	send_message(socketfd, host_name, strlen(host_name), buffer_send, strlen(buffer_send), 1);
	close(socketfd);
      }
      else
	send_message(socketfd, host_name, strlen(host_name), buffer_send, strlen(buffer_send), 0);
    }
    else close(socketfd);
  }
  freeaddrinfo(serverinfo);
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
  /*  printf("str to comp = %s\n", str);*/
  if(strcmp(str,"\\quit\n") == 0 || strcmp(str,"\\quit") == 0){
    *open = 0;
    if(sending)
      printf("Connection to server closed.\n");
    else
      printf("Connection closed by server.\n");
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
    = (*buffer)[num_bytes - 3] = '\0';
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
