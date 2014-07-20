CC = gcc -g
CFLAGS = -Wall

OBJECTS = music_tmux.o


# glibc lib
GLIB2INC = `pkg-config --cflags libmpg123 ao` -D_GNU_SOURCE
GLIB2LIBS = `pkg-config --libs libmpg123 ao`

music_tmux: $(OBJECTS)
	$(CC) $(CFLAGS) $(GLIB2LIBS) -o music_tmux $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS)  $(GLIB2INC) -c $*.c

clean:
	rm -f music_tmux $(OBJECTS)
