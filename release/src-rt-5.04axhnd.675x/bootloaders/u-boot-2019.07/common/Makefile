# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2004-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

# core
ifndef CONFIG_SPL_BUILD
obj-y += init/
obj-y += main.o
obj-y += exports.o
obj-$(CONFIG_HASH) += hash.o
obj-$(CONFIG_HUSH_PARSER) += cli_hush.o
obj-$(CONFIG_AUTOBOOT) += autoboot.o

# This option is not just y/n - it can have a numeric value
ifdef CONFIG_BOOT_RETRY_TIME
obj-y += bootretry.o
endif

# # boards
obj-y += board_f.o
obj-y += board_r.o
obj-$(CONFIG_DISPLAY_BOARDINFO) += board_info.o
obj-$(CONFIG_DISPLAY_BOARDINFO_LATE) += board_info.o

obj-$(CONFIG_CMD_BOOTM) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTZ) += bootm.o bootm_os.o
obj-$(CONFIG_CMD_BOOTI) += bootm.o bootm_os.o

obj-$(CONFIG_CMD_BEDBUG) += bedbug.o
obj-$(CONFIG_$(SPL_TPL_)OF_LIBFDT) += fdt_support.o
obj-$(CONFIG_MII) += miiphyutil.o
obj-$(CONFIG_CMD_MII) += miiphyutil.o
obj-$(CONFIG_PHYLIB) += miiphyutil.o

ifdef CONFIG_USB
obj-y += usb.o usb_hub.o
obj-$(CONFIG_USB_STORAGE) += usb_storage.o
endif

# others
obj-$(CONFIG_CONSOLE_MUX) += iomux.o
obj-$(CONFIG_MTD_NOR_FLASH) += flash.o
obj-$(CONFIG_CMD_KGDB) += kgdb.o kgdb_stubs.o
obj-$(CONFIG_I2C_EDID) += edid.o
obj-$(CONFIG_KALLSYMS) += kallsyms.o
obj-y += splash.o
obj-$(CONFIG_SPLASH_SOURCE) += splash_source.o
ifndef CONFIG_DM_VIDEO
obj-$(CONFIG_LCD) += lcd.o lcd_console.o
endif
obj-$(CONFIG_LCD_ROTATION) += lcd_console_rotation.o
obj-$(CONFIG_LCD_DT_SIMPLEFB) += lcd_simplefb.o
obj-$(CONFIG_LYNXKDI) += lynxkdi.o
obj-$(CONFIG_MENU) += menu.o
obj-$(CONFIG_UPDATE_TFTP) += update.o
obj-$(CONFIG_DFU_TFTP) += update.o
obj-$(CONFIG_USB_KEYBOARD) += usb_kbd.o
obj-$(CONFIG_CMDLINE) += cli_readline.o cli_simple.o

endif # !CONFIG_SPL_BUILD

obj-$(CONFIG_$(SPL_TPL_)BOOTSTAGE) += bootstage.o
obj-$(CONFIG_$(SPL_TPL_)BLOBLIST) += bloblist.o

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_DFU
obj-$(CONFIG_DFU_OVER_USB) += dfu.o
endif
obj-$(CONFIG_SPL_HASH_SUPPORT) += hash.o
obj-$(CONFIG_TPL_HASH_SUPPORT) += hash.o
obj-$(CONFIG_SPL_YMODEM_SUPPORT) += xyzModem.o
obj-$(CONFIG_$(SPL_TPL_)LOAD_FIT) += common_fit.o
obj-$(CONFIG_SPL_NET_SUPPORT) += miiphyutil.o
obj-$(CONFIG_$(SPL_TPL_)OF_LIBFDT) += fdt_support.o

ifdef CONFIG_SPL_USB_HOST_SUPPORT
obj-y += usb.o
obj-y += usb_hub.o
obj-$(CONFIG_SPL_USB_STORAGE) += usb_storage.o
else
obj-$(CONFIG_USB_MUSB_HOST) += usb.o
endif
obj-$(CONFIG_SPL_STANDALONE) += exports.o
endif # CONFIG_SPL_BUILD

#others
obj-$(CONFIG_DDR_SPD) += ddr_spd.o
obj-$(CONFIG_SPD_EEPROM) += ddr_spd.o
obj-$(CONFIG_HWCONFIG) += hwconfig.o
obj-$(CONFIG_BOUNCE_BUFFER) += bouncebuf.o
ifdef CONFIG_SPL_BUILD
ifdef CONFIG_TPL_BUILD
obj-$(CONFIG_TPL_SERIAL_SUPPORT) += console.o
else
obj-$(CONFIG_SPL_SERIAL_SUPPORT) += console.o
endif
else
obj-y += console.o
endif # CONFIG_SPL_BUILD

obj-$(CONFIG_CROS_EC) += cros_ec.o
obj-y += dlmalloc.o
ifdef CONFIG_SYS_MALLOC_F
ifneq ($(CONFIG_$(SPL_TPL_)SYS_MALLOC_F_LEN),0)
obj-y += malloc_simple.o
endif
endif

obj-y += image.o
obj-$(CONFIG_ANDROID_BOOT_IMAGE) += image-android.o
obj-$(CONFIG_$(SPL_TPL_)OF_LIBFDT) += image-fdt.o
obj-$(CONFIG_$(SPL_TPL_)FIT) += image-fit.o
obj-$(CONFIG_$(SPL_)MULTI_DTB_FIT) += boot_fit.o common_fit.o
obj-$(CONFIG_$(SPL_TPL_)FIT_SIGNATURE) += image-sig.o
obj-$(CONFIG_IO_TRACE) += iotrace.o
obj-y += memsize.o
obj-y += stdio.o

obj-$(CONFIG_CMD_DTIMG) += image-android-dt.o

ifdef CONFIG_CMD_EEPROM_LAYOUT
obj-y += eeprom/eeprom_field.o eeprom/eeprom_layout.o
endif

obj-y += cli.o
obj-$(CONFIG_FSL_DDR_INTERACTIVE) += cli_simple.o cli_readline.o
obj-$(CONFIG_STM32MP1_DDR_INTERACTIVE) += cli_simple.o cli_readline.o
obj-$(CONFIG_DFU_OVER_USB) += dfu.o
obj-y += command.o
obj-$(CONFIG_$(SPL_TPL_)LOG) += log.o
obj-$(CONFIG_$(SPL_TPL_)LOG_CONSOLE) += log_console.o
obj-y += s_record.o
obj-$(CONFIG_CMD_LOADB) += xyzModem.o
obj-$(CONFIG_$(SPL_TPL_)YMODEM_SUPPORT) += xyzModem.o

obj-$(CONFIG_AVB_VERIFY) += avb_verify.o
