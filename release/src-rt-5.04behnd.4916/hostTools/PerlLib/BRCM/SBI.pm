#!/usr/bin/env perl 
package BRCM::SBI;

use strict;

#use warnings;
use bytes;
use FindBin qw($Bin);
use lib "$Bin";
use File::stat;
use parent qw(BRCM::SBI_UTIL);

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

#
# args are the reference to a hash as in the following: {
#				'sec_mode' => "MFG/FLD/UNSEC",
#				'chip'=><6xxx>,
#				'cred' => path to the credentils directory,
#				'byteorder' => endianness little/big,
# 				}
# verbose - turn on/off debug info 

my $help = qq[usage: $0
   Supported args:
   Input hash:
      sec_mode => UNSEC/MFG/FLD 
      chip =><4908/63158 or any of Bootrom Secure GEN3 devices> 
      byteorder => little/big - if not specified defaulted to little
      cred =>  Credential Arguments as in the following:
         Cred argument format is a string of:
          arg1<val1,..valM> .. argN<val1,... valM> where, chars '<' or '>' are the arguments' delimiters.
          dir<Directory where credentials are placed> 
      MFG mode header consists both RoE and OEM COTs: 
          roe<RoE Mfg credentials in a specific order as in roeMfgCot C header: sig, kroe2MfgPub, encrKroe2MfgPriv mid> specified as files 
          oem<OeM Mfg credentials in a specific order as in oemMfgCot C header: sig, mid, krsaMfgPub encrMidAndKaesMfg> specified as files
      FLD mode header:  
          rot<RoT FLD credentials in a specific order as in rotFldCot C header: mid krotFldPub> specified as files
      FLD OEM mode header. To build FLD OEM mode header include both FLD and FLD OEM COTS
          rot<RoT FLD credentials >  
          oem<OeM FLD credentials in a specific order as in oemFldCot C header: sig  krsaFldPub encrKaesFld> specified as files
];
sub new {
    my $class = shift;
    my ( $args, $verbose) = @_;
    my $byteorder = 'little';
    if ( !( defined $verbose ) ) {
        $verbose = 0;
    }

    unless ( $verbose == 0 ) {
        print "class: $class byteorder:$byteorder verbose: $verbose \n";
    }
    #my $self = {
    #		byteorder => $byteorder,
    #		verbose => $verbose,
    #		class => $class,
    #};
    #return bless($self,$class);

    my $self = bless {
        verbose         => $verbose,
        class           => $class,
    }, $class;
    if ( !(exists $args->{byteorder}) ) {
         $self->{en} = '<';
    } else {
         $self->{en} = ($args->{byteorder} eq 'little')? '<':'>';
    }
    $self->{'args'} = $args;
    $self->{openssl} = "openssl";
    $self->check_args(); 
    if ($self->{args}->{chip} =~ m/(63138|63148|63268|631181)/g && !$self->{args}->{arch}) {
       $self->{args}->{arch} = "GEN2";
    } 
    #print_hash($args);
    return $self;
}
#input: args
sub mfgOemCotConfig {
    my $opt = $_[0];
    if ( defined $opt->{'skp'} and defined $opt->{'spu'} ) {
        return defined $opt->{'encrypt'} ? 7 : 6;
    }
    if ( !defined $opt->{'skp'} and !defined $opt->{'spu'} ) {
        return defined $opt->{'encrypt'} ? 1 : 0;
    }
    if ( defined $opt->{'skp'} and !defined $opt->{'spu'} ) {
        return defined $opt->{'encrypt'} ? 3 : 2;
    }
    if ( !defined $opt->{'skp'} and defined $opt->{'spu'} ) {
        return defined $opt->{'encrypt'} ? 5 : 4;
    }
    return;
}

sub fldRotConfig {
    my $opt = $_[0];
    if ( defined $opt->{'encrypt'} and defined $opt->{'oem'} ) {
        return 3;
    }
    if ( !defined $opt->{'encrypt'} and defined $opt->{'oem'} ) {
        return 2;
    }
    if ( defined $opt->{'encrypt'} and !defined $opt->{'oem'} ) {
        return 1;
    }

    #(!defined $opt->"encrypt" and !defined $opt->"oem")
    return 0;
}

sub fldOemCotConfig {
    my $opt = $_[0];
    return defined $opt->{'encrypt'} ? 1 : 0;
}



#General argument validation
# Put new arguments processing in here


# every element of "oem" array is either file or an array
# if array then first element is considered to
# be a data else is a filename which is getting read to a var 

# oem[0]  sig[SEC_S_SIGNATURE];
# oem[1]  mid;
# oem[2]  krsaMfgPub[SEC_S_MODULUS];
# oem[3]  encrMidAndKaesMfg[ENCRYPTED_MID_KAES_DATA_LEN]; /* RSA encrypted with Kroe2-mfg-pub */
sub mfgOemCot {
    my $self = shift;
    my @oem = @{$self->{loem}};
    my $arg = $self->{args};

    my $bytes = pack("(SSSS)$self->{en}a*a*a*a*", 0x0002, 0x0001, 0x0000, 
		mfgOemCotConfig($arg->{sec_opt}), $oem[0], $oem[1], $oem[2],$oem[3]) ;
    substr($bytes, 4, 2) = pack("S$self->{en}",length $bytes);
    #printf ("mfgOemCot header size %d \n",length $bytes);
    return $bytes;
}

# every element of "oem" array is either file or an array
# if array then first element is considered to
# be a data else is a filename which is getting read to a var 
#roe[0]  sig[SEC_S_SIGNATURE];
#roe[1]  kroe2MfgPub[SEC_S_MODULUS];
#roe[2]  encrKroe2MfgPriv[ENCRYPTED_KROE2_DATA_LEN]; /* AES encrypted with Kroe1-mfg */
#roe[3]  mid;
sub mfgRoeCot {
	my $self = shift;
	my @roe = @{$self->{lroe}};
	my $arg = $self->{'args'};
	my $ver = $arg->{'chip'} eq "4908"? 1:2;
	
    	my $bytes = pack("(SSSS)$self->{en}a*a*a*", 0x0001, $ver, 0x0000, 0x0000,  
			$roe[0], $roe[1], $roe[2]);
	if (defined $roe[3] and $ver == 2) {
		substr ($bytes, length $bytes, length $bytes) = 
			pack("a2a2", $roe[3], $roe[3]);
	}
	substr ($bytes, 4, 2) = pack("S$self->{en}", length $bytes);
        #printf ("mfgRoeCot header size %d \n",length $bytes);
	return $bytes;
}

sub unauth_header {
    my $self     = shift;
    my $auth_len = shift;
    #my $sig_len  = ($self->{args}->{arch} =~ m/GEN3v[2|3]/)? 512 : 256;  
    my $sig_len  = ($self->sec_arch_ver() > 1 && $self->sec_arch_ver() < 4)?  512 : 256;
    my $crc_len  = 4;
    my $bytes = pack("(IIIIII)$self->{en}", 183954, 145257,1, modeEligible($self->{args}), 28, 
			28 + $auth_len + (2 * $sig_len) + $crc_len);
    substr($bytes, (length $bytes), $crc_len) = pack("I$self->{en}",$self->crc32($bytes));
    return $bytes;
}



sub opt2map {
   my ($tkn, $lst) = @_;
   $lst =~ m/$tkn<([^>]*)>/g;
   return map { get_a_key_for($_) => $_ } split(/ +/,$1);
}

# cred_list "oem<fl fl fl> roe <fl fl fl>" 
sub mfg_header {
    my $self = shift;
    my $img_len = shift;
    my $arg = $self->{args};
    my $cred_dir = shift; 
    my $cred_list = shift; 
    my $hdr_len = 28;
    my $roe = $self->mfgRoeCot();
    my $oem = $self->mfgOemCot();
    my $ver = 0;
    # building authenticated header for MFG; omitting 
    # offsets for field headers
    my $cot_len = (length $oem) + (length $roe);
    my $pad_len = 0;
    if ((($cot_len+$hdr_len)%4) != 0 ) {
        $pad_len = 4 - ($cot_len+$hdr_len)%4;
        $cot_len += $pad_len;
    }
    if ($arg->{sec_opt}->{mfgpad}) {
        #the smallest acceptable value 48,64
        print "$0 padding mfg header\n";
        $cot_len += 1024;
        $pad_len += 1024;
    }
    if ($arg->{sec_opt}->{version}) {
	    #my $ver =  ($arg->{arch} eq 'GEN4')? 0x2 : 0x1;
	    #my $ver =  ($arg->{arch} =~ m/GEN[4|5]/)? 0x2 : 0x1;
	   $ver = hex( $arg->{sec_opt}->{version}); 
    }
    # authenticated length is unused in authenticated header 
    my $bytes = pack("(IIIIIII)$self->{en}", $ver, $hdr_len + $cot_len, 0,
			$hdr_len, $hdr_len + (length $roe), 0, 0).$roe.$oem;
    if ($pad_len != 0) {
        $bytes .= "\000" x $pad_len;
    }
    return $bytes;
}

# cred_list "rot<fl fl fl> " 
sub fld_header {
    my $self = shift;
    my $img_len = shift; 
    my $arg = $self->{args};
    my $hdr_len = 28;

    my $rot = $self->fldRotCot();
    my $cot_len = (length $rot);
    my $pad_len = 0;
    if ((($cot_len+$hdr_len)%4) != 0 ) {
        $pad_len = 4 - ($cot_len+$hdr_len)%4;
        $cot_len += $pad_len;
    }
    #my $ver =  ($arg->{arch} eq 'GEN4')? 0x2 : 0x1;
    my $ver =  0;
    if ($arg->{sec_opt}->{version}) {
	    #my $ver =  ($arg->{arch} eq 'GEN4')? 0x2 : 0x1;
	    #my $ver =  ($arg->{arch} =~ m/GEN[4|5]/)? 0x2 : 0x1;
	   $ver = hex( $arg->{sec_opt}->{version}); 
    }
    my $bytes = pack("(IIIIIII)$self->{en}", $ver, $hdr_len + $cot_len, 0,
			0, 0, $hdr_len, 0).$rot;
    if ($pad_len != 0) {
        $bytes .= "\000" x $pad_len;
    }
    return $bytes;
}

# cred_list "rot<fl fl fl> " 
sub fldOem_header {
    my $self = shift;
    my $img_len = shift; 
    my $arg = $self->{args};
    my $hdr_len = 28;
    # below lines are showcasing the fact that
    # pad is artificially added to the header
    # header's pad length 
    my $pad_len = 2;
    $hdr_len += $pad_len;

    # Disclaimer: offsets to the COTs (fldRotCotOffs, fldOemCotOffs) 
    # should be independent of the alignment considerations 
    # however, this is due to an incorrect C programming practice chosen 
    # for older GEN3 bootroms (4908,63158)
    # This, also based on the fact that a non-variable unuath. header is 4 byte aligned (28) 

    #An entire auth. header must be 4 byte multiple as expected by bootrom 

    #my $ver =  ($arg->{arch} eq 'GEN4')? 0x2 : 0x1;
    my $ver = 0x0; 
    if ($arg->{sec_opt}->{version}) {
	    #my $ver =  ($arg->{arch} eq 'GEN4')? 0x2 : 0x1;
	    #my $ver =  ($arg->{arch} =~ m/GEN[4|5]/)? 0x2 : 0x1;
	   $ver = hex( $arg->{sec_opt}->{version}); 
    }
    my $oem = $self->fldOemCot();
    my $rot = $self->fldRotCot();

    if (((length $rot)+$hdr_len)%4) {
        # padding rot + header to 4 multiple since bootrom to prepare for
        # fldOemCot offset 
	$rot .= "\000" x (4 - ($hdr_len + (length $rot))%4);
    }
    if ((length $oem)%4) {
        $oem .= "\000" x (4 - (length $oem)%4);
    }

    return pack("(IIIIIII)$self->{en}", $ver, $hdr_len + (length $oem) + (length $rot), 0, 0, 0, 
                        #a special case for offset to RotCot - due to bug in the bootrom  
                        # offset to RotCot must be set at 2 aligned quantity 
			$hdr_len, 
                        $hdr_len+(length $rot)).("\000" x $pad_len).$rot.$oem;
}

# To use this api set sec_mode => "UNSEC" in the constructor arg
# arg[0] - length of the image
# returns hash such as {'unauth'=>'string' , 'auth'=>'string'};
sub ubi_header {
    my $self = shift;
    my $img_len = shift;
    if ($self->{args}->{arch} eq 'GEN2') {
        return $self->ubi_header_GEN2($img_len);
    }	
    my $auth_hdr = pack("(II)$self->{en}a*", 1, 32, "\000" x 24);
    return { 'auth'=> $auth_hdr, 'unauth' => $self->unauth_header($img_len + (length $auth_hdr))};
}

# To use this api set sec_mode => "UNSEC" in the constructor arg
#  arg[0] - auth header  string
#  arg[1] - an image string
# returns trailer string
sub ubi_trailer {
    my $self = shift;
    my ( $hdr, $image ) = @_;
    if ($self->{args}->{arch} eq 'GEN2') {
        return $self->ubi_trailer_GEN2($hdr, $image);
    }	
    #printf ("--- building  ubi %s sz %d\n",$arg->{'out'},(length $img));
    # 512 bytes in the beginning == 2 signatures
    #my $sig_len  = ($self->{args}->{arch} eq 'GEN4')? 512 : 256;
    my $ver = $self->sec_arch_ver();
    my $sig_len  =  ($ver > 1 && $ver < 4)?512:256;
    return pack("(a*)I$self->{en}", "\000" x ($sig_len*2) , $self->crc32($hdr . $image)); 
}

sub ubi_trailer_GEN2 {
    my $self = shift;
    my ( $hdr, $image ) = @_;
    # img is a concatenation of {<auth header> .<image>  }
    printf ("**************** trailer size %d image size %d \n",length($hdr), length($image)); 
    return pack("(a*)I$self->{en}", "\000" x 256 , $self->crc32($hdr . $image)); 
}

sub ubi_header_GEN2 {
    my $self        = shift;
    my $img_len     = shift;
    #Authenticated header and empty cots - all 0s
    # 1176 - authenticated empty header length 
    my $auth_len = 584*2+8;
    my $auth = pack("c$auth_len",0);
    # Unauthenticated header length with crc
    my $unauth_len = 20;
    # Trailer length
    # 256(sig) + 4(crc) 
    my $trailer_len = 260;
    #"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1,
    #		'len'=>undef,'crc'=>undef },
    my $unauth = pack("(IIII)$self->{en}", 112233, 445566, 1, $auth_len + $img_len + $unauth_len + $trailer_len);
    $unauth .= pack("I$self->{en}", $self->crc32($unauth));
    return {'auth'=>$auth,'unauth' =>$unauth};
}

sub sbi_header_GEN2 {
	my $self = shift;
    my $img_len = shift;
    my @lcot = @{$self->{lcot}};
    # Unauthenticated header length with crc
    my $unauth_len = 20;
    # Trailer length
    # 256(sig) + 4(crc) 
    my $trailer_len = 260;
    #image with appended signature
    # includes length of the image + length of signature (256) + length of crc32 (4) 
    my $bytes = "\000";
    #Authenticated header - all 0s + COT
    #COT SIG OP_COT OP_SIG 
    my $auth = pack("(SSSS)$self->{en}a*a*a*a*", 0, 0, 0, 0, $lcot[0], $lcot[1], $lcot[2], $lcot[3]);
    #	"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1,
    #		'len'=>undef,'crc'=>undef },
    my $unauth = pack("(IIII)$self->{en}", 112233, 445566, 1, length ($auth) + $img_len + $unauth_len + $trailer_len);
    $unauth .= pack("I$self->{en}", $self->crc32($unauth));
    printf ("header lengths auth: %d unauth %d\n", length($auth),length ($unauth));   
    return {'auth'=>$auth,'unauth' =>$unauth};
}

sub sbi_trailer_GEN2 {
    my $self = shift;
    my ($hdr, $image, $sig) = @_;
    return $sig . pack("I$self->{en}", $self->crc32($hdr . $image . $sig));
}

sub pack_halves {
    my $obj = shift;
    my @half = shift;
    my $pad = shift;
    my $size = shift;
    $size /= 2;	
    unless (length ($half[0]) <= $size && length($half[1]) <= $size) {
                   die "Image sizes are not supported"; 
    }

    return pack("a*a*a*a*", 
        $half[0], "\377" x ($size - (length $half[0])), 
        $half[1], "\377" x ($size - (length $half[1])));
}

sub build_spi_GEN2 {
    my $obj = shift;
    my $image = shift;
    my $spi_image = shift;
    my $size = $obj->{args}->{max_size}*512;
    return $obj->pack_halves(($image, $spi_image), $size, "\377");
}

# every element of "rot" array is either file or an array
# if array then first element is considered to
# be a data else is a filename which is getting read to a var 
# rot[0] mid 
# rot[1] krotFldPub[SEC_S_MODULUS];
sub fldRotCot {
    my $self = shift;
    my @rot = @{$self->{lrot}};
    my $arg = $self->{args};
	# initializer
    my $bytes = pack("(SSSS)$self->{en}a*a*", 0x0003, 0x0001, 
			0x0000, fldRotConfig( $arg->{'sec_opt'} ),  
			$rot[0], $rot[1]);
    substr ($bytes, 4, 2) = pack("S$self->{en}",length $bytes);
    #printf ("fldRotCot header size %d \n",length $bytes);
    return $bytes;
}

# every element of "oem" array is either a filename or an array
# if element is an array then first element is considered to
# be a data else is read from file to a var 
# oem[0]  sig[SEC_S_SIGNATURE];
# oem[1]  krsaFldPub[SEC_S_MODULUS];
# oem[2]  encrKaesFld[ENCRYPTED_KAES_FLD_DATA_LEN]; /* AES encrypted with Kroe-fld */
sub fldOemCot {
    my $self = shift;
    my @oem = @{$self->{loem}};
    my $arg = $self->{args};
    my $bytes = pack("(SSSS)$self->{en}a*a*a*", 4, 1, 0, fldOemCotConfig( $arg->{'sec_opt'} ),  
			$oem[0], $oem[1], $oem[2]);
    substr ($bytes, 4, 2) = pack("S$self->{en}",length $bytes);
    #printf ("fldOemCot header size %d \n",length $bytes);
    return $bytes;
}

#sub modeEligible {
#    my $arg = shift;
#    my $opt = $arg->{'sec_opt'};
#    return ( defined $opt->{'encrypt'} )
#      ? ( $arg->{'sec_mode'} eq 'MFG' ? 2 : 4 )
#      : ( $arg->{'sec_mode'} eq 'UNSEC' ? 1 : ($arg->{sec_mode} eq 'MFG' ?2)&& $opt->{unsec_eligible}? 3:6) );
#}

sub modeEligible {
    my $arg = shift;
    my %mode=('MFG'=>0x2,'FLD'=>0x4,'FLD_OEM'=>0x4,'UNSEC'=>0x1);
    die ("Invalid Eligibility Mask") unless $mode{$arg->{sec_mode}};
    #test mode full eligible
    return ($mode{MFG}|$mode{FLD}|$mode{UNSEC}) unless (!$arg->{sec_opt}->{full_eligible}); 
    return $arg->{sec_opt}->{unsec_eligible} ? ($mode{$arg->{sec_mode}}|$mode{UNSEC}) : 
                                                        $mode{$arg->{sec_mode}} ;
}



sub print_hash {
    my $arg = shift;
    for my $_k ( keys %{$arg} ) {
        if ( defined $arg->{$_k} ) {
            print "$_k => $arg->{$_k}\n";
        }
    }
}

sub check_args {
    my $self = shift;
    my $args = $self->{args};
    
    if ( (! exists($args->{sec_mode}) and 
       ! exists($args->{chip})) ) {
         print $help;
         die ("$0 Invalid Arguments");
    }
}

sub avs_header {
    my $self = shift;
    my $img_len = shift;
    my $sig_len = shift;
    my $la = shift;
    my $crc_len = 4;
    my $hdr_len  = 24;
    # for 64 bit targets + 4 bytes for loading address
    #print "-0-0-0-0-0-0-0- $la";
    my $bytes = pack("(IIIII)$self->{en}", 0xd3e7fec6, $hdr_len,
                                $img_len + $sig_len + $hdr_len, 0, $la);
    substr($bytes, (length $bytes), $crc_len) = pack("I$self->{en}",$self->crc32($bytes));
    return $bytes;
}

sub avs_header_parse {
    my $self = shift;
    my $data = shift;
    # for 64 bit targets + 4 bytes for loading address
    #print "-0-0-0-0-0-0-0- $la";
    my %hdr=(); 
    #my ($magic, $hlen, $len, $flags, $la, $crc) = 
    ($hdr{magic}, $hdr{hlen}, $hdr{len}, $hdr{flags}, $hdr{la}, $hdr{crc}) = 
unpack("(IIIIII)$self->{en}",$data);
    #printf("\n%x CRC %x\n", $hdr{magic},$hdr{crc});
    return \%hdr;
}

sub prepare_header_avs {
    my $self = shift;
    my ( $img_len, $sig_len, $la ) = @_;
    return $self->avs_header($img_len, $sig_len, $la);
}

sub opt_unsec_elig  {
    my $self      = shift;
    my $opt       = $self->{'args'}->{'cred'};
    return ($opt =~ m/.*opt<.*(unsec_eligible).*>/); 
}

sub sec_arch_ver  {
    my $self      = shift;
    my $opt       = $self->{'args'}->{'cred'};
    if ($opt =~ m/.*opt<.*(version[ ]{0,4})=([ ]{0,4}[x0-9a-fA-F]{0,10}).*>/) {
	print "sec arch version $1 = $2 \n"; 
	return hex ($2);
    }
    return 0;
}

 
sub sbi_header {
    my $self      = shift;
    my ($img_len) = @_;
    my $arg       = $self->{'args'};
    my $auth_hdr;
    local *opt_lst_verify = sub  {
        my $lst = shift;
        if ($#$lst == 0) {
               print $help;
               die ("$0 Invalid Arguments");
        }
        foreach (@$lst)  {
           if (!defined($_)) {
               print $help;
               die ("$0 Invalid Arguments");
           }
        }
        return $lst;
    };

    local *opt2path = sub {
          my $tkn = shift;
          my $lst = $self->{args}->{cred};
          my $dir = undef;
          if ($lst =~ m/.*dir<([^>]*)>/g){
              $dir = $1;
          } else {
              $dir = './';
   	  }
          $lst = $self->{args}->{cred};
          $lst =~ m/$tkn<([^>]*)>/g;
           #print "to array $1 \n";
          my @res = map {$dir."/".$_} split(/ +/,$1);
	  opt_lst_verify(\@res);	  
          return \@res;
    };

    local *opt2var = sub {
	my $tkn =shift;
        my $arr = opt2path($tkn);
        #no strict 'refs';	
        #  if an input is an array then 
        #  it is assumed to be 
        #  an array of unsigned chars otherwise filename
        #printf ("sz %s\n", @$arr );
        foreach (@$arr) {
            my @var = $_;
            #printf("..... elem %s \n",$_); 
            $_ = ($#var)? $var[0] : $self->f2var("@var");
        }
        return $arr;
    };

    my  $tmp = $arg->{cred};
    if ($tmp =~ m/.*opt<enc.*/g ) {
        $arg->{sec_opt}->{encrypt} = '';
	print "------------- Will set encryption opt ----------\n";
    }

    if ($arg->{arch} eq 'GEN2') {
        $self->{lcot} = opt2var('cot');
	return $self->sbi_header_GEN2( $img_len );
    }
    if ($arg->{sec_mode} eq 'FLD' && $arg->{cred} =~ m/.*oem<.*/g) {
        $arg->{sec_mode} = 'FLD_OEM';
        $arg->{sec_opt}->{oem} = '';
        print "------------- Will set OEM ---------------------\n";
    }
    #print "Effective Sec Mode $arg->{sec_mode} \n"; 
    if ($tmp =~ m/.*opt<.*(unsec_eligible).*>/) {
       	print "$self->{class} found  $1  \n";
       	$arg->{sec_opt}->{$1} = $1;
    }
    if ($tmp =~ m/.*opt<.*(version[ ]{0,4})=([ ]{0,4}[x0-9a-fA-F]{0,10}).*>/) {
	 print "Got $1 = $2 \n"; 
	$arg->{sec_opt}->{$1} = $2;
    }
    if ($arg->{sec_mode} eq 'MFG') {
	if ($tmp =~ m/.*opt<.*(mfgpad).*>/) {
       	        print "$self->{class} found  $1  \n";
        	$arg->{sec_opt}->{$1} = $1;
        }
        $self->{lroe} = opt2var('roe'); 
        $self->{loem} = opt2var('oem');
	$auth_hdr = $self->mfg_header( $img_len );
    } elsif ($arg->{sec_mode} eq 'FLD') {
        $self->{lrot} = opt2var('rot'); 
        $auth_hdr = $self->fld_header( $img_len );
    } elsif ($arg->{sec_mode} eq 'FLD_OEM' ) {
        $self->{lrot} = opt2var('rot');
        $self->{loem} = opt2var('oem');
        $auth_hdr = $self->fldOem_header( $img_len );
    } else {
        opt_lst_verify([undef]);
    }
     
    return { 'unauth' => $self->unauth_header( $img_len + length $auth_hdr ), 'auth' => $auth_hdr };
}

sub sbi_trailer {
    my $self = shift;
    my ( $hdr, $img, $sig ) = @_;
    if ($self->{args}->{arch} eq 'GEN2') {
        return $self->sbi_trailer_GEN2($hdr, $img, $sig->{mfg});
    }
    my $unsec_crc_pad ;
    my $ver = $self->sec_arch_ver();
    if ($ver > 1 && $ver < 4 ) {
        my $pad = pack("a*", "\000" x (512-length($sig->{mfg})));
	#print " Pad Length : " . length($pad) . "mfg len: " .length($sig->{mfg}) . " fld len:" . length($sig->{fld}) ."\n";
        $sig->{fld} .= $pad;
        $sig->{mfg} .= $pad;
	#print " Pad Length : " . length($pad) . "mfg len: " .length($sig->{mfg}) . " fld len:" . length($sig->{fld}) ."\n";
        if ($self->opt_unsec_elig() && $self->{'args'}->{'sec_mode'} eq 'MFG') {
            $sig->{fld} = pack("a*", "\000" x length($sig->{mfg}));
            my $crc2 = $self->crc32($hdr . $img . "\000" x 512);
            #printf("CRC for mfg %x \n",$crc2);
            return $sig->{fld} . $sig->{mfg} . pack("I$self->{en}",$crc2);
        }   
    }
	
    return $sig->{fld} . $sig->{mfg} . pack("I$self->{en}",$self->crc32($hdr . $img));
}

# Wrapper api to call for header either UNSEC or MFG/FLD
sub prepare_header {
    my $self = shift;
    my ($img_len) = @_;
    return ( $self->{'args'}->{'sec_mode'} eq 'UNSEC' )
      ? $self->ubi_header($img_len)
      : $self->sbi_header($img_len);
}

# Wrapper api to call for trailer either UNSEC or MFG/FLD
sub prepare_trailer {
    my $self = shift;
    my ( $hdr, $img, $sig) = @_;
    #printf ("*********** Prep trailer in mode: %s \n",$self->{args}->{sec_mode});
    return ( $self->{'args'}->{'sec_mode'} eq 'UNSEC' )
      ? $self->ubi_trailer( $hdr, $img )
      : $self->sbi_trailer( $hdr, $img, $sig );
}

sub avs_header {
    my $self = shift;
    my $img_len = shift;
    my $sig_len = shift;
    my $la = shift;
    my $crc_len = 4;
    my $hdr_len  = 24;
    # for 64 bit targets + 4 bytes for loading address
    #print "-0-0-0-0-0-0-0- $la";
    my $bytes = pack("(IIIII)$self->{en}", 0xd3e7fec6, $hdr_len,
                                $img_len + $sig_len + $hdr_len, 0, $la);
    substr($bytes, (length $bytes), $crc_len) = pack("I$self->{en}",$self->crc32($bytes));
    return $bytes;
}

sub prepare_header_avs {
    my $self = shift;
    my ( $img_len, $sig_len, $la ) = @_;
    return $self->avs_header($img_len, $sig_len, $la);
}

1;
