CC=gcc

chinchopa: chinchopa.c
	$(CC) -o chinchopa chinchopa.c

clean:
	rm -f chinchopa *.o *.d
