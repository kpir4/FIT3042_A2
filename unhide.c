#include <stdio.h>
#include <stdlib.h>
#include "ppm_check.h"

void unhide_msg(FILE *inf, int width, int height);

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Error: program requires 1 parameters to run: input.ppm and output.ppm\n");
		printf("Terminating...\n");
		return 1;
	}

	FILE *inf = fopen(argv[1], "rb");
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

// reveals the hidden message
void unhide_msg(FILE *inf, int width, int height) {
	unsigned char msg_char = 0;
	int curr_chan;
	int char_pos = 0;
	int num_zero = 0;
	int num_bits = 0;

	int file_cap = width * height * 3;
	char *hidden_msg = malloc(sizeof(char) * file_cap);

	// Keep decoding pixels until a null character found
	while (num_zero < 8) {
		// get colour channel value
		curr_chan = fgetc(inf);
		// reset variables when 8 bits have been processed
		if (num_bits % 8 == 0) {
			// add processed character to message array
			hidden_msg[char_pos] = msg_char;
			// move to next character position in message array
			char_pos++;
			// reset for next character of hidden message
			msg_char = 0;
			// reset counter for null terminator
			num_zero = 0;
		}
		else {
			// shift bit to find the next binary value of the character in the message
			msg_char <<= 1;
		}
		// current bit in character is changed to the colour channel's LSB
		msg_char |= (curr_chan & 1);
		num_bits++;

		// checks if the last bit is a zero
		if ((curr_chan & 1) == 0) {
			num_zero++;
		}
	}
	// hidden_msg[char_pos] = msg_char;

	int i;
	for (i = 1; i < char_pos; i++) {
		printf("%c", hidden_msg[i]);
	}

	printf("\n");

	free(hidden_msg);
}
