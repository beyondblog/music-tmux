/*
 * =====================================================================================
 *
 *       Filename:  muisc_tmux.c
 *
 *    Description:  终端音乐播放器
 *
 *        Version:  1.0
 *        Created:  2014/07/19
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Occam's Razor
 *   Organization:
 *
 * =====================================================================================
 */


#include <ao/ao.h>
#include <mpg123.h>

#include <stdio.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libconfig.h>
#include <signal.h>
#include <sys/param.h>
#include <unistd.h>

#include "config.h"
#include "common.h"
#include "music_tmux.h"
#include "play_list.h"
#include "menu.h"



static char *fullprogname = NULL; /* Copy of argv[0]. */
static char *prgName = NULL;
static char *binpath; /* Path to myself. */
static char *music_library = NULL;

void init_daemon(void) {
    return;
    int pid , i;

    if((pid = fork()))
        exit(0); //结束当前进程
    else if( pid < 0)
        exit(1);

    setsid();

    if((pid = fork()))
        exit(0); //结束终端控制
    else if(pid < 0)
        exit(1);

    for(i = 0; i < NOFILE; i++)
        ;//close(i);

    umask(0);
}

void exit_callback(void) {
    show_cursor();
}

void safe_exit(int code)
{
    if(fullprogname) free(fullprogname);
    free(music_library);
    free_play_list();
    exit(code);
}

void reload_library() {

    load_library_music(music_library);
}

///*
// * 加载本地库里面的所有音乐
// */
//static int load_library_music(char *library) {
//    DIR *d = NULL;
//    struct dirent *dir = NULL;
//    struct stat st;
//    char filename[1024] = {0};
//    if(library == NULL)
//        return 0;
//    d = opendir(library);
//    if(d) {
//        while((dir = readdir(d)) != NULL)
//        {
//            if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
//                continue;
//            //判断是否是文件夹
//            snprintf(filename, sizeof(filename), "%s/%s", library, dir->d_name);
//            stat(filename, &st);
//            if(S_ISDIR(st.st_mode)) {
//                load_library_music(filename);
//            } else if(str_ends_with(dir->d_name, "mp3") > 0 || str_ends_with(dir->d_name, "flac") > 0) {
//                add_music_to_play_list(dir->d_name, filename);
//            }
//        }
//        closedir(d);
//    }
//    return 0;
//}

/*
 * 加载配置
 */
static void load_config(char *config)
{
    int rt = 0;
    config_t* conf = &(config_t) {};
    char filepath[1204];
    struct passwd *pw = getpwuid(getuid());
    snprintf(filepath, sizeof(filepath), "%s/%s", pw->pw_dir, config);
    config_init(conf);

    if(access(filepath, 0) == -1) {
        //file not exist
        config_setting_t* root = config_root_setting(conf);
        config_setting_t* version = config_setting_add(root, "version", CONFIG_TYPE_STRING);
        config_setting_set_string(version, VERSION);

        config_setting_t* key_up = config_setting_add(root, "UP", CONFIG_TYPE_STRING);
        config_setting_set_string(key_up, "k");

        config_setting_t* key_down = config_setting_add(root, "DOWN", CONFIG_TYPE_STRING);
        config_setting_set_string(key_down, "j");

        config_setting_t* key_left = config_setting_add(root, "LEFT", CONFIG_TYPE_STRING);
        config_setting_set_string(key_left, "h");

        config_setting_t* key_right = config_setting_add(root, "RIGHT", CONFIG_TYPE_STRING);
        config_setting_set_string(key_right, "l");

        config_setting_t* library = config_setting_add(root, "library", CONFIG_TYPE_ARRAY);
        library = config_setting_add(library, NULL, CONFIG_TYPE_STRING);

        config_setting_set_string(library, "~/Music");
        config_write_file(conf, filepath);
        config_destroy(conf);
        return;
    }
    music_library = NULL;
    if(!config_read_file(conf, filepath) == CONFIG_FALSE)
    {
        char *library = NULL, *version = NULL, *key = NULL;
        int key_up, key_down, key_left, key_right;
        rt = config_lookup_string(conf, "version", &version);
        config_setting_t* array = config_setting_get_member(conf->root, "library");
        int count = config_setting_length(array);
        if(count > 0) {
            library = config_setting_get_string_elem(array, 0);
            if(library != NULL && library[0] == '~' && strlen(library) > 0)
            {
                //解析用户路径
                int len = sizeof(char) * (strlen(pw->pw_dir) + strlen(library));
                music_library = (char *) malloc(len);
                snprintf(music_library, len, "%s%s", pw->pw_dir, &library[1]);
            } else {
                music_library = strdup(library);
            }
        }

        //加载上下左右
        if(!config_lookup_string(conf, "UP", &key))
            key_up = 'k';
        else
            key_up = key[0];

        if(!config_lookup_string(conf, "DOWN", &key))
            key_down = 'j';
        else
            key_down = key[0];

        if(!config_lookup_string(conf, "LEFT", &key))
            key_left = 'h';
        else
            key_left = key[0];

        if(!config_lookup_string(conf, "RIGHT", &key))
            key_right = 'l';
        else
            key_right = key[0];

        init_handle_key(key_up, key_down, key_left, key_right);
        config_destroy(conf);
    }
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

    init_daemon();
    atexit(exit_callback);

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
