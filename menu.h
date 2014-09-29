#ifndef MENU_H
#define MENU_H

#include<stdio.h>
#include <sys/ioctl.h> 

#include "common.h"
#include "play_list.h"

void init_menu();

int show_menu(char key);

void add_menu_item(char *menu_name);

#endif
