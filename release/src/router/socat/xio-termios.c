/* source: xio-termios.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for terminal I/O options */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xio-termios.h"

/****** TERMIOS addresses ******/
#if _WITH_TERMIOS
#if WITH_TERMIOS
const struct optdesc opt_tiocsctty={ "tiocsctty", "ctty",OPT_TIOCSCTTY,  GROUP_TERMIOS,   PH_LATE2, TYPE_BOOL,     OFUNC_SPEC };

const struct optdesc opt_brkint  = { "brkint",  NULL, OPT_BRKINT,  GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, BRKINT };
const struct optdesc opt_icrnl   = { "icrnl",   NULL, OPT_ICRNL,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, ICRNL };
const struct optdesc opt_ignbrk  = { "ignbrk",  NULL, OPT_IGNBRK,  GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IGNBRK };
const struct optdesc opt_igncr   = { "igncr",   NULL, OPT_IGNCR,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IGNCR };
const struct optdesc opt_ignpar  = { "ignpar",  NULL, OPT_IGNPAR,  GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IGNPAR };
const struct optdesc opt_imaxbel = { "imaxbel", NULL, OPT_IMAXBEL, GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IMAXBEL };
const struct optdesc opt_inlcr   = { "inlcr",   NULL, OPT_INLCR,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, INLCR };
const struct optdesc opt_inpck   = { "inpck",   NULL, OPT_INPCK,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, INPCK };
const struct optdesc opt_istrip  = { "istrip",  NULL, OPT_ISTRIP,  GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, ISTRIP };
#ifdef IUCLC
const struct optdesc opt_iuclc   = { "iuclc",   NULL, OPT_IUCLC,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IUCLC };
#endif
const struct optdesc opt_ixany   = { "ixany",   NULL, OPT_IXANY,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IXANY };
const struct optdesc opt_ixoff   = { "ixoff",   NULL, OPT_IXOFF,   GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IXOFF };
const struct optdesc opt_ixon    = { "ixon",    NULL, OPT_IXON,    GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, IXON };
const struct optdesc opt_parmrk  = { "parmrk",  NULL, OPT_PARMRK,  GROUP_TERMIOS, PH_FD, TYPE_BOOL, OFUNC_TERMIOS_FLAG, 0, PARMRK };

#ifdef CRDLY
#  ifdef CR0
const struct optdesc opt_cr0     = { "cr0",     NULL, OPT_CR0,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, CR0, CRDLY };
#  endif
#  ifdef CR1
const struct optdesc opt_cr1     = { "cr1",     NULL, OPT_CR1,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, CR1, CRDLY };
#  endif
#  ifdef CR2
const struct optdesc opt_cr2     = { "cr2",     NULL, OPT_CR2,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, CR2, CRDLY };
#  endif
#  ifdef CR3
const struct optdesc opt_cr3     = { "cr3",     NULL, OPT_CR3,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, CR3, CRDLY };
#  endif
#  if CRDLY_SHIFT >= 0
const struct optdesc opt_crdly   = { "crdly",   NULL, OPT_CRDLY,   GROUP_TERMIOS, PH_FD, TYPE_UINT,  OFUNC_TERMIOS_VALUE,   1, CRDLY, CRDLY_SHIFT };
#  endif
#endif /* defined(CRDLY) */
#ifdef NLDLY
#  ifdef NL0
const struct optdesc opt_nl0     = { "nl0",     NULL, OPT_NL0,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, NL0, NLDLY };
#  endif
#  ifdef NL1
const struct optdesc opt_nl1     = { "nl1",     NULL, OPT_NL1,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, NL1, NLDLY };
#  endif
const struct optdesc opt_nldly   = { "nldly",   NULL, OPT_NLDLY,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, NLDLY };
#endif /* defined(NLDLY) */
#ifdef OCRNL
const struct optdesc opt_ocrnl   = { "ocrnl",   NULL, OPT_OCRNL,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, OCRNL };
#endif
#ifdef OFDEL
const struct optdesc opt_ofdel   = { "ofdel",   NULL, OPT_OFDEL,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, OFDEL };
#endif
#ifdef OFILL
const struct optdesc opt_ofill   = { "ofill",   NULL, OPT_OFILL,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, OFILL };
#endif
const struct optdesc opt_opost   = { "opost",   NULL, OPT_OPOST,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, OPOST };
#ifdef OLCUC
const struct optdesc opt_olcuc   = { "olcuc",   NULL, OPT_OLCUC,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, OLCUC };
#endif
const struct optdesc opt_onlcr   = { "onlcr",   NULL, OPT_ONLCR,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, ONLCR };
#ifdef ONLRET
const struct optdesc opt_onlret  = { "onlret",  NULL, OPT_ONLRET,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, ONLRET };
#endif
#ifdef ONOCR
const struct optdesc opt_onocr   = { "onocr",   NULL, OPT_ONOCR,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 1, ONOCR };
#endif
#ifdef TABDLY
#  ifdef TAB0
const struct optdesc opt_tab0    = { "tab0",    NULL, OPT_TAB0,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_FLAG, 1, TAB0, TABDLY };
#  endif
#  ifdef TAB1
const struct optdesc opt_tab1    = { "tab1",    NULL, OPT_TAB1,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_FLAG, 1, TAB1, TABDLY };
#  endif
#  ifdef TAB2
const struct optdesc opt_tab2    = { "tab2",    NULL, OPT_TAB2,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_FLAG, 1, TAB2, TABDLY };
#  endif
#  ifdef TAB3
const struct optdesc opt_tab3    = { "tab3",    NULL, OPT_TAB3,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_FLAG, 1, TAB3, TABDLY };
#  endif
#  ifdef XTABS
const struct optdesc opt_xtabs   = { "xtabs",   NULL, OPT_XTABS,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_FLAG, 1, XTABS, TABDLY };
#  endif
#  if TABDLY_SHIFT >= 0
const struct optdesc opt_tabdly  = { "tabdly",  NULL, OPT_TABDLY,  GROUP_TERMIOS, PH_FD, TYPE_UINT, OFUNC_TERMIOS_VALUE, 1, TABDLY, TABDLY_SHIFT };
#  endif
#endif /* defined(TABDLY) */
#ifdef BSDLY
#  ifdef BS0
const struct optdesc opt_bs0     = { "bs0",     NULL, OPT_BS0,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, BS0, BSDLY };
#endif
#  ifdef BS1
const struct optdesc opt_bs1     = { "bs1",     NULL, OPT_BS1,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, BS1, BSDLY };
#  endif
const struct optdesc opt_bsdly   = { "bsdly",   NULL, OPT_BSDLY,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    1, BSDLY };
#endif
#ifdef VTDLY
#  ifdef VT0
const struct optdesc opt_vt0     = { "vt0",     NULL, OPT_VT0,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, VT0, VTDLY };
#  endif
#  ifdef VT1
const struct optdesc opt_vt1     = { "vt1",     NULL, OPT_VT1,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, VT1, VTDLY };
#  endif
const struct optdesc opt_vtdly   = { "vtdly",   NULL, OPT_VTDLY,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    1, VTDLY };
#endif
#ifdef FFDLY
#  ifdef FF0
const struct optdesc opt_ff0     = { "ff0",     NULL, OPT_FF0,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, FF0, FFDLY };
#  endif
#  ifdef FF1
const struct optdesc opt_ff1     = { "ff1",     NULL, OPT_FF1,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 1, FF1, FFDLY };
#  endif
const struct optdesc opt_ffdly   = { "ffdly",   NULL, OPT_FFDLY,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    1, FFDLY };
#endif

#ifdef CBAUD
const struct optdesc opt_b0      = { "b0",      NULL, OPT_B0,      GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B0,       CBAUD };
const struct optdesc opt_b50     = { "b50",     NULL, OPT_B50,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B50,      CBAUD };
const struct optdesc opt_b75     = { "b75",     NULL, OPT_B75,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B75,      CBAUD };
const struct optdesc opt_b110    = { "b110",    NULL, OPT_B110,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B110,     CBAUD };
const struct optdesc opt_b134    = { "b134",    NULL, OPT_B134,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B134,     CBAUD };
const struct optdesc opt_b150    = { "b150",    NULL, OPT_B150,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B150,     CBAUD };
const struct optdesc opt_b200    = { "b200",    NULL, OPT_B200,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B200,     CBAUD };
const struct optdesc opt_b300    = { "b300",    NULL, OPT_B300,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B300,     CBAUD };
const struct optdesc opt_b600    = { "b600",    NULL, OPT_B600,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B600,     CBAUD };
#ifdef B900  /* HP-UX */
const struct optdesc opt_b900    = { "b900",    NULL, OPT_B900,    GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B900,     CBAUD };
#endif
const struct optdesc opt_b1200   = { "b1200",   NULL, OPT_B1200,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B1200,    CBAUD };
const struct optdesc opt_b1800   = { "b1800",   NULL, OPT_B1800,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B1800,    CBAUD };
const struct optdesc opt_b2400   = { "b2400",   NULL, OPT_B2400,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B2400,    CBAUD };
#ifdef B3600  /* HP-UX */
const struct optdesc opt_b3600   = { "b3600",   NULL, OPT_B3600,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B3600,    CBAUD };
#endif
const struct optdesc opt_b4800   = { "b4800",   NULL, OPT_B4800,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B4800,    CBAUD };
#ifdef B7200  /* HP-UX */
const struct optdesc opt_b7200   = { "b7200",   NULL, OPT_B7200,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B7200,    CBAUD };
#endif
const struct optdesc opt_b9600   = { "b9600",   NULL, OPT_B9600,   GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B9600,    CBAUD };
const struct optdesc opt_b19200  = { "b19200",  NULL, OPT_B19200,  GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B19200,   CBAUD };
const struct optdesc opt_b38400  = { "b38400",  NULL, OPT_B38400,  GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B38400,   CBAUD };
#ifdef B57600
const struct optdesc opt_b57600  = { "b57600",  NULL, OPT_B57600,  GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B57600,   CBAUD };
#endif
#ifdef B115200
const struct optdesc opt_b115200 = { "b115200", NULL, OPT_B115200, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B115200,  CBAUD };
#endif
#ifdef B230400
const struct optdesc opt_b230400 = { "b230400", NULL, OPT_B230400, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B230400,  CBAUD };
#endif
#ifdef B460800
const struct optdesc opt_b460800 = { "b460800", NULL, OPT_B460800, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B460800,  CBAUD };
#endif
#ifdef B500000
const struct optdesc opt_b500000 = { "b500000", NULL, OPT_B500000, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B500000,  CBAUD };
#endif
#ifdef B576000
const struct optdesc opt_b576000 = { "b576000", NULL, OPT_B576000, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B576000,  CBAUD };
#endif
#ifdef B921600
const struct optdesc opt_b921600 = { "b921600", NULL, OPT_B921600, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B921600,  CBAUD };
#endif
#ifdef B1000000
const struct optdesc opt_b1000000= { "b1000000",NULL, OPT_B1000000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B1000000, CBAUD };
#endif
#ifdef B1152000
const struct optdesc opt_b1152000= { "b1152000",NULL, OPT_B1152000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B1152000, CBAUD };
#endif
#ifdef B1500000
const struct optdesc opt_b1500000= { "b1500000",NULL, OPT_B1500000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B1500000, CBAUD };
#endif
#ifdef B2000000
const struct optdesc opt_b2000000= { "b2000000",NULL, OPT_B2000000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B2000000, CBAUD };
#endif
#ifdef B2500000
const struct optdesc opt_b2500000= { "b2500000",NULL, OPT_B2500000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B2500000, CBAUD };
#endif
#ifdef B3000000
const struct optdesc opt_b3000000= { "b3000000",NULL, OPT_B3000000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B3000000, CBAUD };
#endif
#ifdef B3500000
const struct optdesc opt_b3500000= { "b3500000",NULL, OPT_B3500000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B3500000, CBAUD };
#endif
#ifdef B4000000
const struct optdesc opt_b4000000= { "b4000000",NULL, OPT_B4000000,GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, B4000000, CBAUD };
#endif
#endif /* defined(CBAUD) */
const struct optdesc opt_cs5     = { "cs5",     NULL, OPT_CS5,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, CS5, CSIZE };
const struct optdesc opt_cs6     = { "cs6",     NULL, OPT_CS6,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, CS6, CSIZE };
const struct optdesc opt_cs7     = { "cs7",     NULL, OPT_CS7,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, CS7, CSIZE };
const struct optdesc opt_cs8     = { "cs8",     NULL, OPT_CS8,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_PATTERN, 2, CS8, CSIZE };
#if CSIZE_SHIFT >= 0
const struct optdesc opt_csize   = { "csize",   NULL, OPT_CSIZE,   GROUP_TERMIOS, PH_FD, TYPE_UINT,  OFUNC_TERMIOS_VALUE,   2, CSIZE, CSIZE_SHIFT };
#endif
const struct optdesc opt_cstopb  = { "cstopb",  NULL, OPT_CSTOPB,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, CSTOPB };
const struct optdesc opt_cread   = { "cread",   NULL, OPT_CREAD,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, CREAD };
const struct optdesc opt_parenb  = { "parenb",  NULL, OPT_PARENB,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, PARENB };
const struct optdesc opt_parodd  = { "parodd",  NULL, OPT_PARODD,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, PARODD };
const struct optdesc opt_hupcl   = { "hupcl",   NULL, OPT_HUPCL,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, HUPCL };
const struct optdesc opt_clocal  = { "clocal",  NULL, OPT_CLOCAL,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, CLOCAL };
/*const struct optdesc opt_cibaud  = { "cibaud",NULL,   OPT_CIBAUD,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, CIBAUD };*/
#ifdef CRTSCTS
const struct optdesc opt_crtscts = { "crtscts", NULL, OPT_CRTSCTS, GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG,    2, CRTSCTS };
#endif

const struct optdesc opt_isig    = { "isig",    NULL, OPT_ISIG,    GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ISIG };
const struct optdesc opt_icanon  = { "icanon",  NULL, OPT_ICANON,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ICANON };
#ifdef XCASE
const struct optdesc opt_xcase   = { "xcase",   NULL, OPT_XCASE,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, XCASE };
#endif
const struct optdesc opt_echo    = { "echo",    NULL, OPT_ECHO,    GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHO };
const struct optdesc opt_echoe   = { "echoe",   NULL, OPT_ECHOE,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHOE };
const struct optdesc opt_echok   = { "echok",   NULL, OPT_ECHOK,   GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHOK };
const struct optdesc opt_echonl  = { "echonl",  NULL, OPT_ECHONL,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHONL };
const struct optdesc opt_echoctl = { "echoctl", NULL, OPT_ECHOCTL, GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHOCTL };
#ifdef ECHOPRT
const struct optdesc opt_echoprt = { "echoprt", NULL, OPT_ECHOPRT, GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHOPRT };
#endif
const struct optdesc opt_echoke  = { "echoke",  NULL, OPT_ECHOKE,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, ECHOKE };
const struct optdesc opt_flusho  = { "flusho",  NULL, OPT_FLUSHO,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, FLUSHO };
const struct optdesc opt_noflsh  = { "noflsh",  NULL, OPT_NOFLSH,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, NOFLSH };
const struct optdesc opt_tostop  = { "tostop",  NULL, OPT_TOSTOP,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, TOSTOP };
#ifdef PENDIN
const struct optdesc opt_pendin  = { "pendin",  NULL, OPT_PENDIN,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, PENDIN };
#endif
const struct optdesc opt_iexten  = { "iexten",  NULL, OPT_IEXTEN,  GROUP_TERMIOS, PH_FD, TYPE_BOOL,  OFUNC_TERMIOS_FLAG, 3, IEXTEN };

const struct optdesc opt_vintr    = { "vintr",  "intr",  OPT_VINTR,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VINTR };
const struct optdesc opt_vquit    = { "vquit",  "quit",  OPT_VQUIT,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VQUIT };
const struct optdesc opt_verase   = { "verase", "erase", OPT_VERASE,   GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VERASE };
const struct optdesc opt_vkill    = { "vkill",  "kill",  OPT_VKILL,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VKILL };
const struct optdesc opt_veof     = { "veof",   "eof",   OPT_VEOF,     GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VEOF };
const struct optdesc opt_vtime    = { "vtime",  "time",  OPT_VTIME,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VTIME };
const struct optdesc opt_vmin     = { "vmin",   "min",   OPT_VMIN,     GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VMIN };
#ifdef VSWTC
const struct optdesc opt_vswtc    = { "vswtc",  "swtc",  OPT_VSWTC,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VSWTC };
#endif /* VSWTC */
const struct optdesc opt_vstart   = { "vstart", "start", OPT_VSTART,   GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VSTART };
const struct optdesc opt_vstop    = { "vstop",  "stop",  OPT_VSTOP,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VSTOP };
const struct optdesc opt_vsusp    = { "vsusp",  "susp",  OPT_VSUSP,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VSUSP };
#ifdef VDSUSP  /* HP-UX */
const struct optdesc opt_vdsusp   = { "vdsusp", "dsusp", OPT_VDSUSP,   GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VDSUSP };
#endif
const struct optdesc opt_veol     = { "veol",   "eol",   OPT_VEOL,     GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VEOL };
#ifdef VREPRINT
const struct optdesc opt_vreprint = { "vreprint","reprint",OPT_VREPRINT,GROUP_TERMIOS,PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VREPRINT };
#endif
#ifdef VDISCARD
const struct optdesc opt_vdiscard = { "vdiscard","discard",OPT_VDISCARD,GROUP_TERMIOS,PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VDISCARD };
#endif
#ifdef VWERASE
const struct optdesc opt_vwerase  = { "vwerase","werase",OPT_VWERASE,  GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VWERASE };
#endif
const struct optdesc opt_vlnext   = { "vlnext", "lnext", OPT_VLNEXT,   GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VLNEXT };
const struct optdesc opt_veol2    = { "veol2",  "eol2",  OPT_VEOL2,    GROUP_TERMIOS, PH_FD, TYPE_BYTE, OFUNC_TERMIOS_CHAR, VEOL2 };

const struct optdesc opt_raw      = { "raw",    NULL,    OPT_RAW,      GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_SPEC };
const struct optdesc opt_sane     = { "sane",   NULL,    OPT_SANE,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_SPEC };
const struct optdesc opt_termios_cfmakeraw = { "termios-cfmakeraw", "cfmakeraw", OPT_TERMIOS_CFMAKERAW, GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_SPEC };
const struct optdesc opt_termios_rawer     = { "termios-rawer",     "rawer",     OPT_TERMIOS_RAWER,     GROUP_TERMIOS, PH_FD, TYPE_CONST, OFUNC_TERMIOS_SPEC };

#ifdef HAVE_TERMIOS_ISPEED
#if defined(ISPEED_OFFSET) && (ISPEED_OFFSET != -1)
#if defined(OSPEED_OFFSET) && (OSPEED_OFFSET != -1)
const struct optdesc opt_ispeed = { "ispeed", NULL, OPT_ISPEED, GROUP_TERMIOS, PH_FD, TYPE_UINT, OFUNC_TERMIOS_SPEED, ISPEED_OFFSET };
const struct optdesc opt_ospeed = { "ospeed", NULL, OPT_OSPEED, GROUP_TERMIOS, PH_FD, TYPE_UINT, OFUNC_TERMIOS_SPEED, OSPEED_OFFSET };
#endif
#endif
#endif /* HAVE_TERMIOS_ISPEED */


int xiotermiosflag_applyopt(int fd, struct opt *opt) {
   int result;
   if (opt->value.u_bool) {
      result = xiotermios_setflag(fd, opt->desc->major, opt->desc->minor);
   } else {
      result = xiotermios_clrflag(fd, opt->desc->major, opt->desc->minor);
   }
   if (result < 0) {
      opt->desc = ODESC_ERROR;
      return -1;
   }
   return 0;
}

#endif /* WITH_TERMIOS */

int xiotermios_setflag(int fd, int word, tcflag_t mask) {
   union {
      struct termios termarg;
      tcflag_t flags[4];
   } tdata;

   if (Tcgetattr(fd, &tdata.termarg) < 0) {
      Error3("tcgetattr(%d, %p): %s",
	     fd, &tdata.termarg, strerror(errno));
      return -1;
   }
   tdata.flags[word] |= mask;
   if (Tcsetattr(fd, TCSADRAIN, &tdata.termarg) < 0) {
      Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
	     fd, &tdata.termarg, strerror(errno));
      return -1;
   }
   return 0;
}

int xiotermios_clrflag(int fd, int word, tcflag_t mask) {
   union {
      struct termios termarg;
      tcflag_t flags[4];
   } tdata;

   if (Tcgetattr(fd, &tdata.termarg) < 0) {
      Error3("tcgetattr(%d, %p): %s",
	     fd, &tdata.termarg, strerror(errno));
      return -1;
   }
   tdata.flags[word] &= ~mask;
   if (Tcsetattr(fd, TCSADRAIN, &tdata.termarg) < 0) {
      Error3("tcsetattr(%d, TCSADRAIN, %p): %s",
	     fd, &tdata.termarg, strerror(errno));
      return -1;
   }
   return 0;
}

#endif /* _WITH_TERMIOS */

