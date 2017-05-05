#ifndef PREVIEW_IMG_H
#define PREVIEW_IMG_H

#include <stdio.h>

void preview_img(char *filename);

void get_channel_info(FILE *inf, int(*red), int(*green), int(*blue));

char *get_preview_title(char *filename);

#endif