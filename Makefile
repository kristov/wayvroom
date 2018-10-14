CC := gcc
CFLAGS := -Wall -Werror -ggdb

SERVERLINKS := -lwayland-server
OBJECTS := shm.o compositor.o shell.o seat.o shell_surface.o surface.o wayland_buffer.o

wayvroom: main.c $(OBJECTS)
	$(CC) $(CFLAGS) $(SERVERLINKS) -o $@ $< $(OBJECTS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf wayvroom
	rm -rf *.o
