# Makefile for Client project

client: netclient.o wr_bin.o crc32.o check_ip.o
	gcc -o client netclient.o wr_bin.o crc32.o check_ip.o

client.o: netclient.c
	gcc -c netclient.c

wr_bin.o: wr_bin.c
	gcc -c wr_bin.c

crc32.o: crc32.c
	gcc -c crc32.c

check_ip.o: check_ip.c
	gcc -c check_ip.c

clean:
	rm -f *.o client
