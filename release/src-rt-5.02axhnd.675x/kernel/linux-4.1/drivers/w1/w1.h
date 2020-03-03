/*
 *	w1.h
 *
 * Copyright (c) 2004 Evgeniy Polyakov <zbr@ioremap.net>
 *
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __W1_H
#define __W1_H

/**
 * struct w1_reg_num - broken out slave device id
 *
 * @family: identifies the type of device
 * @id: along with family is the unique device id
 * @crc: checksum of the other bytes
 */
struct w1_reg_num
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u64	family:8,
		id:48,
		crc:8;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u64	crc:8,
		id:48,
		family:8;
#else
#error "Please fix <asm/byteorder.h>"
#endif
};

#ifdef __KERNEL__

#include <linux/completion.h>
#include <linux/device.h>
#include <linux/mutex.h>

#include "w1_family.h"

#define W1_MAXNAMELEN		32

#define W1_SEARCH		0xF0
#define W1_ALARM_SEARCH		0xEC
#define W1_CONVERT_TEMP		0x44
#define W1_SKIP_ROM		0xCC
#define W1_READ_SCRATCHPAD	0xBE
#define W1_READ_ROM		0x33
#define W1_READ_PSUPPLY		0xB4
#define W1_MATCH_ROM		0x55
#define W1_RESUME_CMD		0xA5

#define W1_SLAVE_ACTIVE		0
#define W1_SLAVE_DETACH		1

/**
 * struct w1_slave - holds a single slave device on the bus
 *
 * @owner: Points to the one wire "wire" kernel module.
 * @name: Device id is ascii.
 * @w1_slave_entry: data for the linked list
 * @reg_num: the slave id in binary
 * @refcnt: reference count, delete when 0
 * @flags: bit flags for W1_SLAVE_ACTIVE W1_SLAVE_DETACH
 * @ttl: decrement per search this slave isn't found, deatch at 0
 * @master: bus which this slave is on
 * @family: module for device family type
 * @family_data: pointer for use by the family module
 * @dev: kernel device identifier
 *
 */
struct w1_slave
{
	struct module		*owner;
	unsigned char		name[W1_MAXNAMELEN];
	struct list_head	w1_slave_entry;
	struct w1_reg_num	reg_num;
	atomic_t		refcnt;
	int			ttl;
	unsigned long		flags;

	struct w1_master	*master;
	struct w1_family	*family;
	void			*family_data;
	struct device		dev;
};

typedef void (*w1_slave_found_callback)(struct w1_master *, u64);


/**
 * struct w1_bus_master - operations available on a bus master
 *
 * @data: the first parameter in all the functions below
 *
 * @read_bit: Sample the line level @return the level read (0 or 1)
 *
 * @write_bit: Sets the line level
 *
 * @touch_bit: the lowest-level function for devices that really support the
 * 1-wire protocol.
 * touch_bit(0) = write-0 cycle
 * touch_bit(1) = write-1 / read cycle
 * @return the bit read (0 or 1)
 *
 * @read_byte: Reads a bytes. Same as 8 touch_bit(1) calls.
 * @return the byte read
 *
 * @write_byte: Writes a byte. Same as 8 touch_bit(x) calls.
 *
 * @read_block: Same as a series of read_byte() calls
 * @return the number of bytes read
 *
 * @write_block: Same as a series of write_byte() calls
 *
 * @triplet: Combines two reads and a smart write for ROM searches
 * @return bit0=Id bit1=comp_id bit2=dir_taken
 *
 * @reset_bus: long write-0 with a read for the presence pulse detection
 * @return -1=Error, 0=Device present, 1=No device present
 *
 * @set_pullup: Put out a strong pull-up pulse of the specified duration.
 * @return -1=Error, 0=completed
 *
 * @search: Really nice hardware can handles the different types of ROM search
 * w1_master* is passed to the slave found callback.
 * u8 is search_type, W1_SEARCH or W1_ALARM_SEARCH
 *
 * Note: read_bit and write_bit are very low level functions and should only
 * be used with hardware that doesn't really support 1-wire operations,
 * like a parallel/serial port.
 * Either define read_bit and write_bit OR define, at minimum, touch_bit and
 * reset_bus.
 *
 */
struct w1_bus_master
{
	void		*data;

	u8		(*read_bit)(void *);

	void		(*write_bit)(void *, u8);

	u8		(*touch_bit)(void *, u8);

	u8		(*read_byte)(void *);

	void		(*write_byte)(void *, u8);

	u8		(*read_block)(void *, u8 *, int);

	void		(*write_block)(void *, const u8 *, int);

	u8		(*triplet)(void *, u8);

	u8		(*reset_bus)(void *);

	u8		(*set_pullup)(void *, int);

	void		(*search)(void *, struct w1_master *,
		u8, w1_slave_found_callback);
};

/**
 * enum w1_master_flags - bitfields used in w1_master.flags
 * @W1_ABORT_SEARCH: abort searching early on shutdown
 * @W1_WARN_MAX_COUNT: limit warning when the maximum count is reached
 */
enum w1_master_flags {
	W1_ABORT_SEARCH = 0,
	W1_WARN_MAX_COUNT = 1,
};

/**
 * struct w1_master - one per bus master
 * @w1_master_entry:	master linked list
 * @owner:		module owner
 * @name:		dynamically allocate bus name
 * @list_mutex:		protect slist and async_list
 * @slist:		linked list of slaves
 * @async_list:		linked list of netlink commands to execute
 * @max_slave_count:	maximum number of slaves to search for at a time
 * @slave_count:	current number of slaves known
 * @attempts:		number of searches ran
 * @slave_ttl:		number of searches before a slave is timed out
 * @initialized:	prevent init/removal race conditions
 * @id:			w1 bus number
 * @search_count:	number of automatic searches to run, -1 unlimited
 * @search_id:		allows continuing a search
 * @refcnt:		reference count
 * @priv:		private data storage
 * @enable_pullup:	allows a strong pullup
 * @pullup_duration:	time for the next strong pullup
 * @flags:		one of w1_master_flags
 * @thread:		thread for bus search and netlink commands
 * @mutex:		protect most of w1_master
 * @bus_mutex:		pretect concurrent bus access
 * @driver:		sysfs driver
 * @dev:		sysfs device
 * @bus_master:		io operations available
 * @seq:		sequence number used for netlink broadcasts
 */
struct w1_master
{
	struct list_head	w1_master_entry;
	struct module		*owner;
	unsigned char		name[W1_MAXNAMELEN];
	/* list_mutex protects just slist and async_list so slaves can be
	 * searched for and async commands added while the master has
	 * w1_master.mutex locked and is operating on the bus.
	 * lock order w1_mlock, w1_master.mutex, w1_master.list_mutex
	 */
	struct mutex		list_mutex;
	struct list_head	slist;
	struct list_head	async_list;
	int			max_slave_count, slave_count;
	unsigned long		attempts;
	int			slave_ttl;
	int			initialized;
	u32			id;
	int			search_count;
	/* id to start searching on, to continue a search or 0 to restart */
	u64			search_id;

	atomic_t		refcnt;

	void			*priv;

	/** 5V strong pullup enabled flag, 1 enabled, zero disabled. */
	int			enable_pullup;
	/** 5V strong pullup duration in milliseconds, zero disabled. */
	int			pullup_duration;

	long			flags;

	struct task_struct	*thread;
	struct mutex		mutex;
	struct mutex		bus_mutex;

	struct device_driver	*driver;
	struct device		dev;

	struct w1_bus_master	*bus_master;

	u32			seq;
};

/**
 * struct w1_async_cmd - execute callback from the w1_process kthread
 * @async_entry: link entry
 * @cb: callback function, must list_del and destroy this list before
 * returning
 *
 * When inserted into the w1_master async_list, w1_process will execute
 * the callback.  Embed this into the structure with the command details.
 */
struct w1_async_cmd {
	struct list_head	async_entry;
	void (*cb)(struct w1_master *dev, struct w1_async_cmd *async_cmd);
};

int w1_create_master_attributes(struct w1_master *);
void w1_destroy_master_attributes(struct w1_master *master);
void w1_search(struct w1_master *dev, u8 search_type, w1_slave_found_callback cb);
void w1_search_devices(struct w1_master *dev, u8 search_type, w1_slave_found_callback cb);
/* call w1_unref_slave to release the reference counts w1_search_slave added */
struct w1_slave *w1_search_slave(struct w1_reg_num *id);
/* decrements the reference on sl->master and sl, and cleans up if zero
 * returns the reference count after it has been decremented */
int w1_unref_slave(struct w1_slave *sl);
void w1_slave_found(struct w1_master *dev, u64 rn);
void w1_search_process_cb(struct w1_master *dev, u8 search_type,
	w1_slave_found_callback cb);
struct w1_slave *w1_slave_search_device(struct w1_master *dev,
	struct w1_reg_num *rn);
struct w1_master *w1_search_master_id(u32 id);

/* Disconnect and reconnect devices in the given family.  Used for finding
 * unclaimed devices after a family has been registered or releasing devices
 * after a family has been unregistered.  Set attach to 1 when a new family
 * has just been registered, to 0 when it has been unregistered.
 */
void w1_reconnect_slaves(struct w1_family *f, int attach);
int w1_attach_slave_device(struct w1_master *dev, struct w1_reg_num *rn);
/* 0 success, otherwise EBUSY */
int w1_slave_detach(struct w1_slave *sl);

u8 w1_triplet(struct w1_master *dev, int bdir);
void w1_write_8(struct w1_master *, u8);
u8 w1_read_8(struct w1_master *);
int w1_reset_bus(struct w1_master *);
u8 w1_calc_crc8(u8 *, int);
void w1_write_block(struct w1_master *, const u8 *, int);
void w1_touch_block(struct w1_master *, u8 *, int);
u8 w1_read_block(struct w1_master *, u8 *, int);
int w1_reset_select_slave(struct w1_slave *sl);
int w1_reset_resume_command(struct w1_master *);
void w1_next_pullup(struct w1_master *, int);

static inline struct w1_slave* dev_to_w1_slave(struct device *dev)
{
	return container_of(dev, struct w1_slave, dev);
}

static inline struct w1_slave* kobj_to_w1_slave(struct kobject *kobj)
{
	return dev_to_w1_slave(container_of(kobj, struct device, kobj));
}

static inline struct w1_master* dev_to_w1_master(struct device *dev)
{
	return container_of(dev, struct w1_master, dev);
}

extern struct device_driver w1_master_driver;
extern struct device w1_master_device;
extern int w1_max_slave_count;
extern int w1_max_slave_ttl;
extern struct list_head w1_masters;
extern struct mutex w1_mlock;

extern int w1_process_callbacks(struct w1_master *dev);
extern int w1_process(void *);

#endif /* __KERNEL__ */

#endif /* __W1_H */
