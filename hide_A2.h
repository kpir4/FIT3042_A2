#ifndef HIDE_A2_H
#define HIDE_A2_H

#include <stdio.h>

int get_total_cap(char ** files, int num_files);
char * add_file_extension(char *inf_name, int curr_file);
int get_file_cap(FILE *inf);
int get_msg(char **message);
void copy_header(FILE *inf, FILE *outf, int num_header_lines);
int count_header_lines(FILE *inf);
int hide_msg(char *inf_name, char *outf_name, char *msg, int msg_len, int num_files);
void hide_bit(FILE *inf, FILE *outf, char *msg, int bit_shift, int msg_len, int bits_iter);
void encode_length(FILE *in, FILE *out, int length);
void hide_fork(char *file);
void get_parametres(FILE *inf, char **msg_file, char **inf_img, char **outf_img);
char *get_msg_from_file(char *msg_file);
int more_to_hide(FILE *inf);

#endif