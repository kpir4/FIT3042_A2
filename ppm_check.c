#include <stdio.h> 
#include <stdlib.h>
#include "ppm_check.h"


// Returns 1 if P6, else returns 0
int read_magic_num(FILE *fp) {
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
