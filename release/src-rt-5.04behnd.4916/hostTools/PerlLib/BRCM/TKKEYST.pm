#!/usr/bin/env perl
package BRCM::TKKEYST;
use parent 'BRCM::SBI_UTIL';
use strict;

#use warnings;
use bytes;
use Encode;
use FindBin qw($Bin);
use Data::Dumper;
use Convert::Binary::C;
use lib qw($Bin $Bin/../PerlLib);
use File::stat;

use constant {
    TKVER_V1    => 1,
    TKVER_V0    => 0,
    G3SEC_V1    => 0,
    G3SEC_V2    => 2,
    G3SEC_V3    => 3,
    STATE_UNSEC => 0,
    STATE_MFG   => 1,
};

# Arguments
#

# free form args
# Mfg Kaes-mfg-ek/iv encrypted Kaes-fld pair
# optional args
# arch<> req<> mid<mid hex> oid<hex> abort_time<decimal> fld<Kaes-fld-ek.enc Kaes-fld-iv.enc hash_over_Krot-fld-pub.bin> out<output file> cred_dir<credential dir if needed >
#args check

my $help = qq[usage: $0 
   For GEN3 secure boot pass hash as in the following:
    Every value for key hash is a string.
    For each request(FLD or MFG) always provide 
       arch = <GEN3 /GEN2>  
       byteorder = <big/little> - optional, if not specified defaulted to little   
       chip = <4908/6858/6856/63158>  
       abort_timeout = <seconds>
    if 'req=> MFG' then 
       arch= <GEN3>
       out= <output file>
    end	
    if 'req=> FLD' then build needs to be called twice due to
     signing of the encrypted keyinfo ontent: 
      1.Pass One:  
          keyinfo= <output file for the keyinfo>
          ek = <file to encrypted fld ek content>
          iv = <file to encrypted fld iv content>
          hash = <file to encrypted fld iv content>
          mid =< hex> 
      2.Pass Two:  
          keystore= <val> an input file with the keyinfo generated with the first pass signed with 
                          a prepended signature
          out=<output file > 
    end
];

my %cmd_arg = map { $_ => undef } (
    "keyinfo",       "arch", "byteorder", "chip",
    "abort_timeout", "req",  "keyinfo",   "ek",
    "iv",            "hash", "mid",       "keystore",
    "keyinfo",       "out",  "devkey",    "otpmap",
    "ver",           "align"
);

sub check_args {
    my $self = shift;
    my $args = $self->{args};
    for my $_k ( keys $args ) {

        #print "args slurped   $_k \n";
        if ( !( exists $cmd_arg{$_k} ) ) {
            print "option $_k is not supported \n";
            print $help;
            die("Invalid args");
        }
    }
    if (
        ( !( exists $args->{keyinfo} ) && !( exists $args->{out} ) )
        || (
            !(
                   exists $args->{arch}
                && exists $args->{req}
                && exists $args->{abort_timeout}
            )
        )
        || ( exists $args->{keyinfo} && exists $args->{keystore} )
      )
    {
        print $help;
        die("Invalid args");
    }
}

sub new {
    my $class     = shift;
    my $args      = shift;
    my $verbose   = shift;
    my $byteorder = undef;
    if ( !( exists $args->{byteorder} ) ) {
        $byteorder = 'little';
    }
    else {
        $byteorder = $args->{byteorder};
    }

    if ( !$args->{arch} ) {
        $args->{arch} = 'GEN3';
    }
    if ( !( defined $verbose ) ) {
        $verbose = 0;
    }
    my $self = bless {
        byteorder => $byteorder,
        verbose   => $verbose,
        class     => $class,
    }, $class;
    $self->{args} = $args;
    $self->{EN} = $byteorder eq 'little' ? '<' : '>';
    $self->check_args();

    #print "BYTEORDER $self->{EN} \n";
    $self->build();
    if ( $args->{out} ) {
        $self->fdump( $args->{out}, $args->{keystore} );
    }
    return $self;
}

sub set_attr {
    my $val = shift;
    my $rid = shift;
    my $ord = shift;
    $$val |= ( ( ( $rid & 0x3ff ) << 16 ) | ( ( $ord & 0x3f ) << 26 ) );
}

sub set_all_attr {
    my $rid       = shift;
    my $ord       = shift;
    my $data_type = shift;
    return (
        ( ( $rid & 0x3ff ) << 16 ) | ( ( $ord & 0x3f ) << 26 ) | $data_type );
}

#mode -> chip_sec_map -> ver->
#		      	 | len ->
#			 | order|rid|type ->
#                        | val -> len type_state val rid order

sub gen_mode2otp_map {
    my $self = shift;
    my $rq   = shift;
    my $map_bin;

    my $a       = $self->{args};
    my $e       = $self->{EN};
    my %fld_map = (
        mid => [ 0, ( 0x007 | ( STATE_MFG <<12) ), 0x0 ],
			iv => [0, (0x301|(STATE_MFG<<12)), 0x0],
			ek => [0, (0x300|(STATE_MFG<<12)), 0x0],
			hash => [0, (0x006|(STATE_MFG<<12)), 0x0]);

	my $m;
	my $o = $self->{otp_map};
	my @mfg_map = ( [4, set_all_attr($o->{OTP_MAP_BRCM_BTRM_BOOT_ENABLE}, 0x0,(((0x0<<8)|0x9)|(STATE_UNSEC<<12))) ,0x1],
		[4, set_all_attr($o->{OTP_MAP_CUST_BTRM_BOOT_ENABLE}, 0x1,(((0x0<<8)|0xa)|(STATE_UNSEC<<12))) ,0x1],
	# enable for platform requiring this bit to be fused; this will work only for GEN3 v2 and v3 
		[4, set_all_attr($o->{OTP_MAP_BRCM_PRODUCTION_MODE}, 0x2,(((0x0<<8)|0xd)|(STATE_UNSEC<<12))) ,0x1],
	);
	if ($rq eq 'GEN3MFG' || $rq eq 'GEN3FLD') {
            if ($self->{ver} == TKVER_V1) {
		my $i = 0;
	        printf(" bcm_sec2 0x%08x 0x%08x 0%08x\n",$mfg_map[0][0], $mfg_map[0][1], $mfg_map[0][2]);
                for $i  ( 0 .. $#mfg_map) {
	            printf(" packing for %s 0x%08x 0x%08x 0%08x\n",$_ , $mfg_map[$i][0], $mfg_map[$i][1], $mfg_map[$i][2]);
                    $map_bin .= pack("(III)${e}", $mfg_map[$i][0], $mfg_map[$i][1], $mfg_map[$i][2]);
                }
            }
        }
        if ($rq eq 'GEN3FLD') {
               $m  = \%fld_map;
               my $mid = (-e $a->{mid}) ? unpack("nx2",$self->f2var($a->{mid})) : (hex $a->{mid});
               $m->{mid}[2] = pack("I>",$mid); 
               $m->{mid}[0] = length ($m->{mid}[2]);
               $m->{ek}[2] = $self->f2var($a->{ek});
               $m->{ek}[0] = length ($m->{ek}[2]);
               $m->{iv}[2] = $self->f2var($a->{iv});
               $m->{iv}[0] = length ($m->{iv}[2]);
	       $m->{hash}[2] = $self->f2var($a->{hash});
               $m->{hash}[0] = length ($m->{hash}[2]);
	       if ($a->{devkey}) {
                   $m->{devkey}[1] = (($a->{devkey} eq 'rand' ? 0x050c:0x000c)|(STATE_MFG<<12)), 
                   $m->{devkey}[2] = ($a->{devkey} eq 'rand'? "\000" x 32 : $a->{devkey});
                   $m->{devkey}[0] = length($m->{devkey}[2]);
	       } 
               $m->{ek}[1] = 
                (( $self->{sec_arch_ver} == G3SEC_V2 ? 0x0303:($self->{sec_arch_ver} == G3SEC_V3?0x0302:0x0300))|(STATE_MFG<<12));
               if ($self->{ver} == TKVER_V1) {
		   my $o = $self->{otp_map};
                   my $ord = 0;
                   if ($m->{devkey}) {
                       set_attr(\$m->{devkey}[1], $o->{SOTP_MAP_KEY_DEV_SPECIFIC}, $ord++);
                   }
                   set_attr(\$m->{mid}[1], $o->{OTP_MAP_CUST_MFG_MRKTID}, $ord++);
                   set_attr(\$m->{ek}[1], $o->{SOTP_MAP_FLD_ROE}, $ord++);
                   set_attr(\$m->{iv}[1], $o->{SOTP_MAP_FLD_ROE1}, $ord++);
                   set_attr(\$m->{hash}[1], $o->{SOTP_MAP_FLD_HMID}, $ord++);
               } 
               foreach  (keys $m) {
	           printf(" packing for %s 0x%08x 0x%08x 0x%08x\n",$_ , $m->{$_}[0], $m->{$_}[1], $m->{$_}[2]);
			my $dat_l = length($m->{$_}[2]);
                   $map_bin .= pack("(II)${e}a*", $m->{$_}[0], $m->{$_}[1], $m->{$_}[2]);
               }
        }
	return $map_bin;
}

sub gen_keystore {
	my $self = shift;
	my $args = $self->{args};
        my $e = $self->{EN};
        my $key_info_len = 0;
        my $key_info = $args->{keyinfo};
	if (defined ($key_info) && (length $key_info) > 2) {
            #printf("KEYINFO LENGTH %d\n", length $key_info);
	    $key_info = pack("I$e",$self->crc32($key_info)) . $key_info;
            $key_info_len = length $key_info;
           #printf("KEYINFO LENGTH %d --- data :   %s\n", length $key_info_len, unpack("H*",$key_info));
        }
        #printf ("key_info %d\n",length $key_info);
	# building header

	# keystore magic
	# sec_arch, sec request, state, abort delay  and headers' cre
	printf("sec arch ver 0x%x ",$self->{sec_arch_ver});
	my $header = pack("a12(IIII)$e", 'BRCMKEYSTORE',  
                          ($args->{arch}|($self->{sec_arch_ver}<<28)|($self->{ver}<<24)),
                          $self->{req}, $args->{abort_timeout}, $key_info_len);
	$header .= pack("I$e", $self->crc32($header));
	printf(" KeyStore length %d key_info_len %d \n",length ($header . $key_info), $key_info_len);
	return $key_info_len? ($header.$key_info) : $header;
}

sub load_otpmap {
      my $self = shift;
      my $a = $self->{args};
      if (int $self->{ver} == TKVER_V1 ) {
	  my $c = Convert::Binary::C->new(ByteOrder => 'LittleEndian');
	  eval {$c->parse_file($a->{otpmap});};
	  if ($@) {
		die("Error $@: parsing $a->{otpmap} "); 
	  }
	  #print Dumper([$c->dependencies]);
	  my $data =  eval {$c->enum('otp_map_feat')};
	  if ($@) {
		die("Error extracting otp_map_feat "); 
	  }
	  $self->{otp_map} = $data->{enumerators};
      }
}

# main call
sub build {
	my $self = shift;
	my $a = $self->{args};
	my %arch = (GEN3 => 3);
	my %req = (GEN3MFG => 4, GEN3FLD => 5);
	# to encrypt keys
	#2 byte hex numeral
	# print "$0 SEC: $args->{arch} REQ:$args->{req} MID: $args->{mid}\n";


        #gen args
        #Gen2 for MFG  (and in typical use case ) does not require 2 pass
        #scheme however adding it for the sake of completness 
	$self->{sec_arch_ver} = 0;
        if ($a->{arch} =~ m/.*v(.)/g)  {
	   $self->{sec_arch_ver} = int $1;
	   $a->{arch} =~ s/v$1//g;
           printf("||__|| arch  %s\n",$a->{arch});
        }
	$self->{ver} = $a->{ver} ? (int $a->{ver}) : ($self->{sec_arch_ver} > 0 ? TKVER_V1 : TKVER_V0);
        my $rq =  $a->{arch} . $a->{req} ;
	$self->{req} = $req{ $rq };
        $self->load_otpmap();
	
	die "Unsupported request/architecture: $rq " unless ($self->{req} == $req{GEN3FLD} ||$self->{req} == $req{GEN3MFG});
	if ($a->{keyinfo}) {
            $a->{out} = $a->{keyinfo};
            $a->{keystore} = 
			$self->gen_mode2otp_map( $rq ); 
        } else {
            # header with key info 
            $a->{abort_timeout} = int $a->{'abort_timeout'};
	    $a->{arch} = $arch{$a->{arch}};
            if ($a->{keystore}) {
                #args->{keystore} points to previously-generated and signed keyinfo ready to be merged with the keystore header
	        $a->{keyinfo} = $self->f2var($a->{keystore});
            } else { 
                $a->{keyinfo} = $self->gen_mode2otp_map($rq);
            }
            $a->{keystore} = $self->gen_keystore();
       }
}
1;
