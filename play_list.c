#include <ao/ao.h>
#include <mpg123.h>
#include <dirent.h>
#include <sys/stat.h>

#include "play_list.h"
#include "common.h"
#include "menu.h"

#define BITS 8


//TAILQ_HEAD(, _music_file) music_queue;
static arraylist* music_list;
static mpg123_handle *mpg_handle = NULL;
static pthread_t tid;
static pthread_mutex_t mutex;
static music_file* current_music;
/*
 * 显示当前播放的歌曲
 */
static void show_play_music(char *music_name);
static void *thread_play_file(void *file);
static int current_done;

/* Helper for v1 printing, get these strings their zero byte. */
static void safe_print(char* name, char *data, size_t size)
{
    char safe[31];
    if(size > 30) return;

    memcpy(safe, data, size);
    safe[size] = 0;
    printf("%s: %s\n", name, safe);
}


/* Split up a number of lines separated by \n, \r, both or just zero byte
   and print out each line with specified prefix. */
static void print_lines(const char* prefix, mpg123_string *inlines)
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
    for(i = 0; i < len; ++i)
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
            if(line == NULL) line = lines + i;
        }
    }
}




/* Print out ID3v1 info. */
static void print_v1(mpg123_id3v1 *v1)
{
    safe_print("Title",   v1->title,   sizeof(v1->title));
    safe_print("Artist",  v1->artist,  sizeof(v1->artist));
    safe_print("Album",   v1->album,   sizeof(v1->album));
    safe_print("Year",    v1->year,    sizeof(v1->year));
    safe_print("Comment", v1->comment, sizeof(v1->comment));
    printf("Genre: %i", v1->genre);
}


/* Print out the named ID3v2  fields. */
static void print_v2(mpg123_id3v2 *v2)
{
    print_lines("Title: ",   v2->title);
    print_lines("Artist: ",  v2->artist);
    print_lines("Album: ",   v2->album);
    print_lines("Year: ",    v2->year);
    print_lines("Comment: ", v2->comment);
    print_lines("Genre: ",   v2->genre);
}


/*
 * 初始化播放列表
 */
int init_play_list()
{
    ao_initialize();
    mpg123_init();
    mpg_handle = mpg123_new(NULL, NULL);
    mpg123_param(mpg_handle, MPG123_RESYNC_LIMIT, -1, 0);
    music_list = arraylist_create();
    pthread_mutex_init(&mutex, NULL);
    //TAILQ_INIT(&music_queue);
    return 0;
}


/*
 * 加载本地库里面的所有音乐
 */
int load_library_music(char *library) {
    DIR *d = NULL;
    struct dirent *dir = NULL;
    struct stat st;
    char filename[1024] = {0};
    if(library == NULL)
        return 0;
    arraylist_clear(music_list);
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

int add_music_to_play_list(char *filename, char *path)
{
    music_file *file;
    file = malloc(sizeof(music_file));
    if(file) {
        file->filename = strdup(filename);
        file->path = strdup(path);
    }

    arraylist_add(music_list, file);
    return 0;
}


music_file* get_first_music()
{
    return (music_file*) arraylist_pop(music_list);
}

music_file* get_index_music_path(int index)
{
    music_file *item = NULL;
    if(music_list->size == 0)
        return NULL;
    item = (music_file*) arraylist_get(music_list, index);
    return item;
}

/*
 * show muisc list
 */
void print_library_music(int *cursor, int page_size, int columns, int rows) {
    int i = 0, index = 0, count = 0, _cursor = 0;
    for (i = 0 ; i < 5; i++)
        fprintf(stdout, "\n");

    int size = music_list->size;

    if(size == 0) {
        print_center_string("提示:当前路径下, 没有发现歌曲 :)\n");
        return;
    }

    if(*cursor >= size)
        *cursor = 0;
    else if(*cursor < 0)
        *cursor = size - 1;

    _cursor = *cursor;

    int page = _cursor / page_size; //得到当前的页数

    i = page * page_size;//得到当前页数第一个索引

    arraylist* music_slice = NULL;
    if (i + page_size < size)
        music_slice = arraylist_slice(music_list, i, page_size);
    else
        music_slice = arraylist_slice(music_list, i, size - i);

    void* item;

    char music_item[2048];

    arraylist_iterate(music_slice, index, item) {

        set_cursor_point(columns / 2 - 40, rows / 2 - 10 + index);

        if ((i + index) == _cursor) {
            snprintf(music_item, sizeof(music_item), "\t\t\t\t->%d.%s\n", i + index , ((music_file*)item)->filename);
            textcolor(music_item, TEXT_DARKGREEN);
        }
        else {
            snprintf(music_item, sizeof(music_item), "\t\t\t\t  %d.%s\n", i + index , ((music_file*)item)->filename);
            fprintf(stdout, "%s", music_item);
        }
    }

    arraylist_destroy(music_slice);
}

void play_music_file(music_file *file)
{
    current_music = file;
    char *data = strdup(file->path);
    current_done = 1;
    if (pthread_create(&tid, NULL, thread_play_file, (void *)data) != 0) {
        printf("Create thread error!\n");
        exit(1);
    }

}

static void *thread_play_file(void *file)
{
    int driver;
    ao_device *dev;
    ao_sample_format format;
    int channels, encoding;
    long rate;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;

    pthread_detach(pthread_self());
    pthread_mutex_lock(&mutex);

    current_done = 0;
    //mpg123_handle *m = NULL;
    driver = ao_default_driver_id();
    buffer_size = mpg123_outblock(mpg_handle);
    buffer = (unsigned char* )malloc(buffer_size * sizeof(unsigned char));
    mpg123_open(mpg_handle, (char *)file);
    mpg123_getformat(mpg_handle, &rate, &channels, &encoding);

    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
    mpg123_seek(mpg_handle, 0, SEEK_SET);
    /*int meta = mpg123_meta_check(mpg_handle);
    mpg123_id3v1 *v1;
    mpg123_id3v2 *v2;
    if(meta & MPG123_ID3 && mpg123_id3(mpg_handle, &v1, &v2) == MPG123_OK) {
        if (v2 != NULL) {
            print_v2(v2);
        } else if(v1 != NULL) {
            print_v1(v1);
        }
    }*/

    while(mpg123_read(mpg_handle, buffer, buffer_size, &done) == MPG123_OK && current_done == 0) {
        ao_play(dev, (char *)buffer, done);
    }
    free(buffer);
    free(file);
    if (current_done == 0)
        current_music = NULL;
    current_done = 1;
    ao_close(dev);
    mpg123_close(mpg_handle);
    pthread_mutex_unlock(&mutex);
}


static void free_music_file(music_file *item)
{
    if(item != NULL) {
        free(item->filename);
        free(item->path);
        free(item);
    }
}

static void show_play_music(char *music_name)
{
    fprintf(stdout, "当前播放:%s", music_name);
}

music_file* get_current_music()
{

    //mpg123_volume(mpg_handle, 0.5);
    return current_music;
}

void free_play_list()
{
    int i;
    void* item;
    arraylist_iterate(music_list, i, item) {
        free_music_file((music_file*) item);
    }
    arraylist_destroy(music_list);
    mpg123_delete(mpg_handle);
    mpg123_exit();
    ao_shutdown();
}
