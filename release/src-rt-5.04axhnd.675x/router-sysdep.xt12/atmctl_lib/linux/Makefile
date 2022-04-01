name := $(lastword $(MAKEFILE_LIST))
mdir := $(dir $(name))
VPATH=$(mdir)

LIB = oal.a
OBJS = xtm.o xtm_trace.o

ifeq ($(strip $(DESKTOP_LINUX)),y)
OBJS = xtm_desktop.o
endif

all: $(LIB)

ifneq ($(wildcard $(mdir)*.c),)
$(LIB) : $(OBJS)
	$(AR) -rcs $@ $(OBJS)

else
$(LIB) : $(LIB)_shipped
	echo cp $< $@

endif

clean:
	rm -f $(OBJS) *.o *.d $(LIB)

install: $(LIB)
	cp $(LIB) $(PREFIX)/$(LIB)


# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)
