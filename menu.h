#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#include <pwd.h>

#include "common.h"
#include "play_list.h"

void init_menu();

void init_handle_key(int up, int down, int left, int right);

int show_menu(char userKey);

void add_menu_item(char *menu_name);

/*
 * 居中显示字符串
 */
void print_center_string(char *str);

#endif
