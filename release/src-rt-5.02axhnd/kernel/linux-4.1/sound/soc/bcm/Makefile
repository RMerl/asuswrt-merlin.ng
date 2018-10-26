# BCM2835 Platform Support
snd-soc-bcm2835-i2s-objs := bcm2835-i2s.o

obj-$(CONFIG_SND_BCM2835_SOC_I2S) += snd-soc-bcm2835-i2s.o

ifdef BCM_KF # defined(CONFIG_BCM_KF_ASOC_AUDIO)

# BCM DSL xPON platform support
snd-soc-bcm63xx-i2s-objs := bcm63xx-i2s.o
snd-soc-bcm63xx-pcm-objs := bcm63xx-pcm.o

snd-soc-bcm63148-clk-objs  := bcm63148clk.o
snd-soc-bcm63158-clk-objs  := bcm63158clk.o
snd-soc-bcm68560-clk-objs  := bcm68560clk.o

snd-soc-bcm63148-mach-objs := bcm63148refbmu.o
snd-soc-bcm63158-mach-objs := bcm63158ref2.o
snd-soc-bcm68560-mach-objs := bcm68560gref.o

#BCM63xxx I2S engine
obj-$(CONFIG_SND_BCM63xx_SOC_I2S_ENG) += snd-soc-bcm63xx-i2s.o
obj-$(CONFIG_SND_BCM63xx_SOC_I2S_ENG) += snd-soc-bcm63xx-pcm.o

# BCM63148/138 machine/clk support
obj-$(CONFIG_SND_BCM63148_SOC_I2S) += snd-soc-bcm63148-clk.o
obj-$(CONFIG_SND_BCM63148_SOC_I2S) += snd-soc-bcm63148-mach.o

# BCM63158 machine/clk support
obj-$(CONFIG_SND_BCM63158_SOC_I2S) += snd-soc-bcm63158-clk.o
obj-$(CONFIG_SND_BCM63158_SOC_I2S) += snd-soc-bcm63158-mach.o

# BCM6856 machine/clk support
obj-$(CONFIG_SND_BCM6856_SOC_I2S)  += snd-soc-bcm68560-clk.o
obj-$(CONFIG_SND_BCM6856_SOC_I2S)  += snd-soc-bcm68560-mach.o

endif # BCM_KF # defined(CONFIG_BCM_KF_ASOC_AUDIO)
