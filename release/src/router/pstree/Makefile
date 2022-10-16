
OBJS = pstree.o

all: pstree

.c.o:
	$(CC) -c $*.c $(CFLAGS)

pstree: $(OBJS)
	$(CC) -o pstree $(OBJS) $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	--rm -rf stamp-h1 pstree $(OBJS)
