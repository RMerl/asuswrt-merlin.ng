#ifndef __NVKM_BIOS_PRIV_H__
#define __NVKM_BIOS_PRIV_H__
#include <subdev/bios.h>

struct nvbios_source {
	const char *name;
	void *(*init)(struct nvkm_bios *, const char *);
	void  (*fini)(void *);
	u32   (*read)(void *, u32 offset, u32 length, struct nvkm_bios *);
	bool rw;
};

int nvbios_extend(struct nvkm_bios *, u32 length);
int nvbios_shadow(struct nvkm_bios *);

extern const struct nvbios_source nvbios_rom;
extern const struct nvbios_source nvbios_ramin;
extern const struct nvbios_source nvbios_acpi_fast;
extern const struct nvbios_source nvbios_acpi_slow;
extern const struct nvbios_source nvbios_pcirom;
extern const struct nvbios_source nvbios_platform;
extern const struct nvbios_source nvbios_of;
#endif
