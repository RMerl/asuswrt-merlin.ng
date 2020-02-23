#!/usr/bin/env perl 
package BRCM::SBI_DEP;

use strict;

#use warnings;
use bytes;
use FindBin qw($Bin);
use lib "$Bin";
use File::stat;
use parent qw(BRCM::SBI_UTIL);

#
# args a hash reference as in the following: {
#				'sec_mode' => "MFG/FLD/UNSEC",
#				'chip'=><6xxx>,
#				'out' => <output file or result as binary string >,
#				'in' => <input image or binary string>
#				'cred_dir' => path to the credentils directory,
#				'byteorder' => endianness little/big,
# 				}
# verbose - turn on/off versbosity flag
sub new {
    my $class = shift;
    my ( $args, $verbose ) = @_;
    my $byteorder = 'little';
    if ( !( defined $verbose ) ) {
        $verbose = 0;
    }

    unless ( $verbose == 0 ) {
        print "class: $class byteorder:$byteorder verbose: $verbose \n";
    }

    my %impl_features = (
        'GEN3' => {
            'chip' => {
                '63158' =>,
                '',
                '4908'  => '',
                '6858'  => '',
                '6856'  => '',
                '6846'  => '',
                '6878'  => '',
                '6836'  => '',
                '63178' => '',
                '47622' => ''
            },
            'sec_opt' => {
                'encrypt'  => '',
                'spu'      => '',
                'skp'      => '',
                'oem'      => '',
                'sign'     => '',
                'noheader' => ''
            },
            'sec_mode' => { 'MFG' => '', 'FLD' => '', 'UNSEC' => '' },
            'args'     => [
                {
                    'chip'      => '',
                    'sec_arch'  => '',
                    'sec_mode'  => '',
                    'byteorder' => '',
                    'in'        => '',
                    'out'       => ''
                },
                { 'sec_opt' => '', 'cred_dir' => '' }
            ],
        },
        'GEN2' => {
            'chip' => {
                '63138' => '',
                '63148' => '',
                '63381' => '',
                '6838'  => '',
                '6848'  => '',
                '63268' => ''
            },
            'sec_opt' =>
              { 'encrypt' => '', 'sign' => '', 'noheader' => '', 'spi' => '' },
            'sec_mode' => { 'MFG' => '', 'UNSEC' => '' },
            'args'     => [
                {
                    'chip'      => '',
                    'sec_arch'  => '',
                    'sec_mode'  => '',
                    'byteorder' => '',
                    'in'        => '',
                    'out'       => ''
                },
                { 'cred_dir' => '' },
                { 'sec_opt'  => '' },
                { 'max_size' => '', 'in1' => '' }
            ],
        },
        'byteorder' => { 'little' => '', 'big' => '' }
    );

    #my $self = {
    #		byteorder => $byteorder,
    #		verbose => $verbose,
    #		class => $class,
    #};
    #return bless($self,$class);

    my $self = bless {
        verbose         => $verbose,
        class           => $class,
        'impl_features' => \%impl_features
    }, $class;
    if ( defined $args ) {
        $self->init($args);
        if ( defined $args->{'byteorder'} ) {
            $byteorder = $args->{'byteorder'};
        }
        $self->{'args'} = $args;
    }
    $self->{'byteorder'} = $byteorder;
    $self->{'openssl'} = "openssl";
    return $self;
}

sub GEN3 {
    my $class = shift;
    my ( $byteorder, $verbose ) = @_;
    if ( !( defined $verbose ) ) {
        $verbose = 0;
    }

    unless ( $verbose == 0 ) { print "class: $class\n"; }

    my %cred = (
        'MFG' => {
            'MID'       => "mid.bin",
            'KROE2_PUB' => "Kroe2-mfg-pub.bin",
            'PLT'       => {
                'ROE_DATA_SIG'           => "mfgRoeData.sig",
                'ROE_DATA_SKP_SPU_SIG'   => "mfgRoeData_with_skp_spu.sig",
                'ROE_DATA_SKP_SIG'       => "mfgRoeData_with_skp.sig",
                'KROE2_PRIV_ENC_SKP_SPU' => "Kroe2-mfg-priv_with_skp_spu.enc",
                'KROE2_PRIV_ENC_SKP'     => "Kroe2-mfg-priv_with_skp.enc",
                'KROE2_PRIV_ENC'         => "Kroe2-mfg-priv.enc"
            },
            'OEM_DATA_SIG'     => 'mfgOemData.sig',
            'OEM_DATA2_SIG'    => 'mfgOemData2.sig',
            'OEM_KRSA_PUB'     => "Krsa-mfg-pub.bin",
            'OEM_MID_KAES_ENC' => "mid+Kaes-mfg.enc",
            'KAES_IV'          => "Kaes-mfg-iv.bin",
            'KAES_EK'          => "Kaes-mfg-ek.bin",
            'KRSA'             => "Krsa-mfg.pem"
        },
        'FLD' => {
            'MID'          => "mid.bin",
            'KROT_PUB'     => "Krot-fld-pub.bin",
            'OEM_DATA_SIG' => "fldOemData.sig",
            'OEM_KRSA_PUB' => "Krsa-fld-pub.bin",
            'OEM_KAES_ENC' => "Kaes-fld.enc",
            'OEM_KRSA'     => "Krsa-fld.pem",
            'KROE_EK'      => "Kroe-fld-ek.bin",
            'KROE_IV'      => "Kroe-fld-iv.bin",
            'OEM_KAES_EK'  => "Kaes-fld-ek.bin",
            'OEM_KAES_IV'  => "Kaes-fld-iv.bin",
            'KROT_RSA'     => "Krot-fld.pem",
        },
    );

    my %headers = (
        "mfgRoeCot" => {
            'type'             => 0x0001,
            'ver'              => 0x0000,
            'len'              => undef,
            'reserve'          => 0x0,
            'sig'              => undef,
            'kroe2MfgPub'      => undef,
            'encrKroe2MfgPriv' => undef,
            'mid'              => undef
        },
        "mfgOemCot" => {
            'type'              => 0x0002,
            'ver'               => 0x0001,
            'len'               => 0,
            "config"            => undef,
            "sig"               => undef,
            'mid'               => undef,
            "krsaMfgPub"        => undef,
            "encrMidAndKaesMfg" => undef
        },
        "fldRotCot" => {
            "type"       => 0x0003,
            "ver"        => 0x0001,
            "len"        => 0x0,
            "config"     => undef,
            "mid"        => undef,
            "krotFldPub" => undef
        },
        "fldOemCot" => {
            "type"        => 0x0004,
            "ver"         => 0x0001,
            "len"         => 0x0,
            "config"      => 0,
            'sig'         => undef,
            "mid"         => undef,
            "krsaFldPub"  => undef,
            'encrKaesFld' => undef
        },
        "SbiAuthHdrBeginning" => {
            'ver'          => 1,
            'hdrLen'       => 28,
            'authLen'      => undef,
            'mfgRoeCotOfs' => undef,
            'mfgOemCotOfs' => undef,
            'fldRotCotOfs' => undef,
            'fldOemCotOfs' => undef
        },
        "SbiUnauthHdrBeginning" => {
            'magic_1'      => 183954,
            'magic_2'      => 145257,
            'ver'          => 1,
            'modeEligible' => 0,
            'hdrLen'       => 28,
            'sbiSize'      => undef
        },
    );

    #my $self = {
    #		byteorder => $byteorder,
    #		verbose => $verbose,
    #		class => $class,
    #};
    #return bless($self,$class);
    my $self = bless {
        verbose   => $verbose,
        'cred'    => \%cred,
        'headers' => \%headers,
        'class'   => 'GEN3',
    }, $class;
    return $self;
}

sub GEN2 {
    my $class = shift;
    my ($verbose) = shift;
    if ( !( defined $verbose ) ) {
        $verbose = 0;
    }

    my %cred = (
        'MFG' => {
            'PLT' => {
                'COT'    => "mfg.cot.bin",
                'SIG'    => "mfg.cot.sig",
                'OP_COT' => "op.cot.bin",
                'OP_SIG' => "op.cot.sig",

                #Old Gen1 - only 63268 target
                'EK_ENC'    => "mfg.ek.enc",
                'IV_ENC'    => "mfg.iv.enc",
                'OP_EK_ENC' => "op.ek.enc",
                'OP_IV_ENC' => "op.iv.enc"
            },
            'EK'  => "mfg.ek.bin",
            'IV'  => "mfg.iv.bin",
            'RSA' => "mfg.pem"
        }
    );

    my %headers = (
        "SbiAuthHdr" => { 'type' => 0, 'ver' => 0, 'len' => 0, 'config' => 0 },
        "SbiUnauthHdr" => {
            'magic_1' => 112233,
            'magic_2' => 445566,
            'ver'     => 1,
            'len'     => undef,
            'crc'     => undef
        },
    );

    #my $self = {
    #		byteorder => $byteorder,
    #		verbose => $verbose,
    #		class => $class,
    #};
    #return bless($self,$class);
    my $self = bless {
        verbose   => $verbose,
        'cred'    => \%cred,
        'headers' => \%headers,
        'class'   => 'GEN2',
    }, $class;
    unless ( $verbose == 0 ) {
        print "class: $class subclass: $self->{'class'} \n";
    }
    return $self;
}

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

sub opt_to_map {
    my ( $opt, $map ) = @_;
    my %opt_map;
    local *_dfn = sub {
        $_[0] =~ m/.*($_[1]).*/;
        return $1;
    };
    for my $v ( keys %{$map} ) {
        if ( defined _dfn( $opt, $v ) ) {
            $opt_map{$v} = _dfn( $opt, $v );
        }
    }

    #print_hash(\%opt_map);
    return \%opt_map;
}

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
#		'in'=><val>,
#		'image_out'=><val>,
#	}
#);
sub process_args {
    my $self = shift;
    my $arg  = shift;
    my $sec_opt;
    my $impl_features = $self->{'impl_features'};
    local *args_defined = sub {
        my $var  = shift;
        my $feat = shift;
        for my $el ( keys %{$var} ) {
            if ( !defined $feat->{$el} ) {
                die "Invalid argument $el";
            }
        }
    };

    #print_hash($arg);
    # check if arch is supported
    #print_hash($arg);

  BREAK: for my $k_ ( keys %{$impl_features} ) {
        last BREAK if ( defined $arg->{'sec_arch'} );
        if ( defined $impl_features->{$k_}->{'chip'} ) {
            if ( defined $impl_features->{$k_}->{'chip'}->{ $arg->{'chip'} } ) {
                $arg->{'sec_arch'} = $k_;
            }
        }
    }
    print "SEC ARCH $arg->{'sec_arch'}\n";
    if ( !defined $arg->{'sec_arch'} ) {
        die "Invalid arguments";
    }

# verify if corresponding to arch implementation is available including permitted argument's set
    my $feat = $impl_features->{ $arg->{'sec_arch'} };
    args_defined( $feat->{'args'}[0], $arg );
    if ( !defined $feat->{'sec_mode'}->{ $arg->{'sec_mode'} } ) {
        die "Invalid argument $arg->{'sec_mode'}";
    }
    if ( $arg->{'sec_arch'} eq 'GEN3' ) {

        # verify features supported per arch
        if ( !( $arg->{'sec_mode'} eq 'UNSEC' ) ) {

            #args_defined($arg, $impl_features{$arg->{'sec_arch'}}->{'args1'});
            args_defined( $feat->{'args'}[1], $arg );
        }
    }
    if ( !defined $feat->{'chip'}->{ $arg->{'chip'} } ) {
        die "Chip id is not supported $arg->{'chip'}";
    }

    # map options per sec_mode/arch
    if ( defined $arg->{'sec_opt'} ) {
        $sec_opt = opt_to_map( $arg->{'sec_opt'}, $feat->{'sec_opt'} );
        $arg->{'sec_opt'} = $sec_opt;

    }

    #print_hash($arg->{'sec_opt'});
    my @opts_nkeys = keys %{ $arg->{'sec_opt'} };

    #printf ("opts_nkeys length %d\n",scalar @opts_nkeys);
    if ( $arg->{'sec_arch'} eq 'GEN2' ) {
        if ( $arg->{'sec_mode'} eq 'MFG' ) {
            args_defined( $feat->{'args'}[0], $arg );
            args_defined( $feat->{'args'}[1], $arg );
            if ( defined $sec_opt->{'spi'} ) {
                args_defined( $feat->{'args'}[2], $arg );
                args_defined( $feat->{'args'}[3], $arg );
            }
            if ( defined $arg->{'cred_dir'} ) {
                if ( -d "$arg->{'cred_dir'}/$arg->{'chip'}" ) {
                    $arg->{'plat_dir'} = "$arg->{'cred_dir'}/$arg->{'chip'}";
                }
                else {
                    $arg->{'plat_dir'} = "$arg->{'cred_dir'}";
                }
            }

            # Allowable set of options per mode
            if (
                (
                       defined $arg->{'sec_opt'}->{'noheader'}
                    or defined $arg->{'sec_opt'}->{'sign'}
                    or defined $arg->{'sec_opt'}->{'spi'}
                )
                and ( scalar @opts_nkeys ) != 1
              )
            {
                die "Options are not supported for the $arg->{'sec_mode'} mode";
                print_hash( $arg->{'sec_opt'} );
            }

        }
        else {
            if ( $arg->{'sec_mode'} eq 'UNSEC' and defined $arg->{'sec_opt'} ) {
                print "Options are ignored for the $arg->{'sec_mode'} mode\n";
            }
        }

        # verifying valid combinations for each arch{mode}
    }
    if ( $arg->{'sec_arch'} eq 'GEN3' ) {
        if ( defined $arg->{'cred_dir'} ) {
            if ( -d "$arg->{'cred_dir'}/$arg->{'chip'}" ) {
                $arg->{'plat_dir'} = "$arg->{'cred_dir'}/$arg->{'chip'}";
            }
            else {
                $arg->{'plat_dir'} = "$arg->{'cred_dir'}";
            }
        }
        my @keys_num = keys %{ $arg->{'sec_opt'} };
        if ( defined $arg->{'sec_opt'}->{'noheader'} ) {
            ( defined $arg->{'sec_opt'}->{'encrypt'} )
              or
              die "Options are not supported for the $arg->{'sec_mode'} mode";
            if ( ( scalar @keys_num ) == 3
                and !( defined $arg->{'sec_opt'}->{'oem'} ) )
            {
                die "Options are not supported for the $arg->{'sec_mode'} mode";
            }
        }
        elsif ( defined $arg->{'sec_opt'}->{'sign'} ) {
            if ( ( scalar @keys_num ) == 2
                and !( defined $arg->{'sec_opt'}->{'oem'} ) )
            {
                die "Options are not supported for the $arg->{'sec_mode'} mode";
            }
        }
    }

    #print_hash($arg->{'sec_opt'});
}

sub prepare_keys {
    my $self = shift;
    my $arg  = shift;
    my $cred = shift;
    my %keys;
    if ( $arg->{'sec_arch'} eq 'GEN2' ) {
        if ( $arg->{'sec_mode'} eq 'MFG' or $arg->{'sec_mode'} eq 'OP' ) {
            $keys{'MFG'}->{'ek'}  = $self->f2hex( $cred->{'MFG'}->{'EK'} );
            $keys{'MFG'}->{'iv'}  = $self->f2hex( $cred->{'MFG'}->{'IV'} );
            $keys{'MFG'}->{'rsa'} = $cred->{'MFG'}->{'RSA'};
        }
    }
    else {

        if ( defined $arg->{'sec_opt'}->{'encrypt'} ) {
            $keys{'MFG'}->{'ek'} = $self->f2hex( $cred->{'MFG'}->{'KAES_EK'} );
            $keys{'MFG'}->{'iv'} = $self->f2hex( $cred->{'MFG'}->{'KAES_IV'} );
            if ( defined $arg->{'sec_opt'}->{'oem'} ) {
                $keys{'FLD'}->{'ek'} =
                  $self->f2hex( $cred->{'FLD'}->{'OEM_KAES_EK'} );
                $keys{'FLD'}->{'iv'} =
                  $self->f2hex( $cred->{'FLD'}->{'OEM_KAES_IV'} );
            }
            else {
                $keys{'FLD'}->{'ek'} =
                  $self->f2hex( $cred->{'FLD'}->{'KROE_EK'} );
                $keys{'FLD'}->{'iv'} =
                  $self->f2hex( $cred->{'FLD'}->{'KROE_IV'} );
            }
        }
        $keys{'MFG'}->{'rsa'} = $cred->{'MFG'}->{'KRSA'};
        if ( defined $arg->{'sec_opt'}->{'oem'} ) {
            $keys{'FLD'}->{'rsa'} = $cred->{'FLD'}->{'OEM_KRSA'};
        }
        else {
            $keys{'FLD'}->{'rsa'} = $cred->{'FLD'}->{'KROT_RSA'};
        }
    }
    return \%keys;
}

sub prepare_headers {
    my $self = shift;
    my $arg  = shift;
    my $cred = $self->{'cred'};
    my $hdr  = $self->{'headers'};
    local *prepend_path = sub {
        my $_cred = shift;
        my $_dir  = shift;
        for my $key ( keys %{$_cred} ) {
            if ( $key !~ /.*(PLT).*/ ) {
                $_cred->{$key} = $_dir . "/" . $_cred->{$key};
            }
        }
    };

    # Prepare MFG cot
    #Rot Cot
    if ( defined $arg->{'cred_dir'} ) {
        prepend_path( $cred->{'MFG'},          $arg->{'cred_dir'} );
        prepend_path( $cred->{'MFG'}->{'PLT'}, $arg->{'plat_dir'} );
    }
    if ( $arg->{'sec_arch'} eq 'GEN3' ) {
        if ( defined $arg->{'cred_dir'} ) {
            prepend_path( $cred->{'FLD'}, $arg->{'cred_dir'} );
        }
        if (   $arg->{'sec_mode'} eq 'MFG'
            or $arg->{'sec_mode'} eq 'FLD' )
        {
            $hdr->{'mfgRoeCot'}->{'kroe2MfgPub'} =
              $self->f2var( $cred->{'MFG'}->{'KROE2_PUB'} );
            if ( $arg->{'chip'} eq '63158' ) {
                my $opt_map = $arg->{'sec_opt'};
                if ( defined $opt_map->{'skp'} ) {
                    $hdr->{'mfgRoeCot'}->{'sig'} =
                      ( defined $opt_map->{'spu'} )
                      ? $self->f2var(
                        $cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SKP_SPU_SIG'} )
                      : $self->f2var(
                        $cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SKP_SIG'} );
                    $hdr->{'mfgRoeCot'}->{'encrKroe2MfgPriv'} =
                      ( defined $opt_map->{'spu'} )
                      ? $self->f2var(
                        $cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC_SKP_SPU'} )
                      : $self->f2var(
                        $cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC_SKP'} );
                }
                else {
                    $hdr->{'mfgRoeCot'}->{'sig'} =
                      $self->f2var( $cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SIG'} );
                    $hdr->{'mfgRoeCot'}->{'encrKroe2MfgPriv'} =
                      $self->f2var(
                        $cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC'} );
                }

            }
            else {
                $hdr->{'mfgRoeCot'}->{'sig'} =
                  $self->f2var( $cred->{'MFG'}->{'PLT'}->{'ROE_DATA_SIG'} );
                $hdr->{'mfgRoeCot'}->{'encrKroe2MfgPriv'} =
                  $self->f2var( $cred->{'MFG'}->{'PLT'}->{'KROE2_PRIV_ENC'} );
            }
            if ( !( $arg->{'chip'} eq '4908' ) ) {
                $hdr->{'mfgRoeCot'}->{'ver'} = 2;
                $hdr->{'mfgRoeCot'}->{'mid'} =
                  $self->f2var( $cred->{'MFG'}->{'MID'} );
            }
            else {
                $hdr->{'mfgRoeCot'}->{'ver'} = 1;
            }

            # OemCo
            $hdr->{'mfgOemCot'}->{'config'} =
              mfgOemCotConfig( $arg->{'sec_opt'} );
            $hdr->{'mfgOemCot'}->{'mid'} =
              $self->f2var( $cred->{'MFG'}->{'MID'} );
            $hdr->{'mfgOemCot'}->{'sig'} =
              $self->f2var( $arg->{'chip'} eq '4908'
                ? $cred->{'MFG'}->{'OEM_DATA_SIG'}
                : $cred->{'MFG'}->{'OEM_DATA2_SIG'} );
            $hdr->{'mfgOemCot'}->{'krsaMfgPub'} =
              $self->f2var( $cred->{'MFG'}->{'OEM_KRSA_PUB'} );
            $hdr->{'mfgOemCot'}->{'encrMidAndKaesMfg'} =
              $self->f2var( $cred->{'MFG'}->{'OEM_MID_KAES_ENC'} );
        }

        if (   $arg->{'sec_mode'} eq 'MFG'
            or $arg->{'sec_mode'} eq 'FLD' )
        {
            #Prepare FLD cot
            $hdr->{'fldRotCot'}->{'config'} = fldRotConfig( $arg->{'sec_opt'} );
            $hdr->{'fldRotCot'}->{'mid'} =
              $self->f2var( $cred->{'FLD'}->{'MID'} );
            $hdr->{'fldRotCot'}->{'krotFldPub'} =
              $self->f2var( $cred->{'FLD'}->{'KROT_PUB'} );

            #fldOemCot
            $hdr->{'fldOemCot'}->{'config'} =
              fldOemCotConfig( $arg->{'sec_opt'} );
            $hdr->{'fldOemCot'}->{'sig'} =
              $self->f2var( $cred->{'FLD'}->{'OEM_DATA_SIG'} );
            $hdr->{'fldOemCot'}->{'krsaFldPub'} =
              $self->f2var( $cred->{'FLD'}->{'OEM_KRSA_PUB'} );
            $hdr->{'fldOemCot'}->{'encrKaesFld'} =
              $self->f2var( $cred->{'FLD'}->{'OEM_KAES_ENC'} );
        }
        if ( $arg->{'sec_mode'} eq 'UNSEC' ) {
            my $hdr = $hdr->{'SbiAuthHdrBeginning'};
            $hdr->{'hdrLen'}       = 32;
            $hdr->{'authLen'}      = 0;
            $hdr->{'mfgRoeCotOfs'} = 0;
            $hdr->{'mfgOemCotOfs'} = 0;
            $hdr->{'fldRotCotOfs'} = 0;
            $hdr->{'fldOemCotOfs'} = 0;
        }

    }
    else {
        if ( $arg->{'sec_arch'} eq 'GEN2' ) {
            my $hdr = $hdr->{'SbiAuthHdr'};

            #"SbiAuthHdr"=>{'type'=>0,'ver'=>0,'len'=>0, 'config'=>0},
            if ( $arg->{'sec_mode'} eq 'UNSEC' ) {

                #$hdr->{'len'}=32;
                #$hdr->{'authLen'}=0;
                #$hdr->{'mfgCotSig'} = 0;
                #$hdr->{'mfgCot'} = 0;
                #$hdr->{'opCot'} = 0;
                #$hdr->{'opCotSig'} = 0;
            }
            else {
            }
        }
    }

}

sub build_mfgRoeCot {
    my $self  = shift;
    my $arg   = shift;
    my $bytes = "\000";
    my $len_offs;
    my $acsr = BRCM::SBI_UTIL->ByteAccessor( \$bytes, $self->{'byteorder'} );

    $acsr->set_val( $arg->{'type'}, 'u16' );
    $len_offs = $acsr->set_val( $arg->{'ver'}, 'u16' );
    $acsr->set_val( 0,                          'u16' );
    $acsr->set_val( $arg->{'reserve'},          'u16' );
    $acsr->set_val( $arg->{'sig'},              's' );
    $acsr->set_val( $arg->{'kroe2MfgPub'},      's' );
    $acsr->set_val( $arg->{'encrKroe2MfgPriv'}, 's' );
    if ( defined $arg->{'mid'} ) {
        $self->set_val_at( \$bytes, length $bytes, $arg->{'mid'}, 's' );

        # This idiosyncrasy comes from the original bash script (tbd: remove)
        $self->set_val_at( \$bytes, length $bytes, $arg->{'mid'}, 's' );
    }
    $self->set_val_at( \$bytes, $len_offs, length $bytes, 'u16' );

# Done with blessed (clue to GC presumably since ther won't reference to it no more)
    $acsr = undef;
    return $bytes;
}

sub build_mfgOemCot {
    my $self  = shift;
    my $arg   = shift;
    my $bytes = "\000";
    my $len_offs;
    my $acsr = BRCM::SBI_UTIL->ByteAccessor( \$bytes, $self->{'byteorder'} );
    $acsr->set_val( $arg->{'type'}, 'u16' );
    $len_offs = $acsr->set_val( $arg->{'ver'}, 'u16' );
    $acsr->set_val( $arg->{'len'},               'u16' );
    $acsr->set_val( $arg->{'config'},            'u16' );
    $acsr->set_val( $arg->{'sig'},               's' );
    $acsr->set_val( $arg->{'mid'},               's' );
    $acsr->set_val( $arg->{'krsaMfgPub'},        's' );
    $acsr->set_val( $arg->{'encrMidAndKaesMfg'}, 's' );
    $self->set_val_at( \$bytes, $len_offs, ( length $bytes ), 'u16' );
    $acsr = undef;
    return $bytes;
}

sub build_fldRotCot {
    my $self  = shift;
    my $arg   = shift;
    my $bytes = "\000";
    my $len_offs;
    my $acsr = BRCM::SBI_UTIL->ByteAccessor( \$bytes, $self->{'byteorder'} );
    $acsr->set_val( $arg->{'type'}, 'u16', 0 );
    $len_offs = $acsr->set_val( $arg->{'ver'}, 'u16' );
    $acsr->set_val( $arg->{'len'},        'u16' );
    $acsr->set_val( $arg->{'config'},     'u16' );
    $acsr->set_val( $arg->{'mid'},        's' );
    $acsr->set_val( $arg->{'krotFldPub'}, 's' );
    $self->set_val_at( \$bytes, $len_offs, length $bytes, 'u16' );
    $acsr = undef;
    return $bytes;
}

sub build_fldOemCot {
    my $self  = shift;
    my $arg   = shift;
    my $bytes = "\000";
    my $len_offs;
    my $acsr = BRCM::SBI_UTIL->ByteAccessor( \$bytes, $self->{'byteorder'} );
    $acsr->set_val( $arg->{'type'}, 'u16' );
    $len_offs = $acsr->set_val( $arg->{'ver'}, 'u16' );
    $acsr->set_val( $arg->{'len'},         'u16' );
    $acsr->set_val( $arg->{'config'},      'u16' );
    $acsr->set_val( $arg->{'sig'},         's' );
    $acsr->set_val( $arg->{'krsaFldPub'},  's' );
    $acsr->set_val( $arg->{'encrKaesFld'}, 's' );
    $self->set_val_at( \$bytes, $len_offs, length $bytes, 'u16' );
    $acsr = undef;
    return $bytes;
}

sub build_auth_header {
    my $self = shift;
    my ( $arg, $_hdr, $sbi_len ) = @_;
    my $hdr_cot;
    my $bytes       = "\000";
    my $sbi_hdr     = $_hdr->{'SbiAuthHdrBeginning'};
    my $sbi_hdr_len = $sbi_hdr->{'hdrLen'};

    my $acsr = BRCM::SBI_UTIL->ByteAccessor( \$bytes, $self->{'byteorder'} );

    my $sbi_hdr_len_offs = $acsr->set_val( $sbi_hdr->{'ver'}, 'u32' );
    my $auth_len_offs    = $acsr->set_val( $sbi_hdr_len,      'u32' );
    $acsr->set_val( 0,            'u32' );
    $acsr->set_val( $sbi_hdr_len, 'u32' );

    $hdr_cot = $self->build_mfgRoeCot( $_hdr->{'mfgRoeCot'} );

    $acsr->set_val( ( length $hdr_cot ) + $sbi_hdr_len, 'u32' );
    $hdr_cot .= $self->build_mfgOemCot( $_hdr->{'mfgOemCot'} );
    $acsr->set_val( ( length $hdr_cot ) + $sbi_hdr_len, 'u32' );
    $hdr_cot .= $self->build_fldRotCot( $_hdr->{'fldRotCot'} );
    $acsr->set_val( ( length $hdr_cot ) + $sbi_hdr_len, 'u32' );
    $hdr_cot .= $self->build_fldOemCot( $_hdr->{'fldOemCot'} );
    $sbi_hdr_len = ( length $bytes ) + ( length $hdr_cot );
    $self->set_val_at( \$bytes, $sbi_hdr_len_offs, $sbi_hdr_len, 'u32' );
    $acsr = undef;
    return {
        'bytes'    => $bytes . $hdr_cot,
        'auth_len' => ( length $hdr_cot ) + ( length $bytes ) + $sbi_len
    };
}

sub modeEligible {
    my $arg = shift;
    my $opt = $arg->{'sec_opt'};
    return ( defined $opt->{'encrypt'} )
      ? ( $arg->{'sec_mode'} eq 'MFG' ? 2 : 4 )
      : ( $arg->{'sec_mode'} eq 'UNSEC' ? 1 : 6 );
}

sub build_unauth_header {
    my $self     = shift;
    my $arg      = shift;
    my $ubi_hdr  = shift;
    my $auth_len = shift;
    my $sig_len  = 256;
    my $crc_len  = 4;
    my $hdr_cot;
    my $bytes       = "\000";
    my $ubi_hdr_len = $ubi_hdr->{'hdrLen'};
    my $acsr = BRCM::SBI_UTIL->ByteAccessor( \$bytes, $self->{'byteorder'} );
    $acsr->set_val( $ubi_hdr->{'magic_1'},                              'u32' );
    $acsr->set_val( $ubi_hdr->{'magic_2'},                              'u32' );
    $acsr->set_val( $ubi_hdr->{'ver'},                                  'u32' );
    $acsr->set_val( modeEligible($arg),                                 'u32' );
    $acsr->set_val( $ubi_hdr_len,                                       'u32' );
    $acsr->set_val( $ubi_hdr_len + $auth_len + 2 * $sig_len + $crc_len, 'u32' );
    $acsr->set_val( $self->crc32($bytes),                               'u32' );
    $acsr = undef;
    return $bytes;
}

sub encrypt {
    my $self = shift;
    my $keys = shift;
    my $in   = shift;
    return $self->_encrypt_aes_128_cbc( $keys->{'ek'}, $keys->{'iv'},
        $self->f2var($in) );
}

sub print_hash {
    my $arg = shift;
    for my $_k ( keys %{$arg} ) {
        if ( defined $arg->{$_k} ) {
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
sub build_sbi {
    my $self        = shift;
    my $arg         = shift;
    my $cred        = shift;
    my $sbi_headers = shift;
    my $bytes       = "\000";
    my $keys;
    my $image;
    my $mfg_sig;
    my $fld_sig;
    my $auth_hdr;
    my $unauth_hdr;
    my $crc = 0;
    $keys = $self->prepare_keys( $arg, $cred );
    printf( "Building %s:%s headered secure image (SBI)\n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );

    if ( defined $arg->{'sec_opt'}->{'encrypt'} ) {
        $image = $self->encrypt( $keys->{ $arg->{'sec_mode'} }, $arg->{'in'} );
    }
    else {
        $image = $self->f2var( $arg->{'in'} );
    }
    $auth_hdr = $self->build_auth_header( $arg, $sbi_headers, length $image );
    $unauth_hdr = $self->build_unauth_header(
        $arg,
        $sbi_headers->{'SbiUnauthHdrBeginning'},
        $auth_hdr->{'auth_len'}
    );
    $image   = $auth_hdr->{'bytes'} . $image;
    $mfg_sig = $self->_sign_sha256( $keys->{'MFG'}->{'rsa'}, $image );
    $fld_sig = $self->_sign_sha256( $keys->{'FLD'}->{'rsa'}, $image );
    $image   = $unauth_hdr . $image . $fld_sig . $mfg_sig;
    $self->set_val_at( \$crc, 0, $self->crc32($image), 'u32' );
    $self->fdump( $arg->{'out'}, $image . $crc );
}

sub build_sbi_gen1 {
    my $self = shift;
    my $arg  = shift;
    my $cred = shift;
    my $keys;
    my $image;
    my $payload_len = 48 * 1024;
    local *_2var = sub { return $self->f2var( $_[0] ); };
    print "Building GEN1 headered secure image (UBI)\n";

    #print_hash(\%cred_data);
    #print_hash($arg->{'sec_opt'});
    #print_hash($arg);
    $keys = $self->prepare_keys( $arg, $cred );
    $keys = $keys->{ $arg->{'sec_mode'} };
    $self->set_val_at( \$image, 0, _2var( $arg->{'in'} ), 's' );
    if ( defined $arg->{'sec_opt'}->{'spi'} ) {
        if ( length $image > 128 * 1024 ) {
            die "Invalid image size must be less or equal 128K";
        }
        $payload_len = 1024 * 128;
        $self->set_val_at(
            \$image,
            length $image,
            "\377" x ( $payload_len - length $image ), 's'
        );
    }
    else {
        $self->set_val_at(
            \$image,
            length $image,
            "\000" x ( $payload_len - length $image ), 's'
        );
    }

    my $sig = $self->_sign_sha256( $keys->{'rsa'}, $image );

    # legacy calculations inherited from old gen1 image build
    # this will be removed soon for 63268
    # building NVRAM
    my $nvram = substr(
        (
            substr $sig
              . _2var( $cred->{'MFG'}->{'PLT'}->{'SIG'} )
              . _2var( $cred->{'MFG'}->{'PLT'}->{'COT'} ),
            0,
            770
        )
        . _2var( $cred->{'MFG'}->{'PLT'}->{'OP_SIG'} )
          . _2var( $cred->{'MFG'}->{'PLT'}->{'OP_COT'} ),
        0, 1284
      )
      . _2var( $cred->{'MFG'}->{'PLT'}->{'EK_ENC'} )
      . _2var( $cred->{'MFG'}->{'PLT'}->{'OP_EK_ENC'} )
      . _2var( $cred->{'MFG'}->{'PLT'}->{'IV_ENC'} )
      . _2var( $cred->{'MFG'}->{'PLT'}->{'OP_IV_ENC'} );

    #embedding nvram at offset 2432
    $self->set_val_at( \$image, 2432, $nvram, 's' );
    $self->fdump( $arg->{'out'}, $image );
}

sub build_sbi_gen2 {
    my $self = shift;
    my ( $arg, $cred, $sbi_headers ) = @_;
    my $bytes = "\000";
    my $keys;
    my $image;
    my $auth_hdr   = "\000";
    my $unauth_hdr = "\000";
    my $crc        = 0;
    printf( "Building %s:%s headered secure image (SBI)\n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );
    $keys = $self->prepare_keys( $arg, $cred );
    $keys = $keys->{ $arg->{'sec_mode'} };

    #print_hash($keys);
    $image = $self->f2var( $arg->{'in'} );

    #Authenticated header - all 0s
    $self->set_val_at( \$auth_hdr, 0,                0, 'u16' );
    $self->set_val_at( \$auth_hdr, length $auth_hdr, 0, 'u16' );
    $self->set_val_at( \$auth_hdr, length $auth_hdr, 0, 'u16' );
    $self->set_val_at( \$auth_hdr, length $auth_hdr, 0, 'u16' );

    $image =
        $auth_hdr
      . $self->f2var( $cred->{'MFG'}->{'PLT'}->{'COT'} )
      . $self->f2var( $cred->{'MFG'}->{'PLT'}->{'SIG'} )
      . $self->f2var( $cred->{'MFG'}->{'PLT'}->{'OP_COT'} )
      . $self->f2var( $cred->{'MFG'}->{'PLT'}->{'OP_SIG'} )
      . $image;

    my $sig = $self->_sign_sha256( $keys->{'rsa'}, $image );
    $image = $image . $sig;

    #printf ("image length 0x%x\n",length $image);
    $self->set_val_at( \$image, length $image, $self->crc32($image), 'u32' );

    #printf ("image length 0x%x\n",length $image);
    my $_hdr = $sbi_headers->{'SbiUnauthHdr'};

    #	"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1,
    #		'len'=>undef,'crc'=>undef },
    $self->set_val_at( \$unauth_hdr, 0, $_hdr->{'magic_1'}, 'u32' );
    $self->set_val_at( \$unauth_hdr, length $unauth_hdr,
        $_hdr->{'magic_2'}, 'u32' );
    $self->set_val_at( \$unauth_hdr, length $unauth_hdr, $_hdr->{'ver'},
        'u32' );
    $self->set_val_at(
        \$unauth_hdr,
        length $unauth_hdr,
        ( length $image ) + 20, 'u32'
    );
    $self->set_val_at(
        \$unauth_hdr,
        length $unauth_hdr,
        $self->crc32($unauth_hdr), 'u32'
    );
    $image = $unauth_hdr . $image;
    if ( defined $arg->{'sec_opt'}->{'spi'} ) {

   #add spi image which is expected to be second input image whithin the sec_opt
   # pad it up to the max_size then copy spi_image over pad area
   # cat spi image and $image then truncate it to the max_size
        my $spi_image = "\377" x ( $arg->{'max_size'} * ( 1024 / 2 ) );
        $self->set_val_at( \$spi_image, 0, $self->f2var( $arg->{'in1'} ), 's' );
        $image = substr $spi_image . $image, 0, ( $arg->{'max_size'} * 1024 );
        $self->set_val_at(
            \$image,
            length $image,
            "\377" x ( $arg->{'max_size'} * 1024 - ( length $image ) ), 's'
        );
    }
    $self->fdump( $arg->{'out'}, $image );
}

sub build_ubi {
    my $self        = shift;
    my $arg         = shift;
    my $sbi_headers = shift;
    my $bytes       = "\000";
    my $image;
    my $crc  = 0;
    my $_hdr = $sbi_headers->{'SbiAuthHdrBeginning'};
    printf( "Building %s:%s headered non-secure image (UBI)\n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );
    my $offs = $self->set_val_at( \$bytes, 0, $_hdr->{'ver'}, 'u32' );
    $self->set_val_at( \$bytes, $offs, $_hdr->{'hdrLen'}, 'u32' );
    $bytes .= "\000" x ( $_hdr->{'hdrLen'} - 8 );
    $image = $self->f2var( $arg->{'in'} );
    $_hdr  = $sbi_headers->{'SbiUnauthHdrBeginning'};
    $_hdr  = $self->build_unauth_header( $arg, $_hdr,
        ( length $image ) + ( length $bytes ) );
    $image = $bytes . $image;
    $self->set_val_at( \$crc, 0, $self->crc32($image), 'u32' );

    #my $img  =$_hdr.$image.("\000"x512).$crc;

    #printf ("--- building  ubi %s sz %d\n",$arg->{'out'},(length $img));
    $self->fdump( $arg->{'out'}, $_hdr . $image . ( "\000" x 512 ) . $crc );

    #$self->fdump($arg->{'out'}, $img);
}

sub build_ubi_gen2 {
    my $self        = shift;
    my $arg         = shift;
    my $sbi_headers = shift;
    my $bytes       = "\000";
    my $image;
    my $crc        = 0;
    my $auth_hdr   = "\000";
    my $unauth_hdr = "\000";
    my $_hdr       = $sbi_headers->{'SbiAuthHdr'};
    printf( "Building %s:%s headered non-secure image (UBI)\n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );
    $image = $self->f2var( $arg->{'in'} );

    #Authenticated header - all 0s
    $self->set_val_at( \$auth_hdr, 0,                0, 'u16' );
    $self->set_val_at( \$auth_hdr, length $auth_hdr, 0, 'u16' );
    $self->set_val_at( \$auth_hdr, length $auth_hdr, 0, 'u16' );
    $self->set_val_at( \$auth_hdr, length $auth_hdr, 0, 'u16' );

    # empty cots and signature
    $image = $auth_hdr . "\000" x ( 584 * 2 ) . $image;
    $self->set_val_at( \$crc, 0, $self->crc32($image), 'u32' );
    $image = $image . "\000" x (256) . $crc;
    $_hdr = $sbi_headers->{'SbiUnauthHdr'};

    #"SbiUnauthHdr"=>{'magic_1'=>112233,'magic_2'=>445566,'ver'=>1,
    #		'len'=>undef,'crc'=>undef },
    $self->set_val_at( \$unauth_hdr, 0, $_hdr->{'magic_1'}, 'u32' );
    $self->set_val_at( \$unauth_hdr, length $unauth_hdr,
        $_hdr->{'magic_2'}, 'u32' );
    my $offs = $self->set_val_at( \$unauth_hdr, length $unauth_hdr,
        $_hdr->{'ver'}, 'u32' );
    $self->set_val_at(
        \$unauth_hdr,
        length $unauth_hdr,
        ( length $image ) + 20, 'u32'
    );
    $self->set_val_at(
        \$unauth_hdr,
        length $unauth_hdr,
        $self->crc32($unauth_hdr), 'u32'
    );
    $self->fdump( $arg->{'out'}, $unauth_hdr . $image );
}

sub non_header_sbi {
    my $self = shift;
    my $arg  = shift;
    my $cred = shift;
    my $image;
    my $temp           = "./_tmp_data_t.$$";
    my $hdr_size       = 12;
    my $hdr_field_offs = 8;
    my %split_file     = ( "left" => 0, "right" => 0 );
    printf( "Building %s:%s non-headered secure image \n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );
    my $keys = $self->prepare_keys( $arg, $cred );

    #print_hash($arg);
    #print_hash($cred);
    $keys = $keys->{ $arg->{'sec_mode'} };

    #print_hash($keys);
    $self->fsplit( $hdr_size, $arg->{'in'}, \%split_file );
    $self->fdump( $temp, $split_file{"right"} );
    my $size = $self->compress_lzma( $temp, "$temp.comp",
        "$arg->{'buildtop'}/hostTools" );
    $image = $self->f2var("$temp.comp");

    #update header's size field
    $self->set_val_at( \$split_file{"left"}, $hdr_field_offs, $size, 'u32' );

    $image = $split_file{'left'} . $image;
    $image =
      $self->_encrypt_aes_128_cbc( $keys->{'ek'}, $keys->{'iv'}, $image );
    my $sig = $self->_sign_sha256( $keys->{'rsa'}, $image );
    $self->fdump( $arg->{'out'}, $sig . $image );
    $self->run_shell("rm -f $temp $temp.*");
}

sub sign_bi {
    my $self = shift;
    my $arg  = shift;
    my $cred = shift;
    my $keys = $self->prepare_keys( $arg, $cred );

    printf( "Building %s:%s image signature\n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );
    $keys = $keys->{ $arg->{'sec_mode'} };
    $self->sign_sha256( $keys->{'rsa'}, $arg->{'in'}, $arg->{'out'} );
}

sub build_gen3 {
    my $self    = shift;
    my $arg     = shift;
    my $cred    = $self->{'cred'};
    my $sbi_hdr = $self->{'headers'};

    if ( $arg->{'sec_mode'} eq "UNSEC" ) {
        $self->build_ubi( $arg, $sbi_hdr );
    }
    else {
        # MFG or FLD
        if (    defined $arg->{'sec_opt'}->{'noheader'}
            and defined $arg->{'sec_opt'}->{'encrypt'} )
        {
            $self->non_header_sbi( $arg, $cred );
        }
        else {
            if ( defined $arg->{'sec_opt'}->{'sign'} ) {
                $self->sign_bi( $arg, $cred );
            }
            else {
                $self->build_sbi( $arg, $cred, $sbi_hdr );
		#$self->run_shell("cp -rvf $arg->{'out'} sbi.bin");
		#$self->build_sbi3(); 
		#$self->run_shell("cp -rvf $arg->{'out'} sbi3.bin");
            }
        }
    }
}

sub build_gen2 {
    my $self    = shift;
    my $arg     = shift;
    my $cred    = $self->{'cred'};
    my $sbi_hdr = $self->{'headers'};
    if ( $arg->{'sec_mode'} eq "UNSEC" ) {
        $self->build_ubi_gen2( $arg, $sbi_hdr );
    }
    else {
        if ( defined $arg->{'sec_opt'}->{'noheader'} ) {
            $self->non_header_sbi( $arg, $cred );
        }
        else {
            if ( defined $arg->{'sec_opt'}->{'sign'} ) {
                $self->sign_bi( $arg, $cred );
            }
            else {
                if ( $arg->{'chip'} eq '63268' ) {
                    $self->build_sbi_gen1( $arg, $cred );
                }
                else {
                    $self->build_sbi_gen2( $arg, $cred, $sbi_hdr );
                }

            }
        }
    }
}

# To use this api set sec_mode => "UNSEC" in the constructor arg
# arg[0] - length of the image
# returns hash such as {'unauth'=>'string' , 'auth'=>'string'};
sub prepare_ubi_header {
    my $self         = shift;
    my $image_length = shift;
    my $arg          = $self->{'args'};
    my $auth_hdr     = "\000";
    my $hdr          = $self->{'headers'}->{'SbiAuthHdrBeginning'};
    my $offs         = $self->set_val_at( \$auth_hdr, 0, $hdr->{'ver'}, 'u32' );
    $self->set_val_at( \$auth_hdr, $offs, $hdr->{'hdrLen'}, 'u32' );
    $auth_hdr .= "\000" x ( $hdr->{'hdrLen'} - 8 );
    $hdr = $self->{'headers'}->{'SbiUnauthHdrBeginning'};
    $hdr =
      $self->build_unauth_header( $arg, $hdr,
        $image_length + ( length $auth_hdr ) );
    return { 'auth' => $auth_hdr, 'unauth' => $hdr };
}

# To use this api set sec_mode => "UNSEC" in the constructor arg
#  arg[0] - auth header  string
#  arg[1] - an image string
# returns trailer string
sub prepare_ubi_trailer {
    my $self = shift;
    my ( $hdr, $image ) = @_;
    my $crc = 0;
    $self->set_val_at( \$crc, 0, $self->crc32( $hdr . $image ), 'u32' );

    #printf ("--- building  ubi %s sz %d\n",$arg->{'out'},(length $img));
    return ( "\000" x 512 ) . $crc;
}

# To use this api  set sec_mode => "FLD/MFG" in the constructor arg
# arg[0] - length of the image
# returns hash such as {'unauth'=>'string' , 'auth'=>'string'};
sub prepare_sbi_header {
    my $self           = shift;
    my ($image_length) = @_;
    my $arg            = $self->{'args'};
    my $auth_hdr =
      $self->build_auth_header( $arg, $self->{'headers'}, $image_length );
    my $unauth_hdr = $self->build_unauth_header(
        $arg,
        $self->{'headers'}->{'SbiUnauthHdrBeginning'},
        $auth_hdr->{'auth_len'}
    );
    return { 'unauth' => $unauth_hdr, 'auth' => $auth_hdr->{'bytes'} };
}

#  arg[0] - auth/unauth header  string
#  arg[1] - hash to a signatures such as {'fld'=><fld signature>,'mfg'=><mfg signature>}
#  arg[2] - an image string
# returns trailer string
sub prepare_sbi_trailer {
    my $self = shift;
    my ( $hdr, $image, $sig ) = @_;
    my $trl = length $hdr . $image;
    my $crc = 0;
    $image = $hdr . $image . $sig->{'fld'} . $sig->{'mfg'};
    $self->set_val_at( \$crc, 0, $self->crc32($image), 'u32' );
    return substr( $image . $crc, $trl );
}

# Wrapper api to call for header either UNSEC or MFG/FLD
sub prepare_header {
    my $self = shift;
    my ($image_length) = @_;
    return ( $self->{'args'}->{'sec_mode'} eq 'UNSEC' )
      ? $self->prepare_ubi_header($image_length)
      : $self->prepare_sbi_header($image_length);
}

# Wrapper api to call for trailer either UNSEC or MFG/FLD
sub prepare_trailer {
    my $self = shift;
    my ( $hdr, $image, $sig) = @_;
    return ( $self->{'args'}->{'sec_mode'} eq 'UNSEC' )
      ? $self->prepare_ubi_trailer( $hdr, $image )
      : $self->prepare_sbi_trailer( $hdr, $image, $sig );
}

# Input: Image_length
# Credentials for both  FLD/MFG
# Outpu 2 headers
sub gen_sbi_sig {
    my $self    = shift;
    my ($arg, $image, $keys) = @_;
    my $mfg     = $self->_sign_sha256( $keys->{'MFG'}->{'rsa'}, $image );
    my $fld     = $self->_sign_sha256( $keys->{'FLD'}->{'rsa'}, $image );
    return { 'mfg' => $mfg, 'fld' => $fld };
}

sub encrypt_sbi {
    my $self = shift;
    my ( $keys, $image ) = @_;
    return $self->_encrypt_aes_128_cbc( $keys->{'ek'}, $keys->{'iv'}, $image );
}

sub init {
    my $self = shift;
    my $arg  = shift;
    $self->process_args($arg);
    my $obj =
      $arg->{'sec_arch'} eq 'GEN3' ? BRCM::SBI_DEP->GEN3() : BRCM::SBI_DEP->GEN2();
    $self->{'cred'}    = $obj->{'cred'};
    $self->{'headers'} = $obj->{'headers'};
    $self->prepare_headers($arg);
}

sub build_ubi3 {
    my $self  = shift;
    my $arg   = $self->{'args'};
    my $image = $self->f2var( $arg->{'in'} );

    #my $hdr = $self->prepare_ubi_header(length $image);
    my $hdr = $self->prepare_header( length $image );

    #my $trailer = $self->prepare_ubi_trailer($hdr->{'auth'},$image);
    my $trailer = $self->prepare_trailer( $hdr->{'auth'}, $image );
    $self->fdump( $arg->{'out'},
        $hdr->{'unauth'} . $hdr->{'auth'} . $image . $trailer );
}

# Example
sub build_sbi3 {
    my $self  = shift;
    my $arg   = $self->{'args'};
    my $crc   = 0;
    my $image = $self->f2var( $arg->{'in'} );
    my $keys  = $self->prepare_keys( $arg, $self->{'cred'} );
    printf( "Building %s:%s headered secure image (SBI)\n",
        $arg->{'sec_arch'}, $arg->{'sec_mode'} );

    if ( defined $arg->{'sec_opt'}->{'encrypt'} ) {

        #conditionally ecnryp SBI GEN3 as per protocol
        $image = $self->encrypt_sbi( $keys->{ $arg->{'sec_mode'} }, $image );
    }

    # generate authenticated/unauthenticated pair
    #my $hdr = $self->prepare_sbi_header(length $image);
    my $hdr = $self->prepare_header( length $image );

    # generate signature pair for both FLD/MFG
    my $sig = $self->gen_sbi_sig($arg, $hdr->{'auth'} . $image, $keys);
    $hdr = $hdr->{'unauth'} . $hdr->{'auth'};

    #my $trailer = $self->prepare_sbi_trailer($hdr,$sig,$image);
    my $trailer = $self->prepare_trailer( $hdr, $image, $sig );
    $image = $hdr . $image . $trailer;
    $self->fdump( $arg->{'out'}, $image );
}

sub build {
    my $self = shift;
    my $arg  = $self->{'args'};
    $arg->{'sec_arch'} eq 'GEN2'
      ? $self->build_gen2($arg)
      : $self->build_gen3($arg);
}

1;

#sub run {
#
#	$args{'chip'}="63158";
#	$args{'sec_arch'}="GEN3";
#	$args{'sec_opt'}="encrypt";
#	$args{'byteorder'}="little";
#	$args{'sec_mode'}="MFG";
#	$args{'cred_dir'}="../../cfe/cfe/board/bcm63xx_btrm/data/gen3_common";
#	$args{'in'}="../../targets/cfe/cfe63158rom.bin";
#	$args{'out'}="sbi_mfg.bin";
#	build_sbi(\%args);
#}

#	$sbi_lib->build( {'sec_mode' => "UNSEC",
#			'chip'=>$ENV{BRCM_CHIP},
#			'sec_arch'=>"$ENV{SECURE_BOOT_ARCH}",
#			'out' => $btldr,
#			'in' => $cferom,
#			'cred_dir' => nrmz($ENV{SEC_CRED_DIR}),
#			'byteorder' => $ENV{ARCH_ENDIAN}});
#run();
