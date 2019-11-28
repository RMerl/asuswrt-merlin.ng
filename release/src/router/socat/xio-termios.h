/* source: xio-termios.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_termios_h_included
#define __xio_termios_h_included 1

extern const struct optdesc opt_tiocsctty;

extern const struct optdesc opt_brkint;
extern const struct optdesc opt_icrnl;
extern const struct optdesc opt_ignbrk;
extern const struct optdesc opt_igncr;
extern const struct optdesc opt_ignpar;
extern const struct optdesc opt_imaxbel;
extern const struct optdesc opt_inlcr;
extern const struct optdesc opt_inpck;
extern const struct optdesc opt_istrip;
extern const struct optdesc opt_iuclc;
extern const struct optdesc opt_ixany;
extern const struct optdesc opt_ixoff;
extern const struct optdesc opt_ixon;
extern const struct optdesc opt_parmrk;
extern const struct optdesc opt_cr0;
extern const struct optdesc opt_cr1;
extern const struct optdesc opt_cr2;
extern const struct optdesc opt_cr3;
extern const struct optdesc opt_crdly;
extern const struct optdesc opt_nl0;
extern const struct optdesc opt_nl1;
extern const struct optdesc opt_nldly;
extern const struct optdesc opt_ocrnl;
extern const struct optdesc opt_ofdel;
extern const struct optdesc opt_ofill;
extern const struct optdesc opt_opost;
extern const struct optdesc opt_olcuc;
extern const struct optdesc opt_onlcr;
extern const struct optdesc opt_onlret;
extern const struct optdesc opt_onocr;
extern const struct optdesc opt_tab0;
extern const struct optdesc opt_tab1;
extern const struct optdesc opt_tab2;
extern const struct optdesc opt_tab3;
extern const struct optdesc opt_xtabs;
extern const struct optdesc opt_tabdly;
extern const struct optdesc opt_bs0;
extern const struct optdesc opt_bs1;
extern const struct optdesc opt_bsdly;
extern const struct optdesc opt_vt0;
extern const struct optdesc opt_vt1;
extern const struct optdesc opt_vtdly;
extern const struct optdesc opt_ff0;
extern const struct optdesc opt_ff1;
extern const struct optdesc opt_ffdly;
extern const struct optdesc opt_b0;
extern const struct optdesc opt_b50;
extern const struct optdesc opt_b75;
extern const struct optdesc opt_b110;
extern const struct optdesc opt_b134;
extern const struct optdesc opt_b150;
extern const struct optdesc opt_b200;
extern const struct optdesc opt_b300;
extern const struct optdesc opt_b600;
extern const struct optdesc opt_b900;
extern const struct optdesc opt_b1200;
extern const struct optdesc opt_b1800;
extern const struct optdesc opt_b2400;
extern const struct optdesc opt_b3600;
extern const struct optdesc opt_b4800;
extern const struct optdesc opt_b7200;
extern const struct optdesc opt_b9600;
extern const struct optdesc opt_b19200;
extern const struct optdesc opt_b38400;
extern const struct optdesc opt_b57600;
extern const struct optdesc opt_b115200;
extern const struct optdesc opt_b230400;
extern const struct optdesc opt_b460800;
extern const struct optdesc opt_b500000;
extern const struct optdesc opt_b576000;
extern const struct optdesc opt_b921600;
extern const struct optdesc opt_b1000000;
extern const struct optdesc opt_b1152000;
extern const struct optdesc opt_b1500000;
extern const struct optdesc opt_b2000000;
extern const struct optdesc opt_b2500000;
extern const struct optdesc opt_b3000000;
extern const struct optdesc opt_b3500000;
extern const struct optdesc opt_b4000000;
extern const struct optdesc opt_cs5;
extern const struct optdesc opt_cs6;
extern const struct optdesc opt_cs7;
extern const struct optdesc opt_cs8;
extern const struct optdesc opt_csize;
extern const struct optdesc opt_cstopb;
extern const struct optdesc opt_cread;
extern const struct optdesc opt_parenb;
extern const struct optdesc opt_parodd;
extern const struct optdesc opt_hupcl;
extern const struct optdesc opt_clocal;
/*extern const struct optdesc opt_cibaud*/
extern const struct optdesc opt_crtscts;
extern const struct optdesc opt_isig;
extern const struct optdesc opt_icanon;
extern const struct optdesc opt_xcase;
extern const struct optdesc opt_echo;
extern const struct optdesc opt_echoe;
extern const struct optdesc opt_echok;
extern const struct optdesc opt_echonl;
extern const struct optdesc opt_echoctl;
extern const struct optdesc opt_echoprt;
extern const struct optdesc opt_echoke;
extern const struct optdesc opt_flusho;
extern const struct optdesc opt_noflsh;
extern const struct optdesc opt_tostop;
extern const struct optdesc opt_pendin;
extern const struct optdesc opt_iexten;
extern const struct optdesc opt_vintr;
extern const struct optdesc opt_vquit;
extern const struct optdesc opt_verase;
extern const struct optdesc opt_vkill;
extern const struct optdesc opt_veof;
extern const struct optdesc opt_vtime;
extern const struct optdesc opt_vmin;
extern const struct optdesc opt_vswtc;
extern const struct optdesc opt_vstart;
extern const struct optdesc opt_vstop;
extern const struct optdesc opt_vsusp;
extern const struct optdesc opt_vdsusp;
extern const struct optdesc opt_veol;
extern const struct optdesc opt_vreprint;
extern const struct optdesc opt_vdiscard;
extern const struct optdesc opt_vwerase;
extern const struct optdesc opt_vlnext;
extern const struct optdesc opt_veol2;
extern const struct optdesc opt_raw;
extern const struct optdesc opt_sane;

extern const struct optdesc opt_ispeed;
extern const struct optdesc opt_ospeed;

extern const struct optdesc opt_termios_rawer;
extern const struct optdesc opt_termios_cfmakeraw;

#if _WITH_TERMIOS /* otherwise tcflag_t might be reported undefined */
extern int xiotermios_setflag(int fd, int word, tcflag_t mask);
extern int xiotermios_clrflag(int fd, int word, tcflag_t mask);
extern int xiotermiosflag_applyopt(int fd, struct opt *opt);
#endif /* _WITH_TERMIOS */

#endif /* !defined(__xio_termios_h_included) */
