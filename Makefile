#CC = gcc  -O2
CC = gcc -g
CFLAGS = -W -Wall

OBJECTS = music_tmux.o common.o play_list.o menu.o arraylist.o

PREFIX?=/usr/local


# glibc lib
GLIB2INC = `pkg-config --cflags libmpg123 ao libconfig` -D_GNU_SOURCE
GLIB2LIBS = `pkg-config --libs libmpg123 ao libconfig` -lpthread


all: clean music_tmux

music_tmux: $(OBJECTS)
	$(CC) $(CFLAGS) $(GLIB2LIBS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS)  $(GLIB2INC) -c $*.c

clean:
	rm -f music_tmux $(OBJECTS)
	rm -rf *.orig *.swp

install:
	install -m 755 music_tmux $(PREFIX)/bin/music_tmux

uninstall:
	rm -f $(PREFIX)/bin/music_tmux
