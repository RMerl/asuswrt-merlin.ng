/*
 * definitions for MPC8xxx I/O Ports
 *
 * Murray.Jensen@cmst.csiro.au, 20-Oct-00
 */

/*
 * this structure mirrors the layout of the five port registers in
 * the internal memory map
 */
typedef struct {
    unsigned int pdir;		/* Port Data Direction Register (35-3) */
    unsigned int ppar;		/* Port Pin Assignment Register (35-4) */
    unsigned int psor;		/* Port Special Options Register (35-5) */
    unsigned int podr;		/* Port Open Drain Register (35-2) */
    unsigned int pdat;		/* Port Data Register (35-3) */
} ioport_t;

/*
 * this macro calculates the address within the internal
 * memory map (im) of the set of registers for a port (idx)
 *
 * the internal memory map aligns the above structure on
 * a 0x20 byte boundary
 */
#ifdef CONFIG_MPC85xx
#define ioport_addr(im, idx) (ioport_t *)((uint)&(im->im_cpm_iop) + ((idx)*0x20))
#else
#define ioport_addr(im, idx) (ioport_t *)((uint)&(im)->im_ioport + ((idx)*0x20))
#endif

/*
 * this structure provides configuration
 * information for one port pin
 */
typedef struct {
    unsigned char conf:1;	/* if 1, configure this port */
    unsigned char ppar:1;	/* Port Pin Assignment Register (35-4) */
    unsigned char psor:1;	/* Port Special Options Register (35-2) */
    unsigned char pdir:1;	/* Port Data Direction Register (35-3) */
    unsigned char podr:1;	/* Port Open Drain Register (35-2) */
    unsigned char pdat:1;	/* Port Data Register (35-2) */
} iop_conf_t;

/*
 * a table that contains configuration information for all 32 pins
 *
 * NOTE: in the second dimension of this table, index 0 refers to pin 31
 * and index 31 refers to pin 0. this made the code in the table look more
 * like the table in the 8260UM (and in the hymod manuals).
 */
extern const iop_conf_t iop_conf_tab[4][32];

typedef struct {
	unsigned char	port;
	unsigned char	pin;
	int		dir;
	int		open_drain;
	int		assign;
} qe_iop_conf_t;

#define QE_IOP_TAB_END	(-1)
