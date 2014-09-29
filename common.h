
#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>


#define TEXT_BLACK     "\033[30m"
#define TEXT_RED       "\033[31m"
#define TEXT_GREEN     "\033[32m"
#define TEXT_YELLOW    "\033[33m"
#define TEXT_BLUE      "\033[34m"
#define TEXT_PURPLE    "\033[35m"
#define TEXT_DARKGREEN "\033[36m"
#define TEXT_WHITE     "\033[37m"

#define COLOR_RESET   "\033[0m"
#define CURSOR_HIDE   "\33[?25l" 
#define CURSOR_SHOW   "\33[?25h"
#define CURSOR_SAVE   "\33[s"
#define CURSOR_LOAD   "\33[u"


#define BACKGRPUND_BLACK        "\033[40m"
#define BACKGRPUND_RED          "\033[41m"
#define BACKGRPUND_GREEN        "\033[42m"
#define BACKGRPUND_YELLOW       "\033[43m"
#define BACKGRPUND_BLUE         "\033[44m"
#define BACKGRPUND_PURPLE       "\033[45m"
#define BACKGRPUND_DARKGREEN    "\033[46m"
#define BACKGRPUND_WHITE        "\033[47m"



int str_ends_with(const char * str, const char * suffix);

void cls(void);

int getch(void);

int hide_cursor(void);

int show_cursor(void);

void save_cursor();

void load_cursor();

void set_cursor_point(int x, int y);

void backgroud_color(char* color);

void textcolor(char *text, char* color);

void color_reset();

void printf_red(char *str);

void printf_blue(char *str);

#endif

