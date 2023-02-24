/**
 * ntfslabel - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2002 Matthew J. Fanto
 * Copyright (c) 2002-2005 Anton Altaparmakov
 * Copyright (c) 2002-2003 Richard Russon
 * Copyright (c) 2012-2014 Jean-Pierre Andre
 *
 * This utility will display/change the label on an NTFS partition.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "debug.h"
#include "mft.h"
#include "utils.h"
/* #include "version.h" */
#include "logging.h"
#include "misc.h"

static const char *EXEC_NAME = "ntfslabel";

static struct options {
	char	*device;	/* Device/File to work with */
	char	*label;		/* Set the label to this */
	int	 quiet;		/* Less output */
	int	 verbose;	/* Extra output */
	int	 force;		/* Override common sense */
	int	 new_serial;	/* Change the serial number */
	unsigned long long serial;	/* Forced serial number value */
	int	 noaction;	/* Do not write to disk */
} opts;

/**
 * version - Print version information about the program
 *
 * Print a copyright statement and a brief description of the program.
 *
 * Return:  none
 */
static void version(void)
{
	ntfs_log_info("\n%s v%s (libntfs-3g) - Display, or set, the label for an "
			"NTFS Volume.\n\n", EXEC_NAME, VERSION);
	ntfs_log_info("Copyright (c)\n");
	ntfs_log_info("    2002      Matthew J. Fanto\n");
	ntfs_log_info("    2002-2005 Anton Altaparmakov\n");
	ntfs_log_info("    2002-2003 Richard Russon\n");
	ntfs_log_info("    2012-2014 Jean-Pierre Andre\n");
	ntfs_log_info("\n%s\n%s%s\n", ntfs_gpl, ntfs_bugs, ntfs_home);
}

/**
 * usage - Print a list of the parameters to the program
 *
 * Print a list of the parameters and options for the program.
 *
 * Return:  none
 */
static void usage(void)
{
	ntfs_log_info("\nUsage: %s [options] device [label]\n"
	       "    -n, --no-action    Do not write to disk\n"
	       "    -f, --force        Use less caution\n"
	       "        --new-serial   Set a new serial number\n"
	       "        --new-half-serial Set a partial new serial number\n"
	       "    -q, --quiet        Less output\n"
	       "    -v, --verbose      More output\n"
	       "    -V, --version      Display version information\n"
	       "    -h, --help         Display this help\n\n",
	       EXEC_NAME);
	ntfs_log_info("%s%s\n", ntfs_bugs, ntfs_home);
}

/**
 * parse_options - Read and validate the programs command line
 *
 * Read the command line, verify the syntax and parse the options.
 * This function is very long, but quite simple.
 *
 * Return:  1 Success
 *	    0 Error, one or more problems
 */
static int parse_options(int argc, char *argv[])
{
	static const char *sopt = "-fh?IinqvV";
	static const struct option lopt[] = {
		{ "force",	 no_argument,		NULL, 'f' },
		{ "help",	 no_argument,		NULL, 'h' },
		{ "new-serial",  optional_argument,	NULL, 'I' },
		{ "new-half-serial", optional_argument,	NULL, 'i' },
		{ "no-action",	 no_argument,		NULL, 'n' },
		{ "quiet",	 no_argument,		NULL, 'q' },
		{ "verbose",	 no_argument,		NULL, 'v' },
		{ "version",	 no_argument,		NULL, 'V' },
		{ NULL, 0, NULL, 0 },
	};

	int c = -1;
	int err  = 0;
	int ver  = 0;
	int help = 0;
	int levels = 0;
	char *endserial;

	opterr = 0; /* We'll handle the errors, thank you. */

	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
		switch (c) {
		case 1:	/* A non-option argument */
			if (!err && !opts.device)
				opts.device = argv[optind-1];
			else if (!err && !opts.label)
				opts.label = argv[optind-1];
			else
				err++;
			break;
		case 'f':
			opts.force++;
			break;
		case 'h':
			help++;
			break;
		case 'I' :	/* not proposed as a short option letter */
			if (optarg) {
				opts.serial = strtoull(optarg, &endserial, 16);
				if (*endserial)
					ntfs_log_error("Bad hexadecimal serial number.\n");
			}
			opts.new_serial |= 2;
			break;
		case 'i' :	/* not proposed as a short option letter */
			if (optarg) {
				opts.serial = strtoull(optarg, &endserial, 16)
							<< 32;
				if (*endserial)
					ntfs_log_error("Bad hexadecimal serial number.\n");
			}
			opts.new_serial |= 1;
			break;
		case 'n':
			opts.noaction++;
			break;
		case 'q':
			opts.quiet++;
			ntfs_log_clear_levels(NTFS_LOG_LEVEL_QUIET);
			break;
		case 'v':
			opts.verbose++;
			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
			break;
		case 'V':
			ver++;
			break;
		case '?':
			if (strncmp (argv[optind-1], "--log-", 6) == 0) {
				if (!ntfs_log_parse_option (argv[optind-1]))
					err++;
				break;
			}
			/* fall through */
		default:
			ntfs_log_error("Unknown option '%s'.\n", argv[optind-1]);
			err++;
			break;
		}
	}

	/* Make sure we're in sync with the log levels */
	levels = ntfs_log_get_levels();
	if (levels & NTFS_LOG_LEVEL_VERBOSE)
		opts.verbose++;
	if (!(levels & NTFS_LOG_LEVEL_QUIET))
		opts.quiet++;

	if (help || ver) {
		opts.quiet = 0;
	} else {
		if (opts.device == NULL) {
			if (argc > 1)
				ntfs_log_error("You must specify a device.\n");
			err++;
		}

		if (opts.quiet && opts.verbose) {
			ntfs_log_error("You may not use --quiet and --verbose at "
					"the same time.\n");
			err++;
		}
	}

	if (ver)
		version();
	if (help || err)
		usage();

		/* tri-state 0 : done, 1 : error, -1 : proceed */
	return (err ? 1 : (help || ver ? 0 : -1));
}

static int change_serial(ntfs_volume *vol, u64 sector, le64 serial_number,
			NTFS_BOOT_SECTOR *bs, NTFS_BOOT_SECTOR *oldbs)
{
	int res;
	le64 mask;
	BOOL same;

	res = -1;
        if ((ntfs_pread(vol->dev, sector << vol->sector_size_bits,
			vol->sector_size, bs) == vol->sector_size)) {
		same = TRUE;
		if (!sector)
				/* save the real bootsector */
			memcpy(oldbs, bs, vol->sector_size);
		else
				/* backup bootsector must be similar */
			same = !memcmp(oldbs, bs, vol->sector_size);
		if (same) {
			if (opts.new_serial & 2)
				bs->volume_serial_number = serial_number;
			else {
				mask = const_cpu_to_le64(~0x0ffffffffULL);
				bs->volume_serial_number
				    = (serial_number & mask)
					| (bs->volume_serial_number & ~mask);
			}
			if (opts.noaction
			    || (ntfs_pwrite(vol->dev,
				sector << vol->sector_size_bits,
				vol->sector_size, bs) == vol->sector_size)) {
				res = 0;
			}
		} else {
			ntfs_log_info("* Warning : the backup boot sector"
				" does not match (leaving unchanged)\n");
			res = 0;
		}
	}
	return (res);
}

static int set_new_serial(ntfs_volume *vol)
{
	NTFS_BOOT_SECTOR *bs; /* full boot sectors */
	NTFS_BOOT_SECTOR *oldbs; /* full original boot sector */
	le64 serial_number;
	u64 number_of_sectors;
	u64 sn;
	int res;

	res = -1;
	bs = (NTFS_BOOT_SECTOR*)ntfs_malloc(vol->sector_size);
	oldbs = (NTFS_BOOT_SECTOR*)ntfs_malloc(vol->sector_size);
	if (bs && oldbs) {
		if (opts.serial)
			serial_number = cpu_to_le64(opts.serial);
		else {
			/* different values for parallel processes */
			srandom(time((time_t*)NULL) ^ (getpid() << 16));
			sn = ((u64)random() << 32)
					| ((u64)random() & 0xffffffff);
			serial_number = cpu_to_le64(sn);
		}
		if (!change_serial(vol, 0, serial_number, bs, oldbs)) {
			number_of_sectors = ntfs_device_size_get(vol->dev,
						vol->sector_size);
			if (!change_serial(vol, number_of_sectors - 1,
						serial_number, bs, oldbs)) {
				ntfs_log_info("New serial number : %016llx\n",
					(long long)le64_to_cpu(
						bs->volume_serial_number));
				res = 0;
				}
		}
		free(bs);
		free(oldbs);
	}
	if (res)
		ntfs_log_info("Error setting a new serial number\n");
	return (res);
}

static int print_serial(ntfs_volume *vol)
{
	NTFS_BOOT_SECTOR *bs; /* full boot sectors */
	int res;

	res = -1;
	bs = (NTFS_BOOT_SECTOR*)ntfs_malloc(vol->sector_size);
	if (bs
	    && (ntfs_pread(vol->dev, 0,
			vol->sector_size, bs) == vol->sector_size)) {
		ntfs_log_info("Serial number : %016llx\n",
			(long long)le64_to_cpu(bs->volume_serial_number));
		res = 0;
		free(bs);
	}
	if (res)
		ntfs_log_info("Error getting the serial number\n");
	return (res);
}

/**
 * print_label - display the current label of a mounted ntfs partition.
 * @dev:	device to read the label from
 * @mnt_flags:	mount flags of the device or 0 if not mounted
 * @mnt_point:	mount point of the device or NULL
 *
 * Print the label of the device @dev.
 */
static int print_label(ntfs_volume *vol, unsigned long mnt_flags)
{
	int result = 0;
	//XXX significant?
	if ((mnt_flags & (NTFS_MF_MOUNTED | NTFS_MF_READONLY)) ==
			NTFS_MF_MOUNTED) {
		ntfs_log_error("%s is mounted read-write, results may be "
			"unreliable.\n", opts.device);
		result = 1;
	}

	if (opts.verbose)
		ntfs_log_info("Volume label :  %s\n", vol->vol_name);
	else
		ntfs_log_info("%s\n", vol->vol_name);
	return result;
}

/**
 * change_label - change the current label on a device
 * @dev:	device to change the label on
 * @mnt_flags:	mount flags of the device or 0 if not mounted
 * @mnt_point:	mount point of the device or NULL
 * @label:	the new label
 *
 * Change the label on the device @dev to @label.
 */
static int change_label(ntfs_volume *vol, char *label)
{
	ntfschar *new_label = NULL;
	int label_len;
	int result = 0;

	label_len = ntfs_mbstoucs(label, &new_label);
	if (label_len == -1) {
		ntfs_log_perror("Unable to convert label string to Unicode");
		return 1;
	}
	else if (label_len*sizeof(ntfschar) > 0x100) {
		ntfs_log_warning("New label is too long. Maximum %u characters "
				"allowed. Truncating %u excess characters.\n",
				(unsigned)(0x100 / sizeof(ntfschar)),
				(unsigned)(label_len -
				(0x100 / sizeof(ntfschar))));
		label_len = 0x100 / sizeof(ntfschar);
		new_label[label_len] = const_cpu_to_le16(0);
	}

	if(!opts.noaction)
		result = ntfs_volume_rename(vol, new_label, label_len) ? 1 : 0;

	free(new_label);
	return result;
}

/**
 * main - Begin here
 *
 * Start from here.
 *
 * Return:  0  Success, the program worked
 *	    1  Error, something went wrong
 */
int main(int argc, char **argv)
{
	unsigned long mnt_flags = 0;
	int result = 0;
	ntfs_volume *vol;

	ntfs_log_set_handler(ntfs_log_handler_outerr);

	result = parse_options(argc, argv);
	if (result >= 0)
		return (result);

	result = 0;
	utils_set_locale();

	if ((opts.label || opts.new_serial)
	    && !opts.noaction
	    && !opts.force
	    && !ntfs_check_if_mounted(opts.device, &mnt_flags)
	    && (mnt_flags & NTFS_MF_MOUNTED)) {
		ntfs_log_error("Cannot make changes to a mounted device\n");
		result = 1;
		goto abort;
	}

	if (!opts.label && !opts.new_serial)
		opts.noaction++;

	vol = utils_mount_volume(opts.device,
			(opts.noaction ? NTFS_MNT_RDONLY : 0) |
			(opts.force ? NTFS_MNT_RECOVER : 0));
	if (!vol)
		return 1;

	if (opts.new_serial) {
		result = set_new_serial(vol);
		if (result)
			goto unmount;
	} else {
		if (opts.verbose)
			result = print_serial(vol);
	}
	if (opts.label)
		result = change_label(vol, opts.label);
	else
		result = print_label(vol, mnt_flags);

unmount :
	ntfs_umount(vol, FALSE);
abort :
		/* "result" may be a negative reply of a library function */
	return (result ? 1 : 0);
}

