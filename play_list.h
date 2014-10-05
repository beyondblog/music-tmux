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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "arraylist.h"

struct _music_file
{
    char *filename;
    char *path;
};

typedef struct _music_file music_file;


int init_play_list();

int load_library_music(char *library);

int add_music_to_play_list(char *filename, char *path);

music_file* get_first_music();

void print_library_music(int *cursor, int page_size, int columns, int rows);

void free_play_list();

void play_music_file(music_file *file);

void add_play_volume();

void decre_play_volume();

music_file* get_index_music_path(int index);

music_file* get_current_music();

#endif
