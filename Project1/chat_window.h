#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>

static void finish(int);
WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string);
void get_user_input(WINDOW*, char**);
void print_to_chat(WINDOW*, WINDOW*, char*, char*, int*);
void initialize_window(WINDOW**, WINDOW**);
#endif
