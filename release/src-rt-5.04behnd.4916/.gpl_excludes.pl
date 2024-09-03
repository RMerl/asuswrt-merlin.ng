#!/usr/bin/perl
# $1: [model], $2: [srcbasedir]


sub append_gpl_excludes
{
	my $fexclude;
	my $uc_model;
	my $srcbasedir;

	$uc_model=uc($_[0]);
	$srcbasedir=$_[1];

	system("touch ./.gpl_excludes.sysdeps");
	open($fexclude, ">./.gpl_excludes.sysdeps");

	if ($uc_model eq "GT-BE96") {
		$BCM_WLIMPL=impl99;
		$BCM_CHIP=BCM6813;

		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/cmdlist/impl1/*save*\n";

		print $fexclude "${srcbasedir}/rdp/projects/DSL_*\n";
		print $fexclude "${srcbasedir}/rdp/projects/PON_*\n";
		print $fexclude "${srcbasedir}/rdp/projects/Makefile\n";
		print $fexclude "${srcbasedir}/rdp/projects/BCM4912\n";
		print $fexclude "${srcbasedir}/rdp/projects/BCM6813_FPI\n";
		print $fexclude "${srcbasedir}/rdp/projects/WL4908*\n";

		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/Makefile\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/src/.config\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/acsdv2/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/acsdv2/*.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/afcd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/afcmd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/airiq\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/appeventd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/bsd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/cevent_app_common.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/ceventc.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/ceventd.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/*.sh\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/escand\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/ieee1905\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/iqos\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/locpold\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/qosmgmt\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/visualization\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/wbd2\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/wldm\n";

		print $fexclude "${srcbasedir}/router-sysdep/bridge-utils/bridge-utils-1.7.1\n";
		print $fexclude "${srcbasedir}/router-sysdep/bridge-utils/objs\n";

		system("cd ./rdp/projects/${BCM_CHIP}/target && rm -f `find -L -xtype l`");
	}
	elsif ($uc_model eq "TUF-BE3600") {
		$BCM_WLIMPL=impl99;
		$BCM_CHIP=BCM6765;

		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer/impl1/*saved\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer/impl1/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer/impl1/*.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer/impl1/crossbow/*saved\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer/impl1/crossbow/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/archer/impl1/crossbow/*.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/opensource/char/archer/impl1/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/opensource/char/bcmlibs/impl1/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/shared/impl1/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/shared/impl1/srom\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/char/cmdlist/impl1/*save*\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/avs/src/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/avs/src/*o_saved*\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/bcmcrypto/src/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/bcmcrypto/src/*.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/clm-api/src/*.o\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/clm-api/src/wlc_clm_data_4*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/clm-api/src/wlc_clm_data_6*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/clm-api/sysdeps\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/msch/src/*.c\n";

		print $fexclude "${srcbasedir}/rdp\n";

		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/Makefile\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/src/.config\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/acsdv2/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/acsdv2/*.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/afcd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/afcmd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/airiq\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/appeventd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/bsd\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/*.c\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/cevent_app_common.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/ceventc.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/ceventd.h\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/cevent_app/*.sh\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/escand\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/ieee1905\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/iqos\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/locpold\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/qosmgmt\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/visualization\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/wbd2\n";
		print $fexclude "${srcbasedir}/bcmdrivers/broadcom/net/wl/${BCM_WLIMPL}/main/components/apps/wldm\n";

		print $fexclude "${srcbasedir}/router-sysdep/bcm_util/*.c\n";
		print $fexclude "${srcbasedir}/router-sysdep/gen_util/*.c\n";
		print $fexclude "${srcbasedir}/router-sysdep/gen_util/*.h\n";
	}

	close($fexclude);
}

if (@ARGV >= 2) {
	append_gpl_excludes($ARGV[0], $ARGV[1]);
}
else {
	print "usage: .gpl_excludes.pl [model] [srcbasedir]\n";
}

