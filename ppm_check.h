#ifndef STEGO_H_FILE

#include <stdio.h>
#define STEGO_H_FILE

// Check that the file is P6 format
int correct_magic_num(FILE *);

// Skips past the comments in the file
void ignore_comments(FILE *);

// Gets the width of the image 
int get_width(FILE *);

// Gets the height of the image
int get_height(FILE *);

// Returns 1 if 255, else returns 0
int check_colour_channel(FILE *);

char * add_file_extension(char *inf_name, int curr_file);

#endif