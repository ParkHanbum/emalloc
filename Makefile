
test_emalloc:
	gcc -g test_emalloc.c emalloc.c -o $@

all:
	$(MAKE) test_emalloc 

clean:
	rm -f test_emalloc

.DEFAULT: all
