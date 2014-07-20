CC = gcc -g
CFLAGS = -Wall

OBJECTS = music_tmux.o

music_tmux: $(OBJECTS)
	$(CC) $(CFLAGS) -lmpg123 -o music_tmux $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -I/usr/local/include/mpg123.h -c $*.c

clean:
	rm -f music_tmux $(OBJECTS)
