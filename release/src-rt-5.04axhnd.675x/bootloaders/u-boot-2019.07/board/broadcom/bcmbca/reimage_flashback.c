
#include <common.h>
#include <cli.h>
#include <string.h>
#include "reimage.h"

/* CUSTOMIZE THIS-- if you know exactly which FS format is used in data partition - you 
   can optimize following by undefining unused format. For example, if your data partition is JFFS then 
   comment out "#define DATA_PARTITION_UBIFS" */
#define DATA_PARTITION_UBIFS
#define DATA_PARTITION_JFFS

int do_reimage_auto(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	/* CUSTOMIZE -- Sequence of commands to run automatically */
	run_command("safeimage nvram", 0);
	run_command("safeimage revertable", 0);
	run_command("preserve allocate 0x80000", 0);
#ifdef DATA_PARTITION_UBIFS
	if(run_command("ubi part misc1", 0) == 0)
	{
		run_command("ubifsmount ubi:nvram", 0);
		run_command("ubifsload 1000000 nvram.nvm && preserve save /mnt/defaults/wl/nvram.nvm", 0);
		run_command("ubifsumount", 0);
		run_command("ubi detach", 0);
	}
	if(run_command("ubi part misc3", 0) == 0)
	{
		run_command("ubifsmount ubi:mfg_data", 0);
		run_command("ubifsload 1000000 .wlsromcustomerfile.nvm && preserve save /data/.wlsromcustomerfile.nvm", 0);
		run_command("ubifsload 1000000 .wlsromcustomerfile.nvm && preserve save /mnt/defaults/wl/.wlsromcustomerfile.nvm", 0);
		run_command("ubifsumount", 0);
		run_command("ubi detach", 0);
	}
	if(run_command("ubi part data", 0) == 0)
	{
		run_command("ubifsmount ubi:data", 0);
		run_command("ubifsload 1000000 newkey.key && preserve save /data/newkey.key", 0);
		run_command("ubifsload 1000000 newkey && preserve save /data/newkey", 0);
		run_command("ubifsload 1000000 newkey.cert && preserve save /data/newkey.cert", 0);
		run_command("ubifsload 1000000 psi && preserve save /data/psi", 0);
		run_command("ubifsload 1000000 scratchpad && preserve save /data/scratchpad", 0);
		run_command("ubifsload 1000000 pmd_calibration.json && preserve save /data/pmd_calibration.json", 0);
		run_command("ubifsload 1000000 .kernel_nvram.setting && preserve save /data/.kernel_nvram.setting", 0);
		run_command("ubifsumount", 0);
		run_command("ubi detach", 0);
	}
	else
#endif
#ifdef DATA_PARTITION_JFFS
	{
		run_command("setenv partition data", 0);
		run_command("fsload newkey.key && preserve save /data/newkey.key", 0);
		run_command("fsload newkey && preserve save /data/newkey", 0);
		run_command("fsload newkey.cert && preserve save /data/newkey.cert", 0);
		run_command("fsload psi && preserve save /data/psi", 0);
		run_command("fsload scratchpad && preserve save /data/scratchpad", 0);
		run_command("fsload .kernel_nvram.setting && preserve save /data/.kernel_nvram.setting", 0);
	}
#endif
	run_command("safeimage read_recovery nvram", 0);	/* must be last before commit */
	run_command("safeimage commit", 0);
	run_command("safeimage store_preserved", 0);
	run_command("reset", 0);
	return (0);
}

void reimage_env_append(struct reimager *r)
{
	char more_env[1024];
	char tmp1[128];
	char tmp2[128];
	char *bp;
	char *cp;
	int more_env_size;
	cp = more_env;

	/* CUSTOMIZE THIS-- whatever needs to be transferred from NVRAM to the uboot environment, put it here */
	/* START */
	printf("cfe_bootline=%s\n", r->nvram.bootline);
	tmp2[0] = '\0';
	bp = strtok(r->nvram.bootline, "= ");	/* skip to e= */
	bp = strtok(NULL, "= ");	/* find the address after e= to space */
	strcpy(tmp1, bp);
	bp = strtok(NULL, "= ");	/* skip to h= */
	bp = strtok(NULL, "= ");	/* find the address after h= to space */
	strcpy(tmp2, bp);
	bp = strtok(NULL, "^");
	*strchr(tmp1, ':') = '\0';
	cp += sprintf(cp, "ipaddr=%s", tmp1);
	*(cp++) = '\0';
	cp += sprintf(cp, "serverip=%s", tmp2);
	*(cp++) = '\0';
	/* FIXME -- you probably want to hard-code this to your boardID when customizing for your deployment */
	cp += sprintf(cp, "boardid=%s",r->nvram.boardid);
	*(cp++) = '\0';
	cp += sprintf(cp, "ethaddr=%02x:%02x:%02x:%02x:%02x:%02x",
		      r->nvram.ucaBaseMacAddr[0],
		      r->nvram.ucaBaseMacAddr[1],
		      r->nvram.ucaBaseMacAddr[2],
		      r->nvram.ucaBaseMacAddr[3],
		      r->nvram.ucaBaseMacAddr[4], r->nvram.ucaBaseMacAddr[5]);
	*(cp++) = '\0';

	cp += sprintf(cp, "once=sdk metadata 1 1;setenv once true;saveenv;");
	*(cp++) = '\0';

	/* FIXME -- in most cases, you will want to hardcode this to your MCB when customizing for your deployment */
	if((r->nvram.ulMemoryConfig != 0xFFFFFFFF) && (r->nvram.ulMemoryConfig != 0xd0deed))
	{
		cp += sprintf(cp, "MCB=0x%x", r->nvram.ulMemoryConfig);
		*(cp++) = '\0';
	}
	else
	{
		/* FIXME -- on Broadcom reference boards in case no MCB was programmed in environment
		 * it would be selected on first boot by boardid, but this requires reboot.
		 * So modify 'once' variable accordingly (if you hardcoded MCB then you 
		 * don't need this code */
		cp--;
		cp += sprintf(cp, "reset");
		*(cp++) = '\0';
	}
	
	cp += sprintf(cp, "bootcmd=printenv;run once;run check_flashback;sdk boot_img");
	*(cp++) = '\0';

	cp += sprintf(cp, "tries=3");
	*(cp++) = '\0';

	cp += sprintf(cp, "check_flashback=test $tries -eq 0 || echo $tries ;  setexpr tries $tries - 1 ; saveenv ; test $tries -eq 0 && run do_flashback");
	*(cp++) = '\0';

	cp += sprintf(cp, "do_flashback=ubi part image && ubi read 0x1000000 recovery && flashback_ops go_flashback && reset");
	*(cp++) = '\0';

	*(cp++) = '\0';
	/* END */

	more_env_size = cp - more_env + 1;
	reimage_splice_env(r, more_env, more_env_size);
}
