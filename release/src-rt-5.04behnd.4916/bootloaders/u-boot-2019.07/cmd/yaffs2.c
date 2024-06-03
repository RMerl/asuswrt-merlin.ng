/* Yaffs commands.
 * Modified by Charles Manning by adding ydevconfig command.
 *
 * Use ydevconfig to configure a mountpoint before use.
 * For example:
 *  # Configure mountpt xxx using nand device 0 using blocks 100-500
 *  ydevconfig xxx 0 100 500
 *  # Mount it
 *  ymount xxx
 *  # yls, yrdm etc
 *  yls -l xxx
 *  yrdm xxx/boot-image 82000000
 *  ...
 */

#include <common.h>

#include <config.h>
#include <command.h>

#ifdef YAFFS2_DEBUG
#define PRINTF(fmt, args...) printf(fmt, ##args)
#else
#define PRINTF(fmt, args...) do { } while (0)
#endif

extern void cmd_yaffs_dev_ls(void);
extern void cmd_yaffs_tracemask(unsigned set, unsigned mask);
extern void cmd_yaffs_devconfig(char *mp, int flash_dev,
				int start_block, int end_block);
extern void cmd_yaffs_mount(char *mp);
extern void cmd_yaffs_umount(char *mp);
extern void cmd_yaffs_read_file(char *fn);
extern void cmd_yaffs_write_file(char *fn, char bval, int sizeOfFile);
extern void cmd_yaffs_ls(const char *mountpt, int longlist);
extern void cmd_yaffs_mwrite_file(char *fn, char *addr, int size);
extern void cmd_yaffs_mread_file(char *fn, char *addr);
extern void cmd_yaffs_mkdir(const char *dir);
extern void cmd_yaffs_rmdir(const char *dir);
extern void cmd_yaffs_rm(const char *path);
extern void cmd_yaffs_mv(const char *oldPath, const char *newPath);

extern int yaffs_dump_dev(const char *path);

/* ytrace - show/set yaffs trace mask */
int do_ytrace(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc > 1)
		cmd_yaffs_tracemask(1, simple_strtol(argv[1], NULL, 16));
	else
		cmd_yaffs_tracemask(0, 0);

	return 0;
}

/* ydevls - lists yaffs mount points. */
int do_ydevls(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_yaffs_dev_ls();

	return 0;
}

/* ydevconfig mount_pt mtd_dev_num start_block end_block */
int do_ydevconfig(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *mtpoint;
	int mtd_dev;
	int start_block;
	int end_block;

	if (argc != 5) {
		printf
		    ("Bad arguments: ydevconfig mount_pt mtd_dev start_block end_block\n");
		return -1;
	}

	mtpoint = argv[1];
	mtd_dev = simple_strtol(argv[2], NULL, 16);
	start_block = simple_strtol(argv[3], NULL, 16);
	end_block = simple_strtol(argv[4], NULL, 16);

	cmd_yaffs_devconfig(mtpoint, mtd_dev, start_block, end_block);

	return 0;
}

int do_ymount(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *mtpoint;

	if (argc != 2) {
		printf("Bad arguments: ymount mount_pt\n");
		return -1;
	}

	mtpoint = argv[1];
	printf("Mounting yaffs2 mount point %s\n", mtpoint);

	cmd_yaffs_mount(mtpoint);

	return 0;
}

int do_yumount(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *mtpoint;

	if (argc != 2) {
		printf("Bad arguments: yumount mount_pt\n");
		return -1;
	}

	mtpoint = argv[1];
	printf("Unmounting yaffs2 mount point %s\n", mtpoint);
	cmd_yaffs_umount(mtpoint);

	return 0;
}

int do_yls(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *dirname;

	if (argc < 2 || argc > 3 || (argc == 3 && strcmp(argv[1], "-l"))) {
		printf("Bad arguments: yls [-l] dir\n");
		return -1;
	}

	dirname = argv[argc - 1];

	cmd_yaffs_ls(dirname, (argc > 2) ? 1 : 0);

	return 0;
}

int do_yrd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename;

	if (argc != 2) {
		printf("Bad arguments: yrd file_name\n");
		return -1;
	}

	filename = argv[1];

	printf("Reading file %s ", filename);

	cmd_yaffs_read_file(filename);

	printf("done\n");
	return 0;
}

int do_ywr(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename;
	ulong value;
	ulong numValues;

	if (argc != 4) {
		printf("Bad arguments: ywr file_name value n_values\n");
		return -1;
	}

	filename = argv[1];
	value = simple_strtoul(argv[2], NULL, 16);
	numValues = simple_strtoul(argv[3], NULL, 16);

	printf("Writing value (%lx) %lx times to %s... ", value, numValues,
	       filename);

	cmd_yaffs_write_file(filename, value, numValues);

	printf("done\n");
	return 0;
}

int do_yrdm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename;
	ulong addr;

	if (argc != 3) {
		printf("Bad arguments: yrdm file_name addr\n");
		return -1;
	}

	filename = argv[1];
	addr = simple_strtoul(argv[2], NULL, 16);

	cmd_yaffs_mread_file(filename, (char *)addr);

	return 0;
}

int do_ywrm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *filename;
	ulong addr;
	ulong size;

	if (argc != 4) {
		printf("Bad arguments: ywrm file_name addr size\n");
		return -1;
	}

	filename = argv[1];
	addr = simple_strtoul(argv[2], NULL, 16);
	size = simple_strtoul(argv[3], NULL, 16);

	cmd_yaffs_mwrite_file(filename, (char *)addr, size);

	return 0;
}

int do_ymkdir(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *dirname;

	if (argc != 2) {
		printf("Bad arguments: ymkdir dir_name\n");
		return -1;
	}

	dirname = argv[1];
	cmd_yaffs_mkdir(dirname);

	return 0;
}

int do_yrmdir(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *dirname;

	if (argc != 2) {
		printf("Bad arguments: yrmdir dir_name\n");
		return -1;
	}

	dirname = argv[1];
	cmd_yaffs_rmdir(dirname);

	return 0;
}

int do_yrm(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *name;

	if (argc != 2) {
		printf("Bad arguments: yrm name\n");
		return -1;
	}

	name = argv[1];

	cmd_yaffs_rm(name);

	return 0;
}

int do_ymv(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	char *oldPath;
	char *newPath;

	if (argc != 3) {
		printf("Bad arguments: ymv old_path new_path\n");
		return -1;
	}

	oldPath = argv[1];
	newPath = argv[2];

	cmd_yaffs_mv(newPath, oldPath);

	return 0;
}

U_BOOT_CMD(ytrace, 2, 0, do_ytrace,
	   "show/set yaffs trace mask",
	   "[new_mask]  show/set yaffs trace mask");

U_BOOT_CMD(ydevls, 1, 0, do_ydevls,
	   "list yaffs mount points", "list yaffs mount points");

U_BOOT_CMD(ydevconfig, 5, 0, do_ydevconfig,
	   "configure yaffs mount point",
	   "mtpoint mtd_id start_block end_block   configures a yaffs2 mount point");

U_BOOT_CMD(ymount, 2, 0, do_ymount,
	   "mount yaffs", "mtpoint  mounts a yaffs2 mount point");

U_BOOT_CMD(yumount, 2, 0, do_yumount,
	   "unmount yaffs", "mtpoint  unmounts a yaffs2 mount point");

U_BOOT_CMD(yls, 3, 0, do_yls, "yaffs ls", "[-l] dirname");

U_BOOT_CMD(yrd, 2, 0, do_yrd,
	   "read file from yaffs", "path   read file from yaffs");

U_BOOT_CMD(ywr, 4, 0, do_ywr,
	   "write file to yaffs",
	   "filename value num_vlues   write values to yaffs file");

U_BOOT_CMD(yrdm, 3, 0, do_yrdm,
	   "read file to memory from yaffs",
	   "filename offset    reads yaffs file into memory");

U_BOOT_CMD(ywrm, 4, 0, do_ywrm,
	   "write file from memory to yaffs",
	   "filename offset size  writes memory to yaffs file");

U_BOOT_CMD(ymkdir, 2, 0, do_ymkdir,
	   "YAFFS mkdir", "dir    create a yaffs directory");

U_BOOT_CMD(yrmdir, 2, 0, do_yrmdir,
	   "YAFFS rmdir", "dirname   removes a yaffs directory");

U_BOOT_CMD(yrm, 2, 0, do_yrm, "YAFFS rm", "path   removes a yaffs file");

U_BOOT_CMD(ymv, 4, 0, do_ymv,
	   "YAFFS mv",
	   "old_path new_path   moves/rename files within a yaffs mount point");
