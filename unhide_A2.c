#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppm_check.h"

void unhide_char(FILE *inf, unsigned char (*unhidden_char));
int get_msg_len(FILE* inf);
void unhide_msg(char *filename);

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Error: program requires 1 parameters to run: input.ppm\n");
		printf("Terminating...\n");
		return 1;
	}

	unhide_msg(argv[2]);

	return 0;
}


void unhide_msg(char *filename) {
	int curr_img = 0, curr_char_idx, i, j;
	unsigned char unhidden_char = 0;

	FILE *inf = open_file(filename, curr_img);
	unsigned int file_cap = get_file_cap(inf);
	check_colour_channel(inf);
	int msg_len = get_msg_len(inf);
	char *hidden_msg = (char*)malloc(msg_len + 1);
	for (i = 8; i <= (msg_len * 8 + 8); i++) {
		if (i % 8 == 0) {
			curr_char_idx = (i - 8) / 8;
			hidden_msg[curr_char_idx] = unhidden_char;
			unhidden_char = 0;
		}
		if (i >= file_cap) {
			curr_img++;
			if (i != 0) fclose(inf);
			// get the current image that will hide the current portion of the message
			inf = open_file(filename, curr_img);
			// find how many characters can be stored in the current image
			file_cap += get_file_cap(inf);
			check_colour_channel(inf);
		}
		unhide_char(inf, &unhidden_char);
	}
	for (j = 1; j <= msg_len; j++) {
		printf("%c", hidden_msg[j]);
	}
	printf("\n");
	free(hidden_msg);
}


int get_msg_len(FILE* inf) {
	char temp = 0;
	int i, length = 0;

	for (i = 0; i < 8; i++) {
		temp = fgetc(inf);
		//Delay the shift by one to get the 29, otherwise I'd get 58
		if (i > 0) length <<= 1;
		length |= (temp & 1);
	}
	return length;
}


// reveals the hidden message
void unhide_char(FILE *inf, unsigned char (*unhidden_char)) {
	int curr_chan;

	// get colour channel value
	curr_chan = fgetc(inf);
	// shift bit to find the next binary value of the character in the message
	(*unhidden_char) <<= 1;
	// current bit in character is changed to the colour channel's LSB
	(*unhidden_char) |= (curr_chan & 1);
}
