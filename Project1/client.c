/*
  author: Dane Emmerson
  due date: 02/09/2020
  description: client portion of client-server program.
               See README for additional details.
  sources: https://beej.us/guide/bgnet/html/#system-calls-or-bust
*/

#include <pthread.h>
#include <signal.h>
#include <ncurses.h>
#include "chat_window.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* these are used to control window sizes */
#define WIN_CHAT_X_START COLS / 10
#define WIN_CHAT_Y_START 0
#define WIN_CHAT_HEIGHT LINES * 9 / 10 
#define WIN_CHAT_WIDTH COLS * 4 / 5
#define WIN_TYPE_X_START COLS / 10
#define WIN_TYPE_Y_START LINES * 9 / 10 + 1
#define WIN_TYPE_HEIGHT 3
#define WIN_TYPE_WIDTH COLS * 4 / 5

#define HOST_NAME_LENGTH 10
#define MESSAGE_SIZE 500
#define MESSAGE_SIZE_RECV 512
#define PORT "12000"

/* structs used for passing arguments to threads */
/* Mildly annoying, huh? */
struct r_thread_info{
  int socketfd;             /* fd of open connection*/
  char** buffer_receive;    /* address of allocated buffer */
  WINDOW* win_type;         /* curses window where user types */
  WINDOW* win_chat;         /* curses window where chat is displayed */
  int* lines_printed;       /* keep track of where to print next line in chat window */
  int* open;                /* controls while loops */
  pthread_t* s_thread;      /* ref to ensure all threads close successfully */
};
struct s_thread_info{
  int socketfd;
  char** buffer_send;
  WINDOW* win_type;         /* curses window where user types */
  WINDOW* win_chat;         /* curses window where chat is displayed */
  char* host_name;
  int* lines_printed;       /* keep track of where to print next line in chat window */
  int* open;                /* controls while loops */
  pthread_t* r_thread;      /* ref to ensure all threads close successfully */
};

static void* rec_thread(void*);
static void* send_thread(void*);
void send_message(int, const char*, int, const char*, int, int);
void set_host_name(char**);
void receive_message(int, char**, WINDOW*, WINDOW*, int*);
int check_connection_closed(char*, int, int*);
void thread_cleanup(void*);

void *get_in_addr(struct sockaddr *sa){
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char** argv){
  char *buffer_send, *buffer_receive, *host_name;
  size_t send_length = MESSAGE_SIZE;
  int status;
  int open = 1, lines_printed = 1;
  int socketfd;
  WINDOW *win_chat, *win_type;
  struct addrinfo hints, *serverinfo, *p;
  pthread_t r_thread, s_thread;
  struct r_thread_info r_thread_args;
  struct s_thread_info s_thread_args;

  /* allocate send/receive buffers and get host name */
  set_host_name(&host_name);
  buffer_send = malloc((MESSAGE_SIZE + 1) * sizeof(char));
  memset(buffer_send, '\0', sizeof(char) * MESSAGE_SIZE_RECV + 1);
  buffer_receive = malloc((MESSAGE_SIZE_RECV + 1) * sizeof(char));
  memset(buffer_receive, '\0', sizeof(char) * MESSAGE_SIZE_RECV + 1);
 
  /* will display new window and create the chat boxes */
  initialize_window(&win_chat, &win_type);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  
  /**************************************************************************/
  /********************* Connection initialization **************************/
  /**************************************************************************/
  printf("argc = %d\n", argc);
  if(argc == 1){
    if((status = getaddrinfo("flip1.engr.oregonstate.edu", PORT, &hints, &serverinfo) != 0)){
      endwin();
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(0);
    }
  }  
  else if(argc == 2){
    if((status = getaddrinfo("flip1.engr.oregonstate.edu", argv[1], &hints, &serverinfo) != 0)){
      endwin();
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(0);
    }
  }
  else{
    if((status = getaddrinfo(argv[1], argv[2], &hints, &serverinfo) != 0)){
      endwin();
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
      exit(1);
    }
    
  }
    
  /* try connecting to each ai_next in linked list until one works or returns invalid */  
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
    endwin();
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }
  /**************************************************************************/
  /********************* End Connection initialization **********************/
  /**************************************************************************/
  
  /* now we will fill up our structs with the arguments needed when creating new threads */
  r_thread_args.socketfd = socketfd;
  r_thread_args.buffer_receive = &buffer_receive;
  r_thread_args.win_type = win_type;
  r_thread_args.win_chat = win_chat;
  r_thread_args.lines_printed = &lines_printed;
  r_thread_args.open = &open;
  r_thread_args.s_thread = &s_thread;
  
  s_thread_args.socketfd = socketfd;
  s_thread_args.buffer_send = &buffer_send;
  s_thread_args.win_type = win_type;
  s_thread_args.win_chat = win_chat;
  s_thread_args.host_name = host_name;
  s_thread_args.lines_printed = &lines_printed;
  s_thread_args.open = &open;
  s_thread_args.r_thread = &r_thread;

  /**************************************************************************
   Bulk of program starts here. Two separate threads created: one to listen
   for the other host to send messages and another to watch the user and send
   the user's messages to the other host
   **************************************************************************/

  pthread_create(&r_thread, NULL, &rec_thread, &r_thread_args);
  pthread_create(&s_thread, NULL, &send_thread, &s_thread_args);

  pthread_join(r_thread, NULL);

  freeaddrinfo(serverinfo); /* deallocate struct */
  free(buffer_send);
  free(buffer_receive);
  return 0;
}

/*
  name: rec_thread
  arguments: args is an r_thread_info struct - see struct def for detail
  description: rec_thread is called by main thread and waits to receive
               input from other host. Upon receiving input from other
	       host, is displayed in chat.
*/
static void* rec_thread(void* args){
  struct r_thread_info* r_thread_args = args;
  int socketfd = r_thread_args->socketfd;
  char** buffer_receive = r_thread_args->buffer_receive;
  WINDOW* win_type = r_thread_args->win_type;
  WINDOW* win_chat = r_thread_args->win_chat;
  int* lines_printed = r_thread_args->lines_printed;
  int* open = r_thread_args->open;
  pthread_t* s_thread = r_thread_args->s_thread;
  
  while(*open){
    receive_message(socketfd, buffer_receive, win_type, win_chat, lines_printed);
    check_connection_closed(*buffer_receive, 0, open);
  }
  close(socketfd);
  pthread_cancel(*s_thread);
}
/*
  name: send_thread
  arguments: args is an s_thread_info struct - see struct for detail
  description: send_thread is called from the main thread and waits
               for user input. Upon user input, string is sent to
	       server
*/
static void* send_thread(void* args){
  struct s_thread_info* s_thread_args = args;
  int socketfd = s_thread_args->socketfd;
  char** buffer_send = s_thread_args->buffer_send;
  WINDOW* win_type = s_thread_args->win_type;
  WINDOW* win_chat = s_thread_args->win_chat;
  char* host_name = s_thread_args->host_name;
  int* lines_printed = s_thread_args->lines_printed;
  int* open = s_thread_args->open;
  pthread_t* r_thread = s_thread_args->r_thread;
   
  while(*open){
      memset(*buffer_send, '\0', MESSAGE_SIZE + 1);
      get_user_input(win_type, buffer_send);
      print_to_chat(win_type, win_chat, host_name, *buffer_send, lines_printed);

      if(check_connection_closed(*buffer_send, 1, open)){
	send_message(socketfd, host_name, strlen(host_name), *buffer_send, strlen(*buffer_send), 1);
	close(socketfd);
      }
      else
	send_message(socketfd, host_name, strlen(host_name), *buffer_send, strlen(*buffer_send), 0);
  }
  pthread_cancel(*r_thread);
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
    endwin();
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
void receive_message(int fd, char** buffer, WINDOW* win_type, WINDOW* win_chat, int* lines_printed){
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
  print_to_chat(win_type, win_chat, NULL, *buffer, lines_printed);
}

/*
  name: set_host_name
  arguments: host_name is the address the cstring location to store user input host name
  description: Take address of location to store host name, prompt user for input
 */
void set_host_name(char** host_name){
  size_t name_length = HOST_NAME_LENGTH;
  
  *host_name = malloc((HOST_NAME_LENGTH + 1) * sizeof(char));
  memset((*host_name), '\0', HOST_NAME_LENGTH + 1);
  /* set a name for the server host */
  printf("Enter a name to be assigned to this host: ");
  getline(host_name, &name_length, stdin);
  (*host_name)[strlen(*host_name) - 1] = '\0';
  (*host_name)[HOST_NAME_LENGTH] = '\0';
}
