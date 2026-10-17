#
# Makefile for at91 specific clk
#

ifdef CONFIG_CLK_CCF
obj-y += pmc.o sckc.o clk-main.o clk-master.o clk-programmable.o clk-system.o
obj-y += clk-peripheral.o

obj-$(CONFIG_AT91_GENERIC_CLK)	+= clk-generic.o
obj-$(CONFIG_AT91_UTMI)		+= clk-utmi.o
obj-$(CONFIG_AT91_SAM9X60_PLL)	+= clk-sam9x60-pll.o
obj-$(CONFIG_AT91_SAM9X60_USB)	+= clk-sam9x60-usb.o
obj-$(CONFIG_SAMA7G5)		+= sama7g5.o
obj-$(CONFIG_SAM9X60)		+= sam9x60.o
else
obj-y += compat.o
endif
