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

#define KEYUP    0
#define KEYDOWN  1
#define KEYLEFT  2
#define KEYRIGHT 3
#define KEYQUIT  4

#define MUSIC_LIST_COUNT 10

enum MENU_TYPE {
    MAIN_MENU,
    MUSIC_LIST,
    SETTING,
    HELP
};

struct winsize window_size;
static enum MENU_TYPE menu_type;
static int cursor, key_up, key_down, key_right, key_left;
static char *base_menu[] = {
    "列表",
    "设置",
    "帮助",
};


static void show_music_list();

static void show_setting_menu();

static void show_help_info();

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
        //exit(1);
    }
}

/*
 * 设置操作按键
 */
void init_handle_key(int up, int down, int left, int right)
{
    key_up = up;
    key_down = down;
    key_left = left;
    key_right = right;
}

int show_menu(char userKey)
{
    int i, size;
    int row = window_size.ws_row;
    int col = window_size.ws_col;
    int key = -1;

    //if(userKey > 'A' && userKey <= 'Z') userKey += 32;

    if(userKey == key_up)
        key = KEYUP;
    else if(userKey == key_down)
        key = KEYDOWN;
    else if(userKey == key_left)
        key = KEYLEFT;
    else if(userKey == key_right)
        key = KEYRIGHT;
    else if(userKey == 'q')
        key = KEYQUIT;

    size = sizeof(base_menu) / sizeof(base_menu[0]);
    cls();


    for (i = 0 ; i < (row / 2) - 10; i++)
        fprintf(stdout, "\n");

    switch(key) {

    case KEYUP: {
        cursor--;
        break;
    }
    case KEYDOWN: {
        cursor++;
        break;
    }
    case KEYRIGHT:
    {
        switch(menu_type)
        {
        case  MAIN_MENU: {

            /*
            if(cursor == 0) {
                menu_type = MUSIC_LIST;
            } else if (cursor == 1) {
                menu_type = SETTING;
            }
            */

            menu_type = cursor + 1;
            break;
        }
        case MUSIC_LIST:
        {
            music_file* music = get_index_music_path(cursor);
            if(music != NULL) {
                play_music_file(music);
            }
            break;
        }
        case SETTING:
        {
        }
        default:
            break;
        }
        break;
    }
    case KEYLEFT: {
        menu_type = MAIN_MENU;
        cursor = 0;
        break;
    }
    case KEYQUIT: {
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
                snprintf(menu_item, sizeof(menu_item), "->%d.%s\n", i + 1, base_menu[i]);
                set_cursor_point(col / 2 - 6, (row / 3) + i);
                textcolor(menu_item, TEXT_DARKGREEN);
            }
            else {
                snprintf(menu_item, sizeof(menu_item), "  %d.%s\n", i + 1, base_menu[i]);
                set_cursor_point(col / 2 - 6, (row / 3) + i);
                fprintf(stdout, "%s", menu_item);
            }
        }
        break;
    }
    case MUSIC_LIST: {
        show_music_list();
        break;
    }
    case  SETTING: {
        show_setting_menu();
        break;
    }
    case HELP: {
        show_help_info();
        break;
    }
    default:
        break;
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
        fprintf(stdout, "\033[1;0H");//设置到开始
        fprintf(stdout, "\033[0m");//reset

    }

    return 0;
}

void add_menu_item(char *menu_name)
{

}

void print_center_string(char *str)
{
    int row = window_size.ws_row;
    int col = window_size.ws_col;
    set_cursor_point(col / 2 - (strlen(str) / 2), row / 2);
    fprintf(stdout, "%s", str);
}

static void show_setting_menu()
{
    int row = window_size.ws_row;
    int col = window_size.ws_col;
    int i = 0;

    char help_array[][50] = {
        "系统设置",
        "1.重新扫描歌曲",
        "2.歌曲路径设置",
    };

    int len = dim(help_array);

    for( i = 0 ; i < len; i++) {
        set_cursor_point(col / 2 - 20, row / 2 - 10 + i);
        fprintf(stdout, "%s\n", help_array[i]);
    }

	//绿色填充最后一行
    set_cursor_point(0, row);
    backgroud_color(BACKGRPUND_BLUE);
    for(int i = 0; i < col / 2; i++)
        printf("  ");

    char current_info[1024];
    snprintf(current_info, sizeof(current_info), "请输入1-2,进行操作");
    set_cursor_point(col / 2 - (strlen(current_info) / 2), row);
    textcolor(current_info, TEXT_WHITE);
    fprintf(stdout, "\033[0m");//reset


}

static void show_music_list()
{
    cls();
    print_library_music(&cursor, MUSIC_LIST_COUNT);
}

static void show_help_info()
{
    cls();
    int row = window_size.ws_row;
    int col = window_size.ws_col;
    int i = 0;

    char help_array[][50] = {
        "终端音乐播放器: v1.0",
        "Author: Yang Bing",
        "Email: beyondblog@outlook.com",
        "github: https://github.com/beyondblog",
        "blog: https://beyondblog.github.io",
        "source: https://github.com/beyondblog/music-tmux",
    };

    int len = dim(help_array);

    for( i = 0 ; i < len; i++) {
        set_cursor_point(col / 2 - 20, row / 2 - 10 + i);
        fprintf(stdout, "%s\n", help_array[i]);
    }

    //绿色填充最后一行
    set_cursor_point(0, row);
    backgroud_color(BACKGRPUND_BLUE);
    for(int i = 0; i < col / 2; i++)
        printf("  ");

    char current_info[1024];
    snprintf(current_info, sizeof(current_info), "退出请按:q  音量增大:[  音量减小:]  下一首:n  上一首:p");
    set_cursor_point(col / 2 - (strlen(current_info) / 2), row);
    textcolor(current_info, TEXT_WHITE);
    fprintf(stdout, "\033[0m");//reset
}



