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


#include <mpg123.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *fullprogname = NULL; /* Copy of argv[0]. */
char *prgName = NULL;
char *binpath; /* Path to myself. */

void safe_exit(int code)
{
    if(fullprogname) free(fullprogname);
    exit(code);
}


/*
 * 加载配置
 */
void load_config(char *config)
{

}


/* Helper for v1 printing, get these strings their zero byte. */
void safe_print(char* name, char *data, size_t size)
{
	char safe[31];
	if(size>30) return;

	memcpy(safe, data, size);
	safe[size] = 0;
	printf("%s: %s\n", name, safe);
}

/* Split up a number of lines separated by \n, \r, both or just zero byte
   and print out each line with specified prefix. */
void print_lines(const char* prefix, mpg123_string *inlines)
{
	size_t i;
	int hadcr = 0, hadlf = 0;
	char *lines = NULL;
	char *line  = NULL;
	size_t len = 0;

	if(inlines != NULL && inlines->fill)
	{
		lines = inlines->p;
		len   = inlines->fill;
	}
	else return;

	line = lines;
	for(i=0; i<len; ++i)
	{
		if(lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0)
		{
			char save = lines[i]; /* saving, changing, restoring a byte in the data */
			if(save == '\n') ++hadlf;
			if(save == '\r') ++hadcr;
			if((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0) line = "";

			if(line)
			{
				lines[i] = 0;
				printf("%s%s\n", prefix, line);
				line = NULL;
				lines[i] = save;
			}
		}
		else
		{
			hadlf = hadcr = 0;
			if(line == NULL) line = lines+i;
		}
	}
}



/* Print out ID3v1 info. */
void print_v1(mpg123_id3v1 *v1)
{
	safe_print("Title",   v1->title,   sizeof(v1->title));
	safe_print("Artist",  v1->artist,  sizeof(v1->artist));
	safe_print("Album",   v1->album,   sizeof(v1->album));
	safe_print("Year",    v1->year,    sizeof(v1->year));
	safe_print("Comment", v1->comment, sizeof(v1->comment));
	printf("Genre: %i", v1->genre);
}

/* Print out the named ID3v2  fields. */
void print_v2(mpg123_id3v2 *v2)
{
	print_lines("Title: ",   v2->title);
	print_lines("Artist: ",  v2->artist);
	print_lines("Album: ",   v2->album);
	print_lines("Year: ",    v2->year);
	print_lines("Comment: ", v2->comment);
	print_lines("Genre: ",   v2->genre);
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

    fprintf(stdout,"%s\n", binpath);

    //load config
    load_config(NULL);
    //读取路径下所有音频文件信息
    mpg123_handle *m;
    mpg123_init();
    m = mpg123_new(NULL, NULL);
    mpg123_param(m, MPG123_RESYNC_LIMIT, -1, 0);

	int result = mpg123_open(m,"/Users/OccamsRazor/Documents/Code/C/music_tmux/1.mp3");
	if(result != MPG123_OK)
		printf("Cannot open file!\n");
	struct mpg123_frameinfo *mi = NULL;
	result = mpg123_info(m, mi);
	if(result > 0)
		printf("result %d,File %ld", result, mi->rate);

	mpg123_seek(m, 0, SEEK_SET);
	int meta = mpg123_meta_check(m);
	mpg123_id3v1 *v1;
	mpg123_id3v2 *v2;
	if(meta & MPG123_ID3 && mpg123_id3(m, &v1, &v2) == MPG123_OK){
		if (v2 != NULL) {
			print_v2(v2);
		} else if(v1 != NULL){
			print_v1(v1);
		}
	}
	mpg123_close(m);
	mpg123_delete(m);
	mpg123_exit();
	return 0;

}
