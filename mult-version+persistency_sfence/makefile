TARGET = sps
OBJS = sps.o
#CFLAGS = -O0 -g -ggdb -w  -Wall
CFLAGS = -O0 -w -g -fno-builtin #-DSPS_DEBUG
#CFLAGS = -O3 -w -g -fno-builtin #-DSPS_DEBUG
LDFLAGS = -lpthread
#LDFLAGS = /gpfs/home/juz138/work/software/pin-2.11/source/tools/Pthread/libmypthread.a
WRAPPER_OBJ = #/gpfs/home/juz138/work/software/pin-2.11/source/tools/Apps/mcsim_wrapper.o
CC = g++ 

$(TARGET): $(OBJS) $(WRAPPER_OBJ)
	$(CC) $(OBJS) $(WRAPPER_OBJ) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

.SUFFIXES:
.SUFFIXES:	.o .c .C .h .H

$(WRAPPER_OBJ): %.o: %.c
	gcc -O0 -g -c $< -o $@

$(OBJS): %.o: %.cc
	$(CC) -c $(CFLAGS) $(LDLAGS) -c $< -o $@

clean:
	rm -f *.o $(OBJS) $(TARGET) pin*.log
