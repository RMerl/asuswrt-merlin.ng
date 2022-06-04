#!/usr/bin/env perl 
package sbi_lib;
use strict;
use warnings;
use bytes;

use FindBin qw($Bin);
use lib "$Bin";
use BrcmSecUtils;
use File::stat;
use vars qw(%impl_features);
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS);
$VERSION = 1.00;
@ISA = qw(Exporter);
@EXPORT = ();
@EXPORT_OK = qw(build);
%EXPORT_TAGS = (DEFALUT=> [qw(&build) ],
		both=> [qw(&build)]);



#typedef struct
#{
#   uint16_t     type;
#   uint16_t     ver; /* ver 2 for 6858 because sig covers the mid below */
#   uint16_t     len;
#   uint16_t     reserve;
#   uint8_t      sig[SEC_S_SIGNATURE];
#   uint8_t      kroe2MfgPub[SEC_S_MODULUS];
#   uint8_t      encrKroe2MfgPriv[ENCRYPTED_KROE2_DATA_LEN]; /* AES encrypted with Kroe1-mfg */
#   uint16_t     mid;
#   /* padding left out on purpose */
#} __attribute__((packed)) MfgRoeCot;

#typedef struct
#{
#   uint16_t     type;
#   uint16_t     ver;
#   uint16_t     len;
#   uint16_t     config;
#   uint8_t      sig[SEC_S_SIGNATURE];
#   uint16_t     mid;
#   uint8_t      krsaMfgPub[SEC_S_MODULUS];
#   uint8_t      encrMidAndKaesMfg[ENCRYPTED_MID_KAES_DATA_LEN]; /* RSA encrypted with Kroe2-mfg-pub */
#   /* padding left out on purpose */
#} __attribute__((packed)) MfgOemCot;

#typedef struct
#{
#   uint16_t     type;
#   uint16_t     ver;
#   uint16_t     len;
#   uint16_t     config;
#   uint16_t     mid;
#   uint8_t      krotFldPub[SEC_S_MODULUS];
#   /* padding left out on purpose */
#} __attribute__((packed)) FldRotCot;

#typedef struct
#{
#   uint16_t     type;     
#   uint16_t     ver;      
#   uint16_t     len;      
#   uint16_t     config;   
#   uint8_t      sig[SEC_S_SIGNATURE];
#   uint8_t      krsaFldPub[SEC_S_MODULUS];
#   uint8_t      encrKaesFld[ENCRYPTED_KAES_FLD_DATA_LEN]; /* AES encrypted with Kroe-fld */
#   /* padding left out on purpose */
#} __attribute__((packed)) FldOemCot;
#typedef struct                                                                                      
#{                                                                                
#   uint32_t     magic_1;                                                                           
#   uint32_t     magic_2;                                                                        
#   uint32_t     ver;                                                                           
#   uint32_t     modeElegible;                                                                 
#   uint32_t     hdrLen;                                                                          
#   uint32_t     sbiSize;                                                                
#} __attribute__((packed)) SbiUnauthHdrBeginning;                                            
                                                                               
#typedef struct                                                    
#{                                                                                                  
#   uint32_t     ver;                                                                          
#   uint32_t     hdrLen;                                                                  
#   uint32_t     authLen;                                                                
#   uint32_t     mfgRoeCotOfs;                                                            
#   uint32_t     mfgOemCotOfs;                                                            
#   uint32_t     fldRotCotOfs;                                                           
#   uint32_t     fldOemCotOfs;                                                                     
#} __attribute__((packed)) SbiAuthHdrBeginning;
# SBI headers to build 


#input: args
sub mfgOemCotConfig {
	my $opt = $_[0];
	if (defined $opt->{'skp'} and defined $opt->{'spu'}) {
		return defined $opt->{'encrypt'}?7:6;
	}
	if (!defined $opt->{'skp'} and !defined $opt->{'spu'}) {
		return defined $opt->{'encrypt'}?1:0;
	} 
	if (defined $opt->{'skp'} and !defined $opt->{'spu'}) {
		return defined $opt->{'encrypt'}?3:2;
	} 
	if (!defined $opt->{'skp'} and defined $opt->{'spu'}) {
		return defined $opt->{'encrypt'}?5:4;
	}
	return;
};

sub fldRotConfig {
	my $opt = $_[0];
	if (defined $opt->{'encrypt'} and defined $opt->{'oem'}) {
		return 3;
	}
	if (!defined $opt->{'encrypt'} and defined $opt->{'oem'}) {
		return 2;
	}
	if (defined $opt->{'encrypt'} and !defined $opt->{'oem'}) {
		return 1;
	}
	#(!defined $opt->"encrypt" and !defined $opt->"oem")
	return 0;
};

sub fldOemCotConfig {
	my $opt = $_[0];
	return defined $opt->{'encrypt'}?1:0; 
};

sub opt_to_map {
	my ($opt,$map) = @_;
	my %opt_map;
	local *_dfn = sub {
   		$_[0] =~ m/.*($_[1]).*/;
		return $1;
	};
	for my $v (keys %{$map}) {
		if (defined _dfn($opt, $v)) {
			$opt_map{$v} = _dfn($opt,$v);
		}
	}
	#print_hash(\%opt_map);
	return\%opt_map; 
}

%impl_features=( 'GEN3'=>{'chip'=>{'63158'=>,'','4908'=>'','6858'=>'','6856'=>'','6846'=>'','6878'=>'','63178'=>'','47622'=>''},
				'sec_opt'=>{'encrypt' =>'','spu'=>'','skp'=>'','oem'=>'','sign'=>'','noheader'=>''},
				'sec_mode'=>{'MFG'=>'','FLD'=>'','UNSEC'=>''},
				'args'=>[{'chip'=>'' ,'sec_arch'=>'' ,'sec_mode'=>'','byteorder'=>'','image_in'=>'','sbi_out'=>''},
				{'sec_opt'=>'','cred_dir'=>''}],
				},
			'GEN2'=>{
				'chip'=>{'63138'=>'', '63148'=>'', '63381'=>'', '6838'=>'', '6848'=>'','63268'=>''},
				'sec_opt'=>{'encrypt'=>'','sign'=>'','noheader'=>'','spi'=>''},
				'sec_mode'=>{'MFG'=>'','UNSEC'=>''},
				'args'=>[{'chip'=>'','sec_arch'=>'','sec_mode'=>'','byteorder'=>'','image_in'=>'','sbi_out'=>''},
						{'cred_dir'=>''}, {'sec_opt'=>''}, {'max_size'=>'','image_in1'=>''}],
				},
			'byteorder'=>{'little'=>'','big'=>''}
		);
#General argument validation 
# Put new arguments processing in here 

#Passing arg structure
# args[0] and args[1] in impl_features are matched against arg passed to process_args
#  
#%(
#	{
#		'chip'=><val>,
#		'sec_arch'=><val>,
#		'sec_mode'=><val>,
#		'byteorder'=><val>,
#		'image_in'=><val>,
#		'image_out'=><val>,
#	}
#);
sub process_args {
	my $arg = shift;
	my $sec_opt;
	local *args_defined = sub { 
			my $var = shift; 
			my $feat = shift;
			for my $el (keys %{$var}) {
				if (!defined $feat->{$el}) {
					die "Invalid argument $el";	
				}
			}
	};
	#print_hash($arg);
	# check if arch is supported 
	if (!defined $arg->{'sec_arch'} and 
		!defined $impl_features{$arg->{'sec_arch'}})  {
		die "Invalid arguments";
	}

	# verify if corresponding to arch implementation is available including permitted argument's set 
	my $feat = $impl_features{$arg->{'sec_arch'}};
	args_defined( $feat->{'args'}[0],$arg);
	if (!defined $feat->{'sec_mode'}->{$arg->{'sec_mode'}}) {
		die "Invalid argument $arg->{'sec_mode'}";
	}
	if ($arg->{'sec_arch'} eq 'GEN3') {
		# verify features supported per arch	
		if (!($arg->{'sec_mode'} eq 'UNSEC')) {
			#args_defined($arg, $impl_features{$arg->{'sec_arch'}}->{'args1'});
			args_defined($feat->{'args'}[1],$arg);
		}
	}
	if ( !defined $feat->{'chip'}->{$arg->{'chip'}}) {
		die "Chip id is not supported $arg->{'chip'}";
	}
	# map options per sec_mode/arch	
	if (defined $arg->{'sec_opt'}) {
		$sec_opt = opt_to_map($arg->{'sec_opt'}, $feat->{'sec_opt'});
		$arg->{'sec_opt'} = $sec_opt;
		
	}
	my @opts_nkeys=keys %{$arg->{'sec_opt'}};
	if ($arg->{'sec_arch'} eq 'GEN2') {
			if ($arg->{'sec_mode'} eq 'MFG') {
				args_defined($feat->{'args'}[0],$arg);
				args_defined($feat->{'args'}[1],$arg);
				if (defined $sec_opt->{'spi'}) {
					args_defined($feat->{'args'}[2],$arg);
					args_defined($feat->{'args'}[3],$arg);
				}
				if (defined $arg->{'cred_dir'}) {
					$arg->{'plat_dir'}="$arg->{'cred_dir'}/$arg->{'chip'}";
				}
				# Allowable set of options per mode
				if ( (defined $arg->{'sec_opt'}->{'noheader'}  or 
					defined $arg->{'sec_opt'}->{'sign'} or
					defined $arg->{'sec_opt'}->{'spi'})  and	
					(length @opts_nkeys) != 1 ) {
					die "Options are not supported for the $arg->{'sec_mode'} mode";
					print_hash($arg->{'sec_opt'});
				}
				
			} else { 
				if ($arg->{'sec_mode'} eq 'UNSEC' and defined $arg->{'sec_opt'}) {
					print "Options are ignored for the $arg->{'sec_mode'} mode\n";
				}
			}
			# verifying valid combinations for each arch{mode}
	}
	if ($arg->{'sec_arch'} eq 'GEN3') {
		if (defined $arg->{'cred_dir'}) {
			$arg->{'plat_dir'}="$arg->{'cred_dir'}/../$arg->{'chip'}";
		}
		my @keys_num=keys %{$arg->{'sec_opt'}};
		if ( (defined $arg->{'sec_opt'}->{'noheader'} and !(defined $arg->{'sec_opt'}->{'encrypt'}) and (length @opts_nkeys) != 2)  or
					defined $arg->{'sec_opt'}->{'sign'} and	(length @opts_nkeys) != 1 ) {
				print_hash($arg->{'sec_opt'});
				die "Options are not supported for the $arg->{'sec_mode'} mode";
		}
	}


	#print_hash($arg->{'sec_opt'});
}


sub prepare_keys {
	my $arg = shift;
	my $cred = shift;
	my %keys;
	if ($arg->{'sec_arch'} eq 'GEN2') {
		if ($arg->{'sec_mode'} eq 'MFG' or $arg->{'sec_mode'} eq 'OP') {
			$keys{'MFG'}->{'ek'} =  BrcmSecUtils::f2hex($cred->{'MFG'}->{'EK'});
			$keys{'MFG'}->{'iv'} =  BrcmSecUtils::f2hex($cred->{'MFG'}->{'IV'});
			$keys{'MFG'}->{'rsa'} =  $cred->{'MFG'}->{'RSA'};
		}
	} else { 

		if (defined $arg->{'sec_opt'}->{'encrypt'}) {
			$keys{'MFG'}->{'ek'} = BrcmSecUtils::f2hex($cred->{'MFG'}->{'KAES_EK'});
			$keys{'MFG'}->{'iv'} = BrcmSecUtils::f2hex($cred->{'MFG'}->{'KAES_IV'});
			if (defined $arg->{'sec_opt'}->{'oem'}) {
				$keys{'FLD'}->{'ek'} = BrcmSecUtils::f2hex($cred->{'FLD'}->{'OEM_KAES_EK'});
				$keys{'FLD'}->{'iv'} = BrcmSecUtils::f2hex($cred->{'FLD'}->{'OEM_KAES_IV'});
			} else {
				$keys{'FLD'}->{'ek'} = BrcmSecUtils::f2hex($cred->{'FLD'}->{'KROE_EK'});
				$keys{'FLD'}->{'iv'} = BrcmSecUtils::f2hex($cred->{'FLD'}->{'KROE_IV'});
			}
		}
		$keys{'MFG'}->{'rsa'} = $cred->{'MFG'}->{'KRSA'};
		if (defined $arg->{'sec_opt'}->{'oem'}) {
			$keys{'FLD'}->{'rsa'} = $cred->{'FLD'}->{'OEM_KRSA'};
		} else {
			$keys{'FLD'}->{'rsa'} = $cred->{'FLD'}->{'KROT_RSA'};
		}
	}
	return \%keys;
}

sub prepare_headers {
	my $arg = shift;
	my $cred = shift;
	my $hdr = shift;
	local *prepend_path = sub {
		my $_cred = shift;
		my $_dir = shift;
		for my $key (keys %{$_cred}) {
			if ($key !~ /.*(PLT).*/) {
				$_cred->{$key} = $_dir."/".$_cred->{$key};
			}
		}
	};
	# Prepare MFG cot
	#Rot Cot
	if (defined $arg->{'cred_dir'}) {
		prepend_path($cred->{'MFG'},$arg->{'cred_dir'});
		prepend_path($cred->{'MFG'}->{'PLT'},$arg->{'plat_dir'});
	}
	if ($arg->{'sec_arch'} eq 'GEN3') {
		if (defined $arg->{'cred_dir'}) {
			prepend_path($cred->{'FLD'},$arg->{'cred_dir'});
		}
		if ($arg->{'sec_mode'} eq 'MFG' or 
			$arg->{'sec_mode'} eq 'FLD') { 
			$hdr->{'mfgRoeCot'}->{'kroe2MfgPub'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'KROE2_PUB'});
			if ($arg->{'chip'} eq '63158') {
				my $opt_map = $arg->{'sec_opt'};
				if (defined $opt_map->{'skp'}) {
					$hdr->{'mfgRoeCot'}->{'sig'} = (defined $opt_map->{'spu'}) ? BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SKP_SPU_SIG'}): 
								BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SKP_SIG'});
					$hdr->{'mfgRoeCot'}->{'encrKroe2MfgPriv'} = (defined $opt_map->{'spu'}) ? BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC_SKP_SPU'}): 
								BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC_SKP'});
				} else {
					$hdr->{'mfgRoeCot'}->{'sig'} =  BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SIG'});
					$hdr->{'mfgRoeCot'}->{'encrKroe2MfgPriv'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC'});
				}

			} else {
				$hdr->{'mfgRoeCot'}->{'sig'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SIG'});
				$hdr->{'mfgRoeCot'}->{'encrKroe2MfgPriv'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC'});
			}
			if (!($arg->{'chip'} eq '4908')) {
				$hdr->{'mfgRoeCot'}->{'ver'} = 2;
				$hdr->{'mfgRoeCot'}->{'mid'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'MID'});
			} else {
				$hdr->{'mfgRoeCot'}->{'ver'} = 1;
			}
			# OemCo
			$hdr->{'mfgOemCot'}->{'config'} = mfgOemCotConfig($arg->{'sec_opt'}); 
			$hdr->{'mfgOemCot'}->{'mid'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'MID'}); 
			$hdr->{'mfgOemCot'}->{'sig'} = BrcmSecUtils::f2var($arg->{'chip'} eq '4908'?$cred->{'MFG'}->{'OEM_DATA_SIG'}:$cred->{'MFG'}->{'OEM_DATA2_SIG'}); 
			$hdr->{'mfgOemCot'}->{'krsaMfgPub'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'OEM_KRSA_PUB'}); 
			$hdr->{'mfgOemCot'}->{'encrMidAndKaesMfg'} = BrcmSecUtils::f2var($cred->{'MFG'}->{'OEM_MID_KAES_ENC'}); 
		}

		if ($arg->{'sec_mode'} eq 'MFG' or 
			$arg->{'sec_mode'} eq 'FLD') { 
			#Prepare FLD cot
			$hdr->{'fldRotCot'}->{'config'} = fldRotConfig($arg->{'sec_opt'});
			$hdr->{'fldRotCot'}->{'mid'} = BrcmSecUtils::f2var($cred->{'FLD'}->{'MID'});
			$hdr->{'fldRotCot'}->{'krotFldPub'} = BrcmSecUtils::f2var($cred->{'FLD'}->{'KROT_PUB'});
			#fldOemCot
			$hdr->{'fldOemCot'}->{'config'} = fldOemCotConfig($arg->{'sec_opt'}); 
			$hdr->{'fldOemCot'}->{'sig'} = BrcmSecUtils::f2var($cred->{'FLD'}->{'OEM_DATA_SIG'});
			$hdr->{'fldOemCot'}->{'krsaFldPub'} = BrcmSecUtils::f2var($cred->{'FLD'}->{'OEM_KRSA_PUB'});
			$hdr->{'fldOemCot'}->{'encrKaesFld'} = BrcmSecUtils::f2var($cred->{'FLD'}->{'OEM_KAES_ENC'});
		}
		if ($arg->{'sec_mode'} eq 'UNSEC') {
			my $hdr = $hdr->{'SbiAuthHdrBeginning'};
                	$hdr->{'hdrLen'}=32;
                	$hdr->{'authLen'}=0;
                	$hdr->{'mfgRoeCotOfs'}=0;
                	$hdr->{'mfgOemCotOfs'}=0;
                	$hdr->{'fldRotCotOfs'}=0;
                	$hdr->{'fldOemCotOfs'}=0;
		}

	} else {
		if ($arg->{'sec_arch'} eq 'GEN2') {
			my $hdr = $hdr->{'SbiAuthHdr'};
			#"SbiAuthHdr"=>{'type'=>0,'ver'=>0,'len'=>0, 'config'=>0},
			if ($arg->{'sec_mode'} eq 'UNSEC') {
                		#$hdr->{'len'}=32;
                		#$hdr->{'authLen'}=0;
				#$hdr->{'mfgCotSig'} = 0;
				#$hdr->{'mfgCot'} = 0; 
				#$hdr->{'opCot'} = 0; 
				#$hdr->{'opCotSig'} = 0; 
			} else {
			}
		}	
	}

}



sub build_mfgRoeCot
{
	my $arg = shift;
	my $byteorder = shift;
	my $bytes;
	my $len_offs;
	BrcmSecUtils::set_val_at(\$bytes, 0, $arg->{'type'},'u16',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'ver'},'u16',$byteorder);
	$len_offs = length $bytes;
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'reserve'},'u16',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'sig'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'kroe2MfgPub'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'encrKroe2MfgPriv'},'s',$byteorder);
	if (defined  $arg->{'mid'}) {
		BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'mid'},'s',$byteorder);	
		# This idiosyncrasy comes from the original bash script (tbd: remove) 
		BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'mid'},'s',$byteorder);	
	}
	BrcmSecUtils::set_val_at(\$bytes, $len_offs,length $bytes,'u16',$byteorder);
	return $bytes;
}

sub build_mfgOemCot
{
	my $arg = shift;
	my $byteorder = shift;
	my $bytes;
	my $len_offs;
	BrcmSecUtils::set_val_at(\$bytes, 0, $arg->{'type'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'ver'},'u16',$byteorder);
	$len_offs = length $bytes;
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'len'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'config'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'sig'},'s',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'mid'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'krsaMfgPub'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'encrMidAndKaesMfg'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, $len_offs, (length $bytes),'u16',$byteorder);
	return $bytes;
}

sub build_fldRotCot
{
	my $arg = shift;
	my $byteorder = shift;
	my $bytes;
	my $len_offs;
	BrcmSecUtils::set_val_at(\$bytes, 0, $arg->{'type'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'ver'},'u16',$byteorder);
	$len_offs = length $bytes;
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'len'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'config'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'mid'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'krotFldPub'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, $len_offs, length $bytes,'u16',$byteorder);
	return $bytes;
}

sub build_fldOemCot
{
	my $arg = shift;
	my $byteorder = shift;
	my $bytes;
	my $len_offs;
	BrcmSecUtils::set_val_at(\$bytes, 0, $arg->{'type'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'ver'},'u16',$byteorder);
	$len_offs = length $bytes;
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'len'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'config'},'u16',$byteorder);	
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'sig'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'krsaFldPub'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $arg->{'encrKaesFld'},'s',$byteorder);
	BrcmSecUtils::set_val_at(\$bytes, $len_offs, length $bytes,'u16',$byteorder);
	return $bytes;
}

sub build_auth_header
{
	my $arg = shift;
	my $_hdr = shift;
	my $sbi_len = shift;
	my $hdr_cot;
	my $bytes;
	my $sbi_hdr = $_hdr->{'SbiAuthHdrBeginning'};
	my $sbi_hdr_len = $sbi_hdr->{'hdrLen'};
	BrcmSecUtils::set_val_at(\$bytes, 0, $sbi_hdr->{'ver'},'u32',$arg->{'byteorder'});
	my $sbi_hdr_len_offs =  length $bytes;

	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $sbi_hdr_len,'u32',$arg->{'byteorder'});
	my $auth_len_offs = length $bytes;
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $sbi_hdr_len,'u32',$arg->{'byteorder'});
 	$hdr_cot = build_mfgRoeCot($_hdr->{'mfgRoeCot'}, $arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, (length $hdr_cot)+$sbi_hdr_len,
		'u32',$arg->{'byteorder'});
 	$hdr_cot .= build_mfgOemCot($_hdr->{'mfgOemCot'}, $arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, (length $hdr_cot)+$sbi_hdr_len,'u32',$arg->{'byteorder'});
 	$hdr_cot .= build_fldRotCot($_hdr->{'fldRotCot'}, $arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, (length $hdr_cot)+$sbi_hdr_len,'u32',$arg->{'byteorder'});
 	$hdr_cot .= build_fldOemCot($_hdr->{'fldOemCot'},$arg->{'byteorder'});
	$sbi_hdr_len = (length $bytes)+(length $hdr_cot);	
	BrcmSecUtils::set_val_at(\$bytes, $sbi_hdr_len_offs, $sbi_hdr_len,'u32',$arg->{'byteorder'});
	return {'bytes'=>$bytes.$hdr_cot,'auth_len'=>(length $hdr_cot)+(length $bytes)+$sbi_len};	
}

sub modeEligible
{
	my $arg = shift;
	my $opt = $arg->{'sec_opt'};
	return (defined $opt->{'encrypt'})?
		($arg->{'sec_mode'} eq 'MFG'? 2:4) :
			($arg->{'sec_mode'} eq 'UNSEC'?1:6);
}

sub build_unauth_header
{
	my $arg = shift;
	my $ubi_hdr = shift;
	my $auth_len = shift;
	my $sig_len = 256;
	my $crc_len = 4;
	my $hdr_cot;
	my $bytes;
	my $ubi_hdr_len = $ubi_hdr->{'hdrLen'};

	BrcmSecUtils::set_val_at(\$bytes, 0, $ubi_hdr->{'magic_1'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $ubi_hdr->{'magic_2'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $ubi_hdr->{'ver'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, modeEligible($arg),'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $ubi_hdr_len,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 
					$ubi_hdr_len+$auth_len+2*$sig_len+$crc_len,
					'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 
				BrcmSecUtils::crc32($bytes),'u32',$arg->{'byteorder'});
	return $bytes;	
}

sub encrypt {
	my $keys = shift;
	my $in = shift;
	return BrcmSecUtils::_encrypt_aes_128_cbc($keys->{'ek'},$keys->{'iv'},BrcmSecUtils::f2var($in));
}

sub print_hash {
	my $arg = shift;
	for my $_k (keys %{$arg}) {
		if (defined $arg->{$_k}) {
			print "$_k => $arg->{$_k}\n";
		}
	}
}

# Legacy modus of operandi
# Always generates Unauthenticated+Authenticated headears
# Authentiacted header == MFG Header + FLD (OEM) Header
# 2 signatures (MFG & FLD) + crc appended at the end
# OEM option only applies to FLD mode only
# if encrypt option is specified both either FLD or MFG affected but not both
# 	encrypt option is applies to the payload. For MFG payload is encrypted with MFG kaes keys   
# 	For FLD non - oem payload is encrypted with MFG kroe aes keys otherwise, for oem fld oem kaes keys
# otherwise
#	FLD or MFG sec_mode is ignored. Unencrypted payload can be used for both MFG and FLD (OEM) modes    
sub build_sbi
{
	my $arg=shift;
	my $cred=shift;
	my $sbi_headers=shift;
	my $bytes;
	my $keys;
	my $image;
	my $mfg_sig;
	my $fld_sig;
	my $auth_hdr;
	my $unauth_hdr;
	my $crc = 0;
	$keys = prepare_keys($arg, $cred);
	printf("Building %s:%s headered secure image (SBI)\n",$arg->{'sec_arch'},$arg->{'sec_mode'});
	if (defined $arg->{'sec_opt'}->{'encrypt'}) {
		$image = encrypt($keys->{$arg->{'sec_mode'}},$arg->{'image_in'});
	} else {
		$image = BrcmSecUtils::f2var($arg->{'image_in'});
	}
	$auth_hdr = build_auth_header($arg, $sbi_headers, length $image); 
	$unauth_hdr = build_unauth_header($arg, 
			$sbi_headers->{'SbiUnauthHdrBeginning'}, $auth_hdr->{'auth_len'});
	$image = $auth_hdr->{'bytes'}.$image;
	$mfg_sig = BrcmSecUtils::_sign_sha256($keys->{'MFG'}->{'rsa'}, $image);
	$fld_sig = BrcmSecUtils::_sign_sha256($keys->{'FLD'}->{'rsa'}, $image);
	$image = $unauth_hdr.$image.$fld_sig.$mfg_sig;
	BrcmSecUtils::set_val_at(\$crc, 0,BrcmSecUtils::crc32($image), 
					'u32',$arg->{'byteorder'});
	BrcmSecUtils::fdump($arg->{'sbi_out'},$image.$crc);
}

sub build_sbi_gen1 {
	my $arg=shift;
	my $cred=shift;
	my $keys;
	my $image;
	my $payload_len = 48*1024;
	local *_2var = sub{ return BrcmSecUtils::f2var($_[0]);};
	print "Building GEN1 headered secure image (UBI)\n";	
	#print_hash(\%cred_data);
	#print_hash($arg->{'sec_opt'});
	#print_hash($arg);
	$keys = prepare_keys($arg, $cred);
	$keys = $keys->{$arg->{'sec_mode'}};
	BrcmSecUtils::set_val_at(\$image,0,_2var($arg->{'image_in'}),'s',$arg->{'byteorder'});
	if (defined $arg->{'sec_opt'}->{'spi'} ) {
		if (length $image > 128*1024) {
			die "Invalid image size must be less or equal 128K";
		}
		$payload_len = 1024*128;
		BrcmSecUtils::set_val_at(\$image, length $image, "\377"x($payload_len - length $image),'s',$arg->{'byteorder'});
	} else {
		BrcmSecUtils::set_val_at(\$image, length $image, "\000"x($payload_len - length $image),'s',$arg->{'byteorder'});
	}

	my $sig = BrcmSecUtils::_sign_sha256($keys->{'rsa'}, $image);
	# legacy calculations inherited from old gen1 image build
	# this will be removed soon for 63268 
	# building NVRAM
        my $nvram =
		substr( (substr $sig._2var($cred->{'MFG'}->{'PLT'}->{'SIG'})._2var($cred->{'MFG'}->{'PLT'}->{'COT'}),0,770)
				._2var($cred->{'MFG'}->{'PLT'}->{'OP_SIG'})
				._2var($cred->{'MFG'}->{'PLT'}->{'OP_COT'}),0,1284)
				._2var($cred->{'MFG'}->{'PLT'}->{'EK_ENC'})
				._2var($cred->{'MFG'}->{'PLT'}->{'OP_EK_ENC'})
				._2var($cred->{'MFG'}->{'PLT'}->{'IV_ENC'})
				._2var($cred->{'MFG'}->{'PLT'}->{'OP_IV_ENC'});
	#embedding nvram at offset 2432
	BrcmSecUtils::set_val_at(\$image, 2432, $nvram,'s',$arg->{'byteorder'});
	BrcmSecUtils::fdump($arg->{'sbi_out'},$image);
}

sub build_sbi_gen2 {
	my $arg=shift;
	my $cred=shift;
	my $sbi_headers=shift;
	my $bytes;
	my $keys;
	my $image;
	my $auth_hdr;
	my $unauth_hdr;
	my $crc = 0;
	printf("Building %s:%s headered secure image (SBI)\n",$arg->{'sec_arch'},$arg->{'sec_mode'});
	$keys = prepare_keys($arg, $cred);
	$keys = $keys->{$arg->{'sec_mode'}};
	#print_hash($keys);
	$image = BrcmSecUtils::f2var($arg->{'image_in'});
	#Authenticated header - all 0s
	BrcmSecUtils::set_val_at(\$auth_hdr, 0, 0,'u16',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$auth_hdr, length $auth_hdr, 0,'u16',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$auth_hdr, length $auth_hdr, 0,'u16',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$auth_hdr, length $auth_hdr, 0,'u16',$arg->{'byteorder'});

	$image = $auth_hdr.BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'COT'})
			.BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'SIG'})
			.BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'OP_COT'})
			.BrcmSecUtils::f2var($cred->{'MFG'}->{'PLT'}->{'OP_SIG'}).$image;

	my $sig = BrcmSecUtils::_sign_sha256($keys->{'rsa'}, $image);
	$image = $image.$sig;
	#printf ("image length 0x%x\n",length $image);
	BrcmSecUtils::set_val_at(\$image, length $image, BrcmSecUtils::crc32($image), 
				'u32',$arg->{'byteorder'});
	#printf ("image length 0x%x\n",length $image);
	my $_hdr = $sbi_headers->{'SbiUnauthHdr'};
	#	"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1, 
	#		'len'=>undef,'crc'=>undef },
	BrcmSecUtils::set_val_at(\$unauth_hdr, 0, $_hdr->{'magic_1'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, $_hdr->{'magic_2'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, $_hdr->{'ver'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, (length $image)+20,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, BrcmSecUtils::crc32($unauth_hdr),'u32',$arg->{'byteorder'});
	$image = $unauth_hdr.$image;
	if (defined $arg->{'sec_opt'}->{'spi'}) {
		#add spi image which is expected to be second input image whithin the sec_opt
		# pad it up to the max_size then copy spi_image over pad area 
		# cat spi image and $image then truncate it to the max_size 
		my $spi_image =	"\377"x($arg->{'max_size'}*(1024/2));
		BrcmSecUtils::set_val_at(\$spi_image, 0, BrcmSecUtils::f2var($arg->{'image_in1'}),'s',$arg->{'byteorder'});
		$image = substr $spi_image.$image,0,($arg->{'max_size'}*1024);
		BrcmSecUtils::set_val_at(\$image,length $image,"\377"x($arg->{'max_size'}*1024-(length $image)),'s',$arg->{'byteorder'});
	}
	BrcmSecUtils::fdump($arg->{'sbi_out'},$image);
}

sub build_ubi {
	my $arg = shift;
	my $sbi_headers=shift;
	my $bytes;
	my $image;
	my $crc;
	my $_hdr = $sbi_headers->{'SbiAuthHdrBeginning'};
	printf("Building %s:%s headered non-secure image (UBI)\n",$arg->{'sec_arch'},$arg->{'sec_mode'});
	BrcmSecUtils::set_val_at(\$bytes, 0, $_hdr->{'ver'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, $_hdr->{'hdrLen'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$bytes, length $bytes, 0,'u32',$arg->{'byteorder'});
	$image = BrcmSecUtils::f2var($arg->{'image_in'});
	$_hdr = $sbi_headers->{'SbiUnauthHdrBeginning'};
	$_hdr = build_unauth_header($arg, $_hdr,	
					(length $image)+(length $bytes));
	$image = $bytes.$image;
	BrcmSecUtils::set_val_at(\$crc, 0, BrcmSecUtils::crc32($image), 
					'u32',$arg->{'byteorder'});
	#my $img  =$_hdr.$image.("\000"x512).$crc;

	#printf ("--- building  ubi %s sz %d\n",$arg->{'sbi_out'},(length $img)); 
	BrcmSecUtils::fdump($arg->{'sbi_out'}, $_hdr.$image.("\000"x512).$crc);
	#BrcmSecUtils::fdump($arg->{'sbi_out'}, $img);
}

sub build_ubi_gen2 {
	my $arg = shift;
	my $sbi_headers=shift;
	my $bytes;
	my $image;
	my $crc;
	my $auth_hdr;
	my $unauth_hdr;
	my $_hdr = $sbi_headers->{'SbiAuthHdr'};
	printf("Building %s:%s headered non-secure image (UBI)\n",$arg->{'sec_arch'},$arg->{'sec_mode'});	
	$image = BrcmSecUtils::f2var($arg->{'image_in'});
	#Authenticated header - all 0s
	BrcmSecUtils::set_val_at(\$auth_hdr, 0, 0,'u16',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$auth_hdr, length $auth_hdr, 0,'u16',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$auth_hdr, length $auth_hdr, 0,'u16',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$auth_hdr, length $auth_hdr, 0,'u16',$arg->{'byteorder'});
	# empty cots and signature
	$image = $auth_hdr."\000"x(584*2).$image;
	BrcmSecUtils::set_val_at(\$crc,0,BrcmSecUtils::crc32($image),'u32',$arg->{'byteorder'});
	$image = $image."\000"x(256).$crc;
	$_hdr = $sbi_headers->{'SbiUnauthHdr'};
	#"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1, 
	#		'len'=>undef,'crc'=>undef },
	BrcmSecUtils::set_val_at(\$unauth_hdr, 0, $_hdr->{'magic_1'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, $_hdr->{'magic_2'},'u32',$arg->{'byteorder'});
	my $offs =BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, $_hdr->{'ver'},'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, (length $image)+20,'u32',$arg->{'byteorder'});
	BrcmSecUtils::set_val_at(\$unauth_hdr, length $unauth_hdr, BrcmSecUtils::crc32($unauth_hdr),'u32',$arg->{'byteorder'});
	BrcmSecUtils::fdump($arg->{'sbi_out'}, $unauth_hdr.$image);
}

sub non_header_sbi 
{
	my $arg = shift;
	my $cred = shift;
	my $image;
	my $temp="./_tmp_data_t.$$";
	my $hdr_size = 12;
	my $hdr_field_offs = 8;
	my %split_file = ("left"=>0,"right"=>0);
	printf("Building %s:%s non-headered secure image \n",$arg->{'sec_arch'},$arg->{'sec_mode'});
	my $keys = prepare_keys($arg, $cred);
	#print_hash($arg);
	#print_hash($cred);
	$keys = $keys->{$arg->{'sec_mode'}};
	#print_hash($keys);
	BrcmSecUtils::fsplit($hdr_size, $arg->{'image_in'}, \%split_file);
	BrcmSecUtils::fdump($temp, $split_file{"right"});
	my $size = BrcmSecUtils::compress_lzma($temp,"$temp.comp","$arg->{'buildtop'}/hostTools");
	$image = BrcmSecUtils::f2var("$temp.comp");
	#update header's size field
	BrcmSecUtils::set_val_at(\$split_file{"left"}, $hdr_field_offs, 
		$size,'u32', $arg->{'byteorder'});

	$image = $split_file{'left'}.$image;
	$image = BrcmSecUtils::_encrypt_aes_128_cbc($keys->{'ek'},$keys->{'iv'}, $image);
	my $sig = BrcmSecUtils::_sign_sha256($keys->{'rsa'}, $image);
	BrcmSecUtils::fdump($arg->{'sbi_out'}, $sig.$image);
	BrcmSecUtils::run_shell("rm -f $temp $temp.*");
}

sub sign_bi 
{
	my $arg = shift;
	my $cred = shift;
	my $keys = prepare_keys($arg, $cred);
	
	printf("Building %s:%s image signature\n",$arg->{'sec_arch'},$arg->{'sec_mode'});	
	$keys = $keys->{$arg->{'sec_mode'}};
	BrcmSecUtils::sign_sha256($keys->{'rsa'}, $arg->{'image_in'}, $arg->{'sbi_out'});
}

sub build_gen3 {
	my %cred = (
		'MFG'=>{'MID'=>"mid.bin",
			'KROE2_PUB'=>"Kroe2-mfg-pub.bin",
			'PLT'=>{ 'ROE_DATA_SIG'=>"mfgRoeData.sig",
				'ROE_DATA_SKP_SPU_SIG'=>"mfgRoeData_with_skp_spu.sig",
				'ROE_DATA_SKP_SIG'=>"mfgRoeData_with_skp.sig",
				'KROE2_PRIV_ENC_SKP_SPU'=>"Kroe2-mfg-priv_with_skp_spu.enc",
				'KROE2_PRIV_ENC_SKP'=>"Kroe2-mfg-priv_with_skp.enc",
				'KROE2_PRIV_ENC'=>"Kroe2-mfg-priv.enc"
			},
			'OEM_DATA_SIG'=>'mfgOemData.sig',
			'OEM_DATA2_SIG'=>'mfgOemData2.sig',
			'OEM_KRSA_PUB'=>"Krsa-mfg-pub.bin",
			'OEM_MID_KAES_ENC'=>"mid+Kaes-mfg.enc",
			'KAES_IV'=>"Kaes-mfg-iv.bin",
			'KAES_EK'=>"Kaes-mfg-ek.bin",
			'KRSA'=>"Krsa-mfg.pem"},
		'FLD'=>{'MID'=>"mid.bin",
			'KROT_PUB'=>"Krot-fld-pub.bin",
			'OEM_DATA_SIG'=>"fldOemData.sig",
			'OEM_KRSA_PUB'=>"Krsa-fld-pub.bin",
			'OEM_KAES_ENC'=>"Kaes-fld.enc",
			'OEM_KRSA'=>"Krsa-fld.pem",
			'KROE_EK'=>"Kroe-fld-ek.bin",
			'KROE_IV'=>"Kroe-fld-iv.bin",
			'OEM_KAES_EK'=>"Kaes-fld-ek.bin",
			'OEM_KAES_IV'=>"Kaes-fld-iv.bin",
			'KROT_RSA'=>"Krot-fld.pem",
			},
	);

	my %sbi_headers = ( 
		"mfgRoeCot"=>{'type'=>0x0001,'ver'=>0x0000,'len'=>undef,
			'reserve'=>undef,'sig'=>undef,'kroe2MfgPub'=>undef,
			'encrKroe2MfgPriv'=>undef, 'mid'=>undef },
		"mfgOemCot"=>{'type'=>0x0002,'ver'=>0x0001,'len'=>undef,
			"config"=>undef,"sig"=>undef,'mid'=>undef, "krsaMfgPub"=>undef,
			"encrMidAndKaesMfg"=>undef},
		"fldRotCot"=>{"type"=>0x0003,"ver"=>0x0001,"len"=>undef,
			"config"=>undef,"mid"=>undef, "krotFldPub"=>undef},
		"fldOemCot"=>{"type"=>0x0004,"ver"=>0x0001,"len"=>undef,
			"config"=>0,'sig'=>undef,"mid"=>undef, "krsaFldPub"=>undef,
			'encrKaesFld'=>undef},
		"SbiAuthHdrBeginning"=>{ 'ver'=>1,'hdrLen'=>28,'authLen'=>undef, 
				'mfgRoeCotOfs'=>undef, 'mfgOemCotOfs'=>undef,'fldRotCotOfs'=>undef,
			'fldOemCotOfs'=>undef },
		"SbiUnauthHdrBeginning"=>{ 'magic_1'=>183954,'magic_2'=>145257,'ver'=>1, 
			'modeEligible'=>0, 'hdrLen'=>28,'sbiSize'=>undef},
	);


	my $arg = shift;
	prepare_headers($arg,\%cred,\%sbi_headers);
	if ($arg->{'sec_mode'} eq "UNSEC") {
		build_ubi($arg,\%sbi_headers);
	} else  {
		# MFG or FLD
		if (defined $arg->{'sec_opt'}->{'noheader'} and 
			defined $arg->{'sec_opt'}->{'encrypt'}) {
			non_header_sbi($arg,\%cred);
		} else {
			if (defined $arg->{'sec_opt'}->{'sign'}) {
				sign_bi($arg,\%cred);
			} else {
				build_sbi($arg,\%cred,\%sbi_headers);
			} 
		}
	}
}

sub build_gen2 {
	my $arg = shift;
	my %cred = ('MFG'=>{'PLT'=>{ 'COT'=>"mfg.cot.bin",
			 	'SIG'=>"mfg.cot.sig",
			 	'OP_COT'=>"op.cot.bin",
			 	'OP_SIG'=>"op.cot.sig",
				#Old Gen1 - only 63268 target
			 	'EK_ENC'=>"mfg.ek.enc",
			 	'IV_ENC'=>"mfg.iv.enc",
			 	'OP_EK_ENC'=>"op.ek.enc",
			 	'OP_IV_ENC'=>"op.iv.enc"},
			 'EK'=>"mfg.ek.bin",
			 'IV'=>"mfg.iv.bin",
			 'RSA'=>"mfg.pem"}
			);

	my %_hdr = ( 
		"SbiAuthHdr"=>{'type'=>0,'ver'=>0,'len'=>0, 'config'=>0},
		"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1, 
			'len'=>undef,'crc'=>undef },
	);

	prepare_headers($arg,\%cred,\%_hdr);
	if ($arg->{'sec_mode'} eq "UNSEC") {
		build_ubi_gen2($arg,\%_hdr);
	} else { 
		if (defined $arg->{'sec_opt'}->{'noheader'}) {
			non_header_sbi($arg,\%cred);
		} else {
			if (defined $arg->{'sec_opt'}->{'sign'}) { 
				sign_bi($arg,\%cred);
			} else {
				if ($arg->{'chip'} eq '63268') {
					build_sbi_gen1($arg,\%cred);
				} else {
					build_sbi_gen2($arg,\%cred,\%_hdr);
				}
				
			}
		}
	}
}

sub build {
	my $arg = shift;
	process_args($arg);
	$arg->{'sec_arch'} eq 'GEN2'? build_gen2($arg) : build_gen3($arg);
}

#sub run {
#
#	$args{'chip'}="63158";
#	$args{'sec_arch'}="GEN3";
#	$args{'sec_opt'}="encrypt";
#	$args{'byteorder'}="little";
#	$args{'sec_mode'}="MFG";
#	$args{'cred_dir'}="../../cfe/cfe/board/bcm63xx_btrm/data/gen3_common";
#	$args{'image_in'}="../../targets/cfe/cfe63158rom.bin";
#	$args{'sbi_out'}="sbi_mfg.bin";
#	build_sbi(\%args);
#}
#run();
