#ifndef PREVIEW_IMG_H
#define PREVIEW_IMG_H

#include <stdio.h>

void preview_img(char *filename, int file_flag);

void get_channel_info(FILE *inf, int(*red), int(*green), int(*blue));

void *get_preview_title(char *filename, char *preview_title);

#endif