dnl
AC_DEFUN(AC_LBL_TPACKET_STATS,
   [AC_MSG_CHECKING(if if_packet.h has tpacket_stats defined)
   AC_CACHE_VAL(ac_cv_lbl_tpacket_stats,
   AC_TRY_COMPILE([
#  include <linux/if_packet.h>],
   [struct tpacket_stats stats],
   ac_cv_lbl_tpacket_stats=yes,
   ac_cv_lbl_tpacket_stats=no))
   AC_MSG_RESULT($ac_cv_lbl_tpacket_stats)
   if test $ac_cv_lbl_tpacket_stats = yes; then
       AC_DEFINE(HAVE_TPACKET_STATS,1,[if if_packet.h has tpacket_stats defined])
   fi])

AC_DEFUN(AC_LBL_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI,
    [AC_MSG_CHECKING(if tpacket_auxdata struct has tp_vlan_tci member)
    AC_CACHE_VAL(ac_cv_lbl_linux_tpacket_auxdata_tp_vlan_tci,
	AC_TRY_COMPILE([
#	include <sys/types.h>
#	include <linux/if_packet.h>],
	[u_int i = sizeof(((struct tpacket_auxdata *)0)->tp_vlan_tci)],
	ac_cv_lbl_linux_tpacket_auxdata_tp_vlan_tci=yes,
	ac_cv_lbl_linux_tpacket_auxdata_tp_vlan_tci=no))
    AC_MSG_RESULT($ac_cv_lbl_linux_tpacket_auxdata_tp_vlan_tci)
    if test $ac_cv_lbl_linux_tpacket_auxdata_tp_vlan_tci = yes ; then
	    HAVE_LINUX_TPACKET_AUXDATA=tp_vlan_tci
	    AC_SUBST(HAVE_LINUX_TPACKET_AUXDATA)
	    AC_DEFINE(HAVE_LINUX_TPACKET_AUXDATA_TP_VLAN_TCI,1,[if tp_vlan_tci exists])
    fi])