CFLAGS=`sdl2-config --cflags`
SDLLIBS=`sdl2-config --libs`

all: clean unhide hide

unhide: ppm_check.c unhide_A2.c
	$(CC) -g -o unhide ppm_check.c unhide_A2.c

hide: ppm_check.c preview_img.c hide_A2.c
	$(CC) -g -o hide ppm_check.c preview_img.c hide_A2.c $(CFLAGS) $(SDLLIBS)

clean:
	rm -f unhide hide 