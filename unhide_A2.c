#include <stdio.h>
#include <stdlib.h>
#include "ppm_check.h"

void unhide_msg(FILE *inf, int width, int height);

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Error: program requires 1 parameters to run: input.ppm\n");
		printf("Terminating...\n");
		return 1;
	}

	unhide_msg(argv[2]);





	FILE *inf = fopen(argv[1], "r");
	if (inf == NULL) {
		printf("Error: Could not open file.\n");
		printf("Terminating...\n");
		return 1;
	}

	// check that the file is P6 format
	if (correct_magic_num(inf)) {
		// Bypass any comments
		ignore_comments(inf);
		int width = get_width(inf);
		int height = get_height(inf);

		// check the max colour channel intensity is 255
		if (check_colour_channel(inf)) {
			printf("The message has been extracted: ");
			unhide_msg(inf, width, height);
		}
		else {
			printf("Error: Maximum channel value is not 255.\n");
			printf("Terminating...\n");
			return 1;
		}
	}
	else {
		printf("Error: Image format must be P6.\n");
		printf("Terminating...\n");
		return 1;
	}
	return 0;
}


void unhide_msg(char *filename) {
	int msg_len = get_msg_len(filename);
	int bits_hidden = 0, file_cap = 0, i, j;
	char unhidden_char = 0;
	char *hidden_msg = (char*)malloc(msg_len);
	for (i = 0; i < msg_len * 8; i++) {
		if (i >= file_cap) {
			if (i != 0) fclose(inf);
			// get the current image that will hide the current portion of the message
			char *inf_name_copy = (char*)malloc(strlen(inf_name) + 10);
			strcpy(inf_name_copy, inf_name);
			strcpy(inf_name_copy, add_file_extension(inf_name_copy, curr_img));
			FILE *inf = fopen(inf_name_copy, "r");
			// check the image has the correct maximum channel value
			if (!correct_magic_num(inf))
				exit(-1);
			// ignore comments in the ppm image file
			ignore_comments(inf);
			// find how many characters can be stored in the current image
			unsigned int file_cap = get_file_cap(inf);
		}
		unhide_char(inf, i % 8, &unhidden_char);
		if (i % 8 == 0) {
			hidden_msg[i % 8] = unhidden_char;
			unhidden_char = 0;
		}
	}
	for (j = 1; j < msg_len; j++) {
		printf("%c", hidden_msg[j]);
	}
	printf("\n");
	free(hidden_msg);
}


int get_msg_len(char *filename) {
	// get the current image that will hide the current portion of the message
	char *inf_name_copy = (char*)malloc(strlen(inf_name) + 10);
	strcpy(inf_name_copy, inf_name);
	strcpy(inf_name_copy, add_file_extension(inf_name_copy, curr_img));
	FILE *inf = fopen(inf_name_copy, "r");
	// check the image has the correct maximum channel value
	if (!correct_magic_num(inf))
		return -1;
	// ignore comments in the ppm image file
	ignore_comments(inf);
	// find how many characters can be stored in the current image
	unsigned int file_cap = get_file_cap(inf);
	char temp = 0;
	int i, length = 0;

	for (i = 0; i < 8; i++) {
		temp = fgetc(fp);
		//Delay the shift by one to get the 29, otherwise I'd get 58
		if (i > 0) length <<= 1;
		length |= (temp & 1);
	}
	fclose(inf);
	free(inf_name_copy);
	return length;
}


// reveals the hidden message
void unhide_char(FILE *inf, char *unhidden_char) {
	int curr_chan;

	// get colour channel value
	curr_chan = fgetc(inf);
	// shift bit to find the next binary value of the character in the message
	unhidden_char <<= 1;
	// current bit in character is changed to the colour channel's LSB
	unhidden_char |= (curr_chan & 1);
}
