archer-objs := archer_dep.o

obj-$(CONFIG_BCM_ARCHER) += archer.o

clean:
	rm -f core *.ko *.o *.a *.s

$(obj)/archer_dep.o:
	cp $(obj)/archer.o $(obj)/archer_dep.o
