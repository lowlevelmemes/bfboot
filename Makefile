CC = gcc
CFLAGS = -O2 -pipe

all: bfboot.c
	$(CC) bfboot.c $(CFLAGS) -o bfboot
