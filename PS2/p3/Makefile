CC = gcc
CFLAGS = -Wall -pthread

# Target for the final executable
all: test

# Corrected target name to 'test' instead of 'main'
test: main.o solution.o spinlock_ece391.o
	$(CC) $(CFLAGS) -o test main.o solution.o spinlock_ece391.o

main.o: main.c solution.h spinlock_ece391.h
	$(CC) $(CFLAGS) -c main.c

solution.o: solution.c solution.h spinlock_ece391.h
	$(CC) $(CFLAGS) -c solution.c

spinlock_ece391.o: spinlock_ece391.c spinlock_ece391.h
	$(CC) $(CFLAGS) -c spinlock_ece391.c

# Updated clean command to remove 'test' executable
clean:
	rm -f *.o test
