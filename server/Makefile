# Makefile for Server project

server: netserver.o rd_bin.o crc32.o
	gcc -o server netserver.o rd_bin.o crc32.o

server.o: netserver.c
	gcc -c netserver.c

rd_bin.o: rd_bin.c
	gcc -c rd_bin.c

crc32.o: crc32.c
	gcc -c crc32.c

clean:
	rm -f *.o server
