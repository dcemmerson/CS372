# Client-Server, Terminal to Terminal Chat
## About
This is an implementation of a multithreaded, client-server chat program.The client program is written in C while the server program is
written in python. Messages can be exchanged between the two simply
by typing into the command line in any order! The chat server is multithreaded and messages can be sent in any order. 

---

## How to Run

A connection must be established first. To establish a connection, start the server program, enter a name for the sever host. At this point curses will take over and open a new window. The server is now listening, as indicated by message on screen. Now it is safe to run "client". Client will prompt for a host name, followed by starting up a new curses window and connecting to server. At this point, either client or server hosts can start sending messages, and as many as that host wants.

### Server
To run the server, simply use "python chatserv.py [port]" where both port is optional.
For example: 
python chatserv.py		#will attempt to listen on flip1.engr.oregonstate.edu::12000
python chatserv.py 12001	#will attempt to listen on flip1.engr.oregonstate.edu::12001

### Client
To compile the chatclient program, using the provided makefile you may simply type
"make" into command line. This will output a file with -x permissions called
"chatclient" that can then be run by enter "./chatclient [host] [port]" into command line
if cwd contains file.
For example:
chatclient						//attempt to connect to flip1.engr.oregonstate.edu::12000
chatclient 12001			     	     	//attempt to connect to flip1.engr.oregonstate.edu::12001
chatclient flip2.engr.oregonstate.edu 12002	     	//attempt to connect to flip2.engr.oregonstate.edu::12002

Typing "\quit" into either server or client while connected will terminate connection. The client
program will exit but the server program will continue listening on the same port for
additional connections. To exit the server program, you must send SIGINT - CTRL-C.

---

## Notes
IMPORTANT: Server is hardcoded to run on flip1.engr.oregonstate.edu. Trying to unzip/
	   compile/run elsewhere will probably not work without a little bit of 
	   tweaking.
