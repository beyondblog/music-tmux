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

#include "menu.h"

static char *baseMenu[] = {
    "播放列表",
    "设置",
    "退出"
};

void init_menu()
{

}


void show_menu()
{
    int i, size;
    size = strlen(baseMenu);
	cls();
    for(i = 0 ; i < size; i++) {
        printf("%s\n", baseMenu[i]);
    }
}

void add_menu_item(char *menu_name)
{

}

