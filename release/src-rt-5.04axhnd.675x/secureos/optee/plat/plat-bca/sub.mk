global-incdirs-y += .
srcs-y += main.c
srcs-$(CFG_ARM32_$(sm)) += aarch32_helper.S
subdirs-y += bcm_drivers
