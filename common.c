/*
 * =====================================================================================
 *
 *       Filename:  common.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2014/07/23 22时04分45秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YangBing (),
 *   Organization:
 *
 * =====================================================================================
 */
#include "common.h"

#define CSI "\e["

int str_ends_with(const char * str, const char * suffix) {
    if(str == NULL || suffix == NULL)
        return 0;

    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if(suffix_len > str_len)
        return 0;

    return 0 == strncmp(str + str_len - suffix_len, suffix, suffix_len);
}

void cls(void)
{
    printf("\e[1;1H\e[2J");
}


int getch(void)
{
    int c = 0;
    struct termios org_opts, new_opts;
    int res = 0;
    //-----  store old settings -----------
    res = tcgetattr(STDIN_FILENO, &org_opts);
    assert(res == 0);
    //---- set new terminal parms --------
    memcpy(&new_opts, &org_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    c = getchar();
    //------  restore old settings ---------
    res = tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
    assert(res == 0);
    return c;
}

int hide_cursor(void)
{
    fputs(CSI "?25l", stdout);
    return 0;
}

int show_cursor(void)
{
    fputs(CSI "?25h", stdout);
    return 0;
}


/*
void printf_red(char *str)
{
	fprintf(stdout,KRED "%s" RESET, str);
}

void printf_blue(char *str)
{
	fprintf(stdout,KBLU "%s" RESET, str);
}

*/

void textcolor(char *text, char* color) {
    fprintf(stdout, "%s%s" COLOR_RESET, color, text);
}

void backgroud_color(char* color) {
    fprintf(stdout, "%s", color);
}

void color_reset() {
    fprintf(stdout, "%s", COLOR_RESET);
}


void set_cursor_point(int x, int y) {
    fprintf(stdout, "\033[%d;%dH",  y, x);
}

void save_cursor() {
    fprintf(stdout, CURSOR_SAVE);
}

void load_cursor() {
    fprintf(stdout, CURSOR_LOAD);
}








