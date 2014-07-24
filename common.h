
#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>



int str_ends_with(const char * str, const char * suffix);

void cls(void);

int getch(void);

#endif

