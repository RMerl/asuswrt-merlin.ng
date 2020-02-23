
SRCS = main.c fdt.c fdt_ro.c fdt_rw.c fdt_wip.c
OBJS = $(SRCS:%.c=%.o)

CFLAGS=-Wall -I. -I ../include

.c.o:
	$(CC) $(CFLAGS) -c $<

fdt: $(OBJS)
	$(CC) $(CFLAGS) -o fdt $(OBJS)

cleanall:
	-rm -f *.o app
