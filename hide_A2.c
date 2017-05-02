#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ppm_check.h"

int get_total_cap(char ** files, int num_files);
char * add_file_extension(char *inf_name, int curr_file);
int get_file_cap(FILE *inf);
int get_msg(char **message);
void copy_header(FILE *inf, FILE *outf, int num_header_lines);
int count_header_lines(FILE *inf);
int hide_msg(char *inf_name, char *outf_name, char *msg, int msg_len);
void hide_bit(FILE *inf, FILE *outf, char curr_char, int hide_bit, int msg_len, int bits_hidden);

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Error: program requires at least 2 parameters to run: input.ppm and output name\nTerminating...\n");
		return 1;
	}
	
	char *msg;
	int msg_len = get_msg(&msg);

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
				free(msg);
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
	char *inf_name = (char*)malloc(strlen(files[3]) + 10);
	strcpy(inf_name, files[3]);

	for (i = 0; i < num_files; i++) {
		strcpy(inf_name, add_file_extension(inf_name, i));
		FILE *inf = fopen(inf_name, "r");
		if (inf == NULL) {
			printf("Error: Could not open file.\n");
			printf("Terminating...\n");
			return -2;
		}

		if (correct_magic_num(inf)) {
			ignore_comments(inf);
			total_file_cap += get_file_cap(inf);				
		}
		else {
			return -1;
		}
		fclose(inf);
		free(inf_name);
	}
	return total_file_cap;
}

// Appends the sequence number and .ppm extension to the end of the base file name
char * add_file_extension(char *inf_name, int curr_file) {
	char *inf_name_cpy = inf_name;
	char file_num[4];
	sprintf(file_num, "%d", curr_file);
	char extension[9];
	if (curr_file < 10)
		strcat(extension, "-00");
	else if (10 < curr_file < 100)
		strcat(extension, "-0");
	else
		strcat(extension, "-");
	strcat(extension, file_num);
	strcat(extension, ".ppm");
	strcat(inf_name, extension);
	printf("%s", inf_name);
	return inf_name_cpy;
}

// Returns the number of characters that can be hidden within the file
int get_file_cap(FILE *inf) {
	int width = get_width(inf);
	int height = get_height(inf);
	// multiplied by 3 for the 3 channels
	return width * height * 3;
}

/* Returns the message length and updates the pass in message variable.
** Uses input redirection if avaiable or prompts user for message if not.
*/
int get_msg(char **message) {
	int max = 20;
	*message = (char*)malloc(max);

	// checks for input redirection for message, if none, asks user for message
	if (!feof(stdin))
		printf("Enter your message: ");

	int msg_len = 0;
	while (1) {
		int c = getchar();
		if (c == '\n' || c == EOF) {
			(*message)[msg_len] = 0;
			return msg_len;
		}
		(*message)[msg_len] = c;
		// allocate more memory if user input is longer than max
		if (msg_len == max - 1) {
			max += max;
			*message = (char*)realloc(message, max);
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
	int i, curr_img = 0, bits_hidden = 0;
	char curr_char;
	do {
		int bits_hidden_in_file = 0;
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
		// create the corresponding image which will have the message hidden within
		char *outf_name_copy = (char*)malloc(strlen(outf_name) + 10);
		strcpy(outf_name_copy, outf_name);
		strcpy(outf_name_copy, add_file_extension(outf_name_copy, curr_img));
		FILE *outf = fopen(outf_name_copy, "w");
		// copy image header to the output image
		copy_header(inf, outf, count_header_lines(inf));

		// hide character
		while (bits_hidden_in_file <= file_cap) {
			// the current character to hide
			int curr_char_index = bits_hidden / 8;
			curr_char = msg[curr_char_index];
			for (i = 0; i < 8; i++){
				hide_bit(inf, outf, curr_char, i, msg_len, bits_hidden); // hide current character bit in current image
				bits_hidden++;			// total number of bits hidden
				bits_hidden_in_file++;	// number of bits hidden in current image
			}
		}
		fclose(inf);
		fclose(outf);
		free(inf_name_copy);
		free(outf_name_copy);
		curr_img++;
	} while (bits_hidden <= msg_len * 8);
	return 0;
}


// Hides a single character within a .ppm image
void hide_bit(FILE *inf, FILE *outf, char curr_char, int hide_bit, int msg_len, int bits_hidden){
	unsigned char colour_chan;
	char curr_bit;

	colour_chan = fgetc(inf);

	if (bits_hidden < msg_len * 8){
		// current character of the message to hide
		curr_bit = curr_char;

		// start with the LMB of the character
		curr_bit >>= 7 - hide_bit;

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
	}
	// output changed channel value to output image
	fputc(colour_chan, outf);
}