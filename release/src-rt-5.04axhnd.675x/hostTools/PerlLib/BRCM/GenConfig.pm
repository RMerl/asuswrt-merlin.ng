
use strict;
use warnings;

package BRCM::GenConfig;
use Data::Dumper;

sub new {
    my $class = shift;
    my $file  = shift;
    my $self  = {@_};    # after the file, all other arguments are options
    my $mode  = '<';
    if ( -w $file ) {
        $mode = '+<';
    }
    open( my $fh, $mode, $file ) or die("couldnt open file '$file'");
    my %lines;
    while ( my $b = <$fh> ) {
        if ( $b =~ /^(\w+)=/ ) {
            $lines{$1} = $b;
        }
        elsif ( $b =~ /^\#\s*(\w+)/ ) {
            if ( !$lines{$1} ) {
                $lines{$1} = $b;
            }
        }
    }
    seek( $fh, 0, 0 );
    $self->{file}  = $fh;
    $self->{lines} = [ values %lines ];

    #print Dumper($self->{lines}) . "\n";
    return bless( $self, $class );
}

sub get {
    my $self  = shift;
    my $var   = shift;
    my $val   = shift;
    my $lines = $self->{lines};
    my $i;
    my $j = undef;
    for ( $i = 0 ; $i < @{$lines} ; $i++ ) {
        if ( $lines->[$i] =~ /^$var=(.*\S)/ ) {
            $j = $1;
        }
    }
    return $j;
}

sub unset {
    my $self = shift;

    #print "UNSET: " . Dumper(\@_) . "\n";
    return $self->set( @_, undef );
}

sub set {
    my $self = shift;

    #print "SET: " . Dumper(\@_) . "\n";
    my $var   = shift;
    my $val   = shift;
    my $opts  = {@_};
    my $lines = $self->{lines};

    my $vars = ref($var) ? $var : [$var];    # accept list ref or a value
    foreach my $tvar ( @{$vars} ) {
        my $i;
        my $j = -1;
        for ( $i = 0 ; $i < @{$lines} ; $i++ ) {
            if ( $lines->[$i] =~ /(^$tvar=|^#\s*$tvar is)/ ) {
                $j = $i;
            }
        }
        if ( $j >= 0 ) {

            # keep the last one
            $i = $j;
        }
        if ( defined($val) ) {
            $lines->[$i] = "$tvar=$val";
        }
        else {
            $lines->[$i] = "# $tvar is not set" unless $opts->{NoUnset};
        }
    }
    return 1;
}

sub notset {
    my $self = shift;
    my $var  = shift;
    return $self->set($var);
}

sub write {
    my $self = shift;
    my $file = shift;
    my $fh;
    if ($file) {
        open( $fh, ">", $file ) or die("cant write $file");
    }
    else {
        $fh = $self->{file};
        seek( $fh, 0, 0 );
        truncate( $fh, 0 );
    }
    print $fh join( "\n", @{ $self->{lines} } );
    print $fh "\n";
    if ($file) {
        close($fh);
    }

    return 1;
}

sub driver_setup {
    my $self          = shift;
    my $profileDriver = shift;
    my $kernelDriver  = shift;
    my $profile       = $self->{Profile};
    my $driverV       = $profile->get("BRCM_DRIVER_$profileDriver");
    if ($driverV) {
        $self->set( "CONFIG_BCM_$kernelDriver", $driverV );
    }
    my $impl = $self->get("# CONFIG_BCM9$self->{Chip}_${kernelDriver}_IMPL");
    $self->set( "CONFIG_BCM_${kernelDriver}_IMPL", $impl );
}

sub setup_impl {
    my $self = shift;
    while ( my $kernelDriver = shift ) {
        my $impl =
          $self->get("# CONFIG_BCM9$self->{Chip}_${kernelDriver}_IMPL");
        $self->set( "CONFIG_BCM_${kernelDriver}_IMPL", $impl );
    }
}

1;

__END__

=head1 NAME

BRCM::GenConfig -- In-place editing of kconfig-type files

=head1 SYNOPSIS

    use strict;
    use warnings;
    use BRCM::GenConfig;

    my $g = new BRCM::GenConfig('.config'); # open the file
    my $g = new BRCM::GenConfig('.config', Chip => "63268"); # With a chip
    $g->set( "CONFIG_PREFIX", "string" ); # set/create a value

    $g->set( "CONFIG_FEATURE_WGET_HTTPS", "y" ); # set/create
    $g->set( \@list, "y" ); # set/create a whole list
    $g->set( "CONFIG_FEATURE_WGET_AUTHENTICATION" ); # is not set
    }

    $g->write();
    or
    $g->write("newfile");

=cut

