
-include $(SRCBASE)/router/.config

CFLAGS = -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/nuc029ctrl/include/

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS += -L$(LIBDIR)
EXTRACC += -ldl
endif

LD_BINARY  += $(LDFLAGS) $(EXTRACC)
LD_LIBRARY += $(LDFLAGS) $(EXTRALD)

OBJS = libnuc029.o

vpath %.c $(SRCBASE)/nuc029ctrl/

ifeq ($(wildcard $(SRCBASE)/router/nuc029ctrl/lib/*.c),)
all: 
	-cp -f prebuild/libnuc029.so libnuc029.so
else
all: libnuc029.so libnuc029.a
endif

libnuc029.so: $(OBJS)
	@echo " [$@] LD $@"
	@$(LD) $(LD_LIBRARY) -shared -o $@ $^
	@$(SIZECHECK)
	@$(CPTMP)

libnuc029.a: $(OBJS)
	@echo " [$@] AR $@"
	@rm -f $@
	@$(AR) cruv $@ $^

clean:
	rm -f *.o *.so *.a .*.depend

%.o: %.c .%.depend
	@echo " [libnuc029] CC $@ $(CFLAGS)"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

