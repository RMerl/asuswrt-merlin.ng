obj-$(CONFIG_RNG_IPROC200) += rng200.o
obj-$(CONFIG_RNG_BCMBCA_TRNG) += $(if $(wildcard $(srctree)/arch/arm/mach-bcmbca/rng/trng.c),trng.o,trng_preb.o)
obj-y += rng.o

$(obj)/trng_preb.o:
	cp $(srctree)/arch/arm/mach-bcmbca/rng/trng.o $(obj)/trng_preb.o
