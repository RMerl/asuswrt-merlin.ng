
# BCM DSL xPON platform support
snd-soc-bcm63xx-i2s-objs          := bcm63xx-i2s.o
snd-soc-bcm63xx-pcm-objs          := bcm63xx-pcm.o
snd-soc-bcm63xx-i2s-whistler-objs := bcm63xx-i2s-whistler.o
snd-soc-bcm63xx-pcm-whistler-objs := bcm63xx-pcm-whistler.o
snd-soc-bcm63xx-i2s-merritt-objs   := bcm63xx-i2s-merritt.o
snd-soc-bcm63xx-pcm-merritt-objs   := bcm63xx-pcm-merritt.o
snd-soc-bcm63xx-i2s-squamish-objs := bcm63xx-i2s-squamish.o
snd-soc-bcm63xx-pcm-squamish-objs := bcm63xx-pcm-squamish.o

snd-soc-bcm63148-clk-objs  := bcm63148clk.o

snd-soc-bcm63148-mach-objs := bcm63148refbmu.o
snd-soc-bcm63xx-mach-objs  := bcm63xxmachine.o
snd-soc-bcm-tdm-mach-objs  := bcmtdmmachine.o
snd-soc-bcm68560-mach-objs := bcm68560gref.o

#BCM63xxx I2S/TDM engine
obj-$(CONFIG_SND_BCM63xx_SOC_I2S_ENG)  += snd-soc-bcm63xx-i2s.o
obj-$(CONFIG_SND_BCM63xx_SOC_I2S_ENG)  += snd-soc-bcm63xx-pcm.o
obj-$(CONFIG_SND_BCM63xx_I2S_WHISTLER) += snd-soc-bcm63xx-i2s-whistler.o
obj-$(CONFIG_SND_BCM63xx_I2S_WHISTLER) += snd-soc-bcm63xx-pcm-whistler.o
obj-$(CONFIG_SND_BCM63xx_I2S_MERRITT) += snd-soc-bcm63xx-i2s-merritt.o
obj-$(CONFIG_SND_BCM63xx_I2S_MERRITT) += snd-soc-bcm63xx-pcm-merritt.o

#BCM63xxx PCM engine
obj-$(CONFIG_SND_BCM63xx_PCM_SQUAMISH) += snd-soc-bcm63xx-i2s-squamish.o
obj-$(CONFIG_SND_BCM63xx_PCM_SQUAMISH) += snd-soc-bcm63xx-pcm-squamish.o

# BCM63148/138 machine/clk support
obj-$(CONFIG_SND_BCM63148_SOC_I2S) += snd-soc-bcm63148-clk.o
obj-$(CONFIG_SND_BCM63148_SOC_I2S) += snd-soc-bcm63148-mach.o

# BCM6856 machine support
obj-$(CONFIG_SND_BCM6856_SOC_I2S)  += snd-soc-bcm68560-mach.o

# BCM63158/178/47622 machine support
obj-$(CONFIG_SND_BCM63XX_SOC_I2S_MACH) += snd-soc-bcm63xx-mach.o

#BCM63146,4915 I2S/TDM machine support
obj-$(CONFIG_SND_BCM63XX_SOC_TDM_MACH) += snd-soc-bcm-tdm-mach.o

clean:
	rm -f core *.ko *.o *.a *.s