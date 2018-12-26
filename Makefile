
SRCS := emalloc.c test_emalloc.c
SRCS += utils/hashmap.c

test_emalloc: $(SRCS)
	gcc -g $^ -o $@

all:
	$(MAKE) test_emalloc 

clean:
	rm -f test_emalloc

.DEFAULT: all
