#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <bcmdevs.h>
#include "shared.h"

#ifdef RTCONFIG_COMFW
#include "comfw.h"
#define MODELID(a)       #a
char *comfw_modid_s[] = COMFW_MODELID;
#undef MODELID

struct cf_data_desc cf_data_table[] = {
	{FW_386, "386_", "fw version"},
	{FW_384, "384_", "fw version"},
	{FW_38X, "38X_", "fw version"},
	{MAX_FWID, NULL, NULL}
};
#endif

#define MODELID(a)       #a
char *asus_models_str[] = ASUS_MODELS;
#undef MODELID

struct model_s {
	char *pid;
	int model;
};

static const struct model_s model_list[] = {
#if defined(RTCONFIG_REALTEK)
	{ "RP-AC68U",		MODEL_RPAC68U		},
	{ "RP-AC53",		MODEL_RPAC53		},
	{ "RP-AC55",		MODEL_RPAC55		},
	{ "RP-AC92",		MODEL_RPAC92		},
#elif defined(RTCONFIG_RALINK)
#ifdef RTCONFIG_DSL
	{ "DSL-N55U",		MODEL_DSLN55U		},
	{ "DSL-N55U-B",		MODEL_DSLN55U		},
#endif
	{ "EA-N66",		MODEL_EAN66		},
	{ "RT-N11P",		MODEL_RTN11P		},
	{ "RT-N300",		MODEL_RTN300		},
	{ "RT-N13U",		MODEL_RTN13U		},
	{ "RT-N14U",		MODEL_RTN14U		},
	{ "RT-AC52U",		MODEL_RTAC52U		},
	{ "RT-AC51U",		MODEL_RTAC51U		},
	{ "RT-N54U",		MODEL_RTN54U		},
	{ "RT-AC54U",		MODEL_RTAC54U		},
	{ "RT-N56UB1",		MODEL_RTN56UB1		},
	{ "RT-N56UB2",		MODEL_RTN56UB2		},
	{ "RT-N36U3",		MODEL_RTN36U3		},
	{ "RT-N56U",		MODEL_RTN56U		},
	{ "RT-N65U",		MODEL_RTN65U		},
	{ "RT-N67U",		MODEL_RTN67U		},
	{ "RT-AC1200HP",	MODEL_RTAC1200HP	},
	{ "RT-AC1200",		MODEL_RTAC1200		},
	{ "RT-AC1200_V2",	MODEL_RTAC1200V2	},
	{ "RT-ACRH18",		MODEL_RTACRH18		},
	{ "RT-N600",		MODEL_RTAC1200		},
	{ "RT-AC1200GA1",	MODEL_RTAC1200GA1	},
	{ "RT-AC1200GU",	MODEL_RTAC1200GU	},
	{ "RT-AC51U+",		MODEL_RTAC51UP		},
	{ "RT-AC53",		MODEL_RTAC53		},
	{ "RT-N11P_B1",		MODEL_RTN11P_B1		},
	{ "RT-N10P_V3",		MODEL_RTN11P_B1		},
	{ "RP-AC87",		MODEL_RPAC87		},
	{ "RT-AC85U",		MODEL_RTAC85U		},
	{ "RT-AC85P",		MODEL_RTAC85P		},
	{ "RT-ACRH26",		MODEL_RTACRH26		},
	{ "TUF-AC1750",		MODEL_TUFAC1750		},
	{ "RT-AC65U",		MODEL_RTAC85U		},
	{ "RT-N800HP",		MODEL_RTN800HP		},
	{ "4G-AC86U",		MODEL_RT4GAC86U		},
	{ "4G-AX56",		MODEL_RT4GAX56		},
	{ "RT-AX53U",		MODEL_RTAX53U		},
	{ "RT-AX54",		MODEL_RTAX54		},
	{ "XD4S",		MODEL_XD4S		},
	{ "PANTHERA",		MODEL_PANTHERA		},
	{ "PANTHERB",		MODEL_PANTHERB		},
	{ "CHEETAH",		MODEL_CHEETAH		},
	{ "TUF-AX4200",		MODEL_TUFAX4200		},
	{ "TUF-AX6000",		MODEL_TUFAX6000		},
	{ "RT-AX59U",		MODEL_RTAX59U		},
	{ "PRT-AX57_GO",	MODEL_PRTAX57GO		},
	{ "RT-AX52",		MODEL_RTAX52		},
#elif defined(RTCONFIG_QCA)
	{ "RT-AC55U",		MODEL_RTAC55U		},
	{ "RT-AC55UHP",		MODEL_RTAC55UHP		},
	{ "4G-AC55U",		MODEL_RT4GAC55U		},
	{ "RT-N19",		MODEL_RTN19		},
	{ "RT-AC59U",		MODEL_RTAC59U		},
	{ "RT-AC59U_V2",	MODEL_RTAC59U		},
	{ "PL-N12",		MODEL_PLN12		},
	{ "PL-AC56",		MODEL_PLAC56		},
	{ "PL-AC66U",		MODEL_PLAC66U		},
	{ "PL-AX56_XP4",	MODEL_PLAX56XP4		},
	{ "RP-AC66",		MODEL_RPAC66		},
	{ "RP-AC51",		MODEL_RPAC51		},
	{ "RT-AC58U",		MODEL_RTAC58U		},
	{ "4G-AC53U",		MODEL_RT4GAC53U		},
	{ "4G-AC56",		MODEL_RT4GAC56		},
	{ "RT-AC82U",		MODEL_RTAC82U		},
	{ "MAP-AC1300",		MODEL_MAPAC1300		},
	{ "MAP-AC2200",		MODEL_MAPAC2200		},
	{ "VZW-AC1300",		MODEL_VZWAC1300		},
	{ "SH-AC1300",		MODEL_SHAC1300		},
	{ "MAP-AC1750",		MODEL_MAPAC1750		},
	{ "RT-AC59_CD6R",	MODEL_RTAC59CD6R	},
	{ "RT-AC59_CD6N",	MODEL_RTAC59CD6N	},
	{ "RT-AC95U",		MODEL_RTAC95U		},
	{ "RT-AC88N",		MODEL_RTAC88N		},
	{ "BRT-AC828",		MODEL_BRTAC828		},
	{ "RT-AC88S",		MODEL_RTAC88S		},
	{ "RT-AD7200",		MODEL_RTAD7200		},
	{ "GT-AXY16000",	MODEL_GTAXY16000	},
	{ "GT-AX6000N",		MODEL_GTAX6000N		},
	{ "RT-AX89U",		MODEL_RTAX89U		},
	{ "ETJ",		MODEL_ETJ		},
	{ "RT-AX57Q",		MODEL_RTAX57Q		},
#elif defined(RTCONFIG_ALPINE)
	{ "GT-AC9600",		MODEL_GTAC9600		},
#elif defined(RTCONFIG_LANTIQ)
	{ "BLUECAVE",		MODEL_BLUECAVE		},
	{ "BLUE_CAVE",		MODEL_BLUECAVE		},
#else
	{ "RT-N66U",		MODEL_RTN66U		},
	{ "RT-AC56S",		MODEL_RTAC56S		},
	{ "RT-AC56U",		MODEL_RTAC56U		},
	{ "RT-AC66U",		MODEL_RTAC66U		},
#ifdef RT4GAC68U
	{ "4G-AC68U",		MODEL_RTAC68U		},
#endif
	{ "RT-AC68U",		MODEL_RTAC68U		},
	{ "RT-AC68A",		MODEL_RTAC68U		},
	{ "RT-AC87U",		MODEL_RTAC87U		},
	{ "RT-AC53U",		MODEL_RTAC53U		},
	{ "RT-AC3200",		MODEL_RTAC3200		},
	{ "RT-AC88U",		MODEL_RTAC88U		},
	{ "RT-AC86U",		MODEL_RTAC86U		},
	{ "GT-AC2900",		MODEL_RTAC86U		},
	{ "RT-AC3100",		MODEL_RTAC3100		},
	{ "RT-AC5300",		MODEL_RTAC5300		},
	{ "GT-AC5300",		MODEL_GTAC5300		},
	{ "GT-AX11000",		MODEL_GTAX11000		},
	{ "RT-AX88U",		MODEL_RTAX88U		},
	{ "RT-AX92U",		MODEL_RTAX92U		},
	{ "RT-AX95Q",		MODEL_RTAX95Q		},
	{ "XT8PRO",		MODEL_XT8PRO		},
	{ "XT8_V2",		MODEL_XT8_V2		},
	{ "RT-AXE95Q",		MODEL_RTAXE95Q		},
	{ "ET8PRO",		MODEL_ET8PRO		},
	{ "RT-AX56_XD4",	MODEL_RTAX56_XD4	},
	{ "XD4PRO",	MODEL_XD4PRO	},
        { "XC5",     		MODEL_XC5    		},
	{ "EBA63",		MODEL_EBA63	},
	{ "CT-AX56_XD4",	MODEL_CTAX56_XD4	},
	{ "RT-AX58U",		MODEL_RTAX58U		},
	{ "TUF-AX3000",		MODEL_RTAX58U		},
	{ "TUF-AX5400",         MODEL_RTAX58U		},
	{ "TUF-AX5400_V2",      MODEL_TUFAX5400_V2      },
	{ "RT-AX82U",           MODEL_RTAX58U		},
	{ "RT-AX82_XD6",	MODEL_RTAX58U		},
	{ "GS-AX3000",		MODEL_RTAX58U		},
	{ "GS-AX5400",		MODEL_RTAX58U		},
	{ "RT-AX82U_V2",	MODEL_RTAX82U_V2	},
	{ "RT-AX82_XD6S",	MODEL_RTAX82_XD6S	},
	{ "RT-AX3000N",		MODEL_RTAX3000N		},
	{ "BR63",		MODEL_BR63		},
	{ "RT-AX58U_V2",	MODEL_RTAX58U_V2	},
	{ "TUF-AX3000_V2",	MODEL_TUFAX3000_V2	},
	{ "RT-AXE7800",		MODEL_RTAXE7800		},
	{ "GT10",		MODEL_GT10		},
	{ "RT-AX9000",		MODEL_RTAX9000		},
	{ "RT-AX56U",		MODEL_RTAX56U		},
	{ "RP-AX56",            MODEL_RPAX56            },
	{ "RP-AX58",            MODEL_RPAX58            },
	{ "RT-AX55",		MODEL_RTAX55		},
	{ "RT-AX1800",		MODEL_RTAX55		},
	{ "RT-AX86U",		MODEL_RTAX86U		},
	{ "RT-AX86S",		MODEL_RTAX86U		},
	{ "RT-AX5700",		MODEL_RTAX86U		},
	{ "RT-AX68U",		MODEL_RTAX68U		},
	{ "RT-AC68U_V4",	MODEL_RTAC68U_V4	},
	{ "GT-AX6000",		MODEL_GTAX6000		},
	{ "GT-AXE11000",	MODEL_GTAXE11000	},
	{ "GT-AX11000_PRO",     MODEL_GTAX11000_PRO     },
	{ "GT-AXE16000",	MODEL_GTAXE16000	},
	{ "ET12",		MODEL_ET12		},
	{ "XT12",		MODEL_XT12		},
	{ "RT-AX86U_PRO",	MODEL_RTAX86U_PRO	},
	{ "RT-AX88U_PRO",       MODEL_RTAX88U_PRO       },
	{ "DSL-AX82U",		MODEL_DSLAX82U		},
	{ "RT-N53",		MODEL_RTN53		},
	{ "RT-N16",		MODEL_RTN16		},
	{ "RT-N18U",		MODEL_RTN18U		},
	{ "RT-N15U",		MODEL_RTN15U		},
	{ "RT-N12",		MODEL_RTN12		},
	{ "RT-N12B1",		MODEL_RTN12B1		},
	{ "RT-N12C1",		MODEL_RTN12C1		},
	{ "RT-N12D1",		MODEL_RTN12D1		},
	{ "RT-N12VP",		MODEL_RTN12VP		},
	{ "RT-N12HP",		MODEL_RTN12HP		},
	{ "RT-N12HP_B1",	MODEL_RTN12HP_B1	},
	{ "AP-N12",		MODEL_APN12		},
	{ "AP-N12HP",		MODEL_APN12HP		},
	{ "RT-N10U",		MODEL_RTN10U		},
	{ "RT-N14UHP",		MODEL_RTN14UHP		},
	{ "RT-N10+",		MODEL_RTN10P		},
	{ "RT-N10P",		MODEL_RTN10P		},
	{ "RT-N10D1",		MODEL_RTN10D1		},
	{ "RT-N10PV2",		MODEL_RTN10PV2		},
	{ "DSL-AC68U",		MODEL_DSLAC68U		},
	{ "RT-AC1200G",		MODEL_RTAC1200G		},
	{ "RT-AC1200G+",	MODEL_RTAC1200GP	},
	{ "BM68",		MODEL_BM68		},
	{ "ET8_V2",		MODEL_ET8_V2		},
	{ "XD6_V2",		MODEL_XD6_V2		},
	{ "RT-AX5400",		MODEL_RTAX5400		},
	{ "BC109",		MODEL_BC109		},
	{ "EBG19",		MODEL_EBG19		},
	{ "EBG15",		MODEL_EBG15		},
	{ "EBP15",		MODEL_EBP15		},
	{ "BC105",		MODEL_BC105		},
	{ "GT-BE98",		MODEL_GTBE98		},
	{ "RT-BE96U",		MODEL_RTBE96U		},
	{ "GT-BE98_PRO",	MODEL_GTBE98_PRO	},
	{ "BT12",		MODEL_BT12		},
	{ "BQ16",		MODEL_BQ16		},
	{ "GT-BE96",		MODEL_GTBE96		},
	{ "BQ16_PRO",		MODEL_BQ16_PRO		},
	{ "RT-BE88U",           MODEL_RTBE88U           },
	{ "RT-BE86U",		MODEL_RTBE86U		},
	{ "RT-BE58U",		MODEL_RTBE58U		},
	{ "TUF-BE3600",		MODEL_RTBE58U		},
	{ "GT-BE19000",		MODEL_GTBE19000		},
	{ "BT10",		MODEL_BT10		},
	{ "RT-BE92U",		MODEL_RTBE92U		},
	{ "RT-BE95U",		MODEL_RTBE95U		},
#endif	/* !RTCONFIG_RALINK */
	{ NULL, 0 },
};

#if defined(RTCONFIG_RALINK)
#elif defined(RTCONFIG_QCA)
#else
static int get_model_by_hw(void)
{
	char *hw_version = nvram_safe_get("hardware_version");

	if (strncmp(hw_version, "RTN12", 5) == 0) {
		if (strncmp(hw_version, "RTN12B1", 7) == 0)
			return MODEL_RTN12B1;
		if (strncmp(hw_version, "RTN12C1", 7) == 0)
			return MODEL_RTN12C1;
		if (strncmp(hw_version, "RTN12D1", 7) == 0)
			return MODEL_RTN12D1;
		if (strncmp(hw_version, "RTN12VP", 7) == 0)
			return MODEL_RTN12VP;
		if (strncmp(hw_version, "RTN12HP_B1", 10) == 0)
			return MODEL_RTN12HP_B1;
		if (strncmp(hw_version, "RTN12HP", 7) == 0)
			return MODEL_RTN12HP;
		if (strncmp(hw_version, "APN12HP", 7) == 0)
			return MODEL_APN12HP;
		return MODEL_RTN12;
	} else if (strncmp(hw_version, "APN12", 5) == 0) {
		if (strncmp(hw_version, "APN12HP", 7) == 0)
			return MODEL_APN12HP;
		return MODEL_APN12;
	}
	return MODEL_UNKNOWN;
}
#endif

#define BLV_MAX 4
#define BL_VERSION(a,b,c,d) (((a) << 12) + ((b) << 8) + ((c) << 4) + (d))

int get_blver(char *bls)
{
	int bv[BLV_MAX], blver=0, i=0;
	char *tok, *delim = ".";
	char buf[32], *bp=NULL;

	memset(buf, 0, sizeof(buf));
	if(bls)
		strlcpy(buf, bls, sizeof(buf));
	bp = buf;
	while((tok = strsep((char**)&bp, delim)))
		if(i < BLV_MAX)
			bv[i++] = atoi(tok);

	blver = BL_VERSION(bv[0], bv[1], bv[2], bv[3]);

	return blver;
}

int get_fwver(char *buildno, char *extendno)
{
	
	return atoi(buildno)*100000 + atoi(extendno);
}

/* returns MODEL ID
 * result is cached for safe multiple use */
int get_model(void)
{
	static int model = MODEL_UNKNOWN;
	char *pid;
	const struct model_s *p;

	if (model != MODEL_UNKNOWN)
		return model;

	pid = nvram_safe_get("productid");
	for (p = &model_list[0]; p->pid; ++p) {
		if (!strcmp(pid, p->pid)) {
			model = p->model;
			break;
		}
	}
	
#ifdef RTCONFIG_REALTEK
	if(model==MODEL_UNKNOWN || !pid[0]) {
	#ifdef RTCONFIG_RTL819X
		nvram_set("productid", "RT-RTL8881A");
		nvram_set("productid", "RP-AC53");
	#else
		nvram_set("productid", "RT-RTL8198C");
		nvram_set("productid", "RP-AC68U");//bruce
	#endif
	}
	cprintf("%s:%d current productid is %s\n",__FILE__,__LINE__,nvram_get("productid"));
#endif

#if defined(RTCONFIG_RALINK)
#elif defined(RTCONFIG_QCA)
#else
	if (model == MODEL_RTN12)
		model = get_model_by_hw();
	if (model == MODEL_APN12)
		model = get_model_by_hw();
#endif
	return model;
}

#if defined(RTCONFIG_RALINK)
#elif defined(RTCONFIG_QCA)
#else
/* returns product id */
char *get_modelid(int model)
{
	char *pid = "unknown";
	const struct model_s *p;

	for (p = &model_list[0]; p->pid; ++p) {
		if (model == p->model) {
			pid = p->pid;
			break;
		}
	}
	return pid;
}
#endif

/* returns SWITCH ID
 * result is cached for safe multiple use */
int get_switch(void)
{
	static int sw_model = SWITCH_UNKNOWN;

	if (sw_model != SWITCH_UNKNOWN)
		return sw_model;

	sw_model = get_switch_model();
	return sw_model;
}

#ifdef RTCONFIG_COMFW

int is_shared_modelid(int model, char *build_name)
{
	// assume each shared models at max share to 30 models
        switch(model) {
        case MODEL_RTAC68U:
		if(!strcmp(build_name, "4G-AC68U"))
			return model + CFID_BASE_2 + 1;
		if(!strcmp(build_name, "RT-AC68U"))
			return model + CFID_BASE_2 + 2;
		if(!strcmp(build_name, "RT-AC68A"))
			return model + CFID_BASE_2 + 3;
		break;
        case MODEL_RTAC86U:
		if(!strcmp(build_name, "RT-AC86U"))
			return model + CFID_BASE_2 + 1 + 30;
		if(!strcmp(build_name, "GT-AC2900"))
			return model + CFID_BASE_2 + 2 + 30;
		break;
/*
        case MODEL_RTAX58U:
		if(!strcmp(build_name, "RT-AX58U"))
			return model + CFID_BASE_2 + 1 + 30*2;
		if(!strcmp(build_name, "TUF-AX3000"))
			return model + CFID_BASE_2 + 2 + 30*2;
		if(!strcmp(build_name, "TUF-AX5400"))
			return model + CFID_BASE_2 + 3 + 30*2;
		if(!strcmp(build_name, "RT-AX82U"))
			return model + CFID_BASE_2 + 4 + 30*2;
		if(!strcmp(build_name, "RT-AX82_XD6"))
			return model + CFID_BASE_2 + 5 + 30*2;
		if(!strcmp(build_name, "GS-AX3000"))
			return model + CFID_BASE_2 + 6 + 30*2;
		if(!strcmp(build_name, "GS-AX5400"))
			return model + CFID_BASE_2 + 7 + 30*2;
		break;
*/
        case MODEL_RTAX55:
		if(!strcmp(build_name, "RT-AX55"))
			return model + CFID_BASE_2 + 1 + 30*3;
		if(!strcmp(build_name, "RT-AX1800"))
			return model + CFID_BASE_2 + 2 + 30*3;
		break;
        case MODEL_RTAX86U:
		if(!strcmp(build_name, "RT-AX86U"))
			return model + CFID_BASE_2 + 1 + 30*4;
		if(!strcmp(build_name, "RT-AX86S"))
			return model + CFID_BASE_2 + 2 + 30*4;
		break;
        case MODEL_RTN10P:
		if(!strcmp(build_name, "RT-N10+"))
			return model + CFID_BASE_2 + 1 + 30*5;
		if(!strcmp(build_name, "RT-N10P"))
			return model + CFID_BASE_2 + 2 + 30*5;
		break;
	}

	return 0;
}

int get_cf_id(int model, char *name) {
	int i, cfid_sp;
	char tmp[64], *ptr;

	snprintf(tmp, sizeof(tmp), "CF_%s", asus_models_str[model] + strlen("MODEL_"));

	// some models use same modelid, specify them in cfid table (comfw.h)
	if(model == MODEL_RTAX58U) {
#ifdef RTAX82_XD6
		return CF_RTAX82_XD6;
#elif defined(RTAX82U)
		return CF_RTAX82U;
#elif defined(TUFAX3000)
		return CF_TUFAX3000;
#elif defined(TUFAX5400)
		return CF_TUFAX5400;
#elif defined(GSAX3000)
		return CF_GSAX3000;
#elif defined(GSAX5400)
		return CF_GSAX5400;
#else
		return CF_RTAX58U;
#endif
	}

#ifdef RTCONFIG_HND_ROUTER_AX
	if(model == MODEL_RTBE96U) {
		if(hnd_boardid_cmp("RT-BE96U") == 0)
			return model + CFID_BASE_3;
		else
			return model + CFID_BASE_3 + 1;
	}
#endif
	for(i = 0; i < MAX_FTYPE; ++i) {	// for those independent modelid and defined in cfid table
		if((strncmp(tmp, comfw_modid_s[i], strlen(tmp)) == 0) && (strlen(tmp)==strlen(comfw_modid_s[i])))
			return i;
	}

	// for common models not in early cfid_table, use its modelid(could be shared) as cf_id
	if(i == MAX_FTYPE) {
		if(name && (cfid_sp = is_shared_modelid(model, name))) {
			return cfid_sp;
		}
		return model + CFID_BASE;
	}

	return 0;
}

void dump_cfid_from_modellist() 
{
        const struct model_s *p;
	int i;

	_dprintf("%s:", __func__);
        for (p = &model_list[0]; p->pid; ++p) {
		_dprintf("[%d].[%s]-[%d]:cfid=%d\n", i, p->pid, p->model, get_cf_id(p->model, p->pid));
		i++;
        }

	_dprintf("\n");
}

/*
char *get_cf_name(int cfid) {
	if(cfid > CFID_BASE && cfid < CFID_BASE_2)
		return asus_models_str[cfid - CFID_BASE];
	else
		return comfw_modid_s[cfid];	
}
*/

#endif
