#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "ppm_check.h"


// Returns 1 if P6, else returns 0
int correct_magic_num(FILE *fp) {
	char temp = fgetc(fp);
	if (temp == 'P' && fgetc(fp) == '6') {
		fgetc(fp);
		return 1;
	}
	return 0;
}


void ignore_comments(FILE *fp) {
	char temp;
	// checks that the leading character is a #
	while ((temp = fgetc(fp)) == '#') {
		/* Steps through each character in the 
		line until it reaches a newline */
		while (fgetc(fp) != '\n') {}
	}

	// Pushes back the width
	ungetc(temp, fp);
}

// Returns the width of the image
int get_width(FILE *fp) {
	int w;
	fscanf(fp, "%d", &w);
	return w;
}

// Returns the height of the image
int get_height(FILE *fp) {
	int h;
	fscanf(fp, "%d", &h);
	return h;
}

// Reads the color depth, returns 1 if 255, else returns 0
int check_colour_channel(FILE *fp) {
	int c;
	fscanf(fp, "%d", &c);
	if (c == 255) {
		//Get rid of the trailing whitespace 
		fgetc(fp);
		return 1;
	}
	return 0;
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
	return inf_name_cpy;
}