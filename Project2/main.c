/*
  author: Dane Emmerson
  Due Date: 3/1/2020
  Description: Main file for server ftp program. See
	       README for more details.
  sources: https://beej.us/guide/bgnet/html/#system-calls-or-bust
 */
/*
Some "pseudo pseudocode"
  1. server start connection and listening
  2. accept connection from clinet
  3. server waits to recv
  4. parse string from client
  5. switch:
              case -1:
               	      acknowledge client sent valid command
	              open new socket to client (if fail try next 10 ports)
	              send exec(ls)
		      close socket
	      case -g filename:
	                       acknowledge client sent valid command
                               if filename not valid:
			                             send "Error: file not found"
			       else:
			            open new socket to client (if fail try next 10 ports)
			            read file into memory
				    send file to client
				    close socket
	      cause default:
	                    send "Error: invalid command"
      go back to 3
  6. go back to 2
  */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "server_ftp.h"
#include <stdlib.h>
#include <stdio.h>

#define MESSAGE_SIZE_RECV 500
#define STRING_TOKEN_SIZE 256
#define END "\n.\n"


void parse_recvd(char*, char***, int*);
void empty_buffer(char***, int*);

int main(int argc, char** argv){
  char ip4[INET_ADDRSTRLEN];
  char* buffer_receive = NULL;
  char** parsed_buffer = NULL;
  int sockfd, newfd;
  int num_toks = 0;
  struct addrinfo* serverinfo;
  
  open_sock(argv, NULL, &serverinfo);
  bind_sock(&sockfd, &serverinfo);
  server_listen(sockfd, argv[1]);
  
  while(1){
    printf("ready to accept\n");
    if(accept_connection(sockfd, &newfd, serverinfo, ip4) == -1) continue;

    while(1){
      receive_message(newfd, &buffer_receive);
      parse_recvd(buffer_receive, &parsed_buffer, &num_toks);
      send_user_request(newfd, parsed_buffer, argv, ip4);


      empty_buffer(&parsed_buffer, &num_toks);
    }
  }

  free(buffer_receive);
}

/*
  name: parse_recvd
  preconditions: buffer is cstring that contains string received from client
                 *parsed may or may not contain an array of cstrings from previous call to 
                         parse_recvd. Else, parsed should equal NULL
                 *num_toks corresponds to number of cstrings contains in parsed at this time
  postconditions: *parsed has been deallocated, each cstring inside *parsed has been deallocated
  description: Used to free up all memory with cstrings received from recv call from client.
 */
void parse_recvd(char* buffer, char*** parsed, int* num_toks){
  char* strtok_delim = " ";
  char* saveptr = NULL;
  char* temp = NULL;
  char* str;

  for(str = buffer; ; str = NULL){
    temp = strtok_r(str, strtok_delim, &saveptr);

    if(temp == NULL) return;

    if(strlen(temp) > STRING_TOKEN_SIZE)
      temp[STRING_TOKEN_SIZE] = '\0';

    /* check if next token is "\n.\n", which indicates end of recv and no need to
       add this to our parsed string array */
    if(strcmp(temp, END) != 0){
      (*num_toks)++;
      *parsed = realloc(*parsed, (*num_toks) * sizeof(char*));
      (*parsed)[*num_toks - 1] = malloc((strlen(temp) + 1) * sizeof(char));
      memset((*parsed)[*num_toks - 1], '\0', strlen(temp) + 1);
      strcpy((*parsed)[*num_toks - 1], temp);
    }
  }
}

/*
  name: empty_buffer
  preconditions: *parsed may or may not contain an array of cstrings
                 *num_toks corresponds to number of cstrings contains in parsed at this time
  postconditions: *parsed has been deallocated, each cstring inside *parsed has been deallocated
  description: Used to free up all memory with cstrings received from recv call from client.
 */
void empty_buffer(char*** parsed, int* num_toks){
  int i;

  for(i = 0; i < *num_toks; i++){
    free((*parsed)[i]);
    (*parsed)[i] = NULL;
  } 
  free(*parsed);
  *parsed = NULL; 
  *num_toks = 0;
}
