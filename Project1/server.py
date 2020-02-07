#  author: Dane Emmerson
#  due date: 02/09/2020
#  description: server portion of client-server program.
#               See README for additional details.
#  sources: https://docs.python.org/2/library/socket.html#socket.socket

import socket
import sys
import curses
import threading
from chat_window import *


MID = "> "
END = "\n.\n"
MESSAGE_SIZE_RECV = 1000

# ask user what to call this host
def get_host_name():
    return input("Enter host_name: ")

def main():

    host_name = raw_input("Enter handle for host: ")

    if len(sys.argv) == 2 :
        PORT = sys.argv[1]
    else :
        PORT = "12000"
        
    HOST = "flip1.engr.oregonstate.edu"

    #open socket at flip1.engr.oregonstate.edu
    af, socktype, proto, canonname, sa = socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE)[0]

    #initialize windows for curses
    win, win_chat, win_type = initialize_window()

    try:
        sock = socket.socket(af, socktype, proto);
    except socket.error as msg:
        sock = None
    try:
        sock.bind(sa)
        sock.listen(10)
    except socket.error as msg:
        sock.close
        sock = None

    if sock == None:
        curses.endwin()
        print("unable to open socket on port " + PORT)

        sys.exit(1)
    
    while(1):
        #why cant you pass a primitive by reference in python?!?
        lines_printed = [1]
        lines_printed[0] = 2

        #make sure entire chat area is clear before user connects
        #then print message to user that server is listening
        win_chat.clear()
        win_chat.box(0, 0)
        print_to_chat(win_type, win_chat, None,
                      "Listening on " + HOST + " at port " + str(PORT) 
                      + " and ready to accept connection!", lines_printed)
        conn, addr = sock.accept()
        #upon receiving connection, display message to user with information about
        #connected host ip address
        print_to_chat(win_type, win_chat, None,
                        "Connection received from " + str(addr), lines_printed)

        open = [1]
        open[0] = True

        #Start chat exchange. Create two threads: one for user to send messages and
        #second thread to listen for the other host sending messages
        s_thread = threading.Thread(target=send_thread, 
                                          args=(conn, win_type, win_chat, host_name, lines_printed, open,))
        r_thread = threading.Thread(target=rec_thread, 
                                          args=(conn, win_type, win_chat, lines_printed, open,))
        s_thread.start()
        r_thread.start()

        #wait for both threads to exit before allowing program to exit.
        #this basically forces the user to use CTRL-C to exit server program
        r_thread.join()
        s_thread.join()

    curses.endwin()
    exit(0)
#name: rec_thread
#description: called by thread listening for other host to send messages.
#             Receives messages from other host and displays to user.
#             Checks to see if \quit message was sent to end connection.
def rec_thread(conn, win_type, win_chat, lines_printed, open):
    buffer_receive = ""

    while(open[0]):
        buffer_receive = receive_message(conn, win_type, win_chat, lines_printed)
        terminate = check_connection_closed(buffer_receive, False, open, win_type, win_chat, lines_printed)
        #        send_message(conn, host_name, buffer_send, terminate)

    if(conn):
        conn.close()
        conn = None
        print_to_chat(win_type, win_chat, None, "Press [enter] to continue\n", lines_printed)

#name: send_thread
#description: called by thread waiting for user to send messages.
#             Sends messages to other host.
#             Checks to see if \quit message was sent to end connection.
def send_thread(conn, win_type, win_chat, host_name, lines_printed, open):
    conn.send("Connected!" + END)
    while(open[0]):
        buffer_send = get_user_input(win_type)
        print_to_chat(win_type, win_chat, host_name, buffer_send, lines_printed)
        terminate = check_connection_closed(buffer_send, False, open, win_type, win_chat, lines_printed)
        send_message(conn, host_name, buffer_send, terminate)
    
    if(conn): 
        conn.close()
        conn = None

#name: send_message
#description: does the bulk of the work of sending an actual message to other
#             host on conn socket. Loops until entire message is sent
def send_message(conn, handle, msg, terminate):
    sent_bytes = 0
    temp = ""
    if(not terminate):
        temp += handle
        temp += MID
    temp += msg
    temp += END
    while(sent_bytes < len(temp)): 
        sent_bytes += conn.send(temp)

#name: receive_message
#description: does the bulk of the work of receiving an actual message from other
#             host on conn socket. Loops until entire message is received,
#             which is signaled by "\n.\n"
def receive_message(conn, win_type, win_chat, lines_printed):
    buffer = ""
    buffer += conn.recv(MESSAGE_SIZE_RECV)
    end_index = buffer.find(END)
    while(end_index == -1):
        buffer += conn.recv(MESSAGE_SIZE_RECV)
        end_index = buffer.find(END)
    print_to_chat(win_type, win_chat, None, buffer.rstrip(END), lines_printed)
    return buffer.rstrip(END)

#name: check_connection_closed
#description: checks to see if any sent/received messages contain the "\quit"
#             text which indicates time to close connection. Also sets open
#             flag appropriately so that calling methods can react appropriately
def check_connection_closed(buffer, sending, open, win_type, win_chat, lines_printed):
    if buffer == "\\quit" or buffer == "\\quit\n":
        win_chat.clear()
        lines_printed[0] = 2
        if(sending):
            print_to_chat(win_type, win_chat, None, "Connection to client closed.", lines_printed)
        else:
            print_to_chat(win_type, win_chat, None, "Connection closed by client.", lines_printed)
        
        open[0] = False
        return True
        
    return False

        
if __name__ == "__main__" :
    main()
