obj-$(CONFIG_BCM_DPI) += dpicore.o

dpicore-objs := dpi.o dpi_tables.o

ccflags-y += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
ccflags-y += -I$(BRCMDRIVERS_DIR)/broadcom/char/dpiengine/impl$(CONFIG_BCM_DPI_IMPL)/include
ccflags-y += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
ccflags-y += -Werror -Wall
