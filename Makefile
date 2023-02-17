.PHONY: clean

out: calc case_all
	./calc < case_all > out
case_all: casegen.c
	make case_add
	make case_sub
	make case_mul
	make case_div
	cat case_add >> case_all
	cat case_sub >> case_all
	cat case_mul >> case_all
	cat case_div >> case_all
case_add: casegen.c
	gcc -o casegen casegen.c
	./casegen add 100 > case_add
case_sub: casegen.c
	gcc -o casegen casegen.c
	./casegen sub 100 > case_sub
case_mul: casegen.c
	gcc -o casegen casegen.c
	./casegen mul 100 > case_mul
case_div: casegen.c
	gcc -o casegen casegen.c
	./casegen div 100 > case_div



# Your code here.

clean:
	rm -f out calc casegen case_*
