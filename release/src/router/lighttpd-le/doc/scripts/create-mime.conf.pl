#!/usr/bin/perl -w

# Based on create-mime.assign.pl in debian lighttpd (1.4.x) package
# Creates an example mime.conf from /etc/mime.types

use strict;

# text/* subtypes to serve as "text/...; charset=utf-8"
# text/html IS NOT INCLUDED: html has its own method for defining charset
#   (<meta>), but the standards specify that content-type in HTTP wins over
#   the setting in the html document.
my %text_utf8 = map { $_ => 1 } qw(
	css
	csv
	plain
	x-bibtex
	x-boo
	x-c++hdr
	x-c++src
	x-chdr
	x-csh
	x-csrc
	x-dsrc
	x-diff
	x-haskell
	x-java
	x-lilypond
	x-literate-haskell
	x-makefile
	x-moc
	x-pascal
	x-perl
	x-python
	x-scala
	x-sh
	x-tcl
	x-tex
);

# map extension to hash which maps types to the type they should be replaced with
my %manual_conflicts_resolve = (
	'.ra' => {
		'audio/x-pn-realaudio' => 'audio/x-realaudio',
	},
);

open MIMETYPES, "/etc/mime.types" or die "Can't open /etc/mime.types: $!";

my %extensions;
sub set {
	my ($extension, $mimetype) = @_;
	$extensions{$extension} = $mimetype;
}
sub add {
	my ($extension, $mimetype) = @_;
	my $have = $extensions{$extension};

	my $r = $manual_conflicts_resolve{$extension};
	# update @_ too for calls to set
	$_[1] = $mimetype = $r->{$mimetype} if $r && $r->{$mimetype};

	# mime.types can have same extension for different mime types
	if ($have) {
		# application/octet-stream means we couldn't resolve another conflict
		return if $have eq $mimetype || $have eq 'application/octet-stream';

		my ($have_type, $have_subtype) = split /\//, $have, 2;
		my ($type, $subtype) = split /\//, $mimetype, 2;

		my $have_x = ($have_type =~ /^x-/ || $have_subtype =~ /^x-/);
		my $x = ($type =~ /^x-/ || $subtype =~ /^x-/);

		# entries without x- prefix in type/subtype win:
		if ($have_x && !$x) {
			return set @_; # overwrite
		} elsif ($x && !$have_x) {
			return; # ignore
		}

		# text/ wins over application/ for same subtype
		if ($subtype eq $have_subtype) {
			if ($type eq "text" && $have_type eq "application") {
				return set @_; # overwrite
			} elsif ($have_type eq "text" && $type eq "application") {
				return; # ignore
			}
		}

		print STDERR "Duplicate mimetype: '${extension}' => '${mimetype}' (already have '${have}'), merging to 'application/octet-stream'\n";
		set ($extension, 'application/octet-stream');
	} else {
		set @_;
	}
}

sub print_type {
	my ($extension, $mimetype) = @_;
	if ($mimetype =~ /^text\/(.*)$/) {
		$mimetype .= "; charset=utf-8" if $text_utf8{$1};
	}

	print "\t\"${extension}\" => \"${mimetype}\",\n";
}

while (<MIMETYPES>) {
	chomp;
	s/\#.*//;
	next if /^\w*$/;
	if (/^([a-z0-9\/+-.]+)\s+((?:[a-z0-9.+-]+[ ]?)+)$/i) {
		my $mimetype = $1;
		my @extensions = split / /, $2;

		foreach my $ext (@extensions) {
			add(".${ext}", $mimetype);
		}
	}
}

# missing in /etc/mime.types;
# from http://www.iana.org/assignments/media-types/media-types.xhtml
add(".dtd", "application/xml-dtd");

# other useful mappings
my %useful = (
	".tar.gz"  => "application/x-gtar-compressed",
	".gz"      => "application/x-gzip",
	".tbz"     => "application/x-gtar-compressed",
	".tar.bz2" => "application/x-gtar-compressed",
	".bz2"     => "application/x-bzip",
	".log"     => "text/plain",
	".conf"    => "text/plain",
	".spec"    => "text/plain",
	"README"   => "text/plain",
	"Makefile" => "text/x-makefile",
);

while (my ($ext, $mimetype) = each %useful) {
	add($ext, $mimetype) unless $extensions{$ext};
}


print <<EOF;
# created by create-mime.conf.pl

#######################################################################
##
##  MimeType handling
## -------------------
##
## https://redmine.lighttpd.net/projects/lighttpd/wiki/Mimetype_assignDetails

##
## mimetype.xattr-name
## Set the extended file attribute name used to obtain mime type
## (must also set mimetype.use-xattr = "enable")
##
## Default value is "Content-Type"
##
## freedesktop.org Shared MIME-info Database specification suggests
## user-defined value ("user.mime_type") as name for extended file attribute
#mimetype.xattr-name = "user.mime_type"

##
## Use extended attribute named in mimetype.xattr-name (default "Content-Type")
## to obtain mime type if possible
##
## Disabled by default
##
#mimetype.use-xattr = "enable"

##
## mimetype ("Content-Type" HTTP header) mapping for static file handling
##
## The first matching suffix is used. If no mapping is found
## 'application/octet-stream' is used, and caching (etag/last-modified handling)
## is disabled to prevent clients from caching "unknown" mime types.
##
## Therefore the last mapping is:
##   "" => "application/octet-stream"
## This matches all extensions and acts as default mime type, and enables
## caching for those.
mimetype.assign = (
EOF

# sort "x-" and "vnd." prefixed names after everything else
sub mimecmpvalue {
	my ($mimetype) = @_;
	$mimetype =~ s/(^|\/)(x-|vnd\.)/~$1$2/g;
	return $mimetype;
}
sub countdots {
	my ($s) = @_;
	return scalar(() = $s =~ /\./g);
}
# the first matching suffix wins, so we have to sort by "length"
# as all extensions start with "." we use the number of "."s as length
# the exceptions are "README" and "Makefile" which are assumed not to conflict
#  (i.e. are not a suffix of any other extension)
for my $ext (sort { countdots($b) <=> countdots($a) || mimecmpvalue($extensions{$a}) cmp mimecmpvalue($extensions{$b}) || $a cmp $b } keys(%extensions)) {
	print_type($ext, $extensions{$ext});
}

print <<EOF;

	# enable caching for unknown mime types:
	"" => "application/octet-stream"
)
EOF
