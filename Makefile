
# Should we test for correctness?
TEST_FOR_CORRECTNESS := 1

# Default values
DEFS := NUM_WRITERS=1 NUM_READERS=64 SLEEP_READ=100 SLEEP_WRITE=100000 NUM_UPDATES=10000 CORRECTNESS=$(TEST_FOR_CORRECTNESS)

CC := gcc
CFLAGS := -std=gnu99 -Wall -Wextra -Wno-unused-parameter


.PHONY: wrpref optimized posix all clean distclean debug release

all: optimized posix wrpref

debug: CFLAGS += -O0 -g
debug: all

release: CFLAGS += -O2 -DNDEBUG
release: all

clean:
	-$(RM) wrpref.o posix.o optimized.o wrpref_test.o posix_test.o optimized_test.o

distclean: clean
	-$(RM) wrpref_test posix_test optimized_test

optimized: optimized_test

posix: posix_test

wrpref: wrpref_test

%_test: %.o 
	$(CC) $(CFLAGS) $(addprefix -D,$(DEFS)) -D$@ -o $@.o test.c -c
	$(CC) -o $@ $@.o $^ -pthread

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ $< -c
