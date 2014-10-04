#ifndef MUSIC_TMUX_H
#define MUSIC_TMUX_H


#define VERSION "1.0"

struct paramter
{
	char *filename;
	char *listname; //播放列表名称
};


struct play_item
{
	char *music_name; //音乐文件名
};

void reload_library();

#endif
