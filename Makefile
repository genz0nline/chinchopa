all: chinchopa

CC=gcc
SRC= src/chinchopa.c src/server.c src/utils/opt.c src/utils/err.c src/utils/log.c src/sys/net.c src/handler.c src/http/req.c src/http/parse.c src/http/headers.c src/utils/utils.c

chinchopa: $(SRC)
	$(CC) -o chinchopa $(SRC) -g -lm

clean:
	rm -f chinchopa *.o *.d
