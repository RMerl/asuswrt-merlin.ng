CUR_DIR := $(BRCMDRIVERS_DIR)/broadcom/char/pktflow/impl1

obj-$(CONFIG_BCM_PKTFLOW) += pktflow.o

pktflow-objs += pktflow_dep.o

$(obj)/pktflow_dep.o:
	cp $(obj)/pktflow.o $(obj)/pktflow_dep.o
	mkdir -p $(INSTALL_DIR)/bin
	install -m 755 $(CUR_DIR)/scripts/FcStats.sh $(INSTALL_DIR)/bin

applinks:

