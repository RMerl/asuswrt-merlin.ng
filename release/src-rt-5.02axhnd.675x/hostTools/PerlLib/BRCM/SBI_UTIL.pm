#!/usr/bin/env perl
#package BRCM::UTIL;
package BRCM::SBI_UTIL;
use strict;
use Digest::CRC;
use POSIX qw(mkfifo);
use POSIX qw(:sys_wait_h);
#use warnings;
#use Crypt:RSA;
use File::stat;

sub new {
    my $class = shift;
    my ( $byteorder, $verbose, $ssldir ) = @_;
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

    $self->{openssl} = $ssldir? $ssldir . "/openssl":"openssl";
    $self->{fifo_rd} = "__sbi_util_fifo_rd_$$";
    $self->{fifo_wr} = "__sbi_util_fifo_wr_$$";
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

sub rsa_comp2bin {
    my $self  = shift;
    my $in  = shift;
    local *_2H = sub {
	my $data = shift;
	$data =~ s/[^[:xdigit:]]//g;
        #print "\n\nhex@@ $data \n\n" ;
        if ( $data =~ m/^00.*/s)  {
           # Some components may start with a 00 while others may not. Leading 
           # zeros are needed in the DER representation of positive integers     
           # whose most significant nibble is in the range from 8 to F. Otherwise 
           # the leading bit would cause the integer to be interpreted as negative
           # We will strip this out. (Original comment of B. Nay)
           # Clarifying : since we're not DER 2 leading zeros at the beginning of 
           # of the component are stripped out                                             
             #print "\n Unstripped $data \n";
             $data = substr($data, 2, (length($data)-2));
             #print "\n Stripped $data \n";
        }
        return $data; 
    };
    if (ref($in) eq 'HASH' && exists ($in->{file})) {
       $in = $self->f2var($in->{file});
    }
    my $_P = "[^[:xdigit:]:]*(.*)[^[:xdigit:]:]+";
    $in =~ m/prime1:${_P}prime2:${_P}exponent1:${_P}exponent2:${_P}coefficient:${_P}BEGIN/s;
    die "ERROR: Unable to extract components \n $1 \n  $2 \n  $3 \n $4 \n $5 \n" unless ($1 && $2 && $3 && $4 && $5);
    return {"comp",{"-p", $1, "-q", $2, "-dmp1", $3, "-dmq1",$4,"-iqmp", $5},
           "blob", pack("H*",_2H($1) . _2H($2) . _2H($3) ._2H($4) . _2H($5))};
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
    my $_in  = "/tmp/._tmpin_encrypt_$$";
    my $_out = "/tmp/._tmpout_encrypt_$$";

    #printf("Encrypting %s \n",unpack("H*",$in));
    $self->fdump( $_in, $in );
    $self->run_shell(
        "$self->{openssl} enc -aes-128-cbc -K $ek -iv $iv -in $_in -out $_out");
    $out = $self->f2var($_out);

    #printf("%d %s key %s iv %s \n",length $out, unpack("H*",$out),$ek,$iv );
    $self->run_shell("rm -f $_out $_in");
    return $out;
}

sub rand {
    my $self = shift;
    my $cnt = shift;
    my $__out = "/tmp/.__$1_rand_$$";
    $self->run_shell("dd if=/dev/random of=$__out bs=1 count=$cnt");
    my $res = $self->f2var($__out);
    $self->run_shell("rm -rf $__out");
    return $res;
}

sub sha256 {
    my $self = shift;
    my $in = shift;
    my $__in  = "/tmp/._$1__sha256_$$";
    my $__out  = "/tmp/._$1__sha256_out$$";

    #printf("Encrypting %s \n",unpack("H*",$in));
    if (ref($in) eq "HASH" && exists $in->{file}) {
       $__in =  $in->{file};
    } else {
       $self->fdump( $__in, $in );
    } 
    $self->run_shell( "cat $__in|$self->{openssl} dgst -sha256 -binary > $__out");
    my $res = $self->f2var($__out);
    #printf("%d %s key %s iv %s \n",length $out, unpack("H*",$out),$ek,$iv );
    $self->run_shell("rm -f $__out $__in");
    return $res;
}

sub aes_128_cbc {
    my $self = shift;
    my ($in, $key, $decrypt) = @_;
    my $out;
    my $ek = "";
    my $iv = "";
    my $__in  = "/tmp/._aes128cbc_in_$$";
    my $__out = "/tmp/._aes128cbc_out_$$";

    #printf("Encrypting %s \n",unpack("H*",$in));
    if (ref($in) eq "HASH" && exists $in->{file}) {
       $__in =  $in->{file};
    } else {
       $self->fdump( $__in, $in );
    } 
    if (ref($key->{ek}) eq "HASH" && exists $key->{ek}->{file}) {
       $ek = "-K " . $self->f2hex($key->{ek}->{file});
    } elsif ($key->{ek} && !$key->{iv}) {
       $ek = "-k " . $key->{ek};
    } elsif ($key->{ek} && $key->{iv}) {
       $ek = "-K " . $key->{ek};
    }
    if (ref($key->{iv}) eq "HASH" && exists $key->{iv}->{file}) {
       $iv ="-iv " . $self->f2hex($key->{iv}->{file});
    } elsif ($key->{iv}) {
       $iv = "-iv " . $key->{iv};
    } 

    if ($decrypt) {
	$decrypt = "-d";
    } else {
	$decrypt = "";
    }
    #if ek and iv are empty - openssl will wait for password from STDIN 
     print "Running $self->{openssl}";
    $self->run_shell(
          "$self->{openssl} enc $decrypt -aes-128-cbc $ek $iv -in $__in -out $__out");
    my $res = $self->f2var($__out);

    #printf("%d %s key %s iv %s \n",length $out, unpack("H*",$out),$ek,$iv );
    $self->run_shell("rm -f $__out $__in");
    return $res;
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
"$self->{openssl} dgst -sign $pem -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $tmpout $tmpin"
    );
    $out = $self->f2var($tmpout);
    $self->run_shell("rm -f $tmpout $tmpin");
    return $out;
}

sub cat {
	my ($self, @fls) = @_;
	my $str = undef;
	foreach (@fls) {
		if (ref ($_) eq 'HASH' && exists ($_->{data})) { 
		    $str .= $_->{data};
                } else {
		    $str .= $self->f2var($_);
                }
        }
	return $str;
}

sub cipher {
    my ( $self, $data, $key, $flag ) = @_;
    my %enc_key = ( "ek", undef, "iv", undef );
    if (! (ref ($key) eq 'HASH') ) {
        $enc_key{ek} = $key;
    } else {
        ($enc_key{ek}, $enc_key{iv}) = ($key->{ek}, $key->{iv});
    }

    # if flag is set - decrypting
    return $flag
      ? $self->aes_128_cbc( $data, \%enc_key, "true" )
      : $self->aes_128_cbc( $data, \%enc_key );
}

sub sign {
    my $self = shift;
    my $pem  = shift;
    my $in   = shift;
    my $out;
    my $_in  = "/tmp/./.___sign_1_sha_$$";
    my $_out = "/tmp/./.___sign_2_sha_$$";
    my $_pem;
    my $cmd; 
    if (ref($in) eq 'HASH' && exists($in->{file})) {
        $_in = $in->{file};
    } else {
        $self->fdump( $_in, $in );
    }

    if (ref ($pem) eq 'HASH' && exists($pem->{cmd})) {
        $cmd = "cat $_in" . $pem->{cmd} . $_out;
    }else {
        $_pem = "/tmp/./.___sign_3_sha_$$";
        $self->fdump( $_pem, $pem );
        #print "Signing $if with $pem res $of\n";
        $cmd = "$self->{openssl} dgst -sign $_pem -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $_out $_in"
    }


    my $res = system($cmd); 
    $out = $self->f2var($_out);
    $self->run_shell("rm -f $_out $_in");
    if ($_pem) {
        $self->run_shell("rm -f $_pem");
    }
    die "ERROR: $cmd had failed" unless ($res == 0 ); 
    return $out;
}

sub authenticate {
    my $self = shift;
    my $pem  = shift;
    my $in   = shift;
    my $sig   = shift;
    my $_in  = "/tmp/./.___verify_1_sha_$$";
    my $_sig  = "/tmp/./.___verify_3_sha_$$";
    my $_pem;
    my $cmd; 
    if (ref($in) eq 'HASH' && exists($in->{file})) {
        $_in = $in->{file};
    } else {
        $self->fdump( $_in, $in );
    }

        $self->fdump( $_sig, $sig );
    if (ref ($pem) eq 'HASH' && exists($pem->{cmd})) {
        $cmd = "cat $_in" . $pem->{cmd} ;
    }else {
        $_pem = "/tmp/./.___verify_2_sha_$$";
        $self->fdump( $_pem, $pem );
        #print "Signing $if with $pem res $of\n";
        $cmd = "$self->{openssl} dgst -sha256 -sigopt rsa_padding_mode:pss -prverify $_pem -signature $_sig $_in";
    }

    my $res = system($cmd); 
    $self->run_shell("rm -f $_in");
    if ($_pem) {
        $self->run_shell("rm -f $_pem");
    }
    die "ERROR: $cmd had failed" unless ($res == 0 ); 
    return $res;
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
        "$self->{openssl} enc -aes-128-cbc -K $bek -iv $biv -in $if -out $of");
}

sub sign_sha256 {
    my $self = shift;
    my ( $pem, $if, $of ) = @_;
    if ( !( -e $pem ) or !( -e $if ) ) {
        die "Not existing $pem or $if";
    }

    #print "Signing $if with $pem res $of\n";
    $self->run_shell(
"$self->{openssl} dgst -sign $pem -keyform pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out $of $if"
    );
}

sub pipe_init {
    my $self = shift;
    POSIX::mkfifo($self->{fifo_wr},700) || 
                  die "cant' mkfifo $self->{fifo_wr}";
    POSIX::mkfifo($self->{fifo_rd},700) || 
                  die "cant' mkfifo $self->{fifo_rd}";
}

sub pipe_write {
    my $self = shift;
    my $data = shift;
    open (FO,"> $self->{fifo_wr}") || die "can't open $self->{fifo_rd}";
    print FO $data;
    close(FO) || die "can't close $self->{fifo}";
}

sub pipe_read {
    my $self = shift;
    my $data = shift;
    open (FO,"< $self->{fifo_rd}") || die "can't open $self->{fifo_rd}";
    $data = <FO>;
    close(FO) || die "can't close $self->{fifo}";
}

sub pipe_run_cmd {
    my $self = shift;
    my $in = $_[1];
    my $out = $_[2];
    my $cmd = "cat $self->{fifo_wr}|" . $_[3] . "|$self->{fifo_rd} ";
    my %cld;
    $SIG{CHLD} = sub {
        # don't change $! and $? outside handler
        local ($!, $?);
        while ( (my $pid = waitpid(-1, WNOHANG)) > 0 ) {
            delete $cld{$pid};
            cleanup_child($pid, $?);
        }
    };

    my $pid = fork();
            die "cannot fork" unless defined $pid;
    if ($pid == 0) {
        #child 
        system($cmd);
	exit(0);
     } else {
       #parent 
            $cld{$pid} = 1;
            $self->pipe_write($in); 
            $self->pipe_read($out);
            while(  waitpid($pid, WNOHANG) == 0)  {}; 
            return $out;
     }
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
