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
		printf("Error: program requires at least 2 parameters to run: input.ppm and output name\nTerminating...\n");
		return 1;
	}
	
	char *msg;
	int msg_len = get_msg(msg);

	// checks for the '-m' flag for hiding message in multiple images
	if (!strcmp(argv[1], "-m")) {
		int num_files = atoi(argv[2]);
		if (num_files < 225) {
			// finds the combined number of characters that can be hidden by all images
			int total_image_cap = get_total_cap(argv, num_files);
			
			if (total_image_cap == -1) {
				printf("Error: Image format must be P6.\nTerminating...\n");
				return 1;
			// if the message fits
			}else if (msg_len <= total_image_cap) {
				// hides the message with the images
				if (!hide_msg(argv[3], argv[4], msg, msg_len)) {
					printf("Error: Maximum channel value is not 255.\nTerminating...\n");
					return 1;
				}
			}
			else {
				printf("Error: Message is too large to hide in input image.\nTerminating...\n");
				return 1;
			}
		}
		else {
			printf("Error: The message can only be hidden in a maximum of 255 images.\nTerminating...\n");
			return 1;
		}
	}

	return 0;
}

/* Returns the number of characters that can be hidden within the
** available input image files or -1 when at least one of the
** input files are not a P6 format ppm image 
*/
int get_total_cap(char ** files, int num_files) {
	int total_file_cap = 0, file_cap, i;

	for (i = 0; i < num_files; i++) {
		char *inf_name = files[3];
		inf_name = add_file_extension(inf_name, i);
		FILE *inf = fopen(inf_name, 'r');

		if (correct_magic_num(inf)) {
			ignore_comments(inf);
			total_file_cap += get_file_cap(i);				
		}
		else {
			return -1
		}
		fclose(inf);
	}
	return total_file_cap
}

// Appends the sequence number and .ppm extension to the end of the base file name
char * add_file_extension(char *inf_name, int curr_file) {
	if (curr_file < 10)
		char *extension = "-00" + curr_file + ".ppm";
	else if (10 < curr_file < 100)
		char *extension = "-0" + curr_file + ".ppm";
	else
		char *extension = "-" + curr_file + ".ppm";
	strcat(inf_name, extension);
	return inf_name;
}

// Returns the number of characters that can be hidden within the file
int get_file_cap(FILE *inf) {
	width = get_width(inf);
	height = get_height(inf);
	// multiplied by 3 for the 3 channels
	return width * height * 3
}

/* Returns the message length and updates the pass in message variable.
** Uses input redirection if avaiable or prompts user for message if not.
*/
int get_msg(char *message) {
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
			return msg_len;
		}
		message[msg_len] = c;
		// allocate more memory if user input is longer than max
		if (msg_len == max - 1) {
			max += max;
			message = (char*)realloc(message, max);
		}
		msg_len++;
	}
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

/* Returns the number of line in the header of the input file. The input file
** must be a .ppm image.
*/
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

/* Hide the message in the images. Returns 0 is message is successfully hidden or -1
** if an error occurs.
*/
int hide_msg(char *inf_name, char *outf_name, char *msg, int msg_len) {
	int i, curr_img = 0, char_hidden = 0;
	do {
		int char_hidden_in_file = 0;
		// get the current image that will hide the current portion of the message
		char *inf_name_copy = inf_name;
		inf_name_copy = add_file_extension(inf_name_copy, curr_img);
		FILE *inf = fopen(inf_name_copy, 'r');
		// check the image has the correct maximum channel value
		if (!correct_magic_num(inf))
			return -1;
		// ignore comments in the ppm image file
		ignore_comments(inf);
		// find how many characters can be stored in the current image
		unsigned int file_cap = get_file_cap(inf);
		// create the corresponding image which will have the message hidden within
		outf_name_copy = add_file_extension(outf_name_copy, curr_img);
		FILE *outf = fopen(outf_name_copy, 'w');
		// copy image header to the output image
		copy_header(inf, outf, count_header_lines(inf));

		// hide character
		while (char_hidden_in_file <= file_cap && char_hidden_in_file <= msg_len) {
			// hide current character in current image
			hide_char(inf, outf, msg[char_hidden]);
			char_hidden++;			// total number of characters hidden
			char_hidden_in_file++;	// number of characters hidden in current image
		}
		fclose(inf);
		fclose(outf);
		curr_img++;
	} while (char_hidden <= msg_len);
	return 0;
}


// Hides a single character within a .ppm image
void hide_char(FILE *inf, FILE *outf, char curr_char){
	unsigned char colour_chan;
	char curr_bit;
	int i;

	for (i = 0; i < 8; i++) {
		// current character of the message to hide
		curr_bit = curr_char;

		// start with the LMB of the character
		curr_bit >>= 7 - i;

		// check if current character bit to be hidden is a 1
		if ((curr_bit & 1) == 1) {
			// change LSB of colour channel to 1 if it is 0
			if ((colour_chan & 1) == 0) 
				colour_chan++;
		}
		// if current character bit is 0
		else {
			// change LSB of colour channel to 0 if it is 1
			if ((colour_chan & 1) == 1)
				colour_chan--;
		}

		fputc(colour_chan, outf);
	}
}