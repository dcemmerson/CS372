/*
  author: Dane Emmerson
  due date: 02/09/2020
  description: client helper function for client portion of client-server program.
               See README for additional details.
  sources: https://beej.us/guide/bgnet/html/#system-calls-or-bust
*/

#include "chat_window.h"
#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>

#define WIN_CHAT_X_START COLS / 10
#define WIN_CHAT_Y_START 0
#define WIN_CHAT_HEIGHT LINES * 9 / 10 
#define WIN_CHAT_WIDTH COLS * 4 / 5
#define WIN_TYPE_X_START COLS / 10
#define WIN_TYPE_Y_START LINES * 9 / 10 + 1
#define WIN_TYPE_HEIGHT 3
#define WIN_TYPE_WIDTH COLS * 4 / 5

/*
  name: finish
  description: register sigint handler
 */
static void finish(int sig){
  endwin();
  printf("CTRL-C - exiting...\n");
  exit(0);
}

/*
  name: create_newwin
  description: returns a new curses window of height/width
 */
WINDOW *create_newwin(int height, int width, int starty, int startx){
  WINDOW *local_win;

  local_win = newwin(height, width, starty, startx);
  box(local_win, 0, 0);/* 0, 0 gives default characters 
			 * for the vertical and horizontal
			 * lines*/
  wrefresh(local_win);/* Show that box */

  return local_win;
}

/*
  name: destroy_win
  description: used to destroy a curses window and remove border
 */
void destroy_win(WINDOW *local_win){
  wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  wrefresh(local_win);
  delwin(local_win);
}

/*
  name: get_user_input
  description: obtain user input from typing curses box on screen
 */
void get_user_input(WINDOW* win_type, char** str){
  mvwgetstr(win_type, 1, 2, *str);
}

/*
  name: print_to_char
  description: Paints str onto curses chat window
 */
void print_to_chat(WINDOW* win_type, WINDOW* win_chat, char* handle, char* str, int* lines_printed){
  if(*lines_printed >= (WIN_CHAT_HEIGHT - 4)){
    wclear(win_chat);
    *lines_printed = 2;
  }
  if(handle){
    mvwprintw(win_chat, WIN_CHAT_Y_START + *lines_printed, WIN_CHAT_X_START, "%s> %s\n", handle, str);
    wclear(win_type);
    box(win_type, 0 , 0);
    wrefresh(win_type);
  }
  else{
    mvwprintw(win_chat, WIN_CHAT_Y_START + *lines_printed, WIN_CHAT_X_START, "%s\n", str);
  }
  box(win_chat, 0, 0); 
  wrefresh(win_chat);
  (*lines_printed)++;
}

/*
  name: initialize_window
  description: Creates a chat and typing curses window
 */
void initialize_window(WINDOW** chat, WINDOW** type){
  signal(SIGINT, finish);
  initscr();/* Start curses mode */
  cbreak();/* Line buffering disabled, Pass on
	    * everty thing to me */
  keypad(stdscr, TRUE);/* I need that nifty F1 */

  printw("Press CTRL-C\nto exit.\nOr type\n\\quit");
  refresh();
  *type = create_newwin(WIN_TYPE_HEIGHT, WIN_TYPE_WIDTH, WIN_TYPE_Y_START, WIN_TYPE_X_START);
  *chat = create_newwin(WIN_CHAT_HEIGHT, WIN_CHAT_WIDTH, WIN_CHAT_Y_START, WIN_CHAT_X_START);
}
