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
	
	if (!strcmp(argv[1], "-p")){
		hide_fork(argv[2]);
	}else{
		char *msg;
		int msg_len = get_msg(&msg);

		// checks for the '-m' flag for hiding message in multiple images
		if (!strcmp(argv[1], "-m")) {
			int num_files = atoi(argv[2]);
			if (num_files < 225) {
				// finds the combined number of characters that can be hidden by all images
				int total_image_cap = get_total_cap(argv[3], num_files);
				
				if (total_image_cap == -1) {
					printf("Error: Image format must be P6.\nTerminating...\n");
					return 1;
				// if the message fits
				}else if (msg_len * 8 <= total_image_cap) {
					// hides the message with the images
					hide_msg(argv[3], argv[4], msg, msg_len, num_files);
					free(msg);
				}
				else {
					printf("Error: Message is too large to hide in input image.\nTerminating...\n");
					return 1;
				}
			}
		}
		else if (!strcmp(argv[1], "-s")) {
			preview_output(argv, msg, msg_len);
		}
	}

	return 0;
}

/* Find the total number of bits that can be hidden within the number of 
 * .ppm files specified.
 *
 * filename: the name of the .ppm file to be used to hide the message. Files
 *			 must have the same base name and only differ by a sequence number
 *			 (i.e. filename-000.ppm, filename-001.ppm).
 * num_files: the number of files that will be used to hide the message.
 *
 * returns: the number of bits that can be hidden within the number of specified
 *			.ppm files.
 */
int get_total_cap(char * filename, int num_files) {
	int total_file_cap = 0, file_cap, i;

	for (i = 0; i < num_files; i++) {
		/* Opens input image and moves file pointer to the width
		property of the image */
		FILE *inf = open_file(filename, i);
		// The maximum number of bits that can be hidden by combined images
		total_file_cap += get_file_cap(inf);				

		fclose(inf);
	}
	return total_file_cap;
}

/* Dynamically allocates memory to store the message from stdin. If the
 * memory original allocated is not enough, more memory is allocated til
 * the whole message fits.
 *
 * message: a pointer to a character array to contain the message is passed
 *			in to act at a pass by reference
 *
 * returns: the length of the message
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

/* Copies the header of a .ppm file to another file
 * 
 * inf: a file pointer to the .ppm whose header will be copied
 * outf: a file pointer to the file which will contain the copied header
 * num_header_lines: the number of lines within the header of .ppm file
 */
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

/* Counts the number of newline characters in the file to determine
 * how many lines are in the header of the .ppm file.
 *
 * inf: a file pointer to the .ppm file whose header will be counter.
 *
 * returns: the number of lines in the header of the input file
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

/* Hides a message in a .ppm file by using another .ppm file as the base.
 * 
 * inf_name: the base name of the input .ppm file (i.e. without -001.ppm).
 * outf_name: the base name of the output .ppm file (i.e. without -001.ppm).
 * msg: the message to be hidden within the output .ppm file.
 * msg_len: the length of the message to be hidden.
 * num_files: the number of files the message can potentially be hidden within.
 *			  Not all of the files may be needed to hide the message, but all
			  output copies will be produced regardless.
 */
void hide_msg(char *inf_name, char *outf_name, char *msg, int msg_len, int num_files) {
	int i, curr_img = 0, bits_iter = 0;
	char curr_char;
	FILE *inf, *outf;
	do {
		// get the current image that will hide the current portion of the message
		if(num_files < 0) {inf = open_file(inf_name, num_files);}
		else {inf = open_file(inf_name, curr_img);}
		// find how many characters can be stored in the current image
		unsigned int file_cap = get_file_cap(inf);
		// create the corresponding image which will have the message hidden within
		if(num_files < 0) {outf = create_out_file(outf_name, num_files);}
		else {outf = create_out_file(outf_name, curr_img);}
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
		curr_img++;
	} while (curr_img < num_files);
}

/* Hides the length of the message within the first 8 bytes of the .ppm file
 * immediately after the header. Each bit of the message length is hidden in
 * a byte of the .ppm file.
 * 
 * inf: a file pointer to the .ppm file which will used to get each byte to hide
 *     each bit of the message length.
 * outf: a file pointer to the output .ppm file which contains the hidden message.
 * length: the length of the message which will be hidden in the output .ppm file
 */
void encode_length(FILE *inf, FILE *outf, int length) {
	char temp;
	int i, l;
	for (i = 0; i < 8; i++) {
		temp = fgetc(inf);
		l = length;
		l >>= 7 - i;
		if ((l & 1) == 1){
			if ((temp & 1) == 0) temp++;
		}
		else {
			if ((temp & 1) == 1) temp--;
		}
		fputc(temp, outf);
	}

	return;
}


/* Hides a single bit from a byte of a message within a byte of a .ppm file.
 *
 * inf: a file pointer to the input .ppm file.
 * outf: a file pointer to the output .ppm file which will contain the hidden message.
 * msg: the message to be hidden within the output .ppm file.
 * bit_shift: the number of right bitwise shifts required to hide the correct bit of
 *			  the of character from the message.
 * msg_len: the length of the message to be hidden.
 * bits_iter: the number of bits that have been hidded. This counter actually goes beyond
 *			  the number of bits hidden, but is ignored at that point.
 */
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

/* Creates a number of child processes based on the number of message to be
 * to be hidden in parallel.
 * 
 * file: the name of the file that contains the list of input and output files,
 *		 as well as the messages to be hidden.
 */
void hide_fork(char *file) {
	FILE *inf = fopen(file, "r");
	char *msg_file, *inf_img, *outf_img, *msg;
	int more_to_hide = 1;
	
	while (more_to_hide) {
		get_parametres(inf, &msg_file, &inf_img, &outf_img);
		msg = get_msg_from_file(msg_file);
		hide_msg(inf_img, outf_img, msg, strlen(msg), -2);
		switch(fork()) { 
			case 0: // child
				strcpy(msg, get_msg_from_file(msg_file));
				hide_msg(inf_img, outf_img, msg, strlen(msg), -1);
				exit(0);
			case -1:
				perror("Error: Failed to create child process.\nTerminating...\n");
				exit(1);
			default:  // parent
				break;
		}
		more_to_hide = check_for_more(inf);
	}
	wait(NULL);
	fclose(inf);
	free(msg);
}

/* Retrieves a line line of parameters from a text file.
 *
 * inf: a file pointer to the text file that contains the parametres.
 * msg_file: a pointer to a character array which acts as if passed by reference.
 *			 The name of the file containing the message should be passed back.
 * inf_img: a pointer to a character array which acts as if passed by reference.
 *			The name of the input .ppm file should be passed back.
 * outf_img: a pointer to a character array which acts as if passed by reference.
 *			 The name of the output .ppm file should be passed back.
 */
void get_parametres(FILE *inf, char **msg_file, char **inf_img, char **outf_img) {
	char temp;
	int i = 0, str_len = 0;
	int max = 20;
	char *temp_str = (char*)malloc(max);

	while ((temp = fgetc(inf)) != '\n') {
		if (temp == ' ') {
			temp_str[str_len] = 0;
			if (i == 0){
				(*msg_file) = (char*)malloc(max);
				strcpy(*msg_file, temp_str);
			}else if (i == 1) {
				(*inf_img) = (char*)malloc(max);
				strcpy(*inf_img, temp_str);
			}
			else if (i == 2) {
				(*outf_img) = (char*)malloc(max);
				strcpy(*outf_img, temp_str);
			}
			else {
				printf("Error: File is in the wrong format. Only 3 entries permitted for each line.\nTerminating...\n");
				exit(1);
			}
			memset(temp_str,0,strlen(temp_str));
			str_len = 0;
			i++;
		}else {
			temp_str[str_len] = temp;
			str_len++;
		}
		if (str_len == max - 1) {
			max += max;
			temp_str = (char*)realloc(temp_str, max);
		}
	}
	free(temp_str);
}

/* Retrieves a message found within a file.
 * 
 * msg_file: the name of the file containing the message.
 *
 * returns: the message contained within the message file.
 */
char *get_msg_from_file(char *msg_file) {
	FILE *inf = fopen(msg_file, "r");
	int max = 20, c;
	char *message = (char*)malloc(max);

	int msg_len = 0;
	while ((c = fgetc(inf)) != EOF) {
		message[msg_len] = c;
		// allocate more memory if user input is longer than max
		if (msg_len == max - 1) {
			max += max;
			message = (char*)realloc(message, max);
		}
		msg_len++;
	}
	message[msg_len] = 0;
	fclose(inf);
	return message;
}

/* Checks if there are more lines to retrieve data from.
 * 
 * inf: a file pointer to the file containing the data
 *
 * returns: 0 if the end of file has been reached or 1 if the
 *			is still more data to read.
 */
int check_for_more(FILE *inf) {
	char temp = fgetc(inf);
	if (temp == EOF)
		return 0;
	else {
		ungetc(temp, inf);
		return 1;
	}
}

/* Hides a message in a single .ppm image and displays the
 * input .ppm image and the .ppm image hiding the message to
 * the screen.
 *
 * files: the command line inputs are passed in to retrieve
 * 		  the input and output file names.
 * msg: the message to be hidden.
 * msg_len: the length of the message
 */
void preview_output(char **files, char *msg, int msg_len){
	hide_msg(files[2], files[3], msg, msg_len, -1);
	for(int n = 2; n < 4; n++ ) {
		switch( fork()) { 
			case 0: /* child */
				preview_img(files[n], n-2);
				exit(0);
			case -1:
				perror("Error: Failed to create child process.\nTerminating...\n");
				exit(1);
			default:  /* parent */
				break;
		} 
	}
	wait(NULL);
}