all: clean unhide hide

unhide: ppm_check.c unhide.c
	$(CC) -o unhide ppm_check.c unhide.c

hide: ppm_check.c hide_A2.c
	$(CC) -o hide ppm_check.c hide_A2.c

clean:
	rm -f unhide hide
