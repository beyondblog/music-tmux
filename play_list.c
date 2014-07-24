#include "play_list.h"
#include <ao/ao.h>
#include <mpg123.h>

#define BITS 8


TAILQ_HEAD(, _music_file) music_queue;
static mpg123_handle *mpg_handle = NULL;

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
    TAILQ_INIT(&music_queue);
    return 0;
}


int add_music_to_play_list(char *filename, char *path)
{
    struct _music_file *file;
    file = malloc(sizeof(struct _music_file));
    if(file) {
        file->filename = strdup(filename);
        file->path = strdup(path);
    }
    TAILQ_INSERT_TAIL(&music_queue, file, entries);
    return 0;
}


music_file* get_first_music()
{
    /* The tail queue should now be empty. */
    if (!TAILQ_EMPTY(&music_queue))
        return TAILQ_FIRST(&music_queue);
    else
        return NULL;
}

void print_library_music() {
    struct _music_file *item;
    TAILQ_FOREACH(item, &music_queue, entries) {
		printf("%s\n",item->filename);
    }
}

void play_music_file(char *file)
{
    int driver;
    ao_device *dev;
    ao_sample_format format;
    int channels, encoding;
    long rate;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;

    //mpg123_handle *m = NULL;

    driver = ao_default_driver_id();
    buffer_size = mpg123_outblock(mpg_handle);
    buffer = (unsigned char* )malloc(buffer_size * sizeof(unsigned char));
    mpg123_open(mpg_handle, file);
    mpg123_getformat(mpg_handle, &rate, &channels, &encoding);

    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
    mpg123_seek(mpg_handle, 0, SEEK_SET);
    int meta = mpg123_meta_check(mpg_handle);
    mpg123_id3v1 *v1;
    mpg123_id3v2 *v2;
    if(meta & MPG123_ID3 && mpg123_id3(mpg_handle, &v1, &v2) == MPG123_OK) {
        if (v2 != NULL) {
            print_v2(v2);
        } else if(v1 != NULL) {
            print_v1(v1);
        }
    }

    while(mpg123_read(mpg_handle, buffer, buffer_size, &done) == MPG123_OK)
        ao_play(dev, (char *)buffer, done);
    free(buffer);
    ao_close(dev);
    mpg123_close(mpg_handle);
}


static void free_music_file(struct _music_file *item)
{
    if(item != NULL) {
        free(item->filename);
        free(item->path);
        free(item);
    }
}

void free_play_list()
{
    struct _music_file *item;

    if (!TAILQ_EMPTY(&music_queue)) {
        TAILQ_FOREACH(item, &music_queue, entries) {
            free_music_file(item);
        }
    }
    mpg123_delete(mpg_handle);
    mpg123_exit();
    ao_shutdown();
}
