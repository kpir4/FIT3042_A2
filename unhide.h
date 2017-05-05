#ifndef UNHIDE_H
#define UNHIDE_H

#include <stdio.h>

void unhide_char(FILE *inf, unsigned char(*unhidden_char));
int get_msg_len(FILE* inf);
void unhide_msg(char *filename, int file_flag);

#endif