#
# Makefile for at91 specific clk
#

obj-y += pmc.o sckc.o
obj-y += clk-slow.o clk-main.o clk-plla.o clk-plladiv.o clk-master.o
obj-y += clk-system.o clk-peripheral.o

obj-$(CONFIG_AT91_UTMI)		+= clk-utmi.o
obj-$(CONFIG_AT91_USB_CLK)	+= clk-usb.o
obj-$(CONFIG_AT91_H32MX)	+= clk-h32mx.o
obj-$(CONFIG_AT91_GENERIC_CLK)	+= clk-generated.o
