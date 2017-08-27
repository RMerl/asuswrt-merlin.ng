/*
 * optimized mixing code for i386
 */

#define MIX_AREAS_16 mix_areas_16
#define MIX_AREAS_16_MMX mix_areas_16_mmx
#define MIX_AREAS_32 mix_areas_32
#define MIX_AREAS_24 mix_areas_24
#define MIX_AREAS_24_CMOV mix_areas_24_cmov
#define LOCK_PREFIX ""
#define XADD "addl"
#define XSUB "subl"
#include "pcm_dmix_i386.h"
#undef MIX_AREAS_16
#undef MIX_AREAS_16_MMX
#undef MIX_AREAS_32
#undef MIX_AREAS_24
#undef MIX_AREAS_24_CMOV
#undef LOCK_PREFIX
#undef XADD
#undef XSUB

#define MIX_AREAS_16 remix_areas_16
#define MIX_AREAS_16_MMX remix_areas_16_mmx
#define MIX_AREAS_32 remix_areas_32
#define MIX_AREAS_24 remix_areas_24
#define MIX_AREAS_24_CMOV remix_areas_24_cmov
#define LOCK_PREFIX ""
#define XADD "subl"
#define XSUB "addl"
#include "pcm_dmix_i386.h"
#undef MIX_AREAS_16
#undef MIX_AREAS_16_MMX
#undef MIX_AREAS_32
#undef MIX_AREAS_24
#undef MIX_AREAS_24_CMOV
#undef LOCK_PREFIX
#undef XADD
#undef XSUB

#define MIX_AREAS_16 mix_areas_16_smp
#define MIX_AREAS_16_MMX mix_areas_16_smp_mmx
#define MIX_AREAS_32 mix_areas_32_smp
#define MIX_AREAS_24 mix_areas_24_smp
#define MIX_AREAS_24_CMOV mix_areas_24_smp_cmov
#define LOCK_PREFIX "lock ; "
#define XADD "addl"
#define XSUB "subl"
#include "pcm_dmix_i386.h"
#undef MIX_AREAS_16
#undef MIX_AREAS_16_MMX
#undef MIX_AREAS_32
#undef MIX_AREAS_24
#undef MIX_AREAS_24_CMOV
#undef LOCK_PREFIX
#undef XADD
#undef XSUB
 
#define MIX_AREAS_16 remix_areas_16_smp
#define MIX_AREAS_16_MMX remix_areas_16_smp_mmx
#define MIX_AREAS_32 remix_areas_32_smp
#define MIX_AREAS_24 remix_areas_24_smp
#define MIX_AREAS_24_CMOV remix_areas_24_smp_cmov
#define LOCK_PREFIX "lock ; "
#define XADD "subl"
#define XSUB "addl"
#include "pcm_dmix_i386.h"
#undef MIX_AREAS_16
#undef MIX_AREAS_16_MMX
#undef MIX_AREAS_32
#undef MIX_AREAS_24
#undef MIX_AREAS_24_CMOV
#undef LOCK_PREFIX
#undef XADD
#undef XSUB
 
#define i386_dmix_supported_format \
	((1ULL << SND_PCM_FORMAT_S16_LE) |\
	 (1ULL << SND_PCM_FORMAT_S32_LE) |\
	 (1ULL << SND_PCM_FORMAT_S24_LE) |\
	 (1ULL << SND_PCM_FORMAT_S24_3LE))

#define dmix_supported_format \
	(i386_dmix_supported_format | generic_dmix_supported_format)

static void mix_select_callbacks(snd_pcm_direct_t *dmix)
{
	static int smp = 0, mmx = 0, cmov = 0;

	if (!((1ULL<< dmix->shmptr->s.format) & i386_dmix_supported_format)) {
		generic_mix_select_callbacks(dmix);
		return;
	}

	if (!smp) {
		FILE *in;
		char line[255];
	
		/* try to determine the capabilities of the CPU */
		in = fopen("/proc/cpuinfo", "r");
		if (in) {
			while (!feof(in)) {
				fgets(line, sizeof(line), in);
				if (!strncmp(line, "processor", 9))
					smp++;
				else if (!strncmp(line, "flags", 5)) {
					if (strstr(line, " mmx"))
						mmx = 1;
					if (strstr(line, " cmov"))
						cmov = 1;
				}
			}
			fclose(in);
		}
	}
	
	if (mmx) {
		dmix->u.dmix.mix_areas_16 = smp > 1 ? mix_areas_16_smp_mmx : mix_areas_16_mmx;
		dmix->u.dmix.remix_areas_16 = smp > 1 ? remix_areas_16_smp_mmx : remix_areas_16_mmx;
	} else {
		dmix->u.dmix.mix_areas_16 = smp > 1 ? mix_areas_16_smp : mix_areas_16;
		dmix->u.dmix.remix_areas_16 = smp > 1 ? remix_areas_16_smp : remix_areas_16;
	}
	dmix->u.dmix.mix_areas_32 = smp > 1 ? mix_areas_32_smp : mix_areas_32;
	dmix->u.dmix.remix_areas_32 = smp > 1 ? remix_areas_32_smp : remix_areas_32;
	if (cmov) {
		dmix->u.dmix.mix_areas_24 = smp > 1 ? mix_areas_24_smp_cmov : mix_areas_24_cmov;
		dmix->u.dmix.remix_areas_24 = smp > 1 ? remix_areas_24_smp_cmov : remix_areas_24_cmov;
	} else {
		dmix->u.dmix.mix_areas_24 = smp > 1 ? mix_areas_24_smp: mix_areas_24;
		dmix->u.dmix.remix_areas_24 = smp > 1 ? remix_areas_24_smp: remix_areas_24;
	}
}
