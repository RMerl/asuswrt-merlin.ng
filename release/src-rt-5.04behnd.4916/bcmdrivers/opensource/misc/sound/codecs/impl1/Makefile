#
# Makefile for the bcm63xx codec driver
#

snd-soc-cs4345-objs := cs4345.o
snd-soc-pcm5100-objs := pcm5100.o
snd-soc-tlv320dac3203-objs := tlv320dac3203.o
snd-soc-tas2563-objs := tas2563.o
snd-soc-tlv320adc3101-objs := tlv320adc3101.o
snd-soc-tlv320adc5140-objs := tlv320adc5140.o
snd-soc-tlv320adc5120-objs := tlv320adc5120.o
obj-$(CONFIG_SND_SOC_CS4345)	+= snd-soc-cs4345.o
obj-$(CONFIG_SND_SOC_PCM5100)	+= snd-soc-pcm5100.o
obj-$(CONFIG_SND_SOC_MAPLELEAF)	+= snd-soc-tlv320dac3203.o
obj-$(CONFIG_SND_SOC_MAPLELEAF)	+= snd-soc-tlv320adc3101.o
obj-$(CONFIG_SND_SOC_MAPLETREE) += snd-soc-tlv320dac3203.o
obj-$(CONFIG_SND_SOC_MAPLETREE) += snd-soc-tlv320adc5140.o
obj-$(CONFIG_SND_SOC_MAPLEGROVE) += snd-soc-tas2563.o
obj-$(CONFIG_SND_SOC_MAPLEGROVE) += snd-soc-tlv320adc5120.o

clean:
	rm -f core *.ko *.o *.a *.s