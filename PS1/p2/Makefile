# Makefile C to ASM
# Andrew Smith
# 1/24/20


PROG := binarysearch

CFLAGS += -m32 -Wall -std=c99

.PHONY: clean

all: CFLAGS += -O0
all: $(PROG)

debug: CFLAGS += -O0 -g
debug: $(PROG)

$(PROG): binarysearch_asm.o binarysearch.o main.o
	 $(CC) -m32 $^ -o $@

binarysearch_asm.o: binarysearch_asm.S
	$(CC) $(CFLAGS) -c $< -o $@

binarysearch.o: binarysearch.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(PROG)
