all: chinchopa

CC=gcc
SRC= src/chinchopa.c src/server.c src/utils/opt.c

chinchopa: $(SRC)
	$(CC) -o chinchopa $(SRC) -s -lm

clean:
	rm -f chinchopa *.o *.d
