CC = gcc
CFLAGS = -Wall
DFLAGS = -DDEBUG

OUT = a.out

all: blackjack.o
	$(CC) $(CFLAGS) main.c blackjack.o -o $(OUT)

debug: blackjack.o
	$(CC) $(CFLAGS) main.c blackjack.o $(DFLAGS) -o $(OUT)

blackjack.o: blackjack.c blackjack.h
	$(CC) $(CFLAGS) -c blackjack.c

clean:
	rm -f blackjack.o

purge: clean
	rm -f $(OUT)
