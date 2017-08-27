#!/usr/bin/env perl


use Digest::CRC;
use Convert::Binary::C;
#use Data::Dumper;
use bytes;
use Getopt::Long;

my $configfile="";

sub dec2bin {

	$c = Convert::Binary::C->new(%config);
	$struct=$c->parse(<<'CCODE');
	  struct crc {
	    unsigned long crc;
	    unsigned long c;
	  };
CCODE


	$struct->{'crc'}=shift;
	open F, ">./size.tmp" or die "Error ";
	binmode F;
	print F  $c->pack('crc', $struct);
	close(F);

}

sub crc2bin {

	open F, "<./unsigned_cfesbi.tmp"  or die "Open failed ./unsigned_cfesbi.tmp\n";
	binmode F;
	my $binary;
	while(<F>)
	{
		$binary=$binary.$_;
	}
	close(F);
	my $ctx = new Digest::CRC(width => 32, poly => 0x04c11db7, init => 0xffffffff, xorout => 0, refin => 1, refout => 1);
	$ctx->add($binary);
	my $csum=$ctx->digest;
	open F, ">crc.tmp" or die "crc.tmp open failed\n";
	binmode F;

	$format='I';
	if($config{'ByteOrder'})
	{
		if ($config{'ByteOrder'} eq "BigEndian")
		{
			$format='N';
		}
	}
	print F pack($format ,$csum);
	close(F); 


};


my $arg_mapper = {
    'dec2bin' => sub { $val=shift;dec2bin($val);  },
    'crc2bin' => sub { crc2bin(); }
};



Getopt::Long::Configure("no_ignore_case", "prefix_pattern=(--|-|\/)");
GetOptions("config=s" => \$configfile);

if (! $configfile eq "" )
{
        require $configfile;
}

my $fp=$arg_mapper->{$ARGV[0]};
if(ref($fp))
{
	$fp->($ARGV[1]);
}
else
{
	die "called with unrecognized operation $ARGV[1]\n";
}
