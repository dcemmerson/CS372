'''

sources: https://docs.python.org/2/howto/sockets.html
sources: https://docs.python.org/2/howto/sockets.html
'''

import socket
import sys

def open_socket(port, host = "localhost"):
    try: 
        af, socktype, proto, canonname, sa = socket.getaddrinfo(host, port, 
                                                                socket.AF_UNSPEC, 
                                                                socket.SOCK_STREAM, 
                                                                0, socket.AI_PASSIVE)[0]
        sock = socket.socket(af, socktype, proto);
        if(host == "localhost"):
            sock.bind(sa)
            sock.listen(10)
        else:
           sock.connect((host, port))
            
        return sock

    except socket.error as msg:
        sock.close
        sock = None
        print(str(msg) + ": unable to open socket on port " + str(port))
        sys.exit(1)
        
def socket_listen(sock):
    try:
        sock.listen(10)
    except socket.error as msg:
        sock.close
        sock = None
        print(str(msg) + ": unable to open socket on port " + str(port))
        sys.exit(1)
