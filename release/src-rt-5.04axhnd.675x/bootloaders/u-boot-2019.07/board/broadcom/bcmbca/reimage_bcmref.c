
#include <common.h>
#include <cli.h>
#include <string.h>
#include "reimage.h"

int do_reimage_auto(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	/* CUSTOMIZE -- Sequence of commands to run automatically */
	run_command("safeimage nvram", 0);
	run_command("safeimage revertable", 0);
	run_command("preserve allocate 0x80000", 0);
	run_command("setenv partition data", 0);
	run_command("fsload newkey.key && preserve save /data/newkey.key", 0);
	run_command("fsload newkey && preserve save /data/newkey", 0);
	run_command("fsload newkey.cert && preserve save /data/newkey.cert", 0);
	run_command("fsload psi && preserve save /data/psi", 0);
	run_command("fsload scratchpad && preserve save /data/scratchpad", 0);
	run_command
	    ("fsload .kernel_nvram.setting && preserve save /data/.kernel_nvram.setting",
	     0);
	run_command("nand read 0x1000000 nvram", 0);	/* must be last before commit */
	run_command("safeimage commit", 0);
	run_command("safeimage store_preserved", 0);
	run_command("ubi create cfe_nvram $nvram_size static 52", 0);
	run_command("ubi write 0x1000000  cfe_nvram $nvram_size", 0);
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
	int i;
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
	cp += sprintf(cp, "ipaddr=%s", tmp1);
	*(cp++) = '\0';
	cp += sprintf(cp, "serverip=%s", tmp2);
	*(cp++) = '\0';
	cp += sprintf(cp, "MCB=0x%x", r->nvram.ulMemoryConfig);
	*(cp++) = '\0';
	cp += sprintf(cp, "ethaddr=%x:%x:%x:%x:%x:%x",
		      r->nvram.ucaBaseMacAddr[0],
		      r->nvram.ucaBaseMacAddr[1],
		      r->nvram.ucaBaseMacAddr[2],
		      r->nvram.ucaBaseMacAddr[3],
		      r->nvram.ucaBaseMacAddr[4], r->nvram.ucaBaseMacAddr[5]);
	*(cp++) = '\0';

	cp += sprintf(cp, "bootcmd=printenv;run once;sdk boot_img");
	*(cp++) = '\0';

	cp += sprintf(cp, "once=sdk metadata 1 1;setenv once true;saveenv");
	*(cp++) = '\0';

	*(cp++) = '\0';
	/* END */

	more_env_size = cp - more_env + 1;
	reimage_splice_env(r, more_env, more_env_size);
}
