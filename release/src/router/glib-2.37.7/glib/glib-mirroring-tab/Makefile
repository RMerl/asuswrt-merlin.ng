
CFLAGS = `pkg-config --cflags glib-2.0`
LIBS = `pkg-config --libs glib-2.0`


all: gen-mirroring-tab

gen-mirroring-tab: gen-mirroring-tab.o packtab.o

clean:
	$(RM) gen-mirroring-tab *.o
