
DEFS := NUM_WRITERS=4 NUM_READERS=256 SLEEP_READ=100 SLEEP_WRITE=100000 NUM_UPDATES=10000


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
