LIB  = libnetfilter_queue.a
OBJS = libnetfilter_queue.o 
CFLAGS := -D_GNU_SOURCE -I../include\
          -I../../libnfnetlink/include

all: $(LIB)

clean: 
	rm -f $(LIB) *.o

$(LIB): $(OBJS)
	$(AR) -rcs $@ $(OBJS)
	mv $(LIB) ../
