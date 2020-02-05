#sources: https://docs.python.org/2/library/socket.html#socket.socket
import socket
import sys

MID = "> "
END = "\n.\n"
MESSAGE_SIZE_RECV = 1000

def get_host_name():
    return input("Enter host_name: ")

def main():

    host_name = raw_input("Enter handle for host: ")

    if len(sys.argv) > 1 :
        PORT = argv[1]
    else : 
        PORT = raw_input("Enter port number to run server from: ")

    HOST = "flip1.engr.oregonstate.edu"

    af, socktype, proto, canonname, sa = socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE)[0]

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
        print("unable to open socket on port " + PORT)
        sys.exit(1)
    
    while(1):
        print("Listening and ready to accept connection!")
        conn, addr = sock.accept()
        print("Connection received from", addr)
        conn.send("Connected!" + END)
        open = True

        buffer_receive = ""
        while(open):
            buffer_receive = receive_message(conn)
            if(check_connection_closed(buffer_receive, False, open) == False):
                sys.stdout.write(host_name + "> ")
                buffer_send = raw_input()
                terminate = check_connection_closed(buffer_send, True, open)
                send_message(conn, host_name, buffer_send, terminate)
                
                if terminate == True:
                    conn.close()
                    open = False
            else:
                conn.close()
                open = False
                

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
    
def receive_message(conn):
    buffer = ""
    buffer += conn.recv(MESSAGE_SIZE_RECV)
    end_index = buffer.find(END)
    while(end_index == -1):
        buffer += conn.recv(MESSAGE_SIZE_RECV)
        end_index = buffer.find(END)
    print(buffer.rstrip(END))
    return buffer.rstrip(END)

def check_connection_closed(buffer, sending, open):
    if buffer == "\\quit" or buffer == "\\quit\n":
        if(sending):
            print("Connection to client closed.")
        else:
            print("Connection closed by client")
        return True
    return False

        
if __name__ == "__main__" :
    main()
