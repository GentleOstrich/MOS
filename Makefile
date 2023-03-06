.PHONY: clean
all: test.c
	gcc -o test test.c
run: test
	./test
clean:
	rm ./test
