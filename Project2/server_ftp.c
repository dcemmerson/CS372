/*
  author: Dane Emmerson
  Due Date: 3/1/2020
  Description: Server side part of program to implement basic ftp
               for transfer of text file to client program. See
	       README for more details.
  sources: https://beej.us/guide/bgnet/html/#system-calls-or-bust
           https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>

#include "server_ftp.h"


/* name: get_dir_contents
   preconditions: *dir_cont has been allocated and memset to \0
   postconditions: *dir_cont contains cstring of directory contents
   description: places directory contents as string in dir_cont
 */
void get_dir_contents(char* dir_cont){
  DIR* curr_dir;
  struct dirent* ent;
  
  if((curr_dir = opendir("."))){
    while((ent = readdir(curr_dir))){
      strcat(dir_cont, ent->d_name);
      strcat(dir_cont, " ");
    }
    closedir(curr_dir);
  }
  else{
    perror("Error: unable to open cwd\n");
    return;
  }
}

/* name: dir_contains_file
   preconditions: filename contains cstring of file to search dir for
   postconditions: directory was searched for filename
   description: Use strtok to tokenize string returned by getcwd() and check if
                any tokens match filename
 */
int dir_contains_file(const char* filename){
  DIR* curr_dir;
  struct dirent* ent;
  if((curr_dir = opendir("."))){
    while((ent = readdir(curr_dir))){
      if(strcmp(filename, ent->d_name) == 0) return 0;
    }
  }
  return -1;
}

/* name: send_file
   preconditions: fd is an open socket
                  filename contains cstring
   postconditions: attempt was made to send file with filename in cwd
   description: send_file first checks to see if requested file is valid filename
                contained in cwd. If so, the filename is sent to host, followed by
		reading in the contents of the file, 500 bytes at a time, and sending
		to host until entire file has been sent.
 */
void send_file(int fd, const char* filename, char** argv, char* ip4){
  char read_buffer[SEND_FILE_SIZE + 1], temp_filename[DIRECTORY_LENGTH + 1];
  char *msg_error = "Error: file not found", *msg_ack = "ACK_file";
  struct addrinfo *serverinfo;
  int fd_send, fd_read;
  size_t count = SEND_FILE_SIZE * sizeof(char);
  memset(read_buffer, '\0', sizeof(char) * (SEND_FILE_SIZE + 1));
  memset(temp_filename, '\0', sizeof(char) * (DIRECTORY_LENGTH + 1));
  strcpy(temp_filename, filename);
  strcat(temp_filename, MORE);

  if(dir_contains_file(filename) == -1){
    send_message(fd, msg_error, 1);
    return;
  }
  
  open_sock(argv, ip4, &serverinfo);
  connect_sock(&fd_send, &serverinfo);

  /* send ACK to ctrl socket, then send the name of the file, followed 
     by send the conents of the file to the file transfer socket */
  send_message(fd, msg_ack, 1);
  send_message(fd_send, temp_filename, 0);

  fd_read = open(filename, O_RDONLY);
  while(read(fd_read, read_buffer, count)){
    send_message(fd_send, read_buffer, 0);
    memset(read_buffer, '\0', sizeof(char) * (SEND_FILE_SIZE + 1));
  }
  send_message(fd_send, "", 1);
  /*  close(fd_send);*/
  close(fd_read);
}

/* name: connect_sock
   preconditions: fd is not connected fd
                  serverinfo has been filled in by call to open_sock
   postconditions: opened socket connection
   description: Use server info obtained in getaddrinfo call and try connecting to 
                each ai_next in linked list until one works or returns invalid 
 */
void connect_sock(int* fd, struct addrinfo** serverinfo){
  struct addrinfo* p;
  int connected_status;
  for(p = (*serverinfo); p != NULL; p = p->ai_next) {
    if ((*fd = socket(p->ai_family, p->ai_socktype,
		      p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
    printf("now call connect\n");
    if ((connected_status = connect(*fd, p->ai_addr, p->ai_addrlen)) == -1) {
      close(*fd);
      perror("client: connect");
      continue;
    }
    break;
  }

  printf("connected status = %d\n", connected_status);
}

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
  char* error_msg = "Error: invalid request";
  char* ack_pwd = "ACK_pwd";

  if(strcmp(parsed_buffer[0], "-l") == 0){
    send_message(fd, ack_pwd, 1);
    send_dir_contents(fd, argv, ip4);
  }
  else if(strcmp(parsed_buffer[0], "-g") == 0){
    send_file(fd, parsed_buffer[1], argv, ip4);
  }
  else{ /*we did not receive a valid command from client. notify client */
    send_message(fd, error_msg, 1);
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
  struct addrinfo *serverinfo;
  int fd_send;

  memset(pwd, '\0', sizeof(char) * DIRECTORY_LENGTH);
  
  get_dir_contents(pwd);
  /*printf("open sock to: %s:%s\n", ip4*/
  open_sock(argv, ip4, &serverinfo);
  
  connect_sock(&fd_send, &serverinfo);
  printf("connected to clinet - send next\n%s\n",pwd);
  send_message(fd_send, pwd, 1);
  printf("sent, now close sock\n");
  /*  close(fd_send);*/
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
void send_message(int fd, const char* msg, int append_end){
  char* temp = malloc((strlen(msg) + strlen(END) + 1) * sizeof(char));
  int num_bytes_sent = 0;
  memset(temp, '\0', (strlen(msg) + strlen(END) + 1) * sizeof(char));

  strcpy(temp, msg);
  if(append_end == 1) strcat(temp, END);

  while(num_bytes_sent < strlen(temp)){
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
void open_sock(char** argv, char* host, struct addrinfo** serverinfo){
  struct addrinfo hints;
  int status;
  char temp[12];
  char* portstr = temp;

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
  else{ /* then we are opening connection with client who is listening */
    increment_strint(&portstr, argv[1]);
    printf("trying to open sock to %s:%s\n", host, portstr);
    if((status = getaddrinfo("flip1.engr.oregonstate.edu", "12001", &hints, serverinfo)) != 0){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
  } 
}
/* name: bind_sock
   preconditions:
   postconditions:
   description:

*/
void bind_sock(int* sockfd, struct addrinfo** serverinfo){
  int yes = 1;
  struct addrinfo* p;
  for(p = *serverinfo; p != NULL; p = p->ai_next) {
    if((*sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      perror("server: socket\n");
      continue;
    }
    
    if(setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		  sizeof(int)) == -1) {
      perror("setsockopt\n");
      exit(1);
    }
    
    if(bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("server: bind\n");
      continue;
    }
    
    break;
  }
  
  /*  if((*sockfd = socket((*serverinfo)->ai_family, (*serverinfo)->ai_socktype, (*serverinfo)->ai_protocol)) == -1){
    fprintf(stderr, "socket error: %s\n", gai_strerror(*sockfd));
    exit(1);
  }
  */
  freeaddrinfo(*serverinfo);
  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

}
/* name increment_strint
   preconditions: dest has enough space allocated to store int port num as cstring representation
   postconditions: dest contains port num int(source) + 1 in cstring representation
   descriptionn: take in a cstring port num in source that represents an int, increment that int
   and store in destination
*/   
void increment_strint(char** dest, char* source){
  int counter = 0, num = atoi(source), temp;
  memset(*dest, '\0', 12 * sizeof(char));
  num++;
  while(num > 0){
    temp = num % 10;
    num /= 10;
    (*dest)[10 - counter] = temp + 48;
    counter++;
  }
  /* now most *dest to point to the beginning of cstring */
  while(**dest == '\0'){
    (*dest)++;
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
