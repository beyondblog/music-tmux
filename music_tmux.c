/*
 * =====================================================================================
 *
 *       Filename:  muisc_tmux.c
 *
 *    Description:  终端音乐播放器
 *
 *        Version:  1.0
 *        Created:  2014/07/19 16时12分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YangBing
 *   Organization:
 *
 * =====================================================================================
 */


#include <ao/ao.h>
#include <mpg123.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libconfig.h>
#include <signal.h>

#include "config.h"
#include "common.h"
#include "music_tmux.h"
#include "play_list.h"
#include "menu.h"



static char *fullprogname = NULL; /* Copy of argv[0]. */
static char *prgName = NULL;
static char *binpath; /* Path to myself. */
static char *music_library = NULL;


void safe_exit(int code)
{
	show_cursor();
    if(fullprogname) free(fullprogname);
    free(music_library);
    free_play_list();
    exit(code);
}

/*
 * 加载本地库里面的所有音乐
 */
static int load_library_music(char *library) {
    DIR *d = NULL;
    struct dirent *dir = NULL;
    struct stat st;
    char filename[1024] = {0};
    d = opendir(library);
    if(d) {
        while((dir = readdir(d)) != NULL)
        {
            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;
            //判断是否是文件夹
            snprintf(filename, sizeof(filename), "%s/%s", library, dir->d_name);
            stat(filename, &st);
            if(S_ISDIR(st.st_mode)) {
                load_library_music(filename);
            } else if(str_ends_with(dir->d_name, "mp3") > 0 || str_ends_with(dir->d_name, "flac") > 0) {
                add_music_to_play_list(dir->d_name, filename);
            }
        }
        closedir(d);
    }
    return 0;
}

/*
 * 加载配置
 */
static void load_config(char *config)
{

    int rt = 0;
    config_t* conf = &(config_t) {};
    config_init(conf);
    config_read_file(conf, config);

    char *library = NULL, *version = NULL;
    rt = config_lookup_string(conf, "version", &version);
    rt = config_lookup_string(conf, "library", &library);
    music_library = strdup(library);
    config_destroy(conf);
}


int main(int sys_argc, char ** sys_argv)
{
    if(!(fullprogname = strdup(sys_argv[0])))
    {
        safe_exit(1);
    }

    /* Extract binary and path, take stuff before/after last / or \ . */
    if(  (prgName = strrchr(fullprogname, '/'))
            || (prgName = strrchr(fullprogname, '\\')))
    {
        /* There is some explicit path. */
        prgName[0] = 0; /* End byte for path. */
        prgName++;
        binpath = fullprogname;
    }
    else
    {
        prgName = fullprogname; /* No path separators there. */
        binpath = NULL; /* No path at all. */
    }

    //fprintf(stdout, "%s\n", binpath);
    //load config
    init_play_list();
    load_config(CONFIG_FILE);
    load_library_music(music_library);
    init_menu();
    char key;

    show_menu(NULL);
    while((key = getch())) {
        if(show_menu(key) == 1)
            safe_exit(1);
    }
    //music_file *item = NULL;
    //item = get_first_music();
    //play_music_file(item->path);
    //读取路径下所有音频文件信息
    safe_exit(1);
    return 0;
}
