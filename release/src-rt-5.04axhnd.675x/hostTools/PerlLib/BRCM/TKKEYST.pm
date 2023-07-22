#!/usr/bin/env perl
package BRCM::TKKEYST;
use parent 'BRCM::SBI_UTIL';
use strict;
#use warnings;
use bytes;
use Encode;
use FindBin qw($Bin);
#use lib qw($Bin $Bin/../PerlLib);
#use File::stat;

# Arguments 
#

sub new {
	my $class = shift;
	my $args = shift;
	my $verbose = shift;
	my $byteorder = undef;
	my $align4 = undef;
	if (!(exists $args->{byteorder})) {
		$byteorder = 'little';
	}  else {
		$byteorder = $args->{byteorder};
        } 
         
	if (! (exists $args->{arch})) {
		$args->{arch} = 'GEN3';
	} 
	if (!defined $verbose) {
		$verbose = 0;
	}
	my $self = bless {
		byteorder => $byteorder,
		verbose => $verbose,
		class => $class,
      }, $class;
      $self->{args} = $args;
      $self->{EN} = $byteorder eq 'little'?'<':'>';

      if (exists $args->{align}) {
          $self->{"ALIGN4B"} = $args->{align};	
      }
      #print "BYTEORDER $self->{EN} \n";
      $self->build();
      if (defined($args->{out})) {
           $self->fdump($args->{out}, $args->{keystore});
      }
      return $self;
}

sub keyinfo {
	my $self = shift;
	my $args = $self->{args};
        my $e = $self->{EN};
	my $S = $self->{"ALIGN4B"}?'I':'S'; 
        my $key_info = undef;
	if (exists $args->{mid} ) {
                # special case for mid/oid - needs to be big endian
                my $l = $S eq 'S'? 2:4; 
                $key_info = pack("($S$S)$e$S>", $l, 0x0007, $args->{mid});
	}
        #printf("M I D length %d %s \n", length $key_info, unpack("H*",$key_info)); 
	if (exists $args->{oid} ) {
                my $l = $S eq 'S'? 2:4; 
                # special case for mid/oid - needs to be big endian 
                $key_info .= pack("($S$S)$e$S>", $l, 0x0008, $args->{oid});
	}
	if (exists $args->{ek} and exists $args->{iv} and exists $args->{hash}) {
                $key_info .=  pack("($S$S)${e}a*($S$S)${e}a*($S$S)${e}a*", 
                                 length $args->{iv}, 0x0301, $args->{iv}, # aes iv 
                                 length $args->{ek}, 0x0300, $args->{ek}, # aes ek 
                                 length $args->{hash}, 0x0006, $args->{hash});
	}
	if (exists $args->{devkey}) {
             	$key_info .= pack("($S$S)${e}a*", length($args->{devkey}), 
               	               ($args->{devkey} eq "rand"? 0x050c:0x030c),
                               $args->{devkey});
	}
        return $key_info;
}

sub keystore {
	my $self = shift;
	my $args = $self->{args};
        my $e = $self->{EN};
	my $S = $self->{"ALIGN4B"}? 'I' : 'S'; 
	my $c = $self->{"ALIGN4B"}? 'I' : 'c'; 
        my $key_info_len = 0;
        my $key_info = $args->{keyinfo};
	if (defined $key_info and (length $key_info) > 2) {
            #printf("KEYINFO LENGTH %d\n", length $key_info);
	    $key_info = pack("I$e",$self->crc32($key_info)) . $key_info;
            $key_info_len = length $key_info;
           #printf("KEYINFO LENGTH %d --- data :   %s\n", length $key_info_len, unpack("H*",$key_info));
        }
        #printf ("key_info %d\n",length $key_info);
	# building header

	# keystore magic
	# sec_arch, sec request, state, abort delay  and headers' cre
	my $header = pack("a12${c}3$S$e", 'BRCMKEYSTORE',  $args->{arch}, 
            $args->{'req'}, $args->{'abort_timeout'}, $key_info_len);
	$header .= pack("I$e", $self->crc32($header));
	printf(" KeyStore length %d key_info_len %d \n",length ($header . $key_info), $key_info_len);
	return $key_info_len? ($header.$key_info) : $header;
}

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

sub check_args {
	my $self = shift;
	my $args = $self->{args};
 	if ( (!(exists $args->{keyinfo}) && !(exists $args->{out})) || 
	(!(exists $args->{arch} && exists $args->{req} && exists $args->{abort_timeout})) || 
 	(exists $args->{keyinfo} && exists $args->{keystore})) { 
                print $help;
		die("Invalid args3");
       } 
}
# main call
sub build {
	my $self = shift;
	my $args = $self->{args};
	my %arch=(GEN1=>1, GEN2=>2, GEN3=>3);
	my %req=(GEN2BTRM=>1, GEN2MFG=>2, GEN2OP=>3, GEN3MFG=>4, GEN3FLD=>5);
	# to encrypt keys
	#2 byte hex numeral
	# print "$0 SEC: $args->{arch} REQ:$args->{req} MID: $args->{mid}\n";



        #gen args
        #Gen2 for MFG  (and in typical use case ) does not require 2 pass
        #scheme however adding it for the sake of completness 
        $self->check_args(); 
	$args->{req} = $req{$args->{arch} . $args->{req}};
	if ($args->{req} == $req{GEN3FLD}) {
		if (exists $args->{keyinfo} and exists $args->{mid}) {
                    $args->{mid} = (-e $args->{mid}) ? unpack("n",$self->f2var($args->{mid})) : hex $args->{'mid'};
	            $args->{ek} = $self->f2var($args->{ek});
		    $args->{iv} = $self->f2var($args->{iv});
		    $args->{hash} = $self->f2var($args->{hash});
                    #print "building keyinfo\n";
                    $args->{out} = $args->{keyinfo};
                    $args->{keystore} = $self->keyinfo();
                    return;
                }
	} elsif ($args->{req} == $req{GEN3MFG}) {
	} elsif ($args->{req} == $req{GEN2BTRM}) {
	} elsif ($args->{req} == $req{GEN2MFG}) {
		if (exists $args->{keyinfo} && exists $args->{mid}) {
	            $args->{mid} = hex $args->{'mid'};
                    #print "building keyinfo\n";
                    $args->{out} = $args->{keyinfo};
                    $args->{keystore} = $self->keyinfo();
                    return;
                }
	} elsif ($args->{req} == $req{GEN2OP}) {
		if (exists $args->{keyinfo} && exists $args->{oid} && exists $args->{mid}) {
	            $args->{oid} = hex $args->{oid};
	            $args->{mid} = hex $args->{mid};
                    #print "building keyinfo\n";
                    $args->{out} = $args->{keyinfo};
                    $args->{keystore} = $self->keyinfo();
                    return;
                }
	} else {
                print " $args->{req} $args->{mid}\n";
		die "Unsupported request/architecture ";
	}
       # header with key info 
	$args->{abort_timeout} = int $args->{'abort_timeout'};
	$args->{arch} = $arch{$args->{arch}};
        if (exists $args->{keystore}) {
	    $args->{keyinfo} = $self->f2var($args->{keystore});
        } elsif (!(exists $args->{keyinfo})) {
           $args->{keyinfo} = $self->keyinfo();
        }
        $args->{keystore} = $self->keystore();
}
1;
