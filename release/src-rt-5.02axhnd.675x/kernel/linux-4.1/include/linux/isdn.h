/* $Id: isdn.h,v 1.125.2.3 2004/02/10 01:07:14 keil Exp $
 *
 * Main header for the Linux ISDN subsystem (linklevel).
 *
 * Copyright 1994,95,96 by Fritz Elfert (fritz@isdn4linux.de)
 * Copyright 1995,96    by Thinking Objects Software GmbH Wuerzburg
 * Copyright 1995,96    by Michael Hipp (Michael.Hipp@student.uni-tuebingen.de)
 * 
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */
#ifndef __ISDN_H__
#define __ISDN_H__


#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_reg.h>
#include <linux/fcntl.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/tcp.h>
#include <linux/mutex.h>
#include <uapi/linux/isdn.h>

#define ISDN_TTY_MAJOR    43
#define ISDN_TTYAUX_MAJOR 44
#define ISDN_MAJOR        45

/* The minor-devicenumbers for Channel 0 and 1 are used as arguments for
 * physical Channel-Mapping, so they MUST NOT be changed without changing
 * the correspondent code in isdn.c
 */

#define ISDN_MINOR_B        0
#define ISDN_MINOR_BMAX     (ISDN_MAX_CHANNELS-1)
#define ISDN_MINOR_CTRL     64
#define ISDN_MINOR_CTRLMAX  (64 + (ISDN_MAX_CHANNELS-1))
#define ISDN_MINOR_PPP      128
#define ISDN_MINOR_PPPMAX   (128 + (ISDN_MAX_CHANNELS-1))
#define ISDN_MINOR_STATUS   255

#ifdef CONFIG_ISDN_PPP

#ifdef CONFIG_ISDN_PPP_VJ
#  include <net/slhc_vj.h>
#endif

#include <linux/ppp_defs.h>
#include <linux/ppp-ioctl.h>

#include <linux/isdn_ppp.h>
#endif

#ifdef CONFIG_ISDN_X25
#  include <linux/concap.h>
#endif

#include <linux/isdnif.h>

#define ISDN_DRVIOCTL_MASK       0x7f  /* Mask for Device-ioctl */

/* Until now unused */
#define ISDN_SERVICE_VOICE 1
#define ISDN_SERVICE_AB    1<<1 
#define ISDN_SERVICE_X21   1<<2
#define ISDN_SERVICE_G4    1<<3
#define ISDN_SERVICE_BTX   1<<4
#define ISDN_SERVICE_DFUE  1<<5
#define ISDN_SERVICE_X25   1<<6
#define ISDN_SERVICE_TTX   1<<7
#define ISDN_SERVICE_MIXED 1<<8
#define ISDN_SERVICE_FW    1<<9
#define ISDN_SERVICE_GTEL  1<<10
#define ISDN_SERVICE_BTXN  1<<11
#define ISDN_SERVICE_BTEL  1<<12

/* Macros checking plain usage */
#define USG_NONE(x)         ((x & ISDN_USAGE_MASK)==ISDN_USAGE_NONE)
#define USG_RAW(x)          ((x & ISDN_USAGE_MASK)==ISDN_USAGE_RAW)
#define USG_MODEM(x)        ((x & ISDN_USAGE_MASK)==ISDN_USAGE_MODEM)
#define USG_VOICE(x)        ((x & ISDN_USAGE_MASK)==ISDN_USAGE_VOICE)
#define USG_NET(x)          ((x & ISDN_USAGE_MASK)==ISDN_USAGE_NET)
#define USG_FAX(x)          ((x & ISDN_USAGE_MASK)==ISDN_USAGE_FAX)
#define USG_OUTGOING(x)     ((x & ISDN_USAGE_OUTGOING)==ISDN_USAGE_OUTGOING)
#define USG_MODEMORVOICE(x) (((x & ISDN_USAGE_MASK)==ISDN_USAGE_MODEM) || \
                             ((x & ISDN_USAGE_MASK)==ISDN_USAGE_VOICE)     )

/* Timer-delays and scheduling-flags */
#define ISDN_TIMER_RES         4                         /* Main Timer-Resolution   */
#define ISDN_TIMER_02SEC       (HZ/ISDN_TIMER_RES/5)     /* Slow-Timer1 .2 sec      */
#define ISDN_TIMER_1SEC        (HZ/ISDN_TIMER_RES)       /* Slow-Timer2 1 sec       */
#define ISDN_TIMER_RINGING     5 /* tty RINGs = ISDN_TIMER_1SEC * this factor       */
#define ISDN_TIMER_KEEPINT    10 /* Cisco-Keepalive = ISDN_TIMER_1SEC * this factor */
#define ISDN_TIMER_MODEMREAD   1
#define ISDN_TIMER_MODEMPLUS   2
#define ISDN_TIMER_MODEMRING   4
#define ISDN_TIMER_MODEMXMIT   8
#define ISDN_TIMER_NETDIAL    16 
#define ISDN_TIMER_NETHANGUP  32
#define ISDN_TIMER_CARRIER   256 /* Wait for Carrier */
#define ISDN_TIMER_FAST      (ISDN_TIMER_MODEMREAD | ISDN_TIMER_MODEMPLUS | \
                              ISDN_TIMER_MODEMXMIT)
#define ISDN_TIMER_SLOW      (ISDN_TIMER_MODEMRING | ISDN_TIMER_NETHANGUP | \
                              ISDN_TIMER_NETDIAL | ISDN_TIMER_CARRIER)

/* Timeout-Values for isdn_net_dial() */
#define ISDN_TIMER_DTIMEOUT10 (10*HZ/(ISDN_TIMER_02SEC*(ISDN_TIMER_RES+1)))
#define ISDN_TIMER_DTIMEOUT15 (15*HZ/(ISDN_TIMER_02SEC*(ISDN_TIMER_RES+1)))
#define ISDN_TIMER_DTIMEOUT60 (60*HZ/(ISDN_TIMER_02SEC*(ISDN_TIMER_RES+1)))

/* GLOBAL_FLAGS */
#define ISDN_GLOBAL_STOPPED 1

/*=================== Start of ip-over-ISDN stuff =========================*/

/* Feature- and status-flags for a net-interface */
#define ISDN_NET_CONNECTED  0x01       /* Bound to ISDN-Channel             */
#define ISDN_NET_SECURE     0x02       /* Accept calls from phonelist only  */
#define ISDN_NET_CALLBACK   0x04       /* activate callback                 */
#define ISDN_NET_CBHUP      0x08       /* hangup before callback            */
#define ISDN_NET_CBOUT      0x10       /* remote machine does callback      */

#define ISDN_NET_MAGIC      0x49344C02 /* for paranoia-checking             */

/* Phone-list-element */
typedef struct {
  void *next;
  char num[ISDN_MSNLEN];
} isdn_net_phone;

/*
   Principles when extending structures for generic encapsulation protocol
   ("concap") support:
   - Stuff which is hardware specific (here i4l-specific) goes in 
     the netdev -> local structure (here: isdn_net_local)
   - Stuff which is encapsulation protocol specific goes in the structure
     which holds the linux device structure (here: isdn_net_device)
*/

/* Local interface-data */
typedef struct isdn_net_local_s {
  ulong                  magic;
  struct net_device_stats stats;       /* Ethernet Statistics              */
  int                    isdn_device;  /* Index to isdn-device             */
  int                    isdn_channel; /* Index to isdn-channel            */
  int			 ppp_slot;     /* PPPD device slot number          */
  int                    pre_device;   /* Preselected isdn-device          */
  int                    pre_channel;  /* Preselected isdn-channel         */
  int                    exclusive;    /* If non-zero idx to reserved chan.*/
  int                    flags;        /* Connection-flags                 */
  int                    dialretry;    /* Counter for Dialout-retries      */
  int                    dialmax;      /* Max. Number of Dial-retries      */
  int                    cbdelay;      /* Delay before Callback starts     */
  int                    dtimer;       /* Timeout-counter for dialing      */
  char                   msn[ISDN_MSNLEN]; /* MSNs/EAZs for this interface */
  u_char                 cbhup;        /* Flag: Reject Call before Callback*/
  u_char                 dialstate;    /* State for dialing                */
  u_char                 p_encap;      /* Packet encapsulation             */
                                       /*   0 = Ethernet over ISDN         */
				       /*   1 = RAW-IP                     */
                                       /*   2 = IP with type field         */
  u_char                 l2_proto;     /* Layer-2-protocol                 */
				       /* See ISDN_PROTO_L2..-constants in */
                                       /* isdnif.h                         */
                                       /*   0 = X75/LAPB with I-Frames     */
				       /*   1 = X75/LAPB with UI-Frames    */
				       /*   2 = X75/LAPB with BUI-Frames   */
				       /*   3 = HDLC                       */
  u_char                 l3_proto;     /* Layer-3-protocol                 */
				       /* See ISDN_PROTO_L3..-constants in */
                                       /* isdnif.h                         */
                                       /*   0 = Transparent                */
  int                    huptimer;     /* Timeout-counter for auto-hangup  */
  int                    charge;       /* Counter for charging units       */
  ulong                  chargetime;   /* Timer for Charging info          */
  int                    hupflags;     /* Flags for charge-unit-hangup:    */
				       /* bit0: chargeint is invalid       */
				       /* bit1: Getting charge-interval    */
                                       /* bit2: Do charge-unit-hangup      */
                                       /* bit3: Do hangup even on incoming */
  int                    outgoing;     /* Flag: outgoing call              */
  int                    onhtime;      /* Time to keep link up             */
  int                    chargeint;    /* Interval between charge-infos    */
  int                    onum;         /* Flag: at least 1 outgoing number */
  int                    cps;          /* current speed of this interface  */
  int                    transcount;   /* byte-counter for cps-calculation */
  int                    sqfull;       /* Flag: netdev-queue overloaded    */
  ulong                  sqfull_stamp; /* Start-Time of overload           */
  ulong                  slavedelay;   /* Dynamic bundling delaytime       */
  int                    triggercps;   /* BogoCPS needed for trigger slave */
  isdn_net_phone         *phone[2];    /* List of remote-phonenumbers      */
				       /* phone[0] = Incoming Numbers      */
				       /* phone[1] = Outgoing Numbers      */
  isdn_net_phone         *dial;        /* Pointer to dialed number         */
  struct net_device      *master;      /* Ptr to Master device for slaves  */
  struct net_device      *slave;       /* Ptr to Slave device for masters  */
  struct isdn_net_local_s *next;       /* Ptr to next link in bundle       */
  struct isdn_net_local_s *last;       /* Ptr to last link in bundle       */
  struct isdn_net_dev_s  *netdev;      /* Ptr to netdev                    */
  struct sk_buff_head    super_tx_queue; /* List of supervisory frames to  */
	                               /* be transmitted asap              */
  atomic_t frame_cnt;                  /* number of frames currently       */
                        	       /* queued in HL driver              */    
                                       /* Ptr to orig. hard_header_cache   */
  spinlock_t             xmit_lock;    /* used to protect the xmit path of */
                                       /* a particular channel (including  */
                                       /* the frame_cnt                    */

  int  pppbind;                        /* ippp device for bindings         */
  int					dialtimeout;	/* How long shall we try on dialing? (jiffies) */
  int					dialwait;		/* How long shall we wait after failed attempt? (jiffies) */
  ulong					dialstarted;	/* jiffies of first dialing-attempt */
  ulong					dialwait_timer;	/* jiffies of earliest next dialing-attempt */
  int					huptimeout;		/* How long will the connection be up? (seconds) */
#ifdef CONFIG_ISDN_X25
  struct concap_device_ops *dops;      /* callbacks used by encapsulator   */
#endif
  /* use an own struct for that in later versions */
  ulong cisco_myseq;                   /* Local keepalive seq. for Cisco   */
  ulong cisco_mineseen;                /* returned keepalive seq. from remote */
  ulong cisco_yourseq;                 /* Remote keepalive seq. for Cisco  */
  int cisco_keepalive_period;		/* keepalive period */
  ulong cisco_last_slarp_in;		/* jiffie of last keepalive packet we received */
  char cisco_line_state;		/* state of line according to keepalive packets */
  char cisco_debserint;			/* debugging flag of cisco hdlc with slarp */
  struct timer_list cisco_timer;
  struct work_struct tqueue;
} isdn_net_local;

/* the interface itself */
typedef struct isdn_net_dev_s {
  isdn_net_local *local;
  isdn_net_local *queue;               /* circular list of all bundled
					  channels, which are currently
					  online                           */
  spinlock_t queue_lock;               /* lock to protect queue            */
  void *next;                          /* Pointer to next isdn-interface   */
  struct net_device *dev;              /* interface to upper levels        */
#ifdef CONFIG_ISDN_PPP
  ippp_bundle * pb;		/* pointer to the common bundle structure
   			         * with the per-bundle data */
#endif
#ifdef CONFIG_ISDN_X25
  struct concap_proto  *cprot; /* connection oriented encapsulation protocol */
#endif

} isdn_net_dev;

/*===================== End of ip-over-ISDN stuff ===========================*/

/*======================= Start of ISDN-tty stuff ===========================*/

#define ISDN_ASYNC_MAGIC          0x49344C01 /* for paranoia-checking        */
#define ISDN_SERIAL_XMIT_SIZE           1024 /* Default bufsize for write    */
#define ISDN_SERIAL_XMIT_MAX            4000 /* Maximum bufsize for write    */

#ifdef CONFIG_ISDN_AUDIO
/* For using sk_buffs with audio we need some private variables
 * within each sk_buff. For this purpose, we declare a struct here,
 * and put it always at the private skb->cb data array. A few macros help
 * accessing the variables.
 */
typedef struct _isdn_audio_data {
  unsigned short dle_count;
  unsigned char  lock;
} isdn_audio_data_t;

#define ISDN_AUDIO_SKB_DLECOUNT(skb)	(((isdn_audio_data_t *)&skb->cb[0])->dle_count)
#define ISDN_AUDIO_SKB_LOCK(skb)	(((isdn_audio_data_t *)&skb->cb[0])->lock)
#endif

/* Private data of AT-command-interpreter */
typedef struct atemu {
	u_char       profile[ISDN_MODEM_NUMREG]; /* Modem-Regs. Profile 0              */
	u_char       mdmreg[ISDN_MODEM_NUMREG];  /* Modem-Registers                    */
	char         pmsn[ISDN_MSNLEN];          /* EAZ/MSNs Profile 0                 */
	char         msn[ISDN_MSNLEN];           /* EAZ/MSN                            */
	char         plmsn[ISDN_LMSNLEN];        /* Listening MSNs Profile 0           */
	char         lmsn[ISDN_LMSNLEN];         /* Listening MSNs                     */
	char         cpn[ISDN_MSNLEN];           /* CalledPartyNumber on incoming call */
	char         connmsg[ISDN_CMSGLEN];	 /* CONNECT-Msg from HL-Driver	       */
#ifdef CONFIG_ISDN_AUDIO
	u_char       vpar[10];                   /* Voice-parameters                   */
	int          lastDLE;                    /* Flag for voice-coding: DLE seen    */
#endif
	int          mdmcmdl;                    /* Length of Modem-Commandbuffer      */
	int          pluscount;                  /* Counter for +++ sequence           */
	u_long       lastplus;                   /* Timestamp of last +                */
	int	     carrierwait;                /* Seconds of carrier waiting         */
	char         mdmcmd[255];                /* Modem-Commandbuffer                */
	unsigned int charge;                     /* Charge units of current connection */
} atemu;

/* Private data (similar to async_struct in <linux/serial.h>) */
typedef struct modem_info {
  int			magic;
  struct tty_port	port;
  int			x_char;		 /* xon/xoff character             */
  int			mcr;		 /* Modem control register         */
  int                   msr;             /* Modem status register          */
  int                   lsr;             /* Line status register           */
  int			line;
  int                   online;          /* 1 = B-Channel is up, drop data */
					 /* 2 = B-Channel is up, deliver d.*/
  int                   dialing;         /* Dial in progress or ATA        */
  int                   rcvsched;        /* Receive needs schedule         */
  int                   isdn_driver;	 /* Index to isdn-driver           */
  int                   isdn_channel;    /* Index to isdn-channel          */
  int                   drv_index;       /* Index to dev->usage            */
  int                   ncarrier;        /* Flag: schedule NO CARRIER      */
  unsigned char         last_cause[8];   /* Last cause message             */
  unsigned char         last_num[ISDN_MSNLEN];
	                                 /* Last phone-number              */
  unsigned char         last_l2;         /* Last layer-2 protocol          */
  unsigned char         last_si;         /* Last service                   */
  unsigned char         last_lhup;       /* Last hangup local?             */
  unsigned char         last_dir;        /* Last direction (in or out)     */
  struct timer_list     nc_timer;        /* Timer for delayed NO CARRIER   */
  int                   send_outstanding;/* # of outstanding send-requests */
  int                   xmit_size;       /* max. # of chars in xmit_buf    */
  int                   xmit_count;      /* # of chars in xmit_buf         */
  struct sk_buff_head   xmit_queue;      /* transmit queue                 */
  atomic_t              xmit_lock;       /* Semaphore for isdn_tty_write   */
#ifdef CONFIG_ISDN_AUDIO
  int                   vonline;         /* Voice-channel status           */
					 /* Bit 0 = recording              */
					 /* Bit 1 = playback               */
					 /* Bit 2 = playback, DLE-ETX seen */
  struct sk_buff_head   dtmf_queue;      /* queue for dtmf results         */
  void                  *adpcms;         /* state for adpcm decompression  */
  void                  *adpcmr;         /* state for adpcm compression    */
  void                  *dtmf_state;     /* state for dtmf decoder         */
  void                  *silence_state;  /* state for silence detection    */
#endif
#ifdef CONFIG_ISDN_TTY_FAX
  struct T30_s		*fax;		 /* T30 Fax Group 3 data/interface */
  int			faxonline;	 /* Fax-channel status             */
#endif
  atemu                 emu;             /* AT-emulator data               */
  spinlock_t	        readlock;
} modem_info;

#define ISDN_MODEM_WINSIZE 8

/* Description of one ISDN-tty */
typedef struct _isdn_modem {
  int                refcount;				/* Number of opens        */
  struct tty_driver  *tty_modem;			/* tty-device             */
  struct tty_struct  *modem_table[ISDN_MAX_CHANNELS];	/* ?? copied from Orig    */
  struct ktermios     *modem_termios[ISDN_MAX_CHANNELS];
  struct ktermios     *modem_termios_locked[ISDN_MAX_CHANNELS];
  modem_info         info[ISDN_MAX_CHANNELS];	   /* Private data           */
} isdn_modem_t;

/*======================= End of ISDN-tty stuff ============================*/

/*======================== Start of V.110 stuff ============================*/
#define V110_BUFSIZE 1024

typedef struct {
	int nbytes;                    /* 1 Matrixbyte -> nbytes in stream     */
	int nbits;                     /* Number of used bits in streambyte    */
	unsigned char key;             /* Bitmask in stream eg. 11 (nbits=2)   */
	int decodelen;                 /* Amount of data in decodebuf          */
	int SyncInit;                  /* Number of sync frames to send        */
	unsigned char *OnlineFrame;    /* Precalculated V110 idle frame        */
	unsigned char *OfflineFrame;   /* Precalculated V110 sync Frame        */
	int framelen;                  /* Length of frames                     */
	int skbuser;                   /* Number of unacked userdata skbs      */
	int skbidle;                   /* Number of unacked idle/sync skbs     */
	int introducer;                /* Local vars for decoder               */
	int dbit;
	unsigned char b;
	int skbres;                    /* space to reserve in outgoing skb     */
	int maxsize;                   /* maxbufsize of lowlevel driver        */
	unsigned char *encodebuf;      /* temporary buffer for encoding        */
	unsigned char decodebuf[V110_BUFSIZE]; /* incomplete V110 matrices     */
} isdn_v110_stream;

/*========================= End of V.110 stuff =============================*/

/*======================= Start of general stuff ===========================*/

typedef struct {
	char *next;
	char *private;
} infostruct;

#define DRV_FLAG_RUNNING 1
#define DRV_FLAG_REJBUS  2
#define DRV_FLAG_LOADED  4

/* Description of hardware-level-driver */
typedef struct _isdn_driver {
	ulong               online;           /* Channel-Online flags             */
	ulong               flags;            /* Misc driver Flags                */
	int                 locks;            /* Number of locks for this driver  */
	int                 channels;         /* Number of channels               */
	wait_queue_head_t   st_waitq;         /* Wait-Queue for status-read's     */
	int                 maxbufsize;       /* Maximum Buffersize supported     */
	unsigned long       pktcount;         /* Until now: unused                */
	int                 stavail;          /* Chars avail on Status-device     */
	isdn_if            *interface;        /* Interface to driver              */
	int                *rcverr;           /* Error-counters for B-Ch.-receive */
	int                *rcvcount;         /* Byte-counters for B-Ch.-receive  */
#ifdef CONFIG_ISDN_AUDIO
	unsigned long      DLEflag;           /* Flags: Insert DLE at next read   */
#endif
	struct sk_buff_head *rpqueue;         /* Pointers to start of Rcv-Queue   */
	wait_queue_head_t  *rcv_waitq;       /* Wait-Queues for B-Channel-Reads  */
	wait_queue_head_t  *snd_waitq;       /* Wait-Queue for B-Channel-Send's  */
	char               msn2eaz[10][ISDN_MSNLEN];  /* Mapping-Table MSN->EAZ   */
} isdn_driver_t;

/* Main driver-data */
typedef struct isdn_devt {
	struct module     *owner;
	spinlock_t	  lock;
	unsigned short    flags;		      /* Bitmapped Flags:           */
	int               drivers;		      /* Current number of drivers  */
	int               channels;		      /* Current number of channels */
	int               net_verbose;                /* Verbose-Flag               */
	int               modempoll;		      /* Flag: tty-read active      */
	spinlock_t	  timerlock;
	int               tflags;                     /* Timer-Flags:               */
	/*  see ISDN_TIMER_..defines  */
	int               global_flags;
	infostruct        *infochain;                 /* List of open info-devs.    */
	wait_queue_head_t info_waitq;                 /* Wait-Queue for isdninfo    */
	struct timer_list timer;		      /* Misc.-function Timer       */
	int               chanmap[ISDN_MAX_CHANNELS]; /* Map minor->device-channel  */
	int               drvmap[ISDN_MAX_CHANNELS];  /* Map minor->driver-index    */
	int               usage[ISDN_MAX_CHANNELS];   /* Used by tty/ip/voice       */
	char              num[ISDN_MAX_CHANNELS][ISDN_MSNLEN];
	/* Remote number of active ch.*/
	int               m_idx[ISDN_MAX_CHANNELS];   /* Index for mdm....          */
	isdn_driver_t     *drv[ISDN_MAX_DRIVERS];     /* Array of drivers           */
	isdn_net_dev      *netdev;		      /* Linked list of net-if's    */
	char              drvid[ISDN_MAX_DRIVERS][20];/* Driver-ID                 */
	struct task_struct *profd;                    /* For iprofd                 */
	isdn_modem_t      mdm;			      /* tty-driver-data            */
	isdn_net_dev      *rx_netdev[ISDN_MAX_CHANNELS]; /* rx netdev-pointers     */
	isdn_net_dev      *st_netdev[ISDN_MAX_CHANNELS]; /* stat netdev-pointers   */
	ulong             ibytes[ISDN_MAX_CHANNELS];  /* Statistics incoming bytes  */
	ulong             obytes[ISDN_MAX_CHANNELS];  /* Statistics outgoing bytes  */
	int               v110emu[ISDN_MAX_CHANNELS]; /* V.110 emulator-mode 0=none */
	atomic_t          v110use[ISDN_MAX_CHANNELS]; /* Usage-Semaphore for stream */
	isdn_v110_stream  *v110[ISDN_MAX_CHANNELS];   /* V.110 private data         */
	struct mutex      mtx;                        /* serialize list access*/
	unsigned long     global_features;
} isdn_dev;

extern isdn_dev *dev;


#endif /* __ISDN_H__ */
