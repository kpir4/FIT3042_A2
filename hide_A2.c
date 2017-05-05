#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "hide_A2.h"
#include "ppm_check.h"
#include "preview_img.h"

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
			}else if (msg_len * 8 <= total_image_cap) {
				// hides the message with the images
				if (hide_msg(argv[3], argv[4], msg, msg_len, num_files)) {
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
		else if (!strcmp(argv[1], "-s")) {
			hide_msg(argv[2], argv[3], msg, msg_len, -1);
			preview_img(argv[2]);
			preview_img(argv[3]);
		}
		else if (!strcmp(argv[1], "-p")) {
			hide_fork(argv[2]);
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
		char *inf_name = (char*)malloc(strlen(files[3]) + 10);
		strcpy(inf_name, files[3]);
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

/* Returns the message length and updates the pass in message variable.
** Uses input redirection if avaiable or prompts user for message if not.
*/
int get_msg(char **message) {
	int max = 20;
	*message = (char*)malloc(max);

	// checks for input redirection for message, if none, asks user for message
	if (feof(stdin))
		printf("Enter your message: ");

	int msg_len = 0;
	while (1) {
		int c = getchar();
		if (c == EOF) {
			(*message)[msg_len] = 0;
			return msg_len;
		}
		(*message)[msg_len] = c;
		// allocate more memory if user input is longer than max
		if (msg_len == max - 1) {
			max += max;
			*message = (char*)realloc(*message, max);
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
int hide_msg(char *inf_name, char *outf_name, char *msg, int msg_len, int num_files) {
	int i, curr_img = 0, bits_iter = 0;
	char curr_char;
	do {
		// get the current image that will hide the current portion of the message
		FILE *inf = open_file(inf_name, curr_img);
		// find how many characters can be stored in the current image
		unsigned int file_cap = get_file_cap(inf);
		// create the corresponding image which will have the message hidden within
		FILE *outf = open_file(outf_name, curr_img);
		// copy image header to the output image
		copy_header(inf, outf, count_header_lines(inf));
		if (curr_img == 0){
			encode_length(inf, outf, msg_len);
			i = 8;
		} else i = 0;
		// hide character
		for (i; i < file_cap; i++) {
			// the current character to hide		
			hide_bit(inf, outf, msg, bits_iter % 8, msg_len, bits_iter); // hide current character bit in current image
			bits_iter++;			// total number of bits hidden
		}
		fclose(inf);
		fclose(outf);
		free(inf_name_copy);
		free(outf_name_copy);
		curr_img++;
	} while (curr_img < num_files);
	return 0;
}


void encode_length(FILE *in, FILE *out, int length) {
	char temp;
	int i, l;
	for (i = 0; i < 8; i++) {
		temp = fgetc(in);
		l = length;
		l >>= 7 - i;
		if ((l & 1) == 1){
			if ((temp & 1) == 0) temp++;
		}
		else {
			if ((temp & 1) == 1) temp--;
		}
		fputc(temp, out);
	}

	return;
}


// Hides a single character within a .ppm image
void hide_bit(FILE *inf, FILE *outf, char *msg, int bit_shift, int msg_len, int bits_iter){
	unsigned char colour_chan;
	char curr_bit;

	colour_chan = fgetc(inf);

	int curr_char_index = bits_iter / 8;
	if (curr_char_index <= msg_len){
		char curr_char = msg[curr_char_index];

		// current character of the message to hide
		curr_bit = curr_char;

		// start with the LMB of the character
		curr_bit >>= 7 - bit_shift;

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


void hide_fork(char *file) {
	FILE *inf = fopen(file, "r");
	char *msg_file, *inf, *outf;
	pid_t pid;
	
	while (more_to_hide) {
		get_parametres(inf, &msg_file, &inf_img, &outf_img);
		pid = fork();
		if (pid < 0) {
			printf("Error: Failed to create child process.\nTerminating...\n");
		}
		else if (!pid) {
			char *msg;
			strcpy(msg, get_msg_from_file(msg_file));
			hide_msg(inf_img, outf_img, msg, strlen(msg) - 1, -1);
			// stop child process from creating another child process
			break;
		}
		more_to_hide = check_for_more(inf);
	}
	wait(NULL);
	fclose(inf);
	free(msg);
}


void get_parametres(FILE *inf, char **msg_file, char **inf_img, char **outf_img) {
	char temp;
	int i = 0, str_len = 0;
	int max = 20;
	char *temp_str = (char*)malloc(max);

	while ((temp = fgetc(inf)) != "\n") {
		temp_str[str_len] = temp;
		if (temp == " " && i == 0) {
			temp_str[str_len] = 0
			strcpy(*msg_file, msg_file_copy);
		}
		else if (temp == " " && i == 1) {
			temp_str[str_len] = 0
			strcpy(*inf_img, inf_img_cpy);
		}
		else {
			temp_str[str_len] = 0
			strcopy(*outf_img, outf_img_cpy);
		}
		if (str_len == max - 1) {
			max += max;
			temp_str = (char*)realloc(temp_str, max);
		}
		str_len++;
	}
}


char *get_msg_from_file(char *msg_file) {
	FILE *inf = fopen(msg_file, "r");
	int max = 20;
	char *message = (char*)malloc(max);

	int msg_len = 0;
	while (1) {
		int c = fgetc(inf);
		if (c == EOF) {
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
	return message;
}


int more_to_hide(FILE *inf) {
	char temp = fgetc(inf);
	if (temp == EOF)
		return 0;
	else {
		ungetc(temp, inf);
		return 1;
	}
}