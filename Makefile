
#### Many readers, few writers, infrequent writing
#### posix soultion takes forever
DEFS := NUM_WRITERS=8 NUM_READERS=256 SLEEP_READ=100 SLEEP_WRITE=1000000 NUM_UPDATES=10000
#DEFS := NUM_WRITERS=8 NUM_READERS=32 SLEEP_READ=100 SLEEP_WRITE=100000 NUM_UPDATES=1000

#### Few readers, many writers, infrequent writing
#DEFS := NUM_WRITERS=256 NUM_READERS=8 SLEEP_READ=10 SLEEP_WRITE=100000 NUM_UPDATES=1000

#### Many readers, few writers, frequent writing
#### posix solution takes forever
#DEFS := NUM_WRITERS=8 NUM_READERS=256 SLEEP_READ=100000 SLEEP_WRITE=10 NUM_UPDATES=1000

#### Few readers, few writers, frequent writing
#DEFS := NUM_WRITERS=8 NUM_READERS=4 SLEEP_READ=100000 SLEEP_WRITE=10 NUM_UPDATES=1000


CC := gcc
CFLAGS := -std=gnu99 -Wall -Wextra -O2 -DNDEBUG
#CFLAGS := -std=gnu99 -Wall -Wextra -O0 -g
LIBS := pthread

POSIX_OBJS = test.o posix.o
MY_OBJS = test.o rwlock.o

.PHONY: impl_rw posix_rw all clean

all: impl_rw posix_rw

clean:
	-$(RM) $(POSIX_OBJS) $(MY_OBJS)
	-$(RM) posix_rw impl_rw


posix_rw: $(POSIX_OBJS)
	$(CC) -o $@ $^ $(addprefix -l,$(LIBS)) 


impl_rw: $(MY_OBJS)
	$(CC) -o $@ $^ $(addprefix -l,$(LIBS))


%.o: %.c
	$(CC) $(CFLAGS) $(addprefix -D,$(DEFS)) -o $@ $< -c
