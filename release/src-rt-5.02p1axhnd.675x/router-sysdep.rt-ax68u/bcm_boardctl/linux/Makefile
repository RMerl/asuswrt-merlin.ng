LIB  := oal.a

OBJS := board.o

all install: $(LIB)

clean:
	rm -f *.o *.d $(LIB)

$(LIB) : $(OBJS)
	$(AR) -rcs $@ $(OBJS)


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
