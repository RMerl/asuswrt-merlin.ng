/*
 * Copyright (C) 2010-2011 Neil Brown
 * Copyright (C) 2010-2014 Red Hat, Inc. All rights reserved.
 *
 * This file is released under the GPL.
 */

#include <linux/slab.h>
#include <linux/module.h>

#include "md.h"
#include "raid1.h"
#include "raid5.h"
#include "raid10.h"
#include "bitmap.h"

#include <linux/device-mapper.h>

#define DM_MSG_PREFIX "raid"

static bool devices_handle_discard_safely = false;

/*
 * The following flags are used by dm-raid.c to set up the array state.
 * They must be cleared before md_run is called.
 */
#define FirstUse 10             /* rdev flag */

struct raid_dev {
	/*
	 * Two DM devices, one to hold metadata and one to hold the
	 * actual data/parity.  The reason for this is to not confuse
	 * ti->len and give more flexibility in altering size and
	 * characteristics.
	 *
	 * While it is possible for this device to be associated
	 * with a different physical device than the data_dev, it
	 * is intended for it to be the same.
	 *    |--------- Physical Device ---------|
	 *    |- meta_dev -|------ data_dev ------|
	 */
	struct dm_dev *meta_dev;
	struct dm_dev *data_dev;
	struct md_rdev rdev;
};

/*
 * Flags for rs->print_flags field.
 */
#define DMPF_SYNC              0x1
#define DMPF_NOSYNC            0x2
#define DMPF_REBUILD           0x4
#define DMPF_DAEMON_SLEEP      0x8
#define DMPF_MIN_RECOVERY_RATE 0x10
#define DMPF_MAX_RECOVERY_RATE 0x20
#define DMPF_MAX_WRITE_BEHIND  0x40
#define DMPF_STRIPE_CACHE      0x80
#define DMPF_REGION_SIZE       0x100
#define DMPF_RAID10_COPIES     0x200
#define DMPF_RAID10_FORMAT     0x400

struct raid_set {
	struct dm_target *ti;

	uint32_t bitmap_loaded;
	uint32_t print_flags;

	struct mddev md;
	struct raid_type *raid_type;
	struct dm_target_callbacks callbacks;

	struct raid_dev dev[0];
};

/* Supported raid types and properties. */
static struct raid_type {
	const char *name;		/* RAID algorithm. */
	const char *descr;		/* Descriptor text for logging. */
	const unsigned parity_devs;	/* # of parity devices. */
	const unsigned minimal_devs;	/* minimal # of devices in set. */
	const unsigned level;		/* RAID level. */
	const unsigned algorithm;	/* RAID algorithm. */
} raid_types[] = {
	{"raid1",    "RAID1 (mirroring)",               0, 2, 1, 0 /* NONE */},
	{"raid10",   "RAID10 (striped mirrors)",        0, 2, 10, UINT_MAX /* Varies */},
	{"raid4",    "RAID4 (dedicated parity disk)",	1, 2, 5, ALGORITHM_PARITY_0},
	{"raid5_la", "RAID5 (left asymmetric)",		1, 2, 5, ALGORITHM_LEFT_ASYMMETRIC},
	{"raid5_ra", "RAID5 (right asymmetric)",	1, 2, 5, ALGORITHM_RIGHT_ASYMMETRIC},
	{"raid5_ls", "RAID5 (left symmetric)",		1, 2, 5, ALGORITHM_LEFT_SYMMETRIC},
	{"raid5_rs", "RAID5 (right symmetric)",		1, 2, 5, ALGORITHM_RIGHT_SYMMETRIC},
	{"raid6_zr", "RAID6 (zero restart)",		2, 4, 6, ALGORITHM_ROTATING_ZERO_RESTART},
	{"raid6_nr", "RAID6 (N restart)",		2, 4, 6, ALGORITHM_ROTATING_N_RESTART},
	{"raid6_nc", "RAID6 (N continue)",		2, 4, 6, ALGORITHM_ROTATING_N_CONTINUE}
};

static char *raid10_md_layout_to_format(int layout)
{
	/*
	 * Bit 16 and 17 stand for "offset" and "use_far_sets"
	 * Refer to MD's raid10.c for details
	 */
	if ((layout & 0x10000) && (layout & 0x20000))
		return "offset";

	if ((layout & 0xFF) > 1)
		return "near";

	return "far";
}

static unsigned raid10_md_layout_to_copies(int layout)
{
	if ((layout & 0xFF) > 1)
		return layout & 0xFF;
	return (layout >> 8) & 0xFF;
}

static int raid10_format_to_md_layout(char *format, unsigned copies)
{
	unsigned n = 1, f = 1;

	if (!strcmp("near", format))
		n = copies;
	else
		f = copies;

	if (!strcmp("offset", format))
		return 0x30000 | (f << 8) | n;

	if (!strcmp("far", format))
		return 0x20000 | (f << 8) | n;

	return (f << 8) | n;
}

static struct raid_type *get_raid_type(char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(raid_types); i++)
		if (!strcmp(raid_types[i].name, name))
			return &raid_types[i];

	return NULL;
}

static struct raid_set *context_alloc(struct dm_target *ti, struct raid_type *raid_type, unsigned raid_devs)
{
	unsigned i;
	struct raid_set *rs;

	if (raid_devs <= raid_type->parity_devs) {
		ti->error = "Insufficient number of devices";
		return ERR_PTR(-EINVAL);
	}

	rs = kzalloc(sizeof(*rs) + raid_devs * sizeof(rs->dev[0]), GFP_KERNEL);
	if (!rs) {
		ti->error = "Cannot allocate raid context";
		return ERR_PTR(-ENOMEM);
	}

	mddev_init(&rs->md);

	rs->ti = ti;
	rs->raid_type = raid_type;
	rs->md.raid_disks = raid_devs;
	rs->md.level = raid_type->level;
	rs->md.new_level = rs->md.level;
	rs->md.layout = raid_type->algorithm;
	rs->md.new_layout = rs->md.layout;
	rs->md.delta_disks = 0;
	rs->md.recovery_cp = 0;

	for (i = 0; i < raid_devs; i++)
		md_rdev_init(&rs->dev[i].rdev);

	/*
	 * Remaining items to be initialized by further RAID params:
	 *  rs->md.persistent
	 *  rs->md.external
	 *  rs->md.chunk_sectors
	 *  rs->md.new_chunk_sectors
	 *  rs->md.dev_sectors
	 */

	return rs;
}

static void context_free(struct raid_set *rs)
{
	int i;

	for (i = 0; i < rs->md.raid_disks; i++) {
		if (rs->dev[i].meta_dev)
			dm_put_device(rs->ti, rs->dev[i].meta_dev);
		md_rdev_clear(&rs->dev[i].rdev);
		if (rs->dev[i].data_dev)
			dm_put_device(rs->ti, rs->dev[i].data_dev);
	}

	kfree(rs);
}

/*
 * For every device we have two words
 *  <meta_dev>: meta device name or '-' if missing
 *  <data_dev>: data device name or '-' if missing
 *
 * The following are permitted:
 *    - -
 *    - <data_dev>
 *    <meta_dev> <data_dev>
 *
 * The following is not allowed:
 *    <meta_dev> -
 *
 * This code parses those words.  If there is a failure,
 * the caller must use context_free to unwind the operations.
 */
static int dev_parms(struct raid_set *rs, char **argv)
{
	int i;
	int rebuild = 0;
	int metadata_available = 0;
	int ret = 0;

	for (i = 0; i < rs->md.raid_disks; i++, argv += 2) {
		rs->dev[i].rdev.raid_disk = i;

		rs->dev[i].meta_dev = NULL;
		rs->dev[i].data_dev = NULL;

		/*
		 * There are no offsets, since there is a separate device
		 * for data and metadata.
		 */
		rs->dev[i].rdev.data_offset = 0;
		rs->dev[i].rdev.mddev = &rs->md;

		if (strcmp(argv[0], "-")) {
			ret = dm_get_device(rs->ti, argv[0],
					    dm_table_get_mode(rs->ti->table),
					    &rs->dev[i].meta_dev);
			rs->ti->error = "RAID metadata device lookup failure";
			if (ret)
				return ret;

			rs->dev[i].rdev.sb_page = alloc_page(GFP_KERNEL);
			if (!rs->dev[i].rdev.sb_page)
				return -ENOMEM;
		}

		if (!strcmp(argv[1], "-")) {
			if (!test_bit(In_sync, &rs->dev[i].rdev.flags) &&
			    (!rs->dev[i].rdev.recovery_offset)) {
				rs->ti->error = "Drive designated for rebuild not specified";
				return -EINVAL;
			}

			rs->ti->error = "No data device supplied with metadata device";
			if (rs->dev[i].meta_dev)
				return -EINVAL;

			continue;
		}

		ret = dm_get_device(rs->ti, argv[1],
				    dm_table_get_mode(rs->ti->table),
				    &rs->dev[i].data_dev);
		if (ret) {
			rs->ti->error = "RAID device lookup failure";
			return ret;
		}

		if (rs->dev[i].meta_dev) {
			metadata_available = 1;
			rs->dev[i].rdev.meta_bdev = rs->dev[i].meta_dev->bdev;
		}
		rs->dev[i].rdev.bdev = rs->dev[i].data_dev->bdev;
		list_add(&rs->dev[i].rdev.same_set, &rs->md.disks);
		if (!test_bit(In_sync, &rs->dev[i].rdev.flags))
			rebuild++;
	}

	if (metadata_available) {
		rs->md.external = 0;
		rs->md.persistent = 1;
		rs->md.major_version = 2;
	} else if (rebuild && !rs->md.recovery_cp) {
		/*
		 * Without metadata, we will not be able to tell if the array
		 * is in-sync or not - we must assume it is not.  Therefore,
		 * it is impossible to rebuild a drive.
		 *
		 * Even if there is metadata, the on-disk information may
		 * indicate that the array is not in-sync and it will then
		 * fail at that time.
		 *
		 * User could specify 'nosync' option if desperate.
		 */
		DMERR("Unable to rebuild drive while array is not in-sync");
		rs->ti->error = "RAID device lookup failure";
		return -EINVAL;
	}

	return 0;
}

/*
 * validate_region_size
 * @rs
 * @region_size:  region size in sectors.  If 0, pick a size (4MiB default).
 *
 * Set rs->md.bitmap_info.chunksize (which really refers to 'region size').
 * Ensure that (ti->len/region_size < 2^21) - required by MD bitmap.
 *
 * Returns: 0 on success, -EINVAL on failure.
 */
static int validate_region_size(struct raid_set *rs, unsigned long region_size)
{
	unsigned long min_region_size = rs->ti->len / (1 << 21);

	if (!region_size) {
		/*
		 * Choose a reasonable default.  All figures in sectors.
		 */
		if (min_region_size > (1 << 13)) {
			/* If not a power of 2, make it the next power of 2 */
			region_size = roundup_pow_of_two(min_region_size);
			DMINFO("Choosing default region size of %lu sectors",
			       region_size);
		} else {
			DMINFO("Choosing default region size of 4MiB");
			region_size = 1 << 13; /* sectors */
		}
	} else {
		/*
		 * Validate user-supplied value.
		 */
		if (region_size > rs->ti->len) {
			rs->ti->error = "Supplied region size is too large";
			return -EINVAL;
		}

		if (region_size < min_region_size) {
			DMERR("Supplied region_size (%lu sectors) below minimum (%lu)",
			      region_size, min_region_size);
			rs->ti->error = "Supplied region size is too small";
			return -EINVAL;
		}

		if (!is_power_of_2(region_size)) {
			rs->ti->error = "Region size is not a power of 2";
			return -EINVAL;
		}

		if (region_size < rs->md.chunk_sectors) {
			rs->ti->error = "Region size is smaller than the chunk size";
			return -EINVAL;
		}
	}

	/*
	 * Convert sectors to bytes.
	 */
	rs->md.bitmap_info.chunksize = (region_size << 9);

	return 0;
}

/*
 * validate_raid_redundancy
 * @rs
 *
 * Determine if there are enough devices in the array that haven't
 * failed (or are being rebuilt) to form a usable array.
 *
 * Returns: 0 on success, -EINVAL on failure.
 */
static int validate_raid_redundancy(struct raid_set *rs)
{
	unsigned i, rebuild_cnt = 0;
	unsigned rebuilds_per_group = 0, copies, d;
	unsigned group_size, last_group_start;

	for (i = 0; i < rs->md.raid_disks; i++)
		if (!test_bit(In_sync, &rs->dev[i].rdev.flags) ||
		    !rs->dev[i].rdev.sb_page)
			rebuild_cnt++;

	switch (rs->raid_type->level) {
	case 1:
		if (rebuild_cnt >= rs->md.raid_disks)
			goto too_many;
		break;
	case 4:
	case 5:
	case 6:
		if (rebuild_cnt > rs->raid_type->parity_devs)
			goto too_many;
		break;
	case 10:
		copies = raid10_md_layout_to_copies(rs->md.layout);
		if (rebuild_cnt < copies)
			break;

		/*
		 * It is possible to have a higher rebuild count for RAID10,
		 * as long as the failed devices occur in different mirror
		 * groups (i.e. different stripes).
		 *
		 * When checking "near" format, make sure no adjacent devices
		 * have failed beyond what can be handled.  In addition to the
		 * simple case where the number of devices is a multiple of the
		 * number of copies, we must also handle cases where the number
		 * of devices is not a multiple of the number of copies.
		 * E.g.    dev1 dev2 dev3 dev4 dev5
		 *          A    A    B    B    C
		 *          C    D    D    E    E
		 */
		if (!strcmp("near", raid10_md_layout_to_format(rs->md.layout))) {
			for (i = 0; i < rs->md.raid_disks * copies; i++) {
				if (!(i % copies))
					rebuilds_per_group = 0;
				d = i % rs->md.raid_disks;
				if ((!rs->dev[d].rdev.sb_page ||
				     !test_bit(In_sync, &rs->dev[d].rdev.flags)) &&
				    (++rebuilds_per_group >= copies))
					goto too_many;
			}
			break;
		}

		/*
		 * When checking "far" and "offset" formats, we need to ensure
		 * that the device that holds its copy is not also dead or
		 * being rebuilt.  (Note that "far" and "offset" formats only
		 * support two copies right now.  These formats also only ever
		 * use the 'use_far_sets' variant.)
		 *
		 * This check is somewhat complicated by the need to account
		 * for arrays that are not a multiple of (far) copies.  This
		 * results in the need to treat the last (potentially larger)
		 * set differently.
		 */
		group_size = (rs->md.raid_disks / copies);
		last_group_start = (rs->md.raid_disks / group_size) - 1;
		last_group_start *= group_size;
		for (i = 0; i < rs->md.raid_disks; i++) {
			if (!(i % copies) && !(i > last_group_start))
				rebuilds_per_group = 0;
			if ((!rs->dev[i].rdev.sb_page ||
			     !test_bit(In_sync, &rs->dev[i].rdev.flags)) &&
			    (++rebuilds_per_group >= copies))
					goto too_many;
		}
		break;
	default:
		if (rebuild_cnt)
			return -EINVAL;
	}

	return 0;

too_many:
	return -EINVAL;
}

/*
 * Possible arguments are...
 *	<chunk_size> [optional_args]
 *
 * Argument definitions
 *    <chunk_size>			The number of sectors per disk that
 *                                      will form the "stripe"
 *    [[no]sync]			Force or prevent recovery of the
 *                                      entire array
 *    [devices_handle_discard_safely]	Allow discards on RAID4/5/6; useful if RAID
 *					member device(s) properly support TRIM/UNMAP
 *    [rebuild <idx>]			Rebuild the drive indicated by the index
 *    [daemon_sleep <ms>]		Time between bitmap daemon work to
 *                                      clear bits
 *    [min_recovery_rate <kB/sec/disk>]	Throttle RAID initialization
 *    [max_recovery_rate <kB/sec/disk>]	Throttle RAID initialization
 *    [write_mostly <idx>]		Indicate a write mostly drive via index
 *    [max_write_behind <sectors>]	See '-write-behind=' (man mdadm)
 *    [stripe_cache <sectors>]		Stripe cache size for higher RAIDs
 *    [region_size <sectors>]           Defines granularity of bitmap
 *
 * RAID10-only options:
 *    [raid10_copies <# copies>]        Number of copies.  (Default: 2)
 *    [raid10_format <near|far|offset>] Layout algorithm.  (Default: near)
 */
static int parse_raid_params(struct raid_set *rs, char **argv,
			     unsigned num_raid_params)
{
	char *raid10_format = "near";
	unsigned raid10_copies = 2;
	unsigned i;
	unsigned long value, region_size = 0;
	sector_t sectors_per_dev = rs->ti->len;
	sector_t max_io_len;
	char *key;

	/*
	 * First, parse the in-order required arguments
	 * "chunk_size" is the only argument of this type.
	 */
	if ((kstrtoul(argv[0], 10, &value) < 0)) {
		rs->ti->error = "Bad chunk size";
		return -EINVAL;
	} else if (rs->raid_type->level == 1) {
		if (value)
			DMERR("Ignoring chunk size parameter for RAID 1");
		value = 0;
	} else if (!is_power_of_2(value)) {
		rs->ti->error = "Chunk size must be a power of 2";
		return -EINVAL;
	} else if (value < 8) {
		rs->ti->error = "Chunk size value is too small";
		return -EINVAL;
	}

	rs->md.new_chunk_sectors = rs->md.chunk_sectors = value;
	argv++;
	num_raid_params--;

	/*
	 * We set each individual device as In_sync with a completed
	 * 'recovery_offset'.  If there has been a device failure or
	 * replacement then one of the following cases applies:
	 *
	 *   1) User specifies 'rebuild'.
	 *      - Device is reset when param is read.
	 *   2) A new device is supplied.
	 *      - No matching superblock found, resets device.
	 *   3) Device failure was transient and returns on reload.
	 *      - Failure noticed, resets device for bitmap replay.
	 *   4) Device hadn't completed recovery after previous failure.
	 *      - Superblock is read and overrides recovery_offset.
	 *
	 * What is found in the superblocks of the devices is always
	 * authoritative, unless 'rebuild' or '[no]sync' was specified.
	 */
	for (i = 0; i < rs->md.raid_disks; i++) {
		set_bit(In_sync, &rs->dev[i].rdev.flags);
		rs->dev[i].rdev.recovery_offset = MaxSector;
	}

	/*
	 * Second, parse the unordered optional arguments
	 */
	for (i = 0; i < num_raid_params; i++) {
		if (!strcasecmp(argv[i], "nosync")) {
			rs->md.recovery_cp = MaxSector;
			rs->print_flags |= DMPF_NOSYNC;
			continue;
		}
		if (!strcasecmp(argv[i], "sync")) {
			rs->md.recovery_cp = 0;
			rs->print_flags |= DMPF_SYNC;
			continue;
		}

		/* The rest of the optional arguments come in key/value pairs */
		if ((i + 1) >= num_raid_params) {
			rs->ti->error = "Wrong number of raid parameters given";
			return -EINVAL;
		}

		key = argv[i++];

		/* Parameters that take a string value are checked here. */
		if (!strcasecmp(key, "raid10_format")) {
			if (rs->raid_type->level != 10) {
				rs->ti->error = "'raid10_format' is an invalid parameter for this RAID type";
				return -EINVAL;
			}
			if (strcmp("near", argv[i]) &&
			    strcmp("far", argv[i]) &&
			    strcmp("offset", argv[i])) {
				rs->ti->error = "Invalid 'raid10_format' value given";
				return -EINVAL;
			}
			raid10_format = argv[i];
			rs->print_flags |= DMPF_RAID10_FORMAT;
			continue;
		}

		if (kstrtoul(argv[i], 10, &value) < 0) {
			rs->ti->error = "Bad numerical argument given in raid params";
			return -EINVAL;
		}

		/* Parameters that take a numeric value are checked here */
		if (!strcasecmp(key, "rebuild")) {
			if (value >= rs->md.raid_disks) {
				rs->ti->error = "Invalid rebuild index given";
				return -EINVAL;
			}
			clear_bit(In_sync, &rs->dev[value].rdev.flags);
			rs->dev[value].rdev.recovery_offset = 0;
			rs->print_flags |= DMPF_REBUILD;
		} else if (!strcasecmp(key, "write_mostly")) {
			if (rs->raid_type->level != 1) {
				rs->ti->error = "write_mostly option is only valid for RAID1";
				return -EINVAL;
			}
			if (value >= rs->md.raid_disks) {
				rs->ti->error = "Invalid write_mostly drive index given";
				return -EINVAL;
			}
			set_bit(WriteMostly, &rs->dev[value].rdev.flags);
		} else if (!strcasecmp(key, "max_write_behind")) {
			if (rs->raid_type->level != 1) {
				rs->ti->error = "max_write_behind option is only valid for RAID1";
				return -EINVAL;
			}
			rs->print_flags |= DMPF_MAX_WRITE_BEHIND;

			/*
			 * In device-mapper, we specify things in sectors, but
			 * MD records this value in kB
			 */
			value /= 2;
			if (value > COUNTER_MAX) {
				rs->ti->error = "Max write-behind limit out of range";
				return -EINVAL;
			}
			rs->md.bitmap_info.max_write_behind = value;
		} else if (!strcasecmp(key, "daemon_sleep")) {
			rs->print_flags |= DMPF_DAEMON_SLEEP;
			if (!value || (value > MAX_SCHEDULE_TIMEOUT)) {
				rs->ti->error = "daemon sleep period out of range";
				return -EINVAL;
			}
			rs->md.bitmap_info.daemon_sleep = value;
		} else if (!strcasecmp(key, "stripe_cache")) {
			rs->print_flags |= DMPF_STRIPE_CACHE;

			/*
			 * In device-mapper, we specify things in sectors, but
			 * MD records this value in kB
			 */
			value /= 2;

			if ((rs->raid_type->level != 5) &&
			    (rs->raid_type->level != 6)) {
				rs->ti->error = "Inappropriate argument: stripe_cache";
				return -EINVAL;
			}
			if (raid5_set_cache_size(&rs->md, (int)value)) {
				rs->ti->error = "Bad stripe_cache size";
				return -EINVAL;
			}
		} else if (!strcasecmp(key, "min_recovery_rate")) {
			rs->print_flags |= DMPF_MIN_RECOVERY_RATE;
			if (value > INT_MAX) {
				rs->ti->error = "min_recovery_rate out of range";
				return -EINVAL;
			}
			rs->md.sync_speed_min = (int)value;
		} else if (!strcasecmp(key, "max_recovery_rate")) {
			rs->print_flags |= DMPF_MAX_RECOVERY_RATE;
			if (value > INT_MAX) {
				rs->ti->error = "max_recovery_rate out of range";
				return -EINVAL;
			}
			rs->md.sync_speed_max = (int)value;
		} else if (!strcasecmp(key, "region_size")) {
			rs->print_flags |= DMPF_REGION_SIZE;
			region_size = value;
		} else if (!strcasecmp(key, "raid10_copies") &&
			   (rs->raid_type->level == 10)) {
			if ((value < 2) || (value > 0xFF)) {
				rs->ti->error = "Bad value for 'raid10_copies'";
				return -EINVAL;
			}
			rs->print_flags |= DMPF_RAID10_COPIES;
			raid10_copies = value;
		} else {
			DMERR("Unable to parse RAID parameter: %s", key);
			rs->ti->error = "Unable to parse RAID parameters";
			return -EINVAL;
		}
	}

	if (validate_region_size(rs, region_size))
		return -EINVAL;

	if (rs->md.chunk_sectors)
		max_io_len = rs->md.chunk_sectors;
	else
		max_io_len = region_size;

	if (dm_set_target_max_io_len(rs->ti, max_io_len))
		return -EINVAL;

	if (rs->raid_type->level == 10) {
		if (raid10_copies > rs->md.raid_disks) {
			rs->ti->error = "Not enough devices to satisfy specification";
			return -EINVAL;
		}

		/*
		 * If the format is not "near", we only support
		 * two copies at the moment.
		 */
		if (strcmp("near", raid10_format) && (raid10_copies > 2)) {
			rs->ti->error = "Too many copies for given RAID10 format.";
			return -EINVAL;
		}

		/* (Len * #mirrors) / #devices */
		sectors_per_dev = rs->ti->len * raid10_copies;
		sector_div(sectors_per_dev, rs->md.raid_disks);

		rs->md.layout = raid10_format_to_md_layout(raid10_format,
							   raid10_copies);
		rs->md.new_layout = rs->md.layout;
	} else if ((rs->raid_type->level > 1) &&
		   sector_div(sectors_per_dev,
			      (rs->md.raid_disks - rs->raid_type->parity_devs))) {
		rs->ti->error = "Target length not divisible by number of data devices";
		return -EINVAL;
	}
	rs->md.dev_sectors = sectors_per_dev;

	/* Assume there are no metadata devices until the drives are parsed */
	rs->md.persistent = 0;
	rs->md.external = 1;

	return 0;
}

static void do_table_event(struct work_struct *ws)
{
	struct raid_set *rs = container_of(ws, struct raid_set, md.event_work);

	dm_table_event(rs->ti->table);
}

static int raid_is_congested(struct dm_target_callbacks *cb, int bits)
{
	struct raid_set *rs = container_of(cb, struct raid_set, callbacks);

	return mddev_congested(&rs->md, bits);
}

/*
 * This structure is never routinely used by userspace, unlike md superblocks.
 * Devices with this superblock should only ever be accessed via device-mapper.
 */
#define DM_RAID_MAGIC 0x64526D44
struct dm_raid_superblock {
	__le32 magic;		/* "DmRd" */
	__le32 features;	/* Used to indicate possible future changes */

	__le32 num_devices;	/* Number of devices in this array. (Max 64) */
	__le32 array_position;	/* The position of this drive in the array */

	__le64 events;		/* Incremented by md when superblock updated */
	__le64 failed_devices;	/* Bit field of devices to indicate failures */

	/*
	 * This offset tracks the progress of the repair or replacement of
	 * an individual drive.
	 */
	__le64 disk_recovery_offset;

	/*
	 * This offset tracks the progress of the initial array
	 * synchronisation/parity calculation.
	 */
	__le64 array_resync_offset;

	/*
	 * RAID characteristics
	 */
	__le32 level;
	__le32 layout;
	__le32 stripe_sectors;

	/* Remainder of a logical block is zero-filled when writing (see super_sync()). */
} __packed;

static int read_disk_sb(struct md_rdev *rdev, int size)
{
	BUG_ON(!rdev->sb_page);

	if (rdev->sb_loaded)
		return 0;

	if (!sync_page_io(rdev, 0, size, rdev->sb_page, READ, 1)) {
		DMERR("Failed to read superblock of device at position %d",
		      rdev->raid_disk);
		md_error(rdev->mddev, rdev);
		return -EINVAL;
	}

	rdev->sb_loaded = 1;

	return 0;
}

static void super_sync(struct mddev *mddev, struct md_rdev *rdev)
{
	int i;
	uint64_t failed_devices;
	struct dm_raid_superblock *sb;
	struct raid_set *rs = container_of(mddev, struct raid_set, md);

	sb = page_address(rdev->sb_page);
	failed_devices = le64_to_cpu(sb->failed_devices);

	for (i = 0; i < mddev->raid_disks; i++)
		if (!rs->dev[i].data_dev ||
		    test_bit(Faulty, &(rs->dev[i].rdev.flags)))
			failed_devices |= (1ULL << i);

	memset(sb + 1, 0, rdev->sb_size - sizeof(*sb));

	sb->magic = cpu_to_le32(DM_RAID_MAGIC);
	sb->features = cpu_to_le32(0);	/* No features yet */

	sb->num_devices = cpu_to_le32(mddev->raid_disks);
	sb->array_position = cpu_to_le32(rdev->raid_disk);

	sb->events = cpu_to_le64(mddev->events);
	sb->failed_devices = cpu_to_le64(failed_devices);

	sb->disk_recovery_offset = cpu_to_le64(rdev->recovery_offset);
	sb->array_resync_offset = cpu_to_le64(mddev->recovery_cp);

	sb->level = cpu_to_le32(mddev->level);
	sb->layout = cpu_to_le32(mddev->layout);
	sb->stripe_sectors = cpu_to_le32(mddev->chunk_sectors);
}

/*
 * super_load
 *
 * This function creates a superblock if one is not found on the device
 * and will decide which superblock to use if there's a choice.
 *
 * Return: 1 if use rdev, 0 if use refdev, -Exxx otherwise
 */
static int super_load(struct md_rdev *rdev, struct md_rdev *refdev)
{
	int ret;
	struct dm_raid_superblock *sb;
	struct dm_raid_superblock *refsb;
	uint64_t events_sb, events_refsb;

	rdev->sb_start = 0;
	rdev->sb_size = bdev_logical_block_size(rdev->meta_bdev);
	if (rdev->sb_size < sizeof(*sb) || rdev->sb_size > PAGE_SIZE) {
		DMERR("superblock size of a logical block is no longer valid");
		return -EINVAL;
	}

	ret = read_disk_sb(rdev, rdev->sb_size);
	if (ret)
		return ret;

	sb = page_address(rdev->sb_page);

	/*
	 * Two cases that we want to write new superblocks and rebuild:
	 * 1) New device (no matching magic number)
	 * 2) Device specified for rebuild (!In_sync w/ offset == 0)
	 */
	if ((sb->magic != cpu_to_le32(DM_RAID_MAGIC)) ||
	    (!test_bit(In_sync, &rdev->flags) && !rdev->recovery_offset)) {
		super_sync(rdev->mddev, rdev);

		set_bit(FirstUse, &rdev->flags);

		/* Force writing of superblocks to disk */
		set_bit(MD_CHANGE_DEVS, &rdev->mddev->flags);

		/* Any superblock is better than none, choose that if given */
		return refdev ? 0 : 1;
	}

	if (!refdev)
		return 1;

	events_sb = le64_to_cpu(sb->events);

	refsb = page_address(refdev->sb_page);
	events_refsb = le64_to_cpu(refsb->events);

	return (events_sb > events_refsb) ? 1 : 0;
}

static int super_init_validation(struct mddev *mddev, struct md_rdev *rdev)
{
	int role;
	struct raid_set *rs = container_of(mddev, struct raid_set, md);
	uint64_t events_sb;
	uint64_t failed_devices;
	struct dm_raid_superblock *sb;
	uint32_t new_devs = 0;
	uint32_t rebuilds = 0;
	struct md_rdev *r;
	struct dm_raid_superblock *sb2;

	sb = page_address(rdev->sb_page);
	events_sb = le64_to_cpu(sb->events);
	failed_devices = le64_to_cpu(sb->failed_devices);

	/*
	 * Initialise to 1 if this is a new superblock.
	 */
	mddev->events = events_sb ? : 1;

	/*
	 * Reshaping is not currently allowed
	 */
	if (le32_to_cpu(sb->level) != mddev->level) {
		DMERR("Reshaping arrays not yet supported. (RAID level change)");
		return -EINVAL;
	}
	if (le32_to_cpu(sb->layout) != mddev->layout) {
		DMERR("Reshaping arrays not yet supported. (RAID layout change)");
		DMERR("  0x%X vs 0x%X", le32_to_cpu(sb->layout), mddev->layout);
		DMERR("  Old layout: %s w/ %d copies",
		      raid10_md_layout_to_format(le32_to_cpu(sb->layout)),
		      raid10_md_layout_to_copies(le32_to_cpu(sb->layout)));
		DMERR("  New layout: %s w/ %d copies",
		      raid10_md_layout_to_format(mddev->layout),
		      raid10_md_layout_to_copies(mddev->layout));
		return -EINVAL;
	}
	if (le32_to_cpu(sb->stripe_sectors) != mddev->chunk_sectors) {
		DMERR("Reshaping arrays not yet supported. (stripe sectors change)");
		return -EINVAL;
	}

	/* We can only change the number of devices in RAID1 right now */
	if ((rs->raid_type->level != 1) &&
	    (le32_to_cpu(sb->num_devices) != mddev->raid_disks)) {
		DMERR("Reshaping arrays not yet supported. (device count change)");
		return -EINVAL;
	}

	if (!(rs->print_flags & (DMPF_SYNC | DMPF_NOSYNC)))
		mddev->recovery_cp = le64_to_cpu(sb->array_resync_offset);

	/*
	 * During load, we set FirstUse if a new superblock was written.
	 * There are two reasons we might not have a superblock:
	 * 1) The array is brand new - in which case, all of the
	 *    devices must have their In_sync bit set.  Also,
	 *    recovery_cp must be 0, unless forced.
	 * 2) This is a new device being added to an old array
	 *    and the new device needs to be rebuilt - in which
	 *    case the In_sync bit will /not/ be set and
	 *    recovery_cp must be MaxSector.
	 */
	rdev_for_each(r, mddev) {
		if (!test_bit(In_sync, &r->flags)) {
			DMINFO("Device %d specified for rebuild: "
			       "Clearing superblock", r->raid_disk);
			rebuilds++;
		} else if (test_bit(FirstUse, &r->flags))
			new_devs++;
	}

	if (!rebuilds) {
		if (new_devs == mddev->raid_disks) {
			DMINFO("Superblocks created for new array");
			set_bit(MD_ARRAY_FIRST_USE, &mddev->flags);
		} else if (new_devs) {
			DMERR("New device injected "
			      "into existing array without 'rebuild' "
			      "parameter specified");
			return -EINVAL;
		}
	} else if (new_devs) {
		DMERR("'rebuild' devices cannot be "
		      "injected into an array with other first-time devices");
		return -EINVAL;
	} else if (mddev->recovery_cp != MaxSector) {
		DMERR("'rebuild' specified while array is not in-sync");
		return -EINVAL;
	}

	/*
	 * Now we set the Faulty bit for those devices that are
	 * recorded in the superblock as failed.
	 */
	rdev_for_each(r, mddev) {
		if (!r->sb_page)
			continue;
		sb2 = page_address(r->sb_page);
		sb2->failed_devices = 0;

		/*
		 * Check for any device re-ordering.
		 */
		if (!test_bit(FirstUse, &r->flags) && (r->raid_disk >= 0)) {
			role = le32_to_cpu(sb2->array_position);
			if (role != r->raid_disk) {
				if (rs->raid_type->level != 1) {
					rs->ti->error = "Cannot change device "
						"positions in RAID array";
					return -EINVAL;
				}
				DMINFO("RAID1 device #%d now at position #%d",
				       role, r->raid_disk);
			}

			/*
			 * Partial recovery is performed on
			 * returning failed devices.
			 */
			if (failed_devices & (1 << role))
				set_bit(Faulty, &r->flags);
		}
	}

	return 0;
}

static int super_validate(struct mddev *mddev, struct md_rdev *rdev)
{
	struct dm_raid_superblock *sb = page_address(rdev->sb_page);

	/*
	 * If mddev->events is not set, we know we have not yet initialized
	 * the array.
	 */
	if (!mddev->events && super_init_validation(mddev, rdev))
		return -EINVAL;

	mddev->bitmap_info.offset = 4096 >> 9; /* Enable bitmap creation */
	rdev->mddev->bitmap_info.default_offset = 4096 >> 9;
	if (!test_bit(FirstUse, &rdev->flags)) {
		rdev->recovery_offset = le64_to_cpu(sb->disk_recovery_offset);
		if (rdev->recovery_offset != MaxSector)
			clear_bit(In_sync, &rdev->flags);
	}

	/*
	 * If a device comes back, set it as not In_sync and no longer faulty.
	 */
	if (test_bit(Faulty, &rdev->flags)) {
		clear_bit(Faulty, &rdev->flags);
		clear_bit(In_sync, &rdev->flags);
		rdev->saved_raid_disk = rdev->raid_disk;
		rdev->recovery_offset = 0;
	}

	clear_bit(FirstUse, &rdev->flags);

	return 0;
}

/*
 * Analyse superblocks and select the freshest.
 */
static int analyse_superblocks(struct dm_target *ti, struct raid_set *rs)
{
	int ret;
	struct raid_dev *dev;
	struct md_rdev *rdev, *tmp, *freshest;
	struct mddev *mddev = &rs->md;

	freshest = NULL;
	rdev_for_each_safe(rdev, tmp, mddev) {
		/*
		 * Skipping super_load due to DMPF_SYNC will cause
		 * the array to undergo initialization again as
		 * though it were new.  This is the intended effect
		 * of the "sync" directive.
		 *
		 * When reshaping capability is added, we must ensure
		 * that the "sync" directive is disallowed during the
		 * reshape.
		 */
		if (rs->print_flags & DMPF_SYNC)
			continue;

		if (!rdev->meta_bdev)
			continue;

		ret = super_load(rdev, freshest);

		switch (ret) {
		case 1:
			freshest = rdev;
			break;
		case 0:
			break;
		default:
			dev = container_of(rdev, struct raid_dev, rdev);
			if (dev->meta_dev)
				dm_put_device(ti, dev->meta_dev);

			dev->meta_dev = NULL;
			rdev->meta_bdev = NULL;

			if (rdev->sb_page)
				put_page(rdev->sb_page);

			rdev->sb_page = NULL;

			rdev->sb_loaded = 0;

			/*
			 * We might be able to salvage the data device
			 * even though the meta device has failed.  For
			 * now, we behave as though '- -' had been
			 * set for this device in the table.
			 */
			if (dev->data_dev)
				dm_put_device(ti, dev->data_dev);

			dev->data_dev = NULL;
			rdev->bdev = NULL;

			list_del(&rdev->same_set);
		}
	}

	if (!freshest)
		return 0;

	if (validate_raid_redundancy(rs)) {
		rs->ti->error = "Insufficient redundancy to activate array";
		return -EINVAL;
	}

	/*
	 * Validation of the freshest device provides the source of
	 * validation for the remaining devices.
	 */
	ti->error = "Unable to assemble array: Invalid superblocks";
	if (super_validate(mddev, freshest))
		return -EINVAL;

	rdev_for_each(rdev, mddev)
		if ((rdev != freshest) && super_validate(mddev, rdev))
			return -EINVAL;

	return 0;
}

/*
 * Enable/disable discard support on RAID set depending on
 * RAID level and discard properties of underlying RAID members.
 */
static void configure_discard_support(struct dm_target *ti, struct raid_set *rs)
{
	int i;
	bool raid456;

	/* Assume discards not supported until after checks below. */
	ti->discards_supported = false;

	/* RAID level 4,5,6 require discard_zeroes_data for data integrity! */
	raid456 = (rs->md.level == 4 || rs->md.level == 5 || rs->md.level == 6);

	for (i = 0; i < rs->md.raid_disks; i++) {
		struct request_queue *q;

		if (!rs->dev[i].rdev.bdev)
			continue;

		q = bdev_get_queue(rs->dev[i].rdev.bdev);
		if (!q || !blk_queue_discard(q))
			return;

		if (raid456) {
			if (!q->limits.discard_zeroes_data)
				return;
			if (!devices_handle_discard_safely) {
				DMERR("raid456 discard support disabled due to discard_zeroes_data uncertainty.");
				DMERR("Set dm-raid.devices_handle_discard_safely=Y to override.");
				return;
			}
		}
	}

	/* All RAID members properly support discards */
	ti->discards_supported = true;

	/*
	 * RAID1 and RAID10 personalities require bio splitting,
	 * RAID0/4/5/6 don't and process large discard bios properly.
	 */
	ti->split_discard_bios = !!(rs->md.level == 1 || rs->md.level == 10);
	ti->num_discard_bios = 1;
}

/*
 * Construct a RAID4/5/6 mapping:
 * Args:
 *	<raid_type> <#raid_params> <raid_params>		\
 *	<#raid_devs> { <meta_dev1> <dev1> .. <meta_devN> <devN> }
 *
 * <raid_params> varies by <raid_type>.  See 'parse_raid_params' for
 * details on possible <raid_params>.
 */
static int raid_ctr(struct dm_target *ti, unsigned argc, char **argv)
{
	int ret;
	struct raid_type *rt;
	unsigned long num_raid_params, num_raid_devs;
	struct raid_set *rs = NULL;

	/* Must have at least <raid_type> <#raid_params> */
	if (argc < 2) {
		ti->error = "Too few arguments";
		return -EINVAL;
	}

	/* raid type */
	rt = get_raid_type(argv[0]);
	if (!rt) {
		ti->error = "Unrecognised raid_type";
		return -EINVAL;
	}
	argc--;
	argv++;

	/* number of RAID parameters */
	if (kstrtoul(argv[0], 10, &num_raid_params) < 0) {
		ti->error = "Cannot understand number of RAID parameters";
		return -EINVAL;
	}
	argc--;
	argv++;

	/* Skip over RAID params for now and find out # of devices */
	if (num_raid_params >= argc) {
		ti->error = "Arguments do not agree with counts given";
		return -EINVAL;
	}

	if ((kstrtoul(argv[num_raid_params], 10, &num_raid_devs) < 0) ||
	    (num_raid_devs >= INT_MAX)) {
		ti->error = "Cannot understand number of raid devices";
		return -EINVAL;
	}

	argc -= num_raid_params + 1; /* +1: we already have num_raid_devs */
	if (argc != (num_raid_devs * 2)) {
		ti->error = "Supplied RAID devices does not match the count given";
		return -EINVAL;
	}

	rs = context_alloc(ti, rt, (unsigned)num_raid_devs);
	if (IS_ERR(rs))
		return PTR_ERR(rs);

	ret = parse_raid_params(rs, argv, (unsigned)num_raid_params);
	if (ret)
		goto bad;

	argv += num_raid_params + 1;

	ret = dev_parms(rs, argv);
	if (ret)
		goto bad;

	rs->md.sync_super = super_sync;
	ret = analyse_superblocks(ti, rs);
	if (ret)
		goto bad;

	INIT_WORK(&rs->md.event_work, do_table_event);
	ti->private = rs;
	ti->num_flush_bios = 1;

	/*
	 * Disable/enable discard support on RAID set.
	 */
	configure_discard_support(ti, rs);

	mutex_lock(&rs->md.reconfig_mutex);
	ret = md_run(&rs->md);
	rs->md.in_sync = 0; /* Assume already marked dirty */
	mutex_unlock(&rs->md.reconfig_mutex);

	if (ret) {
		ti->error = "Fail to run raid array";
		goto bad;
	}

	if (ti->len != rs->md.array_sectors) {
		ti->error = "Array size does not match requested target length";
		ret = -EINVAL;
		goto size_mismatch;
	}
	rs->callbacks.congested_fn = raid_is_congested;
	dm_table_add_target_callbacks(ti->table, &rs->callbacks);

	mddev_suspend(&rs->md);
	return 0;

size_mismatch:
	md_stop(&rs->md);
bad:
	context_free(rs);

	return ret;
}

static void raid_dtr(struct dm_target *ti)
{
	struct raid_set *rs = ti->private;

	list_del_init(&rs->callbacks.list);
	md_stop(&rs->md);
	context_free(rs);
}

static int raid_map(struct dm_target *ti, struct bio *bio)
{
	struct raid_set *rs = ti->private;
	struct mddev *mddev = &rs->md;

	mddev->pers->make_request(mddev, bio);

	return DM_MAPIO_SUBMITTED;
}

static const char *decipher_sync_action(struct mddev *mddev)
{
	if (test_bit(MD_RECOVERY_FROZEN, &mddev->recovery))
		return "frozen";

	if (test_bit(MD_RECOVERY_RUNNING, &mddev->recovery) ||
	    (!mddev->ro && test_bit(MD_RECOVERY_NEEDED, &mddev->recovery))) {
		if (test_bit(MD_RECOVERY_RESHAPE, &mddev->recovery))
			return "reshape";

		if (test_bit(MD_RECOVERY_SYNC, &mddev->recovery)) {
			if (!test_bit(MD_RECOVERY_REQUESTED, &mddev->recovery))
				return "resync";
			else if (test_bit(MD_RECOVERY_CHECK, &mddev->recovery))
				return "check";
			return "repair";
		}

		if (test_bit(MD_RECOVERY_RECOVER, &mddev->recovery))
			return "recover";
	}

	return "idle";
}

static void raid_status(struct dm_target *ti, status_type_t type,
			unsigned status_flags, char *result, unsigned maxlen)
{
	struct raid_set *rs = ti->private;
	unsigned raid_param_cnt = 1; /* at least 1 for chunksize */
	unsigned sz = 0;
	int i, array_in_sync = 0;
	sector_t sync;

	switch (type) {
	case STATUSTYPE_INFO:
		DMEMIT("%s %d ", rs->raid_type->name, rs->md.raid_disks);

		if (test_bit(MD_RECOVERY_RUNNING, &rs->md.recovery))
			sync = rs->md.curr_resync_completed;
		else
			sync = rs->md.recovery_cp;

		if (sync >= rs->md.resync_max_sectors) {
			/*
			 * Sync complete.
			 */
			array_in_sync = 1;
			sync = rs->md.resync_max_sectors;
		} else if (test_bit(MD_RECOVERY_REQUESTED, &rs->md.recovery)) {
			/*
			 * If "check" or "repair" is occurring, the array has
			 * undergone and initial sync and the health characters
			 * should not be 'a' anymore.
			 */
			array_in_sync = 1;
		} else {
			/*
			 * The array may be doing an initial sync, or it may
			 * be rebuilding individual components.  If all the
			 * devices are In_sync, then it is the array that is
			 * being initialized.
			 */
			for (i = 0; i < rs->md.raid_disks; i++)
				if (!test_bit(In_sync, &rs->dev[i].rdev.flags))
					array_in_sync = 1;
		}

		/*
		 * Status characters:
		 *  'D' = Dead/Failed device
		 *  'a' = Alive but not in-sync
		 *  'A' = Alive and in-sync
		 */
		for (i = 0; i < rs->md.raid_disks; i++) {
			if (test_bit(Faulty, &rs->dev[i].rdev.flags))
				DMEMIT("D");
			else if (!array_in_sync ||
				 !test_bit(In_sync, &rs->dev[i].rdev.flags))
				DMEMIT("a");
			else
				DMEMIT("A");
		}

		/*
		 * In-sync ratio:
		 *  The in-sync ratio shows the progress of:
		 *   - Initializing the array
		 *   - Rebuilding a subset of devices of the array
		 *  The user can distinguish between the two by referring
		 *  to the status characters.
		 */
		DMEMIT(" %llu/%llu",
		       (unsigned long long) sync,
		       (unsigned long long) rs->md.resync_max_sectors);

		/*
		 * Sync action:
		 *   See Documentation/device-mapper/dm-raid.c for
		 *   information on each of these states.
		 */
		DMEMIT(" %s", decipher_sync_action(&rs->md));

		/*
		 * resync_mismatches/mismatch_cnt
		 *   This field shows the number of discrepancies found when
		 *   performing a "check" of the array.
		 */
		DMEMIT(" %llu",
		       (strcmp(rs->md.last_sync_action, "check")) ? 0 :
		       (unsigned long long)
		       atomic64_read(&rs->md.resync_mismatches));
		break;
	case STATUSTYPE_TABLE:
		/* The string you would use to construct this array */
		for (i = 0; i < rs->md.raid_disks; i++) {
			if ((rs->print_flags & DMPF_REBUILD) &&
			    rs->dev[i].data_dev &&
			    !test_bit(In_sync, &rs->dev[i].rdev.flags))
				raid_param_cnt += 2; /* for rebuilds */
			if (rs->dev[i].data_dev &&
			    test_bit(WriteMostly, &rs->dev[i].rdev.flags))
				raid_param_cnt += 2;
		}

		raid_param_cnt += (hweight32(rs->print_flags & ~DMPF_REBUILD) * 2);
		if (rs->print_flags & (DMPF_SYNC | DMPF_NOSYNC))
			raid_param_cnt--;

		DMEMIT("%s %u %u", rs->raid_type->name,
		       raid_param_cnt, rs->md.chunk_sectors);

		if ((rs->print_flags & DMPF_SYNC) &&
		    (rs->md.recovery_cp == MaxSector))
			DMEMIT(" sync");
		if (rs->print_flags & DMPF_NOSYNC)
			DMEMIT(" nosync");

		for (i = 0; i < rs->md.raid_disks; i++)
			if ((rs->print_flags & DMPF_REBUILD) &&
			    rs->dev[i].data_dev &&
			    !test_bit(In_sync, &rs->dev[i].rdev.flags))
				DMEMIT(" rebuild %u", i);

		if (rs->print_flags & DMPF_DAEMON_SLEEP)
			DMEMIT(" daemon_sleep %lu",
			       rs->md.bitmap_info.daemon_sleep);

		if (rs->print_flags & DMPF_MIN_RECOVERY_RATE)
			DMEMIT(" min_recovery_rate %d", rs->md.sync_speed_min);

		if (rs->print_flags & DMPF_MAX_RECOVERY_RATE)
			DMEMIT(" max_recovery_rate %d", rs->md.sync_speed_max);

		for (i = 0; i < rs->md.raid_disks; i++)
			if (rs->dev[i].data_dev &&
			    test_bit(WriteMostly, &rs->dev[i].rdev.flags))
				DMEMIT(" write_mostly %u", i);

		if (rs->print_flags & DMPF_MAX_WRITE_BEHIND)
			DMEMIT(" max_write_behind %lu",
			       rs->md.bitmap_info.max_write_behind);

		if (rs->print_flags & DMPF_STRIPE_CACHE) {
			struct r5conf *conf = rs->md.private;

			/* convert from kiB to sectors */
			DMEMIT(" stripe_cache %d",
			       conf ? conf->max_nr_stripes * 2 : 0);
		}

		if (rs->print_flags & DMPF_REGION_SIZE)
			DMEMIT(" region_size %lu",
			       rs->md.bitmap_info.chunksize >> 9);

		if (rs->print_flags & DMPF_RAID10_COPIES)
			DMEMIT(" raid10_copies %u",
			       raid10_md_layout_to_copies(rs->md.layout));

		if (rs->print_flags & DMPF_RAID10_FORMAT)
			DMEMIT(" raid10_format %s",
			       raid10_md_layout_to_format(rs->md.layout));

		DMEMIT(" %d", rs->md.raid_disks);
		for (i = 0; i < rs->md.raid_disks; i++) {
			if (rs->dev[i].meta_dev)
				DMEMIT(" %s", rs->dev[i].meta_dev->name);
			else
				DMEMIT(" -");

			if (rs->dev[i].data_dev)
				DMEMIT(" %s", rs->dev[i].data_dev->name);
			else
				DMEMIT(" -");
		}
	}
}

static int raid_message(struct dm_target *ti, unsigned argc, char **argv)
{
	struct raid_set *rs = ti->private;
	struct mddev *mddev = &rs->md;

	if (!strcasecmp(argv[0], "reshape")) {
		DMERR("Reshape not supported.");
		return -EINVAL;
	}

	if (!mddev->pers || !mddev->pers->sync_request)
		return -EINVAL;

	if (!strcasecmp(argv[0], "frozen"))
		set_bit(MD_RECOVERY_FROZEN, &mddev->recovery);
	else
		clear_bit(MD_RECOVERY_FROZEN, &mddev->recovery);

	if (!strcasecmp(argv[0], "idle") || !strcasecmp(argv[0], "frozen")) {
		if (mddev->sync_thread) {
			set_bit(MD_RECOVERY_INTR, &mddev->recovery);
			md_reap_sync_thread(mddev);
		}
	} else if (test_bit(MD_RECOVERY_RUNNING, &mddev->recovery) ||
		   test_bit(MD_RECOVERY_NEEDED, &mddev->recovery))
		return -EBUSY;
	else if (!strcasecmp(argv[0], "resync"))
		set_bit(MD_RECOVERY_NEEDED, &mddev->recovery);
	else if (!strcasecmp(argv[0], "recover")) {
		set_bit(MD_RECOVERY_RECOVER, &mddev->recovery);
		set_bit(MD_RECOVERY_NEEDED, &mddev->recovery);
	} else {
		if (!strcasecmp(argv[0], "check"))
			set_bit(MD_RECOVERY_CHECK, &mddev->recovery);
		else if (!!strcasecmp(argv[0], "repair"))
			return -EINVAL;
		set_bit(MD_RECOVERY_REQUESTED, &mddev->recovery);
		set_bit(MD_RECOVERY_SYNC, &mddev->recovery);
	}
	if (mddev->ro == 2) {
		/* A write to sync_action is enough to justify
		 * canceling read-auto mode
		 */
		mddev->ro = 0;
		if (!mddev->suspended)
			md_wakeup_thread(mddev->sync_thread);
	}
	set_bit(MD_RECOVERY_NEEDED, &mddev->recovery);
	if (!mddev->suspended)
		md_wakeup_thread(mddev->thread);

	return 0;
}

static int raid_iterate_devices(struct dm_target *ti,
				iterate_devices_callout_fn fn, void *data)
{
	struct raid_set *rs = ti->private;
	unsigned i;
	int ret = 0;

	for (i = 0; !ret && i < rs->md.raid_disks; i++)
		if (rs->dev[i].data_dev)
			ret = fn(ti,
				 rs->dev[i].data_dev,
				 0, /* No offset on data devs */
				 rs->md.dev_sectors,
				 data);

	return ret;
}

static void raid_io_hints(struct dm_target *ti, struct queue_limits *limits)
{
	struct raid_set *rs = ti->private;
	unsigned chunk_size = rs->md.chunk_sectors << 9;
	struct r5conf *conf = rs->md.private;

	blk_limits_io_min(limits, chunk_size);
	blk_limits_io_opt(limits, chunk_size * (conf->raid_disks - conf->max_degraded));
}

static void raid_presuspend(struct dm_target *ti)
{
	struct raid_set *rs = ti->private;

	md_stop_writes(&rs->md);
}

static void raid_postsuspend(struct dm_target *ti)
{
	struct raid_set *rs = ti->private;

	mddev_suspend(&rs->md);
}

static void attempt_restore_of_faulty_devices(struct raid_set *rs)
{
	int i;
	uint64_t failed_devices, cleared_failed_devices = 0;
	unsigned long flags;
	struct dm_raid_superblock *sb;
	struct md_rdev *r;

	for (i = 0; i < rs->md.raid_disks; i++) {
		r = &rs->dev[i].rdev;
		if (test_bit(Faulty, &r->flags) && r->sb_page &&
		    sync_page_io(r, 0, r->sb_size, r->sb_page, READ, 1)) {
			DMINFO("Faulty %s device #%d has readable super block."
			       "  Attempting to revive it.",
			       rs->raid_type->name, i);

			/*
			 * Faulty bit may be set, but sometimes the array can
			 * be suspended before the personalities can respond
			 * by removing the device from the array (i.e. calling
			 * 'hot_remove_disk').  If they haven't yet removed
			 * the failed device, its 'raid_disk' number will be
			 * '>= 0' - meaning we must call this function
			 * ourselves.
			 */
			if ((r->raid_disk >= 0) &&
			    (r->mddev->pers->hot_remove_disk(r->mddev, r) != 0))
				/* Failed to revive this device, try next */
				continue;

			r->raid_disk = i;
			r->saved_raid_disk = i;
			flags = r->flags;
			clear_bit(Faulty, &r->flags);
			clear_bit(WriteErrorSeen, &r->flags);
			clear_bit(In_sync, &r->flags);
			if (r->mddev->pers->hot_add_disk(r->mddev, r)) {
				r->raid_disk = -1;
				r->saved_raid_disk = -1;
				r->flags = flags;
			} else {
				r->recovery_offset = 0;
				cleared_failed_devices |= 1 << i;
			}
		}
	}
	if (cleared_failed_devices) {
		rdev_for_each(r, &rs->md) {
			sb = page_address(r->sb_page);
			failed_devices = le64_to_cpu(sb->failed_devices);
			failed_devices &= ~cleared_failed_devices;
			sb->failed_devices = cpu_to_le64(failed_devices);
		}
	}
}

static void raid_resume(struct dm_target *ti)
{
	struct raid_set *rs = ti->private;

	set_bit(MD_CHANGE_DEVS, &rs->md.flags);
	if (!rs->bitmap_loaded) {
		bitmap_load(&rs->md);
		rs->bitmap_loaded = 1;
	} else {
		/*
		 * A secondary resume while the device is active.
		 * Take this opportunity to check whether any failed
		 * devices are reachable again.
		 */
		attempt_restore_of_faulty_devices(rs);
	}

	clear_bit(MD_RECOVERY_FROZEN, &rs->md.recovery);
	mddev_resume(&rs->md);
}

static struct target_type raid_target = {
	.name = "raid",
	.version = {1, 6, 0},
	.module = THIS_MODULE,
	.ctr = raid_ctr,
	.dtr = raid_dtr,
	.map = raid_map,
	.status = raid_status,
	.message = raid_message,
	.iterate_devices = raid_iterate_devices,
	.io_hints = raid_io_hints,
	.presuspend = raid_presuspend,
	.postsuspend = raid_postsuspend,
	.resume = raid_resume,
};

static int __init dm_raid_init(void)
{
	DMINFO("Loading target version %u.%u.%u",
	       raid_target.version[0],
	       raid_target.version[1],
	       raid_target.version[2]);
	return dm_register_target(&raid_target);
}

static void __exit dm_raid_exit(void)
{
	dm_unregister_target(&raid_target);
}

module_init(dm_raid_init);
module_exit(dm_raid_exit);

module_param(devices_handle_discard_safely, bool, 0644);
MODULE_PARM_DESC(devices_handle_discard_safely,
		 "Set to Y if all devices in each array reliably return zeroes on reads from discarded regions");

MODULE_DESCRIPTION(DM_NAME " raid4/5/6 target");
MODULE_ALIAS("dm-raid1");
MODULE_ALIAS("dm-raid10");
MODULE_ALIAS("dm-raid4");
MODULE_ALIAS("dm-raid5");
MODULE_ALIAS("dm-raid6");
MODULE_AUTHOR("Neil Brown <dm-devel@redhat.com>");
MODULE_LICENSE("GPL");
