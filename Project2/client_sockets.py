'''

sources: https://docs.python.org/2/howto/sockets.html
sources: https://docs.python.org/2/howto/sockets.html
'''

import socket
import sys
import os
import time
#import threading

MORE = "\n...\n"
END = "\n.\n"
MESSAGE_SIZE_RECV = 500

def open_socket(port, host = "flip1.engr.oregonstate.edu", listen = True):
    try: 
        af, socktype, proto, canonname, sa = socket.getaddrinfo(host, port, 
                                                                socket.AF_UNSPEC, 
                                                                socket.SOCK_STREAM, 
                                                                0, socket.AI_PASSIVE)[0]
        sock = socket.socket(af, socktype, proto);
        if(listen == True):
            sock.bind(sa)
            print("host = " + str(host) + ":" + str(port) + " binded")
        else:
           sock.connect((host, port))
            
        return sock

    except socket.error as msg:
        #sock.close()
        sock = None
        print(str(msg) + ": unable to open socket on port " + str(port))
        sys.exit(1)
    except:
        print("Error: cannot open socket on port " + str(port))
        
def socket_listen(sock, port):
    try:
        sock.listen(10)
    except socket.error as msg:
        sock.close()
        sock = None
        print(str(msg) + ": unable to open socket on port " + str(port))
        sys.exit(1)
    except:
        print("Error: unable to listen on port " + str(port))


def receive_file(sock, port):
    socket_listen(sock, port)
    while(1):
        conn, address = sock.accept()
        while(1):
            buffer = ""
            end_index = -1
            while(end_index == -1):
                buffer += conn.recv(MESSAGE_SIZE_RECV)
                end_index = buffer.find(END) or buffer.find(MORE)
            if buffer.find(MORE) != -1:
                buffer = buffer.rstrip(MORE)
                resolve_filename(buffer)
                fd = open(buffer, 'w')
                buffer = ""
                end_index = -1
                while(end_index == -1):
                    buffer = conn.recv(MESSAGE_SIZE_RECV)
                    end_index = buffer.find(END)
                    if end_index != -1:
                        buffer.rstrip(END)
                    fd.write(buffer)
            else:
                print(buffer.rstrip(END))

def accept_sock_ft():





def resolve_filename(filename):
    num = ""
    while(os.path.isfile('./' + file + num)):
        if num == "":
            num = 1
        else:
            num += 1
    if(num > 0):
        filename += str(num)
'''
name: send_message
preconditions: conn is open socket connection to other host
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

'''
name: recv_message
preconditions: conn is open socket connection to other host
               terminate set to true is message is '\quit'
postconditions: msg received from other host and returned
description: waits until we have received entire message from other host. Message being
             received must terminate with "\n.\n"
'''
def recv_message(conn, terminate=False):
    buffer = ""
    buffer += conn.recv(MESSAGE_SIZE_RECV)
    end_index = buffer.find(END)
    while(end_index == -1):
        buffer += conn.recv(MESSAGE_SIZE_RECV)
        end_index = buffer.find(END)
    return buffer.rstrip(END)

