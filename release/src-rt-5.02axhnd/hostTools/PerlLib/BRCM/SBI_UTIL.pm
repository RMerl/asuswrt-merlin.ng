#!/usr/bin/env perl
#package BRCM::UTIL;
package BRCM::SBI_UTIL;
use strict;
use Digest::CRC;

#use warnings;
#use Crypt:RSA;
use File::stat;

sub new {
    my $class = shift;
    my ( $byteorder, $verbose ) = @_;
    if ( !( defined $byteorder ) ) {
        $byteorder = 'little';
        $verbose   = 0;
    }
    elsif ( !( defined $verbose ) ) {
        $verbose = 0;
    }

    #print "class: $class byteorder:$byteorder verbose:$verbose\n";
    my $self = bless {
        byteorder => $byteorder,
        verbose   => $verbose,
        class     => $class,
    }, $class;
    return $self;
}

sub print_hash {
    my $arg = shift;
    for my $_k ( keys %{$arg} ) {
        if ( defined $arg->{$_k} ) {
            print "$_k => $arg->{$_k}\n";
        }
    }
}

#Dumps file to hexamdecimal chars
sub f2hex {
    my $self  = shift;
    my $fname = shift;
    my $fin   = undef;
    my $data;
    my $var;
    open( $fin, "<", "$fname" ) or die "$0: Failed to open $fname $!";
    binmode $fin;
    local $/ = undef;
    $data = <$fin>;
    $var = unpack( 'H*', $data );
    close($fin) or die "$0: Failed to open $fname $!";
    return $var;
}

#hex char to binary
# $data an input of hexadecimal characters 
sub asciihex2bin {
    my $self  = shift;
    my $data   = shift;
    	
    return pack('H*', $data);
}

sub mod2bin {
	my $self  = shift;
	my $in = shift;
	my $out = shift;
        my $modulus_size = shift;
	if ($modulus_size == undef) {
		#numbers of hex chars in the modulus
		$modulus_size = 512;	
        }
	open(my $fin, "<", $in ) or die "$0: Failed to open $in $!";
	binmode $fin;
	local $/ = undef;
	#my $data = <$fin>;
	my ($preamble,$modulus) = split(/Modulus=/,<$fin>);
	$self->fdump($out, $self->asciihex2bin(substr $modulus, 0, $modulus_size));
	close($fin);
}
sub crc32 {
    my $self = shift;
    my $data = shift;
    my $ctx  = new Digest::CRC(
        width  => 32,
        poly   => 0x04c11db7,
        init   => 0xffffffff,
        xorout => 0,
        refin  => 1,
        refout => 1
    );
    $ctx->add($data);
    return $ctx->digest;
}

sub fappend {
    my $self = shift;
    my $in   = shift;
    my $data = shift;
    open( my $fhndl, ">>", "$in" ) or die "$0: Failed to open $in $!";

    #binmode $fhndl;
    #printf "Length of the data:0x%x\n", length $data;
    print $fhndl $data;
    close($fhndl) or die "$0: Failed to close $!";
}

sub fdump {
    my $self = shift;
    my $in   = shift;
    my $data = shift;
    open( my $fhndl, "+>", "$in" ) or die "$0: Failed to create $in $!";
    binmode $fhndl;

    #printf "Length of the data:0x%x\n", length $data;
    print $fhndl $data;
    close($fhndl) or die "$0: Failed to close $!";
}

sub fsplit {
    my $self = shift;
    my ( $split_size, $in, $split_struct ) = @_;
    open( my $fin, "<", "$in" ) or die "$0: Failed to open $in $!";
    binmode $fin;
    local $/ = undef;
    my $data = <$fin>;
    close($fin) or die "$0: Failed to close $!";
    $split_struct->{"left"} = substr $data, 0, $split_size;
    $split_struct->{"right"} = substr $data, $split_size, length $data;
}

# set value in a byte array
sub set_val_at {
    my $self = shift;

    #print_hash($self);
    my ( $bytes, $offs, $val, $type, $endian ) = @_;
    if ( !( defined $endian ) ) {

#print "Using global endianness flag: $self->{'class'} $self->{'verbose'} $self->{'byteorder'}\n";
        $endian = $self->{'byteorder'};
    }

    #print "---> $bytes $offs $val $type $endian \n";
    my $pack_args = {
        s   => { little => 'a', big => 'a' },
        u8  => { little => 'C', big => 'C' },
        u16 => { little => 'v', big => 'n' },
        u32 => { little => 'V', big => 'N' },
        u64 => { little => 'Q', big => 'Q' }
    };
    my $arg = $pack_args->{$type}->{$endian};

    #print " val =  $val	\n";
    if ( $arg eq 'a' ) {

        #printf("%s offs %d val length %d\n", $arg, $offs, length $val);
        substr( $$bytes, $offs, length $val ) = pack( "$arg*", $val );
    }
    else {
        my $_v = pack( "$arg*", $val );

#printf("val_type %s offset %d val_length %d string length %d\n", $arg, $offs, length $_v,length $$bytes);
        substr( $$bytes, $offs, length $_v ) = $_v;
    }

    #printf("new length %d\n", length $$bytes);

    return length $$bytes;
}

sub ByteAccessor {
    my $class = shift;
    my ( $bytes, $byteorder, $next_offset, $verbose ) = @_;
    defined $bytes or die "ERROR: undefined class args";
    if ( !( defined $next_offset ) ) {
        $next_offset = 0;
    }
    if ( !( defined $byteorder ) ) {
        $byteorder = 'little';
    }

    #$verbose=1;
    if ( !( defined $verbose ) ) {
        $verbose = 0;
    }
    my $self = bless {
        bytes       => $bytes,
        byteorder   => $byteorder,
        next_offset => $next_offset,
        verbose     => $verbose,
        class       => 'ByteAccessor',
    }, $class;
    if ( $verbose == 1 ) {
        print
"owner:$class obj:$self:$self->{'class'}, byteorder:$self->{'byteorder'} verbose:$verbose\n";
    }
    return $self;
}

# must be instantiated to a ByteAccessor
## Set value iteratively to the next offset
# if offset is specified it initializes internal next_offset to the 'offset'
# if next_offset is not defined - defaulted to 0
# otherwise it is being updated to the end of the next value
sub set_val {
    my $self = shift;
    my ( $val, $type, $offset ) = @_;
    $self->{'class'} eq 'ByteAccessor'
      or die "ERROR: Subclass is not supported";

    #print "Setting at offset @ $self->{'next_offset'}\n";
    if ( defined $offset ) {
        $self->{'next_offset'} = $offset;
    }
    $self->{'next_offset'} =
      $self->set_val_at( $self->{'bytes'}, $self->{'next_offset'}, $val,
        $type );

    #print "Next offset $self->{'next_offset'}\n";
    return $self->{'next_offset'};
}

sub append {
    my $self = shift;
    my $str  = shift;
    $self->{'class'} eq 'ByteAccessor'
      or die "ERROR: Subclass is not supported";
    ${ $self->{bytes} } .= $str;

    #printf ("%s length %d\n",unpack('H*',$str), length $str);
    $self->{next_offset} = length ${ $self->{bytes} };
    return $self->{'next_offset'};
}

sub cat_typed_val {
    my $self = shift;
    my ( $bytes, $val, $type, $endian ) = @_;
    if ( !( defined $endian ) ) {

        #print "Using global endianness flag: $self->{'byteorder'}\n";
        $endian = $self->{'byteorder'};
    }
    my $pack_args = {
        s   => { little => 'a', big => 'a' },
        u8  => { little => 'C', big => 'C' },
        u16 => { little => 'v', big => 'n' },
        u32 => { little => 'V', big => 'N' },
        u64 => { little => 'Q', big => 'Q' }
    };

    my $arg = $pack_args->{$type}->{$endian};

    #$$bytes .= pack("$arg*",$val);
    substr( $$bytes, length $$bytes, length $val ) = pack( "$arg*", $val );

    #printf("\n$0 length %d %s \n", length $$bytes, unpack('H*' ,$$bytes));
    return length $$bytes;
}

sub f2var {
    my $self = shift;
    my $in   = shift;
    open( my $fin, "<", "$in" ) or die "$0: Failed to open $in $!";
    binmode $fin;
    local $/ = undef;
    my $data = <$fin>;
    close($fin) or die "$0: Failed to close $!";
    return $data;
}

sub _encrypt_aes_128_cbc {
    my $self = shift;
    my ( $ek, $iv, $in ) = @_;
    my $out;
    my $tmpin  = "/tmp/.tmpin_aes_$$";
    my $tmpout = "/tmp/.tmpout_aes_$$";

    #printf("Encrypting %s \n",unpack("H*",$in));
    $self->fdump( $tmpin, $in );
    $self->run_shell(
        "openssl enc -aes-128-cbc -K $ek -iv $iv -in $tmpin -out $tmpout");
    $out = $self->f2var($tmpout);

    #printf("%d %s key %s iv %s \n",length $out, unpack("H*",$out),$ek,$iv );
    $self->run_shell("rm -f $tmpout $tmpin");
    return $out;
}

sub _sign_sha256 {
    my $self = shift;
    my $pem  = shift;
    my $in   = shift;
    my $out;
    my $tmpin  = "/tmp/.tmpin_sha_$$";
    my $tmpout = "/tmp/.tmpout_sha_$$";
    if ( !( -e $pem ) ) {
        die "Not existing $pem";
    }
    $self->fdump( $tmpin, $in );

    #print "Signing $if with $pem res $of\n";
    $self->run_shell(
"openssl dgst -sign $pem -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $tmpout $tmpin"
    );
    $out = $self->f2var($tmpout);
    $self->run_shell("rm -f $tmpout $tmpin");
    return $out;
}

sub encrypt_aes_128_cbc {
    my $self     = shift;
    my $key_file = shift;
    my $iv_file  = shift;
    my $if       = shift;
    my $of       = shift;
    my $bek      = $self->f2hex("$key_file");
    my $biv      = $self->f2hex("$iv_file");
    $self->run_shell(
        "openssl enc -aes-128-cbc -K $bek -iv $biv -in $if -out $of");
}

sub sign_sha256 {
    my $self = shift;
    my ( $pem, $if, $of ) = @_;
    if ( !( -e $pem ) or !( -e $if ) ) {
        die "Not existing $pem or $if";
    }

    #print "Signing $if with $pem res $of\n";
    $self->run_shell(
"openssl dgst -sign $pem -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $of $if"
    );
}

sub compress_lzma {
    my $self = shift;
    my ( $if, $of, $tools ) = @_;
    $self->run_shell("$tools/lzmacmd e $if $of -d22 -lp2 -lc1");
    my $fi = stat($of);
    return $fi->size;
}

sub run_shell {
    my $self = shift;
    my $cmd  = shift;
    if ( $self->{'verbose'} > 1 ) {
        print "$0: $cmd\n";
    }
    system($cmd) == 0 or die "ERROR: $cmd had failed";
}
1;
