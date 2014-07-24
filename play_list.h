/*
 * =====================================================================================
 *
 *       Filename:  play_list.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014/07/24 12时24分59秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YangBing
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef PLAY_LIST_H
#define PLAY_LIST_H


#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _music_file
{
    char *filename;
    char *path;
    TAILQ_ENTRY(_music_file) entries;
};

typedef struct _music_file music_file;


int init_play_list();

int add_music_to_play_list(char *filename, char *path);

music_file* get_first_music();

void print_library_music();

void free_play_list();

void play_music_file(char *file);



#endif
