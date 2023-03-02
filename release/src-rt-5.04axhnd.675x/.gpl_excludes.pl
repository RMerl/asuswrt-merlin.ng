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

	if ($uc_model == "GT10") {
		print $fexclude "${srcbasedir}/bcmdrivers/opensource/phy/unimac_drv_impl1.c\n";
                print $fexclude "${srcbasedir}/bcmdrivers/opensource/phy/mac_drv_unimac.c\n";
	}

	close($fexclude);
}

if (@ARGV >= 2) {
	append_gpl_excludes($ARGV[0], $ARGV[1]);
}
else {
	print "usage: .gpl_excludes.pl [model] [srcbasedir]\n";
}

