#ifndef _LINUX_FB_H
#define _LINUX_FB_H

#include <linux/types.h>
#include <linux/list.h>

/* Definitions of frame buffers						*/

#define FB_MAX			32	/* sufficient for now */

#define FB_TYPE_PACKED_PIXELS		0	/* Packed Pixels	*/

#define FB_VISUAL_MONO01		0	/* Monochr. 1=Black 0=White */
#define FB_VISUAL_MONO10		1	/* Monochr. 1=White 0=Black */
#define FB_VISUAL_TRUECOLOR		2	/* True color	*/
#define FB_VISUAL_PSEUDOCOLOR		3	/* Pseudo color (like atari) */
#define FB_VISUAL_DIRECTCOLOR		4	/* Direct color */
#define FB_VISUAL_STATIC_PSEUDOCOLOR	5	/* Pseudo color readonly */

#define FB_ACCEL_NONE		0	/* no hardware accelerator	*/

struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	__u32 smem_len;			/* Length of frame buffer mem */
	__u32 type;			/* see FB_TYPE_*		*/
	__u32 type_aux;			/* Interleave for interleaved Planes */
	__u32 visual;			/* see FB_VISUAL_*		*/
	__u16 xpanstep;			/* zero if no hardware panning	*/
	__u16 ypanstep;			/* zero if no hardware panning	*/
	__u16 ywrapstep;		/* zero if no hardware ywrap	*/
	__u32 line_length;		/* length of a line in bytes	*/
	unsigned long mmio_start;	/* Start of Memory Mapped I/O	*/
					/* (physical address) */
	__u32 mmio_len;			/* Length of Memory Mapped I/O	*/
	__u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	__u16 reserved[3];		/* Reserved for future compatibility */
};

/*
 * Interpretation of offset for color fields: All offsets are from the right,
 * inside a "pixel" value, which is exactly 'bits_per_pixel' wide (means: you
 * can use the offset as right argument to <<). A pixel afterwards is a bit
 * stream and is written to video memory as that unmodified.
 *
 * For pseudocolor: offset and length should be the same for all color
 * components. Offset specifies the position of the least significant bit
 * of the pallette index in a pixel value. Length indicates the number
 * of available palette entries (i.e. # of entries = 1 << length).
 */
struct fb_bitfield {
	__u32 offset;			/* beginning of bitfield	*/
	__u32 length;			/* length of bitfield		*/
	__u32 msb_right;

};

#define FB_NONSTD_HAM		1	/* Hold-And-Modify (HAM)	*/
#define FB_NONSTD_REV_PIX_IN_B	2	/* order of pixels in each byte is reversed */

#define FB_ACTIVATE_NOW		0	/* set values immediately (or vbl)*/
#define FB_ACTIVATE_NXTOPEN	1	/* activate on next open	*/
#define FB_ACTIVATE_TEST	2	/* don't set, round up impossible */
#define FB_ACTIVATE_MASK       15
					/* values			*/
#define FB_ACTIVATE_VBL	       16	/* activate values on next vbl	*/
#define FB_CHANGE_CMAP_VBL     32	/* change colormap on vbl	*/
#define FB_ACTIVATE_ALL	       64	/* change all VCs on this fb	*/
#define FB_ACTIVATE_FORCE     128	/* force apply even when no change*/
#define FB_ACTIVATE_INV_MODE  256	/* invalidate videomode */

#define FB_SYNC_HOR_HIGH_ACT	1	/* horizontal sync high active	*/
#define FB_SYNC_VERT_HIGH_ACT	2	/* vertical sync high active	*/
#define FB_SYNC_EXT		4	/* external sync		*/
#define FB_SYNC_COMP_HIGH_ACT	8	/* composite sync high active	*/
#define FB_SYNC_BROADCAST	16	/* broadcast video timings	*/
					/* vtotal = 144d/288n/576i => PAL  */
					/* vtotal = 121d/242n/484i => NTSC */
#define FB_SYNC_ON_GREEN	32	/* sync on green */

#define FB_VMODE_NONINTERLACED	0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced	*/
#define FB_VMODE_DOUBLE		2	/* double scan */
#define FB_VMODE_ODD_FLD_FIRST	4	/* interlaced: top line first */
#define FB_VMODE_MASK		255

#define FB_VMODE_YWRAP		256	/* ywrap instead of panning	*/
#define FB_VMODE_SMOOTH_XPAN	512	/* smooth xpan possible (internally used) */
#define FB_VMODE_CONUPDATE	512	/* don't update x/yoffset	*/

/*
 * Display rotation support
 */
#define FB_ROTATE_UR	  0
#define FB_ROTATE_CW	  1
#define FB_ROTATE_UD	  2
#define FB_ROTATE_CCW	  3

#define PICOS2KHZ(a) (1000000000UL/(a))
#define KHZ2PICOS(a) (1000000000UL/(a))

struct fb_var_screeninfo {
	__u32 xres;			/* visible resolution		*/
	__u32 yres;
	__u32 xres_virtual;		/* virtual resolution		*/
	__u32 yres_virtual;
	__u32 xoffset;			/* offset from virtual to visible */
	__u32 yoffset;			/* resolution			*/

	__u32 bits_per_pixel;		/* guess what			*/
	__u32 grayscale;		/* != 0 Graylevels instead of colors */

	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/

	__u32 nonstd;			/* != 0 Non standard pixel format */

	__u32 activate;			/* see FB_ACTIVATE_*		*/

	__u32 height;			/* height of picture in mm    */
	__u32 width;			/* width of picture in mm     */

	__u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	__u32 pixclock;			/* pixel clock in ps (pico seconds) */
	__u32 left_margin;		/* time from sync to picture	*/
	__u32 right_margin;		/* time from picture to sync	*/
	__u32 upper_margin;		/* time from sync to picture	*/
	__u32 lower_margin;
	__u32 hsync_len;		/* length of horizontal sync	*/
	__u32 vsync_len;		/* length of vertical sync	*/
	__u32 sync;			/* see FB_SYNC_*		*/
	__u32 vmode;			/* see FB_VMODE_*		*/
	__u32 rotate;			/* angle we rotate counter clockwise */
	__u32 reserved[5];		/* Reserved for future compatibility */
};

struct fb_cmap {
	__u32 start;			/* First entry	*/
	__u32 len;			/* Number of entries */
	__u16 *red;			/* Red values	*/
	__u16 *green;
	__u16 *blue;
	__u16 *transp;			/* transparency, can be NULL */
};

struct fb_con2fbmap {
	__u32 console;
	__u32 framebuffer;
};

/* VESA Blanking Levels */
#define VESA_NO_BLANKING	0
#define VESA_VSYNC_SUSPEND	1
#define VESA_HSYNC_SUSPEND	2
#define VESA_POWERDOWN		3


enum {
	/* screen: unblanked, hsync: on,  vsync: on */
	FB_BLANK_UNBLANK       = VESA_NO_BLANKING,

	/* screen: blanked,   hsync: on,  vsync: on */
	FB_BLANK_NORMAL        = VESA_NO_BLANKING + 1,

	/* screen: blanked,   hsync: on,  vsync: off */
	FB_BLANK_VSYNC_SUSPEND = VESA_VSYNC_SUSPEND + 1,

	/* screen: blanked,   hsync: off, vsync: on */
	FB_BLANK_HSYNC_SUSPEND = VESA_HSYNC_SUSPEND + 1,

	/* screen: blanked,   hsync: off, vsync: off */
	FB_BLANK_POWERDOWN     = VESA_POWERDOWN + 1
};

#define FB_VBLANK_VBLANKING	0x001	/* currently in a vertical blank */
#define FB_VBLANK_HBLANKING	0x002	/* currently in a horizontal blank */
#define FB_VBLANK_HAVE_VBLANK	0x004	/* vertical blanks can be detected */
#define FB_VBLANK_HAVE_HBLANK	0x008	/* horizontal blanks can be detected */
#define FB_VBLANK_HAVE_COUNT	0x010	/* global retrace counter is available */
#define FB_VBLANK_HAVE_VCOUNT	0x020	/* the vcount field is valid */
#define FB_VBLANK_HAVE_HCOUNT	0x040	/* the hcount field is valid */
#define FB_VBLANK_VSYNCING	0x080	/* currently in a vsync */
#define FB_VBLANK_HAVE_VSYNC	0x100	/* verical syncs can be detected */

struct fb_vblank {
	__u32 flags;			/* FB_VBLANK flags */
	__u32 count;			/* counter of retraces since boot */
	__u32 vcount;			/* current scanline position */
	__u32 hcount;			/* current scandot position */
	__u32 reserved[4];		/* reserved for future compatibility */
};

/* Internal HW accel */
#define ROP_COPY 0
#define ROP_XOR  1

struct fb_copyarea {
	__u32 dx;
	__u32 dy;
	__u32 width;
	__u32 height;
	__u32 sx;
	__u32 sy;
};

struct fb_fillrect {
	__u32 dx;	/* screen-relative */
	__u32 dy;
	__u32 width;
	__u32 height;
	__u32 color;
	__u32 rop;
};

struct fb_image {
	__u32 dx;		/* Where to place image */
	__u32 dy;
	__u32 width;		/* Size of image */
	__u32 height;
	__u32 fg_color;		/* Only used when a mono bitmap */
	__u32 bg_color;
	__u8  depth;		/* Depth of the image */
	const char *data;	/* Pointer to image data */
	struct fb_cmap cmap;	/* color map info */
};

/*
 * hardware cursor control
 */

#define FB_CUR_SETIMAGE 0x01
#define FB_CUR_SETPOS	0x02
#define FB_CUR_SETHOT	0x04
#define FB_CUR_SETCMAP	0x08
#define FB_CUR_SETSHAPE 0x10
#define FB_CUR_SETSIZE	0x20
#define FB_CUR_SETALL	0xFF

struct fbcurpos {
	__u16 x, y;
};

struct fb_cursor {
	__u16 set;		/* what to set */
	__u16 enable;		/* cursor on/off */
	__u16 rop;		/* bitop operation */
	const char *mask;	/* cursor mask bits */
	struct fbcurpos hot;	/* cursor hot spot */
	struct fb_image	image;	/* Cursor image */
};

#ifdef CONFIG_FB_BACKLIGHT
/* Settings for the generic backlight code */
#define FB_BACKLIGHT_LEVELS	128
#define FB_BACKLIGHT_MAX	0xFF
#endif

#ifdef __KERNEL__

struct vm_area_struct;
struct fb_info;
struct device;
struct file;

/* Definitions below are used in the parsed monitor specs */
#define FB_DPMS_ACTIVE_OFF	1
#define FB_DPMS_SUSPEND		2
#define FB_DPMS_STANDBY		4

#define FB_DISP_DDI		1
#define FB_DISP_ANA_700_300	2
#define FB_DISP_ANA_714_286	4
#define FB_DISP_ANA_1000_400	8
#define FB_DISP_ANA_700_000	16

#define FB_DISP_MONO		32
#define FB_DISP_RGB		64
#define FB_DISP_MULTI		128
#define FB_DISP_UNKNOWN		256

#define FB_SIGNAL_NONE		0
#define FB_SIGNAL_BLANK_BLANK	1
#define FB_SIGNAL_SEPARATE	2
#define FB_SIGNAL_COMPOSITE	4
#define FB_SIGNAL_SYNC_ON_GREEN	8
#define FB_SIGNAL_SERRATION_ON	16

#define FB_MISC_PRIM_COLOR	1
#define FB_MISC_1ST_DETAIL	2	/* First Detailed Timing is preferred */
struct fb_chroma {
	__u32 redx;	/* in fraction of 1024 */
	__u32 greenx;
	__u32 bluex;
	__u32 whitex;
	__u32 redy;
	__u32 greeny;
	__u32 bluey;
	__u32 whitey;
};

struct fb_monspecs {
	struct fb_chroma chroma;
	struct fb_videomode *modedb;	/* mode database */
	__u8  manufacturer[4];		/* Manufacturer */
	__u8  monitor[14];		/* Monitor String */
	__u8  serial_no[14];		/* Serial Number */
	__u8  ascii[14];		/* ? */
	__u32 modedb_len;		/* mode database length */
	__u32 model;			/* Monitor Model */
	__u32 serial;			/* Serial Number - Integer */
	__u32 year;			/* Year manufactured */
	__u32 week;			/* Week Manufactured */
	__u32 hfmin;			/* hfreq lower limit (Hz) */
	__u32 hfmax;			/* hfreq upper limit (Hz) */
	__u32 dclkmin;			/* pixelclock lower limit (Hz) */
	__u32 dclkmax;			/* pixelclock upper limit (Hz) */
	__u16 input;			/* display type - see FB_DISP_* */
	__u16 dpms;			/* DPMS support - see FB_DPMS_ */
	__u16 signal;			/* Signal Type - see FB_SIGNAL_* */
	__u16 vfmin;			/* vfreq lower limit (Hz) */
	__u16 vfmax;			/* vfreq upper limit (Hz) */
	__u16 gamma;			/* Gamma - in fractions of 100 */
	__u16 gtf	: 1;		/* supports GTF */
	__u16 misc;			/* Misc flags - see FB_MISC_* */
	__u8  version;			/* EDID version... */
	__u8  revision;			/* ...and revision */
	__u8  max_x;			/* Maximum horizontal size (cm) */
	__u8  max_y;			/* Maximum vertical size (cm) */
};

struct fb_cmap_user {
	__u32 start;			/* First entry	*/
	__u32 len;			/* Number of entries */
	__u16 *red;		/* Red values	*/
	__u16 *green;
	__u16 *blue;
	__u16 *transp;		/* transparency, can be NULL */
};

struct fb_image_user {
	__u32 dx;			/* Where to place image */
	__u32 dy;
	__u32 width;			/* Size of image */
	__u32 height;
	__u32 fg_color;			/* Only used when a mono bitmap */
	__u32 bg_color;
	__u8  depth;			/* Depth of the image */
	const char *data;	/* Pointer to image data */
	struct fb_cmap_user cmap;	/* color map info */
};

struct fb_cursor_user {
	__u16 set;			/* what to set */
	__u16 enable;			/* cursor on/off */
	__u16 rop;			/* bitop operation */
	const char *mask;	/* cursor mask bits */
	struct fbcurpos hot;		/* cursor hot spot */
	struct fb_image_user image;	/* Cursor image */
};

/*
 * Register/unregister for framebuffer events
 */

/*	The resolution of the passed in fb_info about to change */
#define FB_EVENT_MODE_CHANGE		0x01
/*	The display on this fb_info is beeing suspended, no access to the
 *	framebuffer is allowed any more after that call returns
 */
#define FB_EVENT_SUSPEND		0x02
/*	The display on this fb_info was resumed, you can restore the display
 *	if you own it
 */
#define FB_EVENT_RESUME			0x03
/*	An entry from the modelist was removed */
#define FB_EVENT_MODE_DELETE		0x04
/*	A driver registered itself */
#define FB_EVENT_FB_REGISTERED		0x05
/*	A driver unregistered itself */
#define FB_EVENT_FB_UNREGISTERED	0x06
/*	CONSOLE-SPECIFIC: get console to framebuffer mapping */
#define FB_EVENT_GET_CONSOLE_MAP	0x07
/*	CONSOLE-SPECIFIC: set console to framebuffer mapping */
#define FB_EVENT_SET_CONSOLE_MAP	0x08
/*	A hardware display blank change occurred */
#define FB_EVENT_BLANK			0x09
/*	Private modelist is to be replaced */
#define FB_EVENT_NEW_MODELIST		0x0A
/*	The resolution of the passed in fb_info about to change and
	all vc's should be changed	   */
#define FB_EVENT_MODE_CHANGE_ALL	0x0B
/*	A software display blank change occurred */
#define FB_EVENT_CONBLANK		0x0C
/*	Get drawing requirements	*/
#define FB_EVENT_GET_REQ		0x0D
/*	Unbind from the console if possible */
#define FB_EVENT_FB_UNBIND		0x0E

struct fb_event {
	struct fb_info *info;
	void *data;
};

struct fb_blit_caps {
	u32 x;
	u32 y;
	u32 len;
	u32 flags;
};

/*
 * Pixmap structure definition
 *
 * The purpose of this structure is to translate data
 * from the hardware independent format of fbdev to what
 * format the hardware needs.
 */

#define FB_PIXMAP_DEFAULT 1	/* used internally by fbcon */
#define FB_PIXMAP_SYSTEM  2	/* memory is in system RAM  */
#define FB_PIXMAP_IO	  4	/* memory is iomapped	    */
#define FB_PIXMAP_SYNC	  256	/* set if GPU can DMA	    */

struct fb_pixmap {
	u8  *addr;		/* pointer to memory			*/
	u32 size;		/* size of buffer in bytes		*/
	u32 offset;		/* current offset to buffer		*/
	u32 buf_align;		/* byte alignment of each bitmap	*/
	u32 scan_align;		/* alignment per scanline		*/
	u32 access_align;	/* alignment per read/write (bits)	*/
	u32 flags;		/* see FB_PIXMAP_*			*/
	u32 blit_x;		/* supported bit block dimensions (1-32)*/
	u32 blit_y;		/* Format: blit_x = 1 << (width - 1)	*/
				/*	   blit_y = 1 << (height - 1)	*/
				/* if 0, will be set to 0xffffffff (all)*/
	/* access methods */
	void (*writeio)(struct fb_info *info, void *dst, void *src, unsigned int size);
	void (*readio) (struct fb_info *info, void *dst, void *src, unsigned int size);
};

#ifdef CONFIG_FB_DEFERRED_IO
struct fb_deferred_io {
	/* delay between mkwrite and deferred handler */
	unsigned long delay;
	struct mutex lock; /* mutex that protects the page list */
	struct list_head pagelist; /* list of touched pages */
	/* callback */
	void (*deferred_io)(struct fb_info *info, struct list_head *pagelist);
};
#endif

/* FBINFO_* = fb_info.flags bit flags */
#define FBINFO_MODULE		0x0001	/* Low-level driver is a module */
#define FBINFO_HWACCEL_DISABLED	0x0002
	/* When FBINFO_HWACCEL_DISABLED is set:
	 *  Hardware acceleration is turned off.  Software implementations
	 *  of required functions (copyarea(), fillrect(), and imageblit())
	 *  takes over; acceleration engine should be in a quiescent state */

/* hints */
#define FBINFO_PARTIAL_PAN_OK	0x0040 /* otw use pan only for double-buffering */
#define FBINFO_READS_FAST	0x0080 /* soft-copy faster than rendering */

/*
 * A driver may set this flag to indicate that it does want a set_par to be
 * called every time when fbcon_switch is executed. The advantage is that with
 * this flag set you can really be sure that set_par is always called before
 * any of the functions dependant on the correct hardware state or altering
 * that state, even if you are using some broken X releases. The disadvantage
 * is that it introduces unwanted delays to every console switch if set_par
 * is slow. It is a good idea to try this flag in the drivers initialization
 * code whenever there is a bug report related to switching between X and the
 * framebuffer console.
 */
#define FBINFO_MISC_ALWAYS_SETPAR   0x40000

/*
 * Host and GPU endianness differ.
 */
#define FBINFO_FOREIGN_ENDIAN	0x100000
/*
 * Big endian math. This is the same flags as above, but with different
 * meaning, it is set by the fb subsystem depending FOREIGN_ENDIAN flag
 * and host endianness. Drivers should not use this flag.
 */
#define FBINFO_BE_MATH	0x100000

struct fb_info {
	int node;
	int flags;
	struct fb_var_screeninfo var;	/* Current var */
	struct fb_fix_screeninfo fix;	/* Current fix */
	struct fb_monspecs monspecs;	/* Current Monitor specs */
	struct fb_pixmap pixmap;	/* Image hardware mapper */
	struct fb_pixmap sprite;	/* Cursor hardware mapper */
	struct fb_cmap cmap;		/* Current cmap */
	struct list_head modelist;	/* mode list */
	struct fb_videomode *mode;	/* current mode */

	char *screen_base;	/* Virtual address */
	unsigned long screen_size;	/* Amount of ioremapped VRAM or 0 */
	void *pseudo_palette;		/* Fake palette of 16 colors */
#define FBINFO_STATE_RUNNING	0
#define FBINFO_STATE_SUSPENDED	1
	u32 state;			/* Hardware state i.e suspend */
	void *fbcon_par;		/* fbcon use-only private area */
	/* From here on everything is device dependent */
	void *par;
};

#define FBINFO_DEFAULT	0

#define FBINFO_FLAG_MODULE	FBINFO_MODULE
#define FBINFO_FLAG_DEFAULT	FBINFO_DEFAULT

/* This will go away */
#if defined(__sparc__)

/* We map all of our framebuffers such that big-endian accesses
 * are what we want, so the following is sufficient.
 */

/* This will go away */
#define fb_readb sbus_readb
#define fb_readw sbus_readw
#define fb_readl sbus_readl
#define fb_readq sbus_readq
#define fb_writeb sbus_writeb
#define fb_writew sbus_writew
#define fb_writel sbus_writel
#define fb_writeq sbus_writeq
#define fb_memset sbus_memset_io

#elif defined(__i386__) || defined(__alpha__) || defined(__x86_64__) || defined(__hppa__) || defined(__sh__) || defined(__powerpc__) || defined(__bfin__)

#define fb_readb __raw_readb
#define fb_readw __raw_readw
#define fb_readl __raw_readl
#define fb_readq __raw_readq
#define fb_writeb __raw_writeb
#define fb_writew __raw_writew
#define fb_writel __raw_writel
#define fb_writeq __raw_writeq
#define fb_memset memset_io

#else

#define fb_readb(addr) (*(volatile u8 *) (addr))
#define fb_readw(addr) (*(volatile u16 *) (addr))
#define fb_readl(addr) (*(volatile u32 *) (addr))
#define fb_readq(addr) (*(volatile u64 *) (addr))
#define fb_writeb(b,addr) (*(volatile u8 *) (addr) = (b))
#define fb_writew(b,addr) (*(volatile u16 *) (addr) = (b))
#define fb_writel(b,addr) (*(volatile u32 *) (addr) = (b))
#define fb_writeq(b,addr) (*(volatile u64 *) (addr) = (b))
#define fb_memset memset

#endif

#define FB_LEFT_POS(p, bpp)	     (fb_be_math(p) ? (32 - (bpp)) : 0)
#define FB_SHIFT_HIGH(p, val, bits)  (fb_be_math(p) ? (val) >> (bits) : \
						      (val) << (bits))
#define FB_SHIFT_LOW(p, val, bits)   (fb_be_math(p) ? (val) << (bits) : \
						      (val) >> (bits))
/* drivers/video/fbmon.c */
#define FB_MAXTIMINGS		0
#define FB_VSYNCTIMINGS		1
#define FB_HSYNCTIMINGS		2
#define FB_DCLKTIMINGS		3
#define FB_IGNOREMON		0x100

#define FB_MODE_IS_UNKNOWN	0
#define FB_MODE_IS_DETAILED	1
#define FB_MODE_IS_STANDARD	2
#define FB_MODE_IS_VESA		4
#define FB_MODE_IS_CALCULATED	8
#define FB_MODE_IS_FIRST	16
#define FB_MODE_IS_FROM_VAR	32


/* drivers/video/fbcmap.c */

extern int fb_alloc_cmap(struct fb_cmap *cmap, int len, int transp);
extern void fb_dealloc_cmap(struct fb_cmap *cmap);
extern int fb_copy_cmap(const struct fb_cmap *from, struct fb_cmap *to);
extern int fb_cmap_to_user(const struct fb_cmap *from, struct fb_cmap_user *to);
extern int fb_set_cmap(struct fb_cmap *cmap, struct fb_info *fb_info);
extern int fb_set_user_cmap(struct fb_cmap_user *cmap, struct fb_info *fb_info);
extern const struct fb_cmap *fb_default_cmap(int len);
extern void fb_invert_cmaps(void);

struct fb_videomode {
	const char *name;	/* optional */
	u32 refresh;		/* optional */
	u32 xres;
	u32 yres;
	u32 pixclock;
	u32 left_margin;
	u32 right_margin;
	u32 upper_margin;
	u32 lower_margin;
	u32 hsync_len;
	u32 vsync_len;
	u32 sync;
	u32 vmode;
	u32 flag;
};

int board_video_skip(void);

#endif /* __KERNEL__ */

#endif /* _LINUX_FB_H */
