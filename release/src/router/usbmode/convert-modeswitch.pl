#!/usr/bin/perl
use strict;

my $msg_ctr = 0;
my %messages;
my %devices;

sub add_message {
	my $msg = shift;
	my $val = $messages{$msg};

	$val or do {
		$val = $msg_ctr++;
		$messages{$msg} = $val;
	};

	return $val;
}

sub add_device($) {
	my $id = shift;
	my $dev = {};
	my $match;

	$id =~ /^(\w{4}:\w{4})(:.*)?/ or do {
		warn "Invalid device ID string $id\n";
		return $dev;
	};

	$id = $1;
	$match = $2 or $match = "*";

	$devices{$id} or $devices{$id} = {};
	$devices{$id}->{$match} = $dev;

	return $dev;
}

sub add_hex {
	$_[0] =~ s/^0x//;
	return hex($_[0]);
}

sub add_mode {
	$_[1] =~ s/^(\w+)Mode$/$1/;
	return $_[1];
}

my $hex_option = [ undef, \&add_hex ];
my $msg_option = [ undef, \&add_message ];
my $mode_option = [ "Mode", \&add_mode ];
my %options = (
	TargetVendor => $hex_option,
	TargetProductList => [ "TargetProduct", sub { return [ map(hex,split(/,/, $_[0])) ]; } ],
	TargetProduct => [ "TargetProduct", sub { return [ hex($_[0]) ]; } ],
	TargetClass => $hex_option,
	MessageContent => $msg_option,
	MessageContent2 => $msg_option,
	MessageContent3 => $msg_option,
	WaitBefore => [ ],
	DetachStorageOnly => [ ],
	MBIM => $mode_option,
	HuaweiMode => $mode_option,
	HuaweiNewMode => $mode_option,
	SierraMode => $mode_option,
	SonyMode => $mode_option,
	QisdaMode => $mode_option,
	GCTMode => $mode_option,
	KobilMode => $mode_option,
	SequansMode => $mode_option,
	MobileActionMode => $mode_option,
	CiscoMode => $mode_option,
	StandardEject => $mode_option,
	NoDriverLoading => [],
	MessageEndpoint => $hex_option,
	ReleaseDelay => [],
	NeedResponse => [],
	ResponseEndpoint => $hex_option,
	ResetUSB => [],
	InquireDevice => [],
	CheckSuccess => $hex_option,
	Interface => $hex_option,
	Configuration => $hex_option,
	AltSetting => $hex_option,
);

sub parse_file($) {
	my $file = shift;
	my $id;

	$id = $file;
	$file =~ /\/?([^\/]+)$/ and $id = $1;

	my $dev = add_device($id);

	open FILE, "<$file" or die "Cannot open file '$file'\n";
	while (<FILE>) {
		chomp;
		s/^\s*(.+?)\s*$/$1/;
		s/#.+$//;
		next unless /\w/;
		/(\w+)\s*=\s*(.+)\s*/ or do {
			warn "Invalid Line: $_";
			next;
		};

		my ($var, $val) = ($1, $2);
		$val =~ s/^"(.+)"$/$1/;

		my $opt = $options{$var};
		$opt or do {
			warn "Unrecognized option $var in file $file\n";
			next;
		};

		$opt->[1] and $val = &{$opt->[1]}($val, $var);
		$opt->[0] and $var = $opt->[0];
		$dev->{$var} = $val;
	}
}

foreach my $file (@ARGV) {
	parse_file $file;
}

sub json_chr {
	my $chr = shift;

	$chr eq "\b" and return "\\b";
	$chr eq "\n" and return "\\n";
	$chr eq "\r" and return "\\r";
	$chr eq "\t" and return "\\t";
	$chr eq "\\" and return "\\\\";
	$chr eq "\"" and return "\\\"";
	$chr eq '/' and return "\\/";
	return sprintf("\\u%04x", ord($chr));
};

sub json_str {
	$_[0] =~ s/([\x00- \/"\\])/json_chr($1)/eg;
	return $_[0];
}

sub json_val($$) {
	my ($val, $type) = (shift, shift);
	$type eq 'bool' and $val = $val > 0 ? "true" : "false";
	$type eq 'string' and $val = "\"$val\"";
	return $val;
}

sub dev_opt {
	my ($val, $name, $type, $sep) = (shift, shift, shift, shift);
	return unless defined($val);
	if ($type =~ /array:(.+)/) {
		$type = $1;
		my @val = @$val;
		undef $val;
		foreach my $elem (@val) {
			my $json = json_val($elem, $type);
			next unless defined $json;
			if (defined $val) {
				$val = "$val, $json"
			} else {
				$val = $json;
			}
		}
		$val = "[ $val ]";
	} else {
		$val = json_val($val, $type);
	}
	print "$$sep\t\t\t\t\"".json_str($name)."\": $val";
	$$sep = ",\n";
}

print <<EOF;
{
	"messages" : [
EOF
my $suffix = "";
foreach my $msg (sort { $messages{$a} <=> $messages{$b} } keys %messages) {
	print "$suffix\t\t\"".json_str($msg)."\"";
	$suffix = ",\n";
}
print <<EOF;

	],

	"devices" : {
EOF
my $dev_sep = "";
foreach my $devid (sort keys %devices) {
	my $dev = $devices{$devid};

	print "$dev_sep\t\t\"".json_str($devid)."\": {\n";
	$dev_sep = ",\n";

	my $match_sep = "";
	foreach my $match (sort keys %$dev) {
		my $cur = $dev->{$match};
		my $sep = "";

		print "$match_sep\t\t\t\"".json_str($match)."\": {\n";
		$match_sep = ",\n";

		dev_opt($cur->{TargetVendor}, "t_vendor", "int", \$sep);
		dev_opt($cur->{TargetProduct}, "t_product", "array:int", \$sep);
		dev_opt($cur->{TargetClass}, "t_class", "int", \$sep);
		dev_opt($cur->{DetachStorageOnly}, "detach_storage", "bool", \$sep);
		dev_opt($cur->{Mode}, "mode", "string", \$sep);
		dev_opt($cur->{NoDriverLoading}, "no_driver", "bool", \$sep);
		dev_opt($cur->{MessageEndpoint}, "msg_endpoint", "int", \$sep);
		my $msg = [
			$cur->{MessageContent},
			$cur->{MessageContent2},
			$cur->{MessageContent3}
		];
		dev_opt($msg, "msg", "array:int", \$sep);
		dev_opt($cur->{WaitBefore}, "wait", "int", \$sep);
		dev_opt($cur->{ReleaseDelay}, "release_delay", "int", \$sep);
		dev_opt($cur->{NeedResponse}, "response", "bool", \$sep);
		dev_opt($cur->{ResponseEndpoint}, "response_endpoint", "int", \$sep);
		dev_opt($cur->{ResetUSB}, "reset", "bool", \$sep);
		dev_opt($cur->{InquireDevice}, "inquire", "int", \$sep);
		dev_opt($cur->{CheckSuccess}, "check", "bool", \$sep);
		dev_opt($cur->{Interface}, "interface", "int", \$sep);
		dev_opt($cur->{Configuration}, "config", "int", \$sep);
		dev_opt($cur->{AltSetting}, "alt", "int", \$sep);
		print "\n\t\t\t}";
	}
	print "\n\t\t}"
}
print <<EOF;

	}
}
EOF
