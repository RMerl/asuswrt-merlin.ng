#!/usr/bin/perl
# A simple system for making software releases
# (c) 2003--2011 Martin Mares <mj@ucw.cz>

package UCW::Release;
use strict;
use warnings;
use Getopt::Long;

our $verbose = 0;

sub new($$) {
	my ($class,$basename) = @_;
	my $s = {
		"PACKAGE" => $basename,
		"rules" => [
			# p=preprocess, s=subst, -=discard
			'(^|/)(CVS|\.arch-ids|{arch}|\.git|tmp)/' => '-',
			'\.sw[a-z]$' => '-',
			'\.(lsm|spec)$' => 'ps',
			'(^|/)README$' => 's'
			],
		"directories" => [
			],
		"conditions" => {
			},
		"DATE" => `date '+%Y-%m-%d' | tr -d '\n'`,
		"LSMDATE" => `date '+%y%m%d' | tr -d '\n'`,
		"distfiles" => [
			],
		"archivedir" => "/home/mj/tmp/archives/$basename",
		"uploads" => [
			],
		# Options
		"do_test" => 1,
		"do_patch" => 1,
		"diff_against" => "",
		"do_upload" => 1,
		"do_sign" => 1,
	};
	bless $s;
	return $s;
}

sub GetVersionFromFile($) {
	my ($s,$file,$rx) = @_;
	open F, $file or die "Unable to open $file for version autodetection";
	while (<F>) {
		chomp;
		if (/$rx/) {
			$s->{"VERSION"} = $1;
			print "Detected version $1 from $file\n" if $verbose;
			last;
		}
	}
	close F;
	if (!defined $s->{"VERSION"}) { die "Failed to auto-detect version"; }
	return $s->{"VERSION"};
}

sub GetVersionsFromChangelog($) {
	my ($s,$file,$rx) = @_;
	open F, $file or die "Unable to open $file for version autodetection";
	while (<F>) {
		chomp;
		if (/$rx/) {
			if (!defined $s->{"VERSION"}) {
				$s->{"VERSION"} = $1;
				print "Detected version $1 from $file\n" if $verbose;
			} elsif ($s->{"VERSION"} eq $1) {
				# do nothing
			} else {
				$s->{"OLDVERSION"} = $1;
				print "Detected previous version $1 from $file\n" if $verbose;
				last;
			}
		}
	}
	close F;
	if (!defined $s->{"VERSION"}) { die "Failed to auto-detect version"; }
	return $s->{"VERSION"};
}

sub InitDist($) {
	my ($s,$dd) = @_;
	$s->{"DISTDIR"} = $dd;
	print "Initializing dist directory $dd\n" if $verbose;
	`rm -rf $dd`; die if $?;
	`mkdir -p $dd`; die if $?;
}

sub ExpandVar($$) {
	my ($s,$v) = @_;
	if (defined $s->{$v}) {
		return $s->{$v};
	} else {
		die "Reference to unknown variable $v";
	}
}

sub CopyFile($$$$) {
	my ($s,$f,$dir,$action) = @_;

	(my $d = $f) =~ s@(^|/)[^/]*$@@;
	$d = "$dir/$d";
	-d $d || `mkdir -p $d`; die if $?;

	my $preprocess = ($action =~ /p/);
	my $subst = ($action =~ /s/);
	if ($preprocess || $subst) {
		open I, "$f" or die "open($f): $?";
		open O, ">$dir/$f" or die "open($dir/$f): $!";
		my @ifs = ();	# stack of conditions, 1=satisfied
		my $empty = 0;	# last line was empty
		my $is_makefile = ($f =~ /(Makefile|.mk)$/);
		while (<I>) {
			if ($subst) {
				s/@([0-9A-Za-z_]+)@/$s->ExpandVar($1)/ge;
			}
			if ($preprocess) {
				if (/^#/ || $is_makefile) {
					if (/^#?ifdef\s+(\w+)/) {
						if (defined ${$s->{"conditions"}}{$1}) {
							push @ifs, ${$s->{"conditions"}}{$1};
							next;
						}
						push @ifs, 0;
					} elsif (/^#ifndef\s+(\w+)/) {
						if (defined ${$s->{"conditions"}}{$1}) {
							push @ifs, -${$s->{"conditions"}}{$1};
							next;
						}
						push @ifs, 0;
					} elsif (/^#if\s+/) {
						push @ifs, 0;
					} elsif (/^#?endif/) {
						my $x = pop @ifs;
						defined $x or die "Improper nesting of conditionals";
						$x && next;
					} elsif (/^#?else/) {
						my $x = pop @ifs;
						defined $x or die "Improper nesting of conditionals";
						push @ifs, -$x;
						$x && next;
					}
				}
				@ifs && $ifs[$#ifs] < 0 && next;
				if (/^$/) {
					$empty && next;
					$empty = 1;
				} else { $empty = 0; }
			}
			print O;
		}
		close O;
		close I;
		! -x $f or chmod(0755, "$dir/$f") or die "chmod($dir/$f): $!";
	} else {
		`cp -a "$f" "$dir/$f"`; die if $?;
	}
}

sub GenPackage($) {
	my ($s) = @_;
	$s->{"PKG"} = $s->{"PACKAGE"} . "-" . $s->{"VERSION"};
	my $dd = $s->{"DISTDIR"};
	my $pkg = $s->{"PKG"};
	my $dir = "$dd/$pkg";
	print "Generating $dir\n";

	FILES: foreach my $f (`find . -type f`) {
		chomp $f;
		$f =~ s/^\.\///;
		my $action = "";
		my @rules = @{$s->{"rules"}};
		while (@rules) {
			my $rule = shift @rules;
			my $act = shift @rules;
			if ($f =~ $rule) {
				$action = $act;
				last;
			}
		}
		($action =~ /-/) && next FILES;
		print "$f ($action)\n" if $verbose;
		$s->CopyFile($f, $dir, $action);
	}

	foreach my $d (@{$s->{"directories"}}) {
		`mkdir -p $dir/$d`; die if $?;
	}

	if (-f "$dir/Makefile") {
		print "Cleaning up\n";
		`cd $dir && make distclean >&2`; die if $?;
	}

	print "Creating $dd/$pkg.tar.gz\n";
	my $tarvv = $verbose ? "vv" : "";
	`cd $dd && tar cz${tarvv}f $pkg.tar.gz $pkg >&2`; die if $?;
	push @{$s->{"distfiles"}}, "$dd/$pkg.tar.gz";

	if ($s->{'do_sign'}) {
		print "Signing package\n";
		system "gpg", "--armor", "--detach-sig", "$dd/$pkg.tar.gz";
		die if $?;
		rename "$dd/$pkg.tar.gz.asc", "$dd/$pkg.tar.gz.sign" or die "No signature produced!?\n";
		push @{$s->{"distfiles"}}, "$dd/$pkg.tar.gz.sign";
	}

	my $adir = $s->{"archivedir"};
	my $afile = "$adir/$pkg.tar.gz";
	print "Archiving to $afile\n";
	-d $adir or `mkdir -p $adir`;
	`cp $dd/$pkg.tar.gz $afile`; die if $?;

	return $dir;
}

sub GenFile($$) {
	my ($s,$f) = @_;
	my $sf = $s->{"DISTDIR"} . "/" . $s->{"PKG"} . "/$f";
	my $df = $s->{"DISTDIR"} . "/$f";
	print "Generating $df\n";
	`cp $sf $df`; die if $?;
	push @{$s->{"distfiles"}}, $df;
}

sub ParseOptions($) {
	my ($s) = @_;
	GetOptions(
		"verbose!" => \$verbose,
		"test!" => \$s->{"do_test"},
		"patch!" => \$s->{"do_patch"},
		"diff-against=s" => \$s->{"diff_against"},
		"upload!" => \$s->{"do_upload"},
		"sign!" => \$s->{"do_sign"},
	) || die "Syntax: release [--verbose] [--test] [--nopatch] [--diff-against=<version>] [--noupload] [--nosign]";
}

sub Test($) {
	my ($s) = @_;
	my $dd = $s->{"DISTDIR"};
	my $pkg = $s->{"PKG"};
	my $log = "$dd/$pkg.log";
	print "Doing a test compilation\n";
	`( cd $dd/$pkg && make ) >$log 2>&1`;
	die "There were errors. Please inspect $log" if $?;
	`grep -q [Ww]arning $log`;
	$? or print "There were warnings! Please inspect $log.\n";
	print "Cleaning up\n";
	`cd $dd/$pkg && make distclean`; die if $?;
}

sub MakePatch($) {
	my ($s) = @_;
	my $dd = $s->{"DISTDIR"};
	my $pkg1 = $s->{"PKG"};
	my $oldver;
	if ($s->{"diff_against"} ne "") {
		$oldver = $s->{"diff_against"};
	} elsif (defined $s->{"OLDVERSION"}) {
		$oldver = $s->{"OLDVERSION"};
	} else {
		print "WARNING: No previous version known. No patch generated.\n";
		return;
	}
	my $pkg0 = $s->{"PACKAGE"} . "-" . $oldver;

	my $oldarch = $s->{"archivedir"} . "/" . $pkg0 . ".tar.gz";
	-f $oldarch or die "MakePatch: $oldarch not found";
	print "Unpacking $pkg0 from $oldarch\n";
	`cd $dd && tar xzf $oldarch`; die if $?;

	my $diff = $s->{"PACKAGE"} . "-" . $oldver . "-" . $s->{"VERSION"} . ".diff.gz";
	print "Creating a patch from $pkg0 to $pkg1: $diff\n";
	`cd $dd && diff -ruN $pkg0 $pkg1 | gzip >$diff`; die if $?;
	push @{$s->{"distfiles"}}, "$dd/$diff";
}

sub Upload($) {
	my ($s) = @_;
	foreach my $u (@{$s->{"uploads"}}) {
		my $url = $u->{"url"};
		print "Upload to $url :\n";
		my @files = ();
		my $filter = $u->{"filter"} || ".*";
		foreach my $f (@{$s->{"distfiles"}}) {
			if ($f =~ $filter) {
				print "\t$f\n";
				push @files, $f;
			}
		}
		print "<confirm> "; <STDIN>;
		if ($url =~ m@^scp://([^/]+)(.*)@) {
			$, = " ";
			my $host = $1;
			my $dir = $2;
			$dir =~ s@^/~@~@;
			$dir =~ s@^/\./@@;
			my $cmd = "scp @files $host:$dir\n";
			`$cmd`; die if $?;
		} elsif ($url =~ m@ftp://([^/]+)(.*)@) {
			my $host = $1;
			my $dir = $2;
			open FTP, "|ftp -v $host" or die;
			print FTP "cd $dir\n";
			foreach my $f (@files) {
				(my $ff = $f) =~ s@.*\/([^/].*)@$1@;
				print FTP "put $f $ff\n";
			}
			print FTP "bye\n";
			close FTP;
			die if $?;
		} else {
			die "Don't know how to handle this URL scheme";
		}
	}
}

sub Dispatch($) {
	my ($s) = @_;
	$s->Test if $s->{"do_test"};
	$s->MakePatch if $s->{"do_patch"};
	$s->Upload if $s->{"do_upload"};
}

1;
