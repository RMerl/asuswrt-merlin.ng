/*
 * sar/sadc: Report system activity
 * (C) 1999-2011 by Sebastien Godard (sysstat <at> orange.fr)
 */

#ifndef _SA_H
#define _SA_H

#include "common.h"
#include "rd_stats.h"
#include "rd_sensors.h"

/*
 ***************************************************************************
 * Activity identification values.
 ***************************************************************************
 */

/* Number of activities */
#define NR_ACT	36

/* Activities */
#define A_CPU		1
#define A_PCSW		2
#define A_IRQ		3
#define A_SWAP		4
#define A_PAGE		5
#define A_IO		6
#define A_MEMORY	7
#define A_KTABLES	8
#define A_QUEUE		9
#define A_SERIAL	10
#define A_DISK		11
#define A_NET_DEV	12
#define A_NET_EDEV	13
#define A_NET_NFS	14
#define A_NET_NFSD	15
#define A_NET_SOCK	16
#define A_NET_IP	17
#define A_NET_EIP	18
#define A_NET_ICMP	19
#define A_NET_EICMP	20
#define A_NET_TCP	21
#define A_NET_ETCP	22
#define A_NET_UDP	23
#define A_NET_SOCK6	24
#define A_NET_IP6	25
#define A_NET_EIP6	26
#define A_NET_ICMP6	27
#define A_NET_EICMP6	28
#define A_NET_UDP6	29
#define A_PWR_CPUFREQ	30
#define A_PWR_FAN	31
#define A_PWR_TEMP	32
#define A_PWR_IN	33
#define A_HUGE		34
#define A_PWR_WGHFREQ	35
#define A_PWR_USB	36


/* Macro used to flag an activity that should be collected */
#define COLLECT_ACTIVITY(m)	act[get_activity_position(act, m)]->options |= AO_COLLECTED

/* Macro used to flag an activity that should be selected */
#define SELECT_ACTIVITY(m)	act[get_activity_position(act, m)]->options |= AO_SELECTED


/*
 ***************************************************************************
 * Flags.
 ***************************************************************************
 */

#define S_F_SINCE_BOOT		0x00000001
#define S_F_SA_ROTAT      	0x00000002
#define S_F_DEV_PRETTY		0x00000004
#define S_F_FORCE_FILE		0x00000008
#define S_F_INTERVAL_SET	0x00000010
#define S_F_TRUE_TIME		0x00000020
#define S_F_LOCK_FILE		0x00000040
#define S_F_SEC_EPOCH		0x00000080
#define S_F_HDR_ONLY		0x00000100
#define S_F_FILE_LOCKED		0x00000200
#define S_F_PER_PROC		0x00000400
#define S_F_HORIZONTALLY	0x00000800
#define S_F_COMMENT		0x00001000

#define WANT_SINCE_BOOT(m)	(((m) & S_F_SINCE_BOOT)   == S_F_SINCE_BOOT)
#define WANT_SA_ROTAT(m)	(((m) & S_F_SA_ROTAT)     == S_F_SA_ROTAT)
#define USE_PRETTY_OPTION(m)	(((m) & S_F_DEV_PRETTY)   == S_F_DEV_PRETTY)
#define FORCE_FILE(m)		(((m) & S_F_FORCE_FILE)   == S_F_FORCE_FILE)
#define INTERVAL_SET(m)		(((m) & S_F_INTERVAL_SET) == S_F_INTERVAL_SET)
#define PRINT_TRUE_TIME(m)	(((m) & S_F_TRUE_TIME)    == S_F_TRUE_TIME)
#define LOCK_FILE(m)		(((m) & S_F_LOCK_FILE)    == S_F_LOCK_FILE)
#define PRINT_SEC_EPOCH(m)	(((m) & S_F_SEC_EPOCH)    == S_F_SEC_EPOCH)
#define DISPLAY_HDR_ONLY(m)	(((m) & S_F_HDR_ONLY)     == S_F_HDR_ONLY)
#define FILE_LOCKED(m)		(((m) & S_F_FILE_LOCKED)  == S_F_FILE_LOCKED)
#define WANT_PER_PROC(m)	(((m) & S_F_PER_PROC)     == S_F_PER_PROC)
#define DISPLAY_HORIZONTALLY(m)	(((m) & S_F_HORIZONTALLY) == S_F_HORIZONTALLY)
#define DISPLAY_COMMENT(m)	(((m) & S_F_COMMENT)      == S_F_COMMENT)

#define AO_F_NULL		0x00000000

/* Output flags for options -R / -r / -S */
#define AO_F_MEM_DIA		0x00000001
#define AO_F_MEM_AMT		0x00000002
#define AO_F_MEM_SWAP		0x00000004

#define DISPLAY_MEMORY(m)	(((m) & AO_F_MEM_DIA)     == AO_F_MEM_DIA)
#define DISPLAY_MEM_AMT(m)	(((m) & AO_F_MEM_AMT)     == AO_F_MEM_AMT)
#define DISPLAY_SWAP(m)		(((m) & AO_F_MEM_SWAP)    == AO_F_MEM_SWAP)

/* Output flags for option -u [ ALL ] */
#define AO_F_CPU_DEF		0x00000001
#define AO_F_CPU_ALL		0x00000002

#define DISPLAY_CPU_DEF(m)	(((m) & AO_F_CPU_DEF)     == AO_F_CPU_DEF)
#define DISPLAY_CPU_ALL(m)	(((m) & AO_F_CPU_ALL)     == AO_F_CPU_ALL)

/* Output flags for option -d */
#define AO_F_DISK_PART		0x00000001

#define COLLECT_PARTITIONS(m)	(((m) & AO_F_DISK_PART)   == AO_F_DISK_PART)

/*
 ***************************************************************************
 * Various keywords and constants.
 ***************************************************************************
 */

/* Keywords */
#define K_XALL		"XALL"
#define K_SUM		"SUM"
#define K_DEV		"DEV"
#define K_EDEV		"EDEV"
#define K_NFS		"NFS"
#define K_NFSD		"NFSD"
#define K_SOCK		"SOCK"
#define K_IP		"IP"
#define K_EIP		"EIP"
#define K_ICMP		"ICMP"
#define K_EICMP		"EICMP"
#define K_TCP		"TCP"
#define K_ETCP		"ETCP"
#define K_UDP		"UDP"
#define K_SOCK6		"SOCK6"
#define K_IP6		"IP6"
#define K_EIP6		"EIP6"
#define K_ICMP6		"ICMP6"
#define K_EICMP6	"EICMP6"
#define K_UDP6		"UDP6"
#define K_CPU		"CPU"
#define K_FAN		"FAN"
#define K_TEMP		"TEMP"
#define K_IN		"IN"
#define K_FREQ		"FREQ"

#define K_INT		"INT"
#define K_DISK		"DISK"
#define K_XDISK		"XDISK"
#define K_SNMP		"SNMP"
#define K_IPV6		"IPV6"
#define K_POWER		"POWER"
#define K_USB		"USB"

/* Groups of activities */
#define G_DEFAULT	0x00
#define G_INT		0x01
#define G_DISK		0x02
#define G_SNMP		0x04
#define G_IPV6		0x08
#define G_POWER		0x10

/* sadc program */
#define SADC		"sadc"

/* Time must have the format HH:MM:SS with HH in 24-hour format */
#define DEF_TMSTART	"08:00:00"
#define DEF_TMEND	"18:00:00"

/*
 * Macro used to define activity bitmap size.
 * All those bitmaps have an additional bit used for global activity
 * (eg. CPU "all" or total number of interrupts). That's why we do "(m) + 1".
 */
#define BITMAP_SIZE(m)	((((m) + 1) / 8) + 1)

/* Pre-allocation constants */
#define NR_IFACE_PREALLOC	2
#define NR_SERIAL_PREALLOC	2
#define NR_DISK_PREALLOC	3
#define NR_FREQ_PREALLOC	0
#define NR_USB_PREALLOC		5

#define UTSNAME_LEN		65
#define TIMESTAMP_LEN		16
#define HEADER_LINE_LEN		512

/* Maximum number of args that can be passed to sadc */
#define MAX_ARGV_NR	32

/* Miscellaneous constants */
#define USE_SADC	0
#define USE_SA_FILE	1
#define NO_TM_START	0
#define NO_TM_END	0
#define NO_RESET	0
#define NON_FATAL	0
#define FATAL		1
#define C_SAR		0
#define C_SADF		1
#define ALL_ACTIVITIES	~0U

#define SOFT_SIZE	0
#define HARD_SIZE	1

#define CLOSE_XML_MARKUP	0
#define OPEN_XML_MARKUP		1

#define CLOSE_JSON_MARKUP	0
#define OPEN_JSON_MARKUP	1

#define COUNT_ACTIVITIES	0
#define COUNT_OUTPUTS		1


/*
 ***************************************************************************
 * Generic description of an activity.
 ***************************************************************************
 */

/* Activity options */
#define AO_NULL			0x00
/*
 * Indicate that corresponding activity should be collected by sadc.
 */
#define AO_COLLECTED		0x01
/*
 * Indicate that corresponding activity should be displayed by sar.
 */
#define AO_SELECTED		0x02
/*
 * Indicate that, when registered again, activity counters will get back
 * the values they had when they were unregistered (eg. CPUs, which can
 * be disabled/enabled on the fly).
 */
#define AO_REMANENT		0x04
/*
 * Indicate that the interval of time, given to f_print() function
 * displaying statistics, should be the interval of time in jiffies
 * multiplied by the number of processors.
 */
#define AO_GLOBAL_ITV		0x08
/*
 * This flag should be set for every activity closing a markup used
 * by several activities. Used by sadf f_xml_print() functions to
 * display XML output.
 */
#define AO_CLOSE_MARKUP		0x10
/*
 * Indicate that corresponding activity has multiple different
 * output formats. This is the case for example for memory activity
 * with options -r and -R.
 */
#define AO_MULTIPLE_OUTPUTS	0x20

#define IS_COLLECTED(m)		(((m) & AO_COLLECTED)        == AO_COLLECTED)
#define IS_SELECTED(m)		(((m) & AO_SELECTED)         == AO_SELECTED)
#define IS_REMANENT(m)		(((m) & AO_REMANENT)         == AO_REMANENT)
#define NEED_GLOBAL_ITV(m)	(((m) & AO_GLOBAL_ITV)       == AO_GLOBAL_ITV)
#define CLOSE_MARKUP(m)		(((m) & AO_CLOSE_MARKUP)     == AO_CLOSE_MARKUP)
#define HAS_MULTIPLE_OUTPUTS(m)	(((m) & AO_MULTIPLE_OUTPUTS) == AO_MULTIPLE_OUTPUTS)

/* Type for all functions counting items */
#define __nr_t		int
/* Type for all functions reading statistics */
#define __read_funct_t	void
/* Type for all functions displaying statistics */
#define __print_funct_t void

#define _buf0	buf[0]

/* Structure used to define a bitmap needed by an activity */
struct act_bitmap {
	/*
	 * Bitmap for activities that need one. Remember to allocate it
	 * before use!
	 */
	unsigned char *b_array;
	/*
	 * Size of the bitmap in bits. In fact, bitmap is sized to bitmap_size + 1
	 * to take into account CPU "all"
	 */
	int b_size;
};

/*
 * Structure used to define an activity.
 * Note; This structure can be modified without changing the format of data files.
 */
struct activity {
	/*
	 * This variable contains the identification value (A_...) for this activity.
	 */
	unsigned int id;
	/*
	 * Activity options (AO_...)
	 */
	unsigned int options;
	/*
	 * Activity magical number. This number changes when activity format in file
	 * is no longer compatible with the format of that same activity from
	 * previous versions.
	 */
	unsigned int magic;
	/*
	 * An activity belongs to a group (and only one).
	 * Groups are those selected with option -S of sadc.
	 */
	unsigned int group;
	/*
	 * The f_count() function is used to count the number of
	 * items (serial lines, network interfaces, etc.) -> @nr
	 * Such a function should _always_ return a value greater than
	 * or equal to 0.
	 *
	 * A NULL value for this function pointer indicates that the number of items
	 * is a constant (and @nr is set to this value).
	 *
	 * This function is called even if activity has not been selected, to make
	 * sure that all items have been calculated (including #CPU, etc.)
	 */
	__nr_t (*f_count) (struct activity *);
	/*
	 * The f_count2() function is used to count the number of
	 * sub-items -> @nr2
	 * Such a function should _always_ return a value greater than
	 * or equal to 0.
	 *
	 * A NULL value for this function pointer indicates that the number of items
	 * is a constant (and @nr2 is set to this value).
	 */
	__nr_t (*f_count2) (struct activity *);
	/*
	 * This function reads the relevant file and fill the buffer
	 * with statistics corresponding to given activity.
	 */
	__read_funct_t (*f_read) (struct activity *);
	/*
	 * This function displays activity statistics onto the screen.
	 */
	__print_funct_t (*f_print) (struct activity *, int, int, unsigned long long);
	/*
	 * This function displays average activity statistics onto the screen.
	 */
	__print_funct_t (*f_print_avg) (struct activity *, int, int, unsigned long long);
	/*
	 * This function is used by sadf to display activity in a format that can
	 * easily be ingested by a relational database, or a format that can be
	 * handled by pattern processing commands like "awk".
	 */
	__print_funct_t (*f_render) (struct activity *, int, char *, int, unsigned long long);
	/*
	 * This function is used by sadf to display activity statistics in XML.
	 */
	__print_funct_t (*f_xml_print) (struct activity *, int, int, unsigned long long);
	/*
	 * This function is used by sadf to display activity statistics in JSON.
	 */
	__print_funct_t (*f_json_print) (struct activity *, int, int, unsigned long long);
	/*
	 * Header string displayed by sadf -d/-D.
	 */
	char *hdr_line;
	/*
	 * Name of activity.
	 */
	char *name;
	/*
	 * Number of items on the system.
	 * A negative value (-1) is the default value and indicates that this number
	 * has still not been calculated by the f_count() function.
	 * A value of 0 means that this number has been calculated, but no items have
	 * been found.
	 * A positive value (>0) has either been calculated or is a constant.
	 */
	__nr_t nr;
	/*
	 * Number of sub-items on the system.
	 * @nr2 is in fact the second dimension of a matrix of items, the first
	 * one being @nr. @nr is the number of lines, and @nr2 the number of columns.
	 * A negative value (-1) is the default value and indicates that this number
	 * has still not been calculated by the f_count2() function.
	 * A value of 0 means that this number has been calculated, but no sub-items have
	 * been found.
	 * A positive value (>0) has either been calculated or is a constant.
	 * Rules:
	 * 1) IF @nr2 = 0 THEN @nr = 0
	 *    Note: If @nr = 0, then @nr2 is undetermined (may be -1, 0 or >0).
	 * 2) IF @nr > 0 THEN @nr2 > 0.
	 *    Note: If @nr2 > 0 then @nr is undetermined (may be -1, 0 or >0).
	 */
	__nr_t nr2;
	/*
	 * Size of an item.
	 * This is the size of the corresponding structure, as read from or written
	 * to a file, or read from or written by the data collector.
	 */
	int fsize;
	/*
	 * Size of an item.
	 * This is the size of the corresponding structure as mapped into memory.
	 * @msize can be different from @fsize when data are read from or written to
	 * a data file from a different sysstat version.
	 */
	int msize;
	/*
	 * Optional flags for activity. This is eg. used when AO_MULTIPLE_OUTPUTS
	 * option is set.
	 */
	unsigned int opt_flags;
	/*
	 * Buffers that will contain the statistics read. Its size is @nr * @size each.
	 * [0]: used by sadc. Used by sar to save first collected stats (used later to
	 * compute average).
	 * [1] and [2]: current/previous statistics values (used by sar).
	 */
	void *buf[3];
	/*
	 * Bitmap for activities that need one. Such a bitmap is needed by activity
	 * if @bitmap is not NULL.
	 */
	struct act_bitmap *bitmap;
};

/*
 ***************************************************************************
 * Definitions of header structures.
 *
 * Format of system activity data files:
 *	 __
 *	|
 *	| file_magic structure
 *	|
 * 	|--
 *	|
 * 	| file_header structure
 * 	|
 * 	|--                         --|
 * 	|                             |
 * 	| file_activity structure     | * sa_nr_act
 * 	|                             |
 * 	|--                         --|
 * 	|                             |
 * 	| record_header structure     |
 * 	|                             |
 * 	|--                           | * <count>
 * 	|                             |
 * 	| Statistics structures...(*) |
 * 	|                             |
 * 	|--                         --|
 *
 * (*)Note: If it's a special record, we may find a comment instead of
 * statistics (R_COMMENT record type) or even nothing at all (R_RESTART
 * record type).
 ***************************************************************************
 */

/*
 * Sysstat magic number. Should never be modified.
 * Indicate that the file was created by sysstat.
 */
#define SYSSTAT_MAGIC	0xd596

/*
 * Datafile format magic number.
 * Modified to indicate that the format of the file is
 * no longer compatible with that of previous sysstat versions.
 */
#define FORMAT_MAGIC	0x2171

/* Structure for file magic header data */
struct file_magic {
	/*
	 * This field identifies the file as a file created by sysstat.
	 */
	unsigned short sysstat_magic;
	/*
	 * The value of this field varies whenever datafile format changes.
	 */
	unsigned short format_magic;
	/*
	 * Sysstat version used to create the file.
	 */
	unsigned char  sysstat_version;
	unsigned char  sysstat_patchlevel;
	unsigned char  sysstat_sublevel;
	unsigned char  sysstat_extraversion;
};

#define FILE_MAGIC_SIZE	(sizeof(struct file_magic))


/* Header structure for system activity data file */
struct file_header {
	/*
	 * Timestamp in seconds since the epoch.
	 */
	unsigned long sa_ust_time	__attribute__ ((aligned (8)));
	/*
	 * Number of activities saved in the file
	 */
	unsigned int sa_nr_act		__attribute__ ((aligned (8)));
	/*
	 * Current day, month and year.
	 * No need to save DST (Daylight Saving Time) flag, since it is not taken
	 * into account by the strftime() function used to print the timestamp.
	 */
	unsigned char sa_day;
	unsigned char sa_month;
	unsigned char sa_year;
	/*
	 * Size of a long integer. Useful to know the architecture on which
	 * the datafile was created.
	 */
	char sa_sizeof_long;
	/*
	 * Operating system name.
	 */
	char sa_sysname[UTSNAME_LEN];
	/*
	 * Machine hostname.
	 */
	char sa_nodename[UTSNAME_LEN];
	/*
	 * Operating system release number.
	 */
	char sa_release[UTSNAME_LEN];
	/*
	 * Machine architecture.
	 */
	char sa_machine[UTSNAME_LEN];
};

#define FILE_HEADER_SIZE	(sizeof(struct file_header))


/*
 * Base magical number for activities.
 */
#define ACTIVITY_MAGIC_BASE	0x8a
/*
 * Magical value used for activities with
 * unknown format (used for sadf -H only).
 */
#define ACTIVITY_MAGIC_UNKNOWN	0x89

/* List of activities saved in file */
struct file_activity {
	/*
	 * Identification value of activity.
	 */
	unsigned int id		__attribute__ ((aligned (4)));
	/*
	 * Activity magical number.
	 */
	unsigned int magic	__attribute__ ((packed));
	/*
	 * Number of items for this activity.
	 */
	__nr_t nr		__attribute__ ((packed));
	/*
	 * Number of sub-items for this activity.
	 */
	__nr_t nr2		__attribute__ ((packed));
	/*
	 * Size of an item structure.
	 */
	int size		__attribute__ ((packed));
};

#define FILE_ACTIVITY_SIZE	(sizeof(struct file_activity))


/* Record type */
/*
 * R_STATS means that this is a record of statistics.
 */
#define R_STATS		1
/*
 * R_RESTART means that this is a special record containing
 * a LINUX RESTART message.
 */
#define R_RESTART	2
/*
 * R_LAST_STATS warns sar that this is the last record to be written
 * to file before a file rotation, and that the next data to come will
 * be a header file.
 * Such a record is tagged R_STATS anyway before being written to file.
 */
#define R_LAST_STATS	3
/*
 * R_COMMENT means that this is a special record containing
 * a comment.
 */
#define R_COMMENT	4

/* Maximum length of a comment */
#define MAX_COMMENT_LEN	64

/* Header structure for every record */
struct record_header {
	/*
	 * Machine uptime (multiplied by the # of proc).
	 */
	unsigned long long uptime	__attribute__ ((aligned (16)));
	/*
	 * Uptime reduced to one processor. Always set, even on UP machines.
	 */
	unsigned long long uptime0	__attribute__ ((aligned (16)));
	/*
	 * Timestamp (number of seconds since the epoch).
	 */
	unsigned long ust_time		__attribute__ ((aligned (16)));
	/*
	 * Record type: R_STATS, R_RESTART,...
	 */
	unsigned char record_type	__attribute__ ((aligned (8)));
	/*
	 * Timestamp: Hour (0-23), minute (0-59) and second (0-59).
	 * Used to determine TRUE time (immutable, non locale dependent time).
	 */
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
};

#define RECORD_HEADER_SIZE	(sizeof(struct record_header))


/*
 ***************************************************************************
 * Macro functions definitions.
 *
 * Note: Using 'do ... while' makes the macros safer to use
 * (remember that macro use are followed by a semicolon).
 ***************************************************************************
 */

/* Close file descriptors */
#define CLOSE_ALL(_fd_)		do {			\
					close(_fd_[0]); \
					close(_fd_[1]); \
				} while (0)

#define CLOSE(_fd_)		if (_fd_ >= 0)		\
					close(_fd_)


/*
 ***************************************************************************
 * Various structure definitions.
 ***************************************************************************
 */

/* Structure for timestamps */
struct tstamp {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int use;
};


/*
 ***************************************************************************
 * Functions prototypes.
 ***************************************************************************
 */

/* Functions used to count number of items */
extern __nr_t
	wrap_get_cpu_nr(struct activity *);
extern __nr_t
	wrap_get_irq_nr(struct activity *);
extern __nr_t
	wrap_get_serial_nr(struct activity *);
extern __nr_t
	wrap_get_disk_nr(struct activity *);
extern __nr_t
	wrap_get_iface_nr(struct activity *);
extern __nr_t
	wrap_get_fan_nr(struct activity *);
extern __nr_t
	wrap_get_temp_nr(struct activity *);
extern __nr_t
	wrap_get_in_nr(struct activity *);
extern __nr_t
	wrap_get_freq_nr(struct activity *);
extern __nr_t
	wrap_get_usb_nr(struct activity *);
	
/* Functions used to read activities statistics */
extern __read_funct_t
	wrap_read_stat_cpu(struct activity *);
extern __read_funct_t
	wrap_read_stat_pcsw(struct activity *);
extern __read_funct_t
	wrap_read_stat_irq(struct activity *);
extern __read_funct_t
	wrap_read_swap(struct activity *);
extern __read_funct_t
	wrap_read_paging(struct activity *);
extern __read_funct_t
	wrap_read_io(struct activity *);
extern __read_funct_t
	wrap_read_meminfo(struct activity *);
extern __read_funct_t
	wrap_read_kernel_tables(struct activity *);
extern __read_funct_t
	wrap_read_loadavg(struct activity *);
extern __read_funct_t
	wrap_read_tty_driver_serial(struct activity *);
extern __read_funct_t
	wrap_read_disk(struct activity *);
extern __read_funct_t
	wrap_read_net_dev(struct activity *);
extern __read_funct_t
	wrap_read_net_edev(struct activity *);
extern __read_funct_t
	wrap_read_net_nfs(struct activity *);
extern __read_funct_t
	wrap_read_net_nfsd(struct activity *);
extern __read_funct_t
	wrap_read_net_sock(struct activity *);
extern __read_funct_t
	wrap_read_net_ip(struct activity *);
extern __read_funct_t
	wrap_read_net_eip(struct activity *);
extern __read_funct_t
	wrap_read_net_icmp(struct activity *);
extern __read_funct_t
	wrap_read_net_eicmp(struct activity *);
extern __read_funct_t
	wrap_read_net_tcp(struct activity *);
extern __read_funct_t
	wrap_read_net_etcp(struct activity *);
extern __read_funct_t
	wrap_read_net_udp(struct activity *);
extern __read_funct_t
	wrap_read_net_sock6(struct activity *);
extern __read_funct_t
	wrap_read_net_ip6(struct activity *);
extern __read_funct_t
	wrap_read_net_eip6(struct activity *);
extern __read_funct_t
	wrap_read_net_icmp6(struct activity *);
extern __read_funct_t
	wrap_read_net_eicmp6(struct activity *);
extern __read_funct_t
	wrap_read_net_udp6(struct activity *);
extern __read_funct_t
	wrap_read_cpuinfo(struct activity *);
extern __read_funct_t
	wrap_read_fan(struct activity *);
extern __read_funct_t
	wrap_read_temp(struct activity *);
extern __read_funct_t
	wrap_read_in(struct activity *);
extern __read_funct_t
	wrap_read_meminfo_huge(struct activity *);
extern __read_funct_t
	wrap_read_time_in_state(struct activity *);
extern __read_funct_t
	wrap_read_bus_usb_dev(struct activity *);

/* Other functions */
extern void
	allocate_bitmaps(struct activity * []);
extern void
	allocate_structures(struct activity * []);
extern int
	check_disk_reg(struct activity *, int, int, int);
extern void
	check_file_actlst(int *, char *, struct activity * [], struct file_magic *,
			  struct file_header *, struct file_activity **,
			  unsigned int [], int);
extern unsigned int
	check_net_dev_reg(struct activity *, int, int, unsigned int);
extern unsigned int
	check_net_edev_reg(struct activity *, int, int, unsigned int);
extern void
	copy_structures(struct activity * [], unsigned int [],
			struct record_header [], int, int);
extern int
	datecmp(struct tm *, struct tstamp *);
extern void
	display_sa_file_version(struct file_magic *);
extern void
	free_bitmaps(struct activity * []);
extern void
	free_structures(struct activity * []);
extern int
	get_activity_nr(struct activity * [], unsigned int, int);
extern int
	get_activity_position(struct activity * [], unsigned int);
extern char *
	get_devname(unsigned int, unsigned int, int);
extern void
	get_file_timestamp_struct(unsigned int, struct tm *, struct file_header *);
extern void
	get_itv_value(struct record_header *, struct record_header *,
		      unsigned int, unsigned long long *, unsigned long long *);
extern void
	handle_invalid_sa_file(int *, struct file_magic *, char *, int);
extern int
	next_slice(unsigned long long, unsigned long long, int, long);
extern int
	parse_sar_opt(char * [], int *, struct activity * [], unsigned int *, int);
extern int
	parse_sar_I_opt(char * [], int *, struct activity * []);
extern int
	parse_sa_P_opt(char * [], int *, unsigned int *, struct activity * []);
extern int
	parse_sar_m_opt(char * [], int *, struct activity * []);
extern int
	parse_sar_n_opt(char * [], int *, struct activity * []);
extern int
	parse_timestamp(char * [], int *, struct tstamp *, const char *);
extern void
	print_report_hdr(unsigned int, struct tm *, struct file_header *, int);
extern void
	read_file_stat_bunch(struct activity * [], int, int, int, struct file_activity *);
extern int
	sa_fread(int, void *, int, int);
extern void
	select_all_activities(struct activity * []);
extern void
	select_default_activity(struct activity * []);
extern void
	set_bitmap(unsigned char [], unsigned char, unsigned int);
extern void
	set_default_file(struct tm *, char *);
extern void
	set_hdr_rectime(unsigned int, struct tm *, struct file_header *);

#endif  /* _SA_H */
