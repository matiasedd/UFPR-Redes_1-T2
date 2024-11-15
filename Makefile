all:
	gcc -Wall main.c

debug:
	gcc -Wall main.c -DDEBUG
