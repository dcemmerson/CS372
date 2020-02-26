#define BACKLOG 10
#define MESSAGE_SIZE 1000
#define HOST_NAME_LENGTH 10
#define SERVERPORT_DEFAULT "12001"
#define MESSAGE_SIZE_RECV 500
#define END "\n.\n"
#define MORE "\n...\n"
#define DIRECTORY_LENGTH 512
#define SEND_FILE_SIZE 500

void open_sock(char**, char*, struct addrinfo**);
void bind_sock(int*, struct addrinfo**);
void server_listen(int, char*);
int accept_connection(int, int*, struct addrinfo*, char*);
void receive_message(int, char**);
void send_user_request(int, char**, char**, char*);
void send_dir_contents(int, char**, char*);
void send_message(int, const char*, int);
void send_file(int, const char*, char**, char*);
void connect_sock(int*, struct addrinfo**);
int dir_contains_file(const char*);
void get_dir_contents(char*);
void increment_strint(char**, char*);
