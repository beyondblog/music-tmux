CC = gcc -g
CFLAGS = -Wall

OBJECTS = music_tmux.o

music_tmux: $(OBJECTS)
	$(CC) $(CFLAGS) -lmpg123 -lao -o music_tmux $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -lao -I/usr/local/include/mpg123.h -c $*.c

clean:
	rm -f music_tmux $(OBJECTS)
