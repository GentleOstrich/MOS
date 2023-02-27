.PHONY: clean

all: fibo.c main.c
	gcc -c main.c fibo.c -I ../include
	gcc -o fibo main.o fibo.o
	mv fibo ../
clean:
	rm *.o
