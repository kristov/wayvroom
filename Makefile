CC := gcc
CFLAGS := -Wall -Werror -ggdb

SERVERLINKS := -lwayland-server
OBJECTS := shm.o compositor.o shell.o seat.o shell_surface.o surface.o wayland_buffer.o

wayvroom.so: main.c $(OBJECTS)
	$(CC) $(CFLAGS) -c -fPIC -o wayvroom.o $<
	$(CC) $(CFLAGS) -shared -o $@ -fPIC wayvroom.o $(OBJECTS) $(SERVERLINKS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

clean:
	rm -rf wayvroom.so
	rm -rf *.o
