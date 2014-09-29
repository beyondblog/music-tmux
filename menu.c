/*
 * =====================================================================================
 *
 *       Filename:  menu.c
 *
 *    Description:  菜单
 *
 *        Version:  1.0
 *        Created:  2014/07/24 21时20分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YangBing
 *   Organization:
 *
 * =====================================================================================
 */

#include <signal.h>
#include <errno.h>
#include "menu.h"

#define TEL_IAC "\377"
#define TEL_WILL "\373"
#define TEL_ECHO "\001"

#define MUSIC_LIST_COUNT 10

enum MENU_TYPE {
    MAIN_MENU,
    MUSIC_LIST,
    SETTING,
    HELP
};

struct winsize window_size;
static enum MENU_TYPE menu_type;
static int cursor;
static char *baseMenu[] = {
    "列表",
    "设置",
    "帮助",
    "关于"
};

static void show_music_list();

static void sig_quit(int signo)
{
    fprintf(stdout, "sigquit:%d", signo);
}

void init_menu()
{
    hide_cursor();
    if(signal(SIGINT, sig_quit) == SIG_ERR)
        fprintf(stdout, "can't sigquit");
    cursor = 0;
    cls();
    menu_type = MAIN_MENU;
    if (ioctl(0, TIOCGWINSZ, &window_size) != 0) {
        fprintf(stderr, "TIOCGWINSZ:%s/n", strerror(errno));
        exit(1);
    }
}

int show_menu(char key)
{
    int i, size;
    int row = window_size.ws_row;
    int col = window_size.ws_col;


    size = sizeof(baseMenu) / sizeof(baseMenu[0]);
    cls();


    for (i = 0 ; i < (row / 2) - 10; i++)
        fprintf(stdout, "\n");

    if(key > 'A' && key <= 'Z') key += 32;

    switch(key) {
    case 'k': {
        cursor--;
        break;
    }
    case 'j': {
        cursor++;
        break;
    }
    case 'l':
    {
        if(menu_type == MAIN_MENU && cursor == 0)
        {
            show_music_list();
            menu_type = MUSIC_LIST;
        } else if( menu_type == MUSIC_LIST) {
            music_file* music = get_index_music_path(cursor);
            if(music != NULL) {
                play_music_file(music);
            }
        }
        break;
    }
    case 'h': {
        if(menu_type > 0)
            menu_type --;
        cursor = 0;
        break;
    }
    case 'q': {
        return 1;
        break;
    }
    default:
        break;
    }

    switch(menu_type) {
    case MAIN_MENU: {
        if(cursor >= size)
            cursor = 0;
        else if(cursor < 0)
            cursor = size - 1;

        char menu_item[1024];
        for(i = 0 ; i < size; i++) {
            if(i == cursor) {
                snprintf(menu_item, sizeof(menu_item), "->%d.%s\n", i + 1, baseMenu[i]);
                set_cursor_point(col / 2 - 6, (row / 3) + i);
                textcolor(menu_item, TEXT_DARKGREEN);
            }
            else {
                snprintf(menu_item, sizeof(menu_item), "  %d.%s\n", i + 1, baseMenu[i]);
                set_cursor_point(col / 2 - 6, (row / 3) + i);
                fprintf(stdout, menu_item);
            }
        }

        set_cursor_point(col - 20, (row / 3) + 10);
        fprintf(stdout, "退出请按'q'");
        break;
    }
    case MUSIC_LIST: {
        show_music_list();
        break;
    }
    }

    music_file* music = get_current_music();

    if (music != NULL) {

        set_cursor_point(0, row);
        backgroud_color(BACKGRPUND_GREEN);
        for(int i = 0; i < col / 2; i++)
            printf("  ");
        char current_info[1024];
        snprintf(current_info, sizeof(current_info), "当前播放:%s", music->filename);
        set_cursor_point(col / 2 - (strlen(current_info) / 2), row);
        textcolor(current_info, TEXT_WHITE);
        fprintf(stdout, "\033[1;0H");
        fprintf(stdout, "\033[0m");
    }

    return 0;
}

void add_menu_item(char *menu_name)
{

}

static void show_music_list()
{
    cls();
    print_library_music(&cursor, MUSIC_LIST_COUNT);
}



