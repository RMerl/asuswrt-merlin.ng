.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Patrick Delaunay <patrick.delaunay@foss.st.com>

U-Boot device tree bindings
---------------------------

The U-Boot specific bindings are defined in the U-Boot directory:
doc/device-tree-bindings

* clock
        - :download:`clock/st,stm32mp1.txt <../../device-tree-bindings/clock/st,stm32mp1.txt>`
* ram
        - :download:`memory-controllers/st,stm32mp1-ddr.txt <../../device-tree-bindings/memory-controllers/st,stm32mp1-ddr.txt>`

All the other device tree bindings used in U-Boot are specified in Linux
kernel. Please refer dt bindings from below specified paths in the Linux
kernel binding directory = Documentation/devicetree/bindings/

* acd
	- iio/adc/st,stm32-adc.yaml
* clock
        - clock/st,stm32-rcc.txt
        - clock/st,stm32h7-rcc.txt
	- clock/st,stm32mp1-rcc.yaml
* display
	- display/st,stm32-dsi.yaml
	- display/st,stm32-ltdc.yaml
* gpio
	- pinctrl/st,stm32-pinctrl.yaml
* hwlock
	- hwlock/st,stm32-hwspinlock.yaml
* i2c
	- i2c/st,stm32-i2c.yaml
* mailbox
	- mailbox/st,stm32-ipcc.yaml
* mmc
	- mmc/arm,pl18x.yaml
* nand
	- mtd/st,stm32-fmc2-nand.yaml
	- memory-controllers/st,stm32-fmc2-ebi.yaml
* net
        - net/stm32-dwmac.yaml
* nvmem
        - nvmem/st,stm32-romem.yaml
* remoteproc
	- remoteproc/st,stm32-rproc.yaml
* regulator
	- regulator/st,stm32mp1-pwr-reg.yaml
	- regulator/st,stm32-vrefbuf.yaml
* reset
	- reset/st,stm32-rcc.txt
	- reset/st,stm32mp1-rcc.txt
* rng
	- rng/st,stm32-rng.yaml
* rtc
	- rtc/st,stm32-rtc.yaml
* serial
	- serial/st,stm32-uart.yaml
* spi
	- spi/st,stm32-spi.yaml
	- spi/st,stm32-qspi.yaml
* syscon
        - arm/stm32/st,stm32-syscon.yaml
* usb
	- phy/phy-stm32-usbphyc.yaml
        - usb/dwc2.yaml
* watchdog
	- watchdog/st,stm32-iwdg.yaml
