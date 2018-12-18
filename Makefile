
test_ealloc:
	gcc -g -pg emalloc.c -o $@

all:
	$(MAKE) test_ealloc 

clean:
	rm -f test_ealloc

.DEFAULT: all
