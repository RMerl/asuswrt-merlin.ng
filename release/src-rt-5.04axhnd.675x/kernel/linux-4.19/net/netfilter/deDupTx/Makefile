include ../../.config

obj-$(CONFIG_NETFILTER_ROUTER_BOOST) += deDupTx.o

deDupTx-objs := deDupTx_preb.o

$(obj)/deDupTx_preb.o:
	cp $(obj)/deDupTx.o $(obj)/deDupTx_preb.o

clean-files += *.o .*.o.cmd
