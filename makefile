all: clean unhide hide

unhide: ppm_check.c unhide.c
	$(CC) -o unhide ppm_check.c unhide.c

hide: ppm_check.c hide.c
	$(CC) -o hide ppm_check.c hide.c

clean:
	rm -f unhide hide
