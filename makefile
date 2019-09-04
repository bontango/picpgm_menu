# for simple_flasher

CC      = /usr/bin/gcc
CFLAGS  = -Wall -g -D_REENTRANT -DDEBUG

LDFLAGS = -L/usr/local/lib
LDLIBS    = -lwiringPi -lwiringPiDev -lpthread -lm -lcrypt -lrt


OBJ = picpgm_menu.o

picpgm_menu: $(OBJ)
	$(CC) $(CFLAGS) -o picpgm_menu $(OBJ) $(LDFLAGS) $(LDLIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o picpgm_menu
