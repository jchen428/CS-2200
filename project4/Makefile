# Makefile for Project 4 - Cache - Spring 2016

SUBMIT = cachesim.c cachesim.h cachesim_driver.c Makefile
CFLAGS = -g -Wall -std=c99 -lm -pedantic-errors
CC=gcc

all: cachesim

cachesim: cachesim.o cachesim_driver.o
	$(CC) -o cachesim cachesim.o cachesim_driver.o $(CFLAGS)

analyzer: cachesim.o analyzer.o
	$(CC) -o cachesim cachesim.o analyzer.o $(CFLAGS)

clean:
	rm -f cachesim *.o out.txt

submit: clean
	tar zcvf prj4-submit.tar.gz $(SUBMIT)
