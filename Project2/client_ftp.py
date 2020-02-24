'''
  author: Dane Emmerson
  Due Date: 3/1/2020
  Description: Main file for python client ftp program. See
	       README for more details.
  sources: https://beej.us/guide/bgnet/html/#system-calls-or-bust
           https://docs.python.org/2/howto/sockets.html
           https://docs.python.org/2/howto/sockets.html
Some "pseudo pseudocode"
  1. client opens 2 sockets
     a. one socket to connect to server
     b. another socket to listen on for server to connect. If unable to open port
        keep trying for next 10 port numbers
  2a. client attempts to connect to server on first socket
  3a. obtain input from user to send to server
  4a. recv string from client on same socket
     if recv()ed string is anything other than "\n.\n", the print string
     else, print "Receiving file...please." and " Complete." upon finishing
  
  2b. acccept connection from server
  3b. recv from server, placing text in buffer
  4b. Upon receiving all text, write to file
      if filename exists, append (i) to filename
  5b. close connection
'''

from client_sockets import *
import sys
#import threading

END = "\n.\n"
MESSAGE_SIZE_RECV = 500

def extract_arguments(argv):
    if len(argv) == 3:
        return argv[1], int(argv[2])
    elif len(argv) == 2:
        return "flip1.engr.oregonstate.edu", int(argv[1])
    else:
        return "flip1.engr.oregonstate.edu", 12000

'''
name: send_message
preconditions: conn is open socket connection to other host
               handle is this hosts user handle string
               msg contains message string we want to send to other host
               terminate set to true is message is '\quit'
postconditions: msg sent to other host
description: does the bulk of the work of sending an actual message to other
             host on conn socket. Loops until entire message is sent
'''
def send_message(conn, msg, terminate=False):
    received_bytes = sent_bytes = 0
    msg += END

    #first send msg, then wait to recv response from server
    while(sent_bytes < len(msg)): 
        sent_bytes += conn.send(msg)

    

def main():
    host, port = extract_arguments(sys.argv)
    sock_ft = open_socket(port + 1)
    sock_ctrl = open_socket(port, host)

    while(1):
        userinput = raw_input(">")
        send_message(sock_ctrl, userinput)

if __name__ == "__main__":
    main()

