#include <SDL.h>
#include "preview_img.h"
#include "ppm_check.h"

/* Setting up screen size*/

void preview_img(char *filename) {
	FILE *inf = open_file(filename, -1);
	int width = get_width(inf);
	int height = get_height(inf);
	int num_pixels = width * height;
	check_colour_channel(inf);

	/* The window to render to */
	SDL_Window *window = NULL;

	/* The surface contained by the window */
	SDL_Surface *screenSurface = NULL;

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	else {
		char *preview_title = get_preview_title(filename);
		/* Create the window */
		window = SDL_CreateWindow(preview_title,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height,
			SDL_WINDOW_SHOWN);

		if (window == NULL) {
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		}
		else {
			/* Get screen surface */
			screenSurface = SDL_GetWindowSurface(window);

			/* Fill surface with white */
			SDL_FillRect(screenSurface, NULL, 
				SDL_MapRGB(screenSurface->format, 0xff, 0xff, 0xff));


          	/* Make p point to the place we want to draw the pixel */      
			int i, red, green, blue;			
			for (i = 0; i < num_pixels; i++){
				int x = i % width;
				int y = i / width;
				int *p = (int *)screenSurface->pixels + y * screenSurface->pitch + x * screenSurface->format->BytesPerPixel;            
	
				/* Draw the pixel! */     
				get_channel_info(inf, &red, &green, &blue);
				*p=SDL_MapRGB(screenSurface->format, red, green, blue);            
			}
			SDL_UpdateWindowSurface(window);

			/* Wait two seconds */
			SDL_Delay(10000); 
		}

		/* Destroy the window */
		SDL_DestroyWindow(window);
		SDL_Quit();
		fclose(inf);
	}
}

void get_channel_info(FILE *inf, int (*red), int (*green), int (*blue)) {
	(*red) = fgetc(inf);
	(*green) = fgetc(inf);
	(*blue) = fgetc(inf);
}

char *get_preview_title(char *filename) {
	char* preview_title = "Image Preview";
	char* filename_cpy;
	strcpy(filename_cpy, filename);
	strcat(filename_cpy, ".ppm");
	strcat(preview_title, filename_cpy);
	return preview_title;
}