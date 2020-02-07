#  author: Dane Emmerson
#  due date: 02/09/2020
#  description: server helper methods of client-server chat program.
#               See README for additional details.
#  sources: https://docs.python.org/2/library/socket.html#socket.socket


import curses
import sys
import signal


#these should be treated as contants - we set them one type when creating windows
#then they shouldnt be touched again
WIN_CHAT_X_START = 0
WIN_CHAT_Y_START = 0
WIN_CHAT_HEIGHT = 0
WIN_CHAT_WIDTH = 0
WIN_TYPE_X_START = 0
WIN_TYPE_Y_START = 0
WIN_TYPE_HEIGHT = 0
WIN_TYPE_WIDTH = 0

def set_constants(rows, cols):
    global WIN_CHAT_X_START
    global WIN_CHAT_Y_START
    global WIN_CHAT_HEIGHT 
    global WIN_CHAT_WIDTH
    global WIN_TYPE_X_START
    global WIN_TYPE_Y_START
    global WIN_TYPE_HEIGHT
    global WIN_TYPE_WIDTH 

    WIN_CHAT_X_START = cols / 10
    WIN_CHAT_Y_START = 0
    WIN_CHAT_HEIGHT = rows * 9 / 10 
    WIN_CHAT_WIDTH = cols * 4 / 5
    WIN_TYPE_X_START = cols / 10
    WIN_TYPE_Y_START = rows * 9 / 10 + 1
    WIN_TYPE_HEIGHT = 3
    WIN_TYPE_WIDTH = cols * 4 / 5
    
#name: finish
#description: register signal upon receiving sigint
def finish(signal, test):
    curses.endwin()
    print("CTRL-C - exiting...")
    exit(0)

#name: create_newwin
#description: Uses curses to creat new window of heigh/width
def create_newwin(height, width, starty, startx):
    local_win = curses.newwin(height, width, starty, startx)
    local_win.box(0, 0)
    local_win.refresh()
    return local_win

#name: destroy_win
#description: Used to remove borders and get rid of window
def destroy_win(local_win):
    local_win.wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ')
    local_win.wrefresh(local_win)
    local_win.delwin(local_win)


#name: get_user_input
#description: Wait in typing window and return user's input
def get_user_input(win_type):
    win_type.move(1,2)
    return win_type.getstr(1, 2)

#name: print_to_chat
#description: displays string in chat box
def print_to_chat(win_type, win_chat, handle, string, lines_printed):
    if(lines_printed[0] >= (WIN_CHAT_HEIGHT - 4)):
        lines_printed[0] = 2
        win_chat.clear()
            
    if(handle):
        win_chat.addstr(WIN_CHAT_Y_START + lines_printed[0], WIN_CHAT_X_START, handle + "> " + string)
        win_type.clear()
        win_type.box(0, 0)
        win_type.refresh()
        
    else:
        win_chat.addstr(WIN_CHAT_Y_START + lines_printed[0], WIN_CHAT_X_START, string)
    
    win_chat.box(0, 0)
    win_chat.refresh()
    lines_printed[0] = lines_printed[0] + 1
    win_type.move(1,2)
                    

#name: initialize_window
#description: creates new windows for chat and typing
def initialize_window():
    signal.signal(signal.SIGINT, finish)
    win = curses.initscr() # Start curses mode */
    num_rows, num_cols = win.getmaxyx()
    set_constants(num_rows, num_cols)
    curses.cbreak() # Line buffering disabled, Pass on
    # everty thing to me */
        #    keypad(stdscr, TRUE);
    win.addstr("CTRL-C\nto exit")
    win.refresh()
    typ = create_newwin(WIN_TYPE_HEIGHT, WIN_TYPE_WIDTH, WIN_TYPE_Y_START, WIN_TYPE_X_START)
    chat = create_newwin(WIN_CHAT_HEIGHT, WIN_CHAT_WIDTH, WIN_CHAT_Y_START, WIN_CHAT_X_START)
    return win, chat, typ
