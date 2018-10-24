CC := gcc
CFLAGS := -Wall -Werror -ggdb

INCLUDE := ../vroom/include
SERVERLINKS := -lwayland-server
OBJECTS := shm.o compositor.o shell.o seat.o shell_surface.o surface.o wayland_buffer.o geometry.o

wayvroom.so: wayvroom.c $(OBJECTS)
	$(CC) $(CFLAGS) -c -fPIC -I$(INCLUDE) -o wayvroom.o $<
	$(CC) $(CFLAGS) -shared -o $@ -fPIC wayvroom.o $(OBJECTS) $(SERVERLINKS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -fPIC -I$(INCLUDE) -o $@ $<

clean:
	rm -rf wayvroom.so
	rm -rf *.o
