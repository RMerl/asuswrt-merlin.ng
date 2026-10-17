# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_DM
obj-$(CONFIG_$(SPL_TPL_)BACKLIGHT) += backlight-uclass.o
obj-$(CONFIG_BACKLIGHT_GPIO) += backlight_gpio.o
obj-$(CONFIG_BACKLIGHT_PWM) += pwm_backlight.o
obj-$(CONFIG_$(SPL_TPL_)CONSOLE_NORMAL) += console_normal.o
obj-$(CONFIG_CONSOLE_ROTATION) += console_rotate.o
ifdef CONFIG_$(SPL_TPL_)CONSOLE_NORMAL
obj-y += console_core.o
else ifdef CONFIG_CONSOLE_ROTATION
obj-y += console_core.o
endif
obj-$(CONFIG_CONSOLE_ROTATION) += console_core.o
obj-$(CONFIG_CONSOLE_TRUETYPE) += console_truetype.o fonts/
obj-$(CONFIG_DISPLAY) += display-uclass.o
obj-$(CONFIG_VIDEO_MIPI_DSI) += dsi-host-uclass.o
obj-$(CONFIG_$(SPL_TPL_)VIDEO) += video-uclass.o vidconsole-uclass.o
obj-$(CONFIG_$(SPL_TPL_)VIDEO) += video_bmp.o
obj-$(CONFIG_$(SPL_TPL_)PANEL) += panel-uclass.o
obj-$(CONFIG_PANEL_HX8238D) += hx8238d.o
obj-$(CONFIG_$(SPL_TPL_)SIMPLE_PANEL) += simple_panel.o

obj-$(CONFIG_VIDEO_LOGO) += u_boot_logo.o
obj-$(CONFIG_$(SPL_TPL_)BMP) += bmp.o

endif

obj-$(CONFIG_BACKLIGHT_LM3533) += lm3533_backlight.o
obj-${CONFIG_EXYNOS_FB} += exynos/
obj-${CONFIG_VIDEO_ROCKCHIP} += rockchip/
obj-${CONFIG_VIDEO_STM32} += stm32/
obj-${CONFIG_VIDEO_TEGRA124} += tegra124/
obj-${CONFIG_$(SPL_)VIDEO_TIDSS} += tidss/
obj-y += ti/

obj-$(CONFIG_ATMEL_HLCD) += atmel_hlcdfb.o
obj-$(CONFIG_ATMEL_LCD) += atmel_lcdfb.o
obj-$(CONFIG_IHS_VIDEO_OUT) += ihs_video_out.o
obj-$(CONFIG_LOGICORE_DP_TX) += logicore_dp_tx.o
obj-$(CONFIG_NXP_TDA19988) += tda19988.o
obj-$(CONFIG_OSD) += video_osd-uclass.o
obj-$(CONFIG_SANDBOX_OSD) += sandbox_osd.o
obj-$(CONFIG_VIDEO_ARM_MALIDP) += mali_dp.o
obj-$(CONFIG_VIDEO_BCM2835) += bcm2835.o
obj-$(CONFIG_VIDEO_BOCHS) += bochs.o
obj-$(CONFIG_VIDEO_BROADWELL_IGD) += broadwell_igd.o
obj-$(CONFIG_VIDEO_COREBOOT) += coreboot.o
obj-$(CONFIG_VIDEO_DW_HDMI) += dw_hdmi.o
obj-$(CONFIG_VIDEO_DW_MIPI_DSI) += dw_mipi_dsi.o
obj-$(CONFIG_VIDEO_EFI) += efi.o
obj-$(CONFIG_VIDEO_IPUV3) += imx/
obj-$(CONFIG_VIDEO_IVYBRIDGE_IGD) += ivybridge_igd.o
obj-$(CONFIG_VIDEO_LCD_ANX9804) += anx9804.o
obj-$(CONFIG_VIDEO_LCD_ENDEAVORU) += endeavoru-panel.o
obj-$(CONFIG_VIDEO_LCD_HIMAX_HX8394) += himax-hx8394.o
obj-$(CONFIG_VIDEO_LCD_HITACHI_TX18D42VM) += hitachi_tx18d42vm_lcd.o
obj-$(CONFIG_VIDEO_LCD_ORISETECH_OTM8009A) += orisetech_otm8009a.o
obj-$(CONFIG_VIDEO_LCD_RAYDIUM_RM68200) += raydium-rm68200.o
obj-$(CONFIG_VIDEO_LCD_RENESAS_R61307) += renesas-r61307.o
obj-$(CONFIG_VIDEO_LCD_RENESAS_R69328) += renesas-r69328.o
obj-$(CONFIG_VIDEO_LCD_SSD2828) += ssd2828.o
obj-$(CONFIG_VIDEO_LCD_TDO_TL070WSH30) += tdo-tl070wsh30.o
obj-$(CONFIG_VIDEO_MCDE_SIMPLE) += mcde_simple.o
obj-${CONFIG_VIDEO_MESON} += meson/
obj-${CONFIG_VIDEO_MIPI_DSI} += mipi_dsi.o
obj-$(CONFIG_VIDEO_MVEBU) += mvebu_lcd.o
obj-$(CONFIG_VIDEO_MXS) += mxsfb.o videomodes.o
obj-$(CONFIG_VIDEO_NX) += nexell_display.o videomodes.o nexell/
obj-$(CONFIG_VIDEO_OMAP3) += omap3_dss.o
obj-$(CONFIG_VIDEO_DSI_HOST_SANDBOX) += sandbox_dsi_host.o
obj-$(CONFIG_VIDEO_SANDBOX_SDL) += sandbox_sdl.o
obj-$(CONFIG_VIDEO_SIMPLE) += simplefb.o
obj-$(CONFIG_VIDEO_VESA) += vesa.o
obj-$(CONFIG_VIDEO_SEPS525) += seps525.o
obj-$(CONFIG_VIDEO_ZYNQMP_DPSUB) += zynqmp/

obj-y += bridge/
obj-y += sunxi/
obj-y += tegra20/
