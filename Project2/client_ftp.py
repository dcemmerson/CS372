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
from stoppable_thread import StoppableListenThread
import signal
import threading

def extract_arguments(argv):
    if len(argv) == 3:
        return argv[1], int(argv[2])
    elif len(argv) == 2:
        return "flip1.engr.oregonstate.edu", int(argv[1])
    else:
        return "flip1.engr.oregonstate.edu", 12000

    

def main():
#    lock = threading.RLock()
    host, port = extract_arguments(sys.argv)
    sock_ctrl = open_socket(port, host, False)
    sock_ft = open_socket(port + 1)
    listen_thread = StoppableListenThread(receive_file, 
                                          "listen_thread", 
                                          (sock_ft, port + 1), {})
    listen_thread.assign_sock(sock_ft)
    listen_thread.start()

    while(1):

        userinput = raw_input(">")
        send_message(sock_ctrl, userinput)

        response = recv_message(sock_ctrl)

        if response == "ACK_pwd":
 
#            receive_file(sock_ft, port + 1)
        elif response == "ACK_file":
            receive_file(sock_ft, port + 1)
        else:
            print(response)




if __name__ == "__main__":
    main()

