HOST_EXTRACFLAGS := -DLOCALE 
HOST_LOADLIBES   := -lncurses 

ifneq ($(wildcard /usr/lib/libtinfo.so),)
HOST_LOADLIBES   += -ltinfo
endif

ifeq (/usr/include/ncurses/ncurses.h, $(wildcard /usr/include/ncurses/ncurses.h))
        HOST_EXTRACFLAGS += -I/usr/include/ncurses -DCURSES_LOC="<ncurses.h>"
else
ifeq (/usr/include/ncurses/curses.h, $(wildcard /usr/include/ncurses/curses.h))
        HOST_EXTRACFLAGS += -I/usr/include/ncurses -DCURSES_LOC="<ncurses/curses.h>"
else
ifeq (/usr/include/ncurses.h, $(wildcard /usr/include/ncurses.h))
        HOST_EXTRACFLAGS += -DCURSES_LOC="<ncurses.h>"
else
	HOST_EXTRACFLAGS += -DCURSES_LOC="<curses.h>"
endif
endif
endif

host-progs	:= lxdialog
always		:= ncurses $(host-progs)

lxdialog-objs := checklist.o menubox.o textbox.o yesno.o inputbox.o \
		 util.o lxdialog.o msgbox.o

# CONFIG_MIPS_BRCM Begin Broadcom changed code.
%.o: %.c
	$(HOSTCC) $(HOST_EXTRACFLAGS) -c -o $@ $<

lxdialog: $(obj)/ncurses $(lxdialog-objs)
	$(HOSTCC) -o lxdialog $(lxdialog-objs) $(HOST_LOADLIBES) 
# CONFIG_MIPS_BRCM End Broadcom changed code.

.PHONY: $(obj)/ncurses
$(obj)/ncurses:
	@echo "main() {}" > lxtemp.c
	@if $(HOSTCC) lxtemp.c  $(HOST_LOADLIBES); then \
		rm -f lxtemp.c a.out; \
	else \
		rm -f lxtemp.c; \
		echo -e "\007" ;\
		echo ">> Unable to find the Ncurses libraries." ;\
		echo ">>" ;\
		echo ">> You must install ncurses-devel in order" ;\
		echo ">> to use 'make menuconfig'" ;\
		echo ;\
		exit 1 ;\
	fi
