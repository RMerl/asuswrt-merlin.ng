LIB  := oal.a


ifeq ($(strip $(DESKTOP_LINUX)),y)
# Use fake file for LINUX_DESKTOP builds
OBJS := xtm_desktop.o
else
# Normal object files
OBJS := xtm.o xtm_trace.o
endif


all: $(LIB)


$(LIB) : $(OBJS)
	$(AR) -rcs $@ $(OBJS)


clean:
	rm -f *.o *.d $(LIB)


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
