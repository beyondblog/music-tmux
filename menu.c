/*
 * =====================================================================================
 *
 *       Filename:  menu.c
 *
 *    Description:  菜单
 *
 *        Version:  1.0
 *        Created:  2014/07/24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Occam's Razor
 *   Organization:
 *
 * =====================================================================================
 */

#include "menu.h"
#include "config.h"
#include "music_tmux.h"

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
    else if(userKey == '[')
        key = VOLUME_ADD;
    else if(userKey == ']')
        key = VOLUME_DECRE;
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
            menu_type = cursor + 1;
            cursor = 0;
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
            if(cursor == 1) {
                struct passwd *pw = getpwuid(getuid());
                char shell[2048];
                snprintf(shell, sizeof(shell), "vi %s/%s", pw->pw_dir, CONFIG_FILE);
                system(shell);
                hide_cursor();
            } else if (cursor == 0) {
                //reload library
                reload_library();
                print_center_string("音乐库已经重新加载!");
            }
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
    case VOLUME_ADD: {
        add_play_volume();
        break;
    }
    case VOLUME_DECRE: {
        decre_play_volume();
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

        set_cursor_point(col / 2 - 10, (row / 2) - size - 5);

        int color = rand() % 7 + 1;
		char c[1024];
		get_color(c,color);
        textcolor("♪♪ｖ(⌒ｏ⌒)ｖ♪♪イエーイ", c);

        char menu_item[1024];
        for(i = 0 ; i < size; i++) {
            if(i == cursor) {
                snprintf(menu_item, sizeof(menu_item), "->%d.%s\n", i + 1, base_menu[i]);
                set_cursor_point(col / 2 - 6, (row / 2) - size + i);
                textcolor(menu_item, TEXT_DARKGREEN);
            }
            else {
                snprintf(menu_item, sizeof(menu_item), "  %d.%s\n", i + 1, base_menu[i]);
                set_cursor_point(col / 2 - 6, (row / 2) - size + i);
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
        "重新加载配置",
        "歌曲路径设置",
    };
    char setting_item[50];
    int len = dim(help_array);
    int size = sizeof(help_array) / sizeof(help_array[0]);

    if(cursor >= size)
        cursor = 0;
    else if(cursor < 0)
        cursor = size - 1;

    set_cursor_point(col / 2 - 13, row / 2 - 10);
    fprintf(stdout, "系统设置\n");

    for( i = 0 ; i < len; i++) {
        set_cursor_point(col / 2 - 15, row / 2 - 9 + i);
        if(i == cursor) {
            snprintf(setting_item, sizeof(setting_item), "->%d.%s\n", i + 1, help_array[i]);
            textcolor(setting_item, TEXT_DARKGREEN);
        }
        else
        {
            snprintf(setting_item, sizeof(setting_item), "  %d.%s\n", i + 1, help_array[i]);
            fprintf(stdout, "%s", setting_item);
        }
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
    print_library_music(&cursor, MUSIC_LIST_COUNT, window_size.ws_col, window_size.ws_row);
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
        "\n\n",
        "                                      2014年10月",
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



