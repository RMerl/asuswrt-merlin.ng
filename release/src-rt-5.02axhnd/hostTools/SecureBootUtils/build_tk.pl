#!/usr/bin/env perl
package build_tk;
use strict;
use warnings;
use bytes;

use FindBin qw($Bin);
use lib "$Bin";
use lib "$Bin/../PerlLib";
use parent qw(BRCM::SBI_UTIL);
use BRCM::SBI_DEP;
use gen_keystore;
use File::stat;
use Getopt::Long;

my $sec_arch;
my $byteorder;
my $tk_req;
my $abort_timeout;
my $mid;

my $cred_dir;
my $key_store_offs;
my $wdir;
my $image_in;
my $image_out;
my $boot_offset;
my $sbi_boot_size;
my $boot_part_size;


my $chip;
my $media;
my $cferom;

GetOptions("tk_req=s", \$tk_req, "sec_arch=s", \$sec_arch, "byteorder=s", \$byteorder, 
	"abort_timeout=s", \$abort_timeout, "mid=s", \$mid, 
	"keystore_offs_kb=s", \$key_store_offs,"boot_offs_kb=s",\$boot_offset,
	"cred_dir=s",\$cred_dir, "wdir=s",  \$wdir, "image=s", \$image_in, 
        "sbi_boot_size_kb=s",\$sbi_boot_size, "boot_part_size_kb=s",\$boot_part_size, 
	"cferom=s", \$cferom, "out=s", \$image_out,"media=s",\$media,"chip=s",\$chip
	) or die("bad option");

my %args=("6858"=>{"tk_req"=>'FLD',"sec_arch"=>"GEN3",byteorder=>"little",
		"mid"=>"1234","keystore_offs_kb"=>1016,"boot_offs_kb"=>64,
		"abort_timeout"=>0,"media"=>"nand", "sbi_boot_size_kb"=>96,
		"cred_dir"=>"cfe/cfe/board/bcm63xx_btrm/data/gen3_common"},
        "4908"=> {"tk_req"=>"FLD","sec_arch"=>"GEN3",byteorder=>"little",
		"mid"=>"1234","keystore_offs_kb"=>1016,"boot_offs_kb"=>64,
		"abort_timeout"=>9,"media"=>"nand", "sbi_boot_size_kb"=>96,
		"cred_dir"=>"cfe/cfe/board/bcm63xx_btrm/data/gen3_common"},
        "63158"=> {"tk_req"=>"FLD","sec_arch"=>"GEN3",byteorder=>"little",
		"mid"=>"1234","keystore_offs_kb"=>1016,"boot_offs_kb"=>64,
		"abort_timeout"=>1,"media"=>"nand", "sbi_boot_size_kb"=>96,
		"cred_dir"=>"cfe/cfe/board/bcm63xx_btrm/data/gen3_common"},
        "6856"=> {"tk_req"=>"FLD","sec_arch"=>"GEN3",byteorder=>"little",
		"mid"=>"1234","keystore_offs_kb"=>1016,"boot_offs_kb"=>64,
		"abort_timeout"=>1,"media"=>"nand", "sbi_boot_size_kb"=>96,
		"cred_dir"=>"cfe/cfe/board/bcm63xx_btrm/data/gen3_common"},
        "6846"=> {"tk_req"=>"FLD","sec_arch"=>"GEN3",byteorder=>"little",
		"mid"=>"1234","keystore_offs_kb"=>1016,"boot_offs_kb"=>64,
		"abort_timeout"=>1,"media"=>"nand", "sbi_boot_size_kb"=>96,
		"cred_dir"=>"cfe/cfe/board/bcm63xx_btrm/data/gen3_common"}
        );
sub new {
	my $class = shift;
	my ($args, $verbose) = @_;
	if (!defined $verbose ) {
		$verbose = 0;
	}
	#print "class: $class byteorder:$byteorder verbose:$verbose\n";
	my $self = bless {
		verbose => $verbose,
		class => $class,
	}, $class;
	$self->process_opts($args);
	$self->{'byteorder'} = $args->{'byteorder'},
	print "wdir $wdir\n";
	if (!defined $cferom) {
		print "Updating $image_in with keystore at $wdir\n";
		my $st = $self->gen_keystore($args->{$chip},$wdir);
		my $data = $self->f2var($image_in);
		#$self->set_val_at(\$data, ($key_store_offs*1024), $keystore,'s',$args->{byteorder});
		printf( "Data length %d %d\n", length $data, length $st->{'keystore'});
		$self->set_val_at(\$data, $key_store_offs*1024, 
				$st->{keystore},
				's',
				$args->{$chip}->{byteorder});
		$self->fdump($image_out, $data);
	} else { 
		$self->update_image($args->{$chip}, $boot_part_size*1024,
			$self->gen_sbi($args->{$chip}, $chip, $cferom, $wdir),
			$self->gen_keystore($args->{$chip},$wdir), 
			$image_in, $image_out);
	}
}

sub gen_keystore {
        my $self = shift;
	my $arg = shift;
	my $workdir = shift;

	my $image = "$workdir/keystore.bin.$$";
	local *nrmz = sub{ my $var=shift; $var=~s/\"//g; return $var;};
	my %sec_arg = ('secarc'=>$arg->{'sec_arch'},
		'secreq'=>$arg->{'tk_req'},
		'byteorder'=>$arg->{'byteorder'},
		'abort_timeout'=>$arg->{'abort_timeout'},
		'mid'=>$arg->{'mid'});
	#my $sec_dir = nrmz($sec_key_dir);
	my $kst_obj = gen_keystore->new($sec_arg{'byteorder'},$arg->{'sec_arch'}); 
    	$kst_obj->build(\%sec_arg, $image, nrmz($arg->{'cred_dir'}));
	my %data = ("keystore"=>$self->f2var($image));
	#$self->run_shell("rm -f $image");
	return \%data;
}

sub gen_sbi {
        my $self = shift;
	my ($arg, $chip, $in, $workdir) = @_;
	my $sbi = "$workdir/.sbi.mfg.$$";
	my $ubi = "$workdir/.sbi.unsec.$$";
	my $sbi_lib = BRCM::SBI_DEP->new( {'sec_mode' => "UNSEC",
				'chip'=>$chip,
				'sec_arch'=>"GEN3",
				'out' => $ubi,
				'in' => $in,
				'byteorder' => $arg->{'byteorder'}});
	$sbi_lib->build(); 
	if ( ! (-e $ubi) ) {
		die "The file $sbi was not successfully created. Exiting.";
	}
	$sbi_lib = BRCM::SBI_DEP->new( {'sec_mode' => "MFG",
				'chip'=>$chip,
				'sec_arch'=>"GEN3",
				'out' => $sbi,
				'in' => $in,
				'sec_opt' => "encrypt",
				'cred_dir' => $arg->{'cred_dir'},
				'byteorder' => $arg->{'byteorder'}});
	$sbi_lib->build(); 
	if ( ! (-e $sbi) ) {
		die "The file $sbi was not successfully created. Exiting.";
	}
	my %data = ("unsec"=>$self->f2var("$ubi"), "mfgsec"=>$self->f2var("$sbi"));
	$self->run_shell("rm -f $sbi $ubi");
	return \%data;
}

sub update_image {
        my $self = shift;
	my ($arg, $split_size, $sbi, $tk, $img_in, $img_out) = @_;
	my %data=("left"=>"","right"=>"");
	my $TOKEN_LEN = 20;
	my $crc;
	$self->fsplit($split_size, $image_in,\%data);
	$self->set_val_at(\$data{'left'}, $arg->{'boot_offs_kb'}*1024+4096, $sbi->{'unsec'} ,'s',$arg->{'byteorder'});
	$self->set_val_at(\$data{'left'}, ($arg->{'boot_offs_kb'}+$arg->{'sbi_boot_size_kb'})*1024, $sbi->{'mfgsec'} ,'s',$arg->{'byteorder'});
	$self->set_val_at(\$data{'left'}, $arg->{'keystore_offs_kb'}*1024, $tk->{'keystore'},'s',$arg->{'byteorder'});
	my $bytes = $data{'left'}.$data{'right'};
	$data{'left'} = substr $bytes, 0, (length $bytes)-$TOKEN_LEN;
	$data{'right'} = substr $bytes, (length $bytes)-($TOKEN_LEN-4), $TOKEN_LEN-4;
	$self->set_val_at(\$crc, 0, $self->crc32($data{'left'}),'u32',$arg->{'byteorder'});
	$bytes = $data{'left'}.$crc.$data{'right'};
	print "Created image : $img_out\n";
	$self->fdump($img_out, $bytes);
}

sub process_opts {
        my $self = shift;
	my $arg = shift;

	local *assign = sub { 
				my ($lval, $rval, $defval)=@_; 
				if (defined  $defval) {
     					${$lval}=defined $rval?$rval:$defval;
				} else {
					#assuming lval is defined and set  
					if (defined $rval) {
     						${$lval} = $rval;
					}
				}
				#print "${$lval}\n";
        }; 
	if ( !defined $chip or !defined $image_in 
		or !defined $boot_part_size) {
     		die("ERROR: specify at least 4 arguments: chip, image, cferom, boot_part_size");
	}
	assign(\$arg->{$chip}->{'tk_req'}, $tk_req);
	assign(\$arg->{$chip}->{"sec_arch"}, $sec_arch);
	assign(\$arg->{$chip}->{"byteorder"}, $byteorder);
	assign(\$arg->{$chip}->{"abort_timeout"}, $abort_timeout);
	assign(\$arg->{$chip}->{"mid"}, $mid);
	assign(\$arg->{$chip}->{"keystore_offs_kb"}, $key_store_offs);
	assign(\$arg->{$chip}->{"boot_offs_kb"}, $boot_offset);
	assign(\$arg->{$chip}->{'sbi_boot_size_kb'}, $sbi_boot_size);
	assign(\$arg->{$chip}->{"media"}, $media);
	assign(\$arg->{$chip}->{"cred_dir"}, $cred_dir);
	assign(\$image_out, $image_out,"$image_in.tk");
	assign(\$wdir, $wdir, ".");
}

build_tk->new(\%args);
1;
