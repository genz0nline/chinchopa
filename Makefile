all: chinchopa tests

CC=gcc
SRC=src/server.c src/utils/opt.c src/utils/err.c src/utils/log.c src/sys/net.c src/handler.c src/http/req.c src/http/parse.c src/http/headers.c src/utils/utils.c src/http/resp.c src/http/status.c src/sys/dsk.c src/http/route.c
SRC_TST= ${SRC} src/tests.c src/http/tests/route_tests.c src/http/tests/headers_tests.c

chinchopa: $(SRC)
	$(CC) -o chinchopa src/chinchopa.c $(SRC) -g -lm

tests: $(SRC_TST)
	$(CC) -o tests $(SRC_TST) -g -lm

clean:
	rm -f chinchopa tests *.o *.d
