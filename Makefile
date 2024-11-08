CFLAGS=-g -Wall -O2
all: irreader parsedata irwriter ircode.c

irreader: irreader.o
	gcc -o irreader irreader.o -lrt -lpigpio -lpthread
irwriter: irwriter.o ircode.o
	gcc -o irwriter irwriter.o ircode.o -lrt -lpigpio -lpthread
parsedata: parsedata.o
	gcc -o parsedata parsedata.o
ircode.c: parsedata data.txt
	./parsedata > ircode.c

upload: clean
	scp -pr * extra:src/irreader
backup: clean
	tar -jcf - . | jbackup src.irreader.tar.bz2

clean:
	rm -f *.o irreader parsedata irwriter ircode.c
