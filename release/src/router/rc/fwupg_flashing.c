/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <sys/mount.h>
#include <sys/reboot.h>

int
fwupg_flashing_main(int argc, char *argv[])
{
	char *const new_argv[] = { "/bin/bcm_flasher", "/tmp/linux.trx", "1", NULL };
	char *const new_argv_test[] = { "/bin/sh", NULL };
	int i,r;

	chdir("/");
	chroot("/");

	/*close(0);
	close(1);
	close(2);
	close(3);
	*/

	printf("%s, flashing task start..\n", __func__);

	for(i=3; i<20; ++i) {
		printf("closing %d\n", i);
		close(i);
	}

	printf("chk 1\n");	
	system("/usr/bin/fwupg_flashing2.sh");
	sleep(2);
	printf("pre flasher done\n");	

	r = umount("/old-root/tmp/mnt/defaults");
	if(r)	perror("/old-root/tmp/mnt/defaults(1)");
	r = umount("/old-root/tmp/mnt");
	if(r)	perror("/old-root/tmp/mnt(1)");
	r = umount("/old-root/jffs");
	if(r)	perror("/old-root/jffs");
	r = umount("/old-root/tmp/mnt/defaults");
	if(r)	perror("/old-root/tmp/mnt/defaults");
	r = umount("/old-root/tmp/mnt");
	if(r)	perror("/old-root/tmp/mnt");
	r = umount("/old-root");
	if(r)	perror("/old-root");

	sleep(1);

	if(nvram_match("sing_test", "1")) {
		printf("fwupg test sh\n");
		if ((r = execv(new_argv[0], new_argv))) {
			printf("%s, execv err:%d\n", __func__, r);
			return -1;
		}
		printf("\n\n!!! never here if exec ok\n\n");
	} else {
		if(nvram_match("sing_eval", "1")) {
			printf("eval bcm_flasher\n");
			eval("/bin/bcm_flasher", "/tmp/linux.trx", "1");
		} else {
			printf("system bcm_flasher\n");
			system("/bin/bcm_flasher /tmp/linux.trx 1");
		}
	}

	sleep(2);
	printf("eval_bcm_flasher done, going reboot...\n");
	reboot(RB_AUTOBOOT);

	return 0;
}
