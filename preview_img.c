#include <SDL.h>

/* Setting up screen size*/
const int SCREEN_WIDTH = get_width(inf);
const int SCREEN_HEIGHT = get_height(inf);

int main(int argc, char *argv[]) {
	/* The window to render to */
	SDL_Window *window = NULL;

	/* The surface contained by the window */
	SDL_Surface *screenSurface = NULL;

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	else {
		/* Create the window */
		window = SDL_CreateWindow("Hello Pixel",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH, SCREEN_HEIGHT,
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
			int i;			
			for (i = 0; i < 10; i++){
				int x = 10 + i *2;
				int y = 20 + i * 3;
				int *p = (int *)screenSurface->pixels + y * screenSurface->pitch + x * screenSurface->format->BytesPerPixel;            
	
				/* Draw the pixel! */     
	
				*p=SDL_MapRGB(screenSurface->format, 244 - i * 14, 58  + i *7, 0);            
				SDL_UpdateWindowSurface(window);
			}


			/* Wait two seconds */
			SDL_Delay(10000); 
		}

		/* Destroy the window */
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
	return 0;
}
