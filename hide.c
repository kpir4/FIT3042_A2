#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ppm_check.h"

void copy_header(FILE *inf, FILE *outf, int num_header_lines);
int check_msg_size(int msg_len, int width, int height);
int count_header_lines(FILE *inf);
void hide_msg(FILE *inf, FILE *outf, int msg_len, char *message, int width, int height);

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Error: program requires at least 2 parameters to run: input.ppm and output name\n");
		printf("Terminating...\n");
		return 1;
	}

	if (!strcmp(argv[1], "-m")) {
		int num_files = atoi(argv[2]);
		char ** image_filenames = malloc(num_files * sizeof(char *));
		int i;
		unsigned int filename_len;

		// create an array that contains the names of each image file
		for (i = 0; i < num_files; i++) {
			filename_len = strlen(argv[3 + i]);
			image_filenames[i] = (char *)malloc(filename_len + 1);
			image_filenames[i] = argv[3 + i];
			// free memory
		}

		if (image_fits(image_filenames, msg_len)) {
			hide_msg(inf, outf, msg_len, msg)
		}
		else {
			printf("Error: Message is too large to hide in input image.\n");
			printf("Terminating...\n");
			return 1;
		}
	}

	FILE *inf = fopen(argv[1], "r");
	if (inf == NULL) {
		printf("Error: Could not open file.\n");
		printf("Terminating...\n");
		return 1;
	}

	// checks if ppm file is P6 format
	if (read_magic_num(inf)) {
		// moves pointer past comments
		ignore_comments(inf);		

		// gets message from user through stdin and returns message length
		int max = 20;
		char *message = (char*)malloc(max);

		// checks for input redirection for message, if none, asks user for message
		if (!feof(stdin))
			printf("Enter your message: ");

		int msg_len = 0;
		while (1) {
			int c = getchar();
			if (c == '\n' || c == EOF) {
				message[msg_len] = 0;
				break;
			}
			message[msg_len] = c;
			// allocate more memory if user input is longer than max
			if (msg_len == max - 1) {
				max += max;
				message = (char*)realloc(message, max);
			}
			msg_len++;
		}
		
		// multiply message length by number of bits per character
		msg_len *= 8;

		// int msg_len = get_msg_len(message);
		int width = get_width(inf);
		int height = get_height(inf);

		// checks if the message can fit in the image
		if (check_msg_size(msg_len, width, height)) {	
			// checks if the maximum colour channel is 255
			if (check_colour_channel(inf)) {
				char *extension = ".ppm";
				char *outf_name = argv[2];
				strcat(outf_name, extension);
				FILE *outf = fopen(outf_name, "w");
				// copy the header of the image file
				int header_lines = count_header_lines(inf);
				copy_header(inf, outf, header_lines);
				hide_msg(inf, outf, msg_len, message, width, height);
				printf("Message hidden. Output to %s.\n", argv[2]);

				fclose(inf);
				fclose(outf);
				free(message);
			}
			else {
				printf("Error: Maximum channel value is not 255.\n");
				printf("Terminating...\n");
				return 1;
			}
		}
		else {
			printf("Error: Message is too large to hide in input image.\n");
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

void copy_header(FILE *inf, FILE *outf, int num_header_lines) {
	int i;
	char temp;

	// move pointer to start of file
	rewind(inf);

	for (i = 0; i < num_header_lines; i++) {
		temp = fgetc(inf);
		// copy every character until end of file or newline
		while (temp != EOF && temp != '\n') {
			fputc(temp, outf);
			temp = fgetc(inf);
		}
		fputc('\n', outf);
	}
}


// checks if message fits in image
int check_msg_size(int msg_len, int width, int height) {
	// multiplied by 3 colour channels
	return msg_len < width * height * 3;
}


int count_header_lines(FILE *inf) {
	char temp;
	int count = 0;

	// move to the start of the file
	rewind(inf);
	temp = fgetc(inf);

	// count number of newlines in file
	while (temp != EOF) {
		if (temp == '\n') {
			count++;
		}
		temp = fgetc(inf);
	}

	return count;
}


void hide_msg(FILE *inf, FILE *outf, int msg_len, char *message, int width, int height) {
	int num_char_hidden = 0;
	unsigned char colour_chan;
	int curr_char = 0;
	int bits_copied = 0;
	char curr_bit;

	int file_cap = width * height * 3;
	int i;

	for (i = 0; i < file_cap; i++) {
		// get value of the current colour channel
		colour_chan = fgetc(inf);
		// current character of the message to hide
		curr_bit = message[curr_char];

		// start with the LMB of the character
		curr_bit >>= 7 - bits_copied;
		bits_copied++;

		// checks if the whole message has been hidden
		if (num_char_hidden <= msg_len) {
			num_char_hidden++;
			// check if current character bit to be hidden is a 1
			if ((curr_bit & 1) == 1) {
				// change LSB of colour channel to 1 if it is 0
				if ((colour_chan & 1) == 0) {
					colour_chan++;
				}
			}
			// if current character bit is 0
			else {
				// change LSB of colour channel to 0 if it is 1
				if ((colour_chan & 1) == 1) {
					colour_chan--;
				}
			}
			// check if the whole character has been hidden
			if (bits_copied == 8) {
				// move to next character
				curr_char++;
				bits_copied = 0;
			}
		}

		fputc(colour_chan, outf);
	}
}
