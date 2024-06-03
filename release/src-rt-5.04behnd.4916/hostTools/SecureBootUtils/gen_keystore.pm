#!/usr/bin/env perl
package gen_keystore;
#use parent 'BRCM::SBI';
use parent 'BRCM::SBI_UTIL';
use strict;
#use warnings;
use bytes;
use Encode;
use Getopt::Long;
use FindBin qw($Bin);
use lib qw($Bin $Bin/../PerlLib);

use File::stat;

# Arguments 
#

sub new {
	my $class = shift;
	my ($byteorder, $secarch, $verbose) = @_;
	if (!defined $byteorder) {
		$byteorder = 'little';
	} 
	if (!defined $secarch) {
		$secarch = 'GEN3';
	} 
	if (!defined $verbose) {
		$verbose = 0;
	}
	my $self = bless {
		byteorder => $byteorder,
		verbose => $verbose,
		class => $class,
		secarch => $secarch,
      }, $class;
      # initialize parent openssl var 
      $self->{openssl} = "openssl"; 
      return $self;
}

sub build_keystore
{
	my $self = shift;
	my ($args, $fout) = @_;
	my $bytes;
	my $key_data;
	my $key_info;
	my $crc = 0;

	my $acs_key_info = BRCM::SBI_UTIL->ByteAccessor(\$key_info,$self->{'byteorder'});
	if (exists $args->{'mid'} ) {
		$key_data =undef;
		$self->set_val_at(\$key_data, 0, $args->{'mid'},'u16','big');
		$acs_key_info->set_val(length $key_data, 'u16');
		$acs_key_info->set_val(0x0007, 'u16');
		$acs_key_info->append($key_data);
	}
	if (exists $args->{'oid'} ) {
		$self->set_val_at(\$key_data, 0, $args->{'oid'},'u16');
		$acs_key_info->set_val(length $key_data, 'u16');
		$acs_key_info->set_val(0x0008, 'u16');
		$acs_key_info->append($key_data);
	}
	if (exists $args->{'fld'}
			and exists $args->{'fld'}->{'enc'}) {
		my $fld_ek = $self->_encrypt_aes_128_cbc($args->{'fld'}->{'enc'}->{'aes_k'}, 
							$args->{'fld'}->{'enc'}->{'aes_iv'},
							$args->{'fld'}->{'kroe_ek'});
		$acs_key_info->set_val(length $fld_ek, 'u16');
		# key_info->type_state 
		$acs_key_info->set_val(0x0300, 'u16');
		$acs_key_info->append($fld_ek);

		# aes iv

		my $fld_iv = $self->_encrypt_aes_128_cbc($args->{'fld'}->{'enc'}->{'aes_k'}, 
							$args->{'fld'}->{'enc'}->{'aes_iv'},
							$args->{'fld'}->{'kroe_iv'});
		$acs_key_info->set_val(length $fld_iv, 'u16');
		$acs_key_info->set_val(0x0301, 'u16');
		$acs_key_info->append($fld_iv);

		# hash 
		$self->set_val_at(\$key_data, 0,$args->{'fld'}->{'hash'},'s');
		$acs_key_info->set_val(length $args->{'fld'}->{'hash'}, 'u16');
		$acs_key_info->set_val(0x0006, 'u16');
		$acs_key_info->append($key_data);
		#calculate signature
		my $sig = $self->_sign_sha256($args->{'fld'}->{'enc'}->{'rsa_pub'}, $key_info);
		$key_info = $sig.$key_info;
	}
	#printf("sig %d  key_data %d  key_info %d \n",length $sig, length $key_data, length $key_info);
	#printf("%x\n",$mid);
	#calc crc
	if (defined $key_info and (length $key_info) > 2) {
		$self->set_val_at(\$crc, 0, $self->crc32($key_info),'u32');
		$bytes = $crc.$key_info;
		#printf("crc 0x%s sz 0x%08x  key_info size %d\n",unpack("H*",$crc) ,length $crc, length $key_info);
	}

	# building header

	# keystore magic
	my $header = '\000';
	my $acs_hdr = BRCM::SBI_UTIL->ByteAccessor(\$header,$self->{'byteorder'});
	$acs_hdr->set_val('BRCMKEYSTORE','s');
	# sec_arch
	$acs_hdr->set_val($args->{'arch'},'u8');
	# sec request
	#state
	$acs_hdr->set_val($args->{'req'},'u8');
	#abort_delay
	$acs_hdr->set_val($args->{'abort_timeout'},'u8');
	#info_size
	printf ("bytes %d\n",length $bytes);
	$acs_hdr->set_val(defined $bytes?length $bytes:0, 'u16');
	#header crc 
	$acs_hdr->set_val($self->crc32($header),'u32');
	if (defined $bytes and length $bytes > 2) {
		$bytes = $header.$bytes;
	} else {
		$bytes = $header;
		printf(" bytes length %d \n",length $bytes);
	}

	$self->fdump($fout, $bytes);
}

# main call
sub build {
	my $self = shift;
	my ($args, $f_out, $sec_data_path)=@_;
	my %arch=(GEN1=>1,
		GEN2=>2,
		GEN3=>3);
	my %req=(GEN2BTRM=>1,
		GEN2MFG=>2,
		GEN2OP=>3,
		GEN3MFG=>4,
		GEN3FLD=>5);
	# to encrypt keys
	#2 byte hex numeral
	my $f_krsa_mfg_pub = $sec_data_path.'/Krsa-mfg.pem';
	my $f_kaes_mfg_ek = $sec_data_path.'/Kaes-mfg-ek.bin';
	my $f_kaes_mfg_iv = $sec_data_path.'/Kaes-mfg-iv.bin';
	my $f_kroe_fld_ek = $sec_data_path.'/Kroe-fld-ek.bin';
	my $f_kroe_fld_iv = $sec_data_path.'/Kroe-fld-iv.bin';
	my $f_hmid_rot_fld_pub_hash = $sec_data_path.'/Hmid-rot-fld-pub.bin';
	print "SEC:$args->{'secarc'} REQ:$args->{'secreq'} out: $f_out \n";
#gen args
		if ($args->{'secarc'}.$args->{'secreq'} eq 'GEN3FLD'
				and exists $args->{'mid'}) {
			$self->build_keystore({ 
				arch=>$arch{$args->{'secarc'}},
				req=>$req{$args->{'secarc'}.$args->{'secreq'}},
				mid=>hex $args->{'mid'},
				abort_timeout=>int $args->{'abort_timeout'},
				fld=>{kroe_ek=>$self->f2var($f_kroe_fld_ek),
					kroe_iv=>$self->f2var($f_kroe_fld_iv),
					hash=>$self->f2var($f_hmid_rot_fld_pub_hash),
					enc=>{	aes_k=>$self->f2hex($f_kaes_mfg_ek),
						aes_iv=>$self->f2hex($f_kaes_mfg_iv),
						rsa_pub=>$f_krsa_mfg_pub}},
					},$f_out);
		} elsif ($args->{'secarc'}.$args->{'secreq'} eq 'GEN3MFG') {
			$self->build_keystore({ arch=>$arch{$args->{'secarc'}},
				req=>$req{$args->{'secarc'}.$args->{'secreq'}},
				abort_timeout=>hex $args->{'abort_timeout'},
				},
				$f_out);
		} elsif ($args->{'secarc'}.$args->{'secreq'} eq 'GEN2BTRM') {
			$self->build_keystore({ arch=>$arch{$args->{'secarc'}},
				req=>$req{$args->{'secarc'}.$args->{'secreq'}},
				abort_timeout=>hex $args->{'abort_timeout'}, },
				$f_out);
		} elsif ($args->{'secarc'}.$args->{'secreq'} eq 'GEN2MFG' and exists $args->{'mid'}) {
			$self->build_keystore({ arch=>$arch{$args->{'secarc'}},
				req=>$req{$args->{'secarc'}.$args->{'secreq'}},
				abort_timeout=>hex $args->{'abort_timeout'},
				mid=>hex $args->{'mid'},
				},
				$f_out);
		} elsif ($args->{'secarc'}.$args->{'secreq'} eq 'GEN2OP' and exists $args->{'oid'}) {
			$self->build_keystore({ arch=>$arch{$args->{'secarc'}},
				req=>$req{$args->{'secarc'}.$args->{'secreq'}},
				abort_timeout=>hex $args->{'abort_timeout'},
				oid=>hex $args->{'oid'}, },
				$f_out);
		} else {
			die "Unsupported request/architecture ";
		}
}
1;
