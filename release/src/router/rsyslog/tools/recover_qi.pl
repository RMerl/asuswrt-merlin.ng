#!/usr/bin/perl -w
# recover rsyslog disk queue index (.qi) from queue files (.nnnnnnnn).
#
# See:
#   runtime/queue.c: qqueuePersist()
#   runtime/queue.c: qqueueTryLoadPersistedInfo()
#
# kaiwang.chen@gmail.com 2012-03-14
#
use strict;
use Getopt::Long;

my %opt = ();
GetOptions(\%opt,"spool|w=s","basename|f=s","digits|d=i","help!");
if ($opt{help}) {
  print "Usage:
\t$0 -w WorkDirectory -f QueueFileName -d 8 > QueueFileName.qi
";
  exit;
}

# runtime/queue.c: qConstructDisk()
my $iMaxFiles = 10000000; # 0+"1".( "0"x($opt{digits} - 1));

# get the list of queue files, spool directory excluded
my $re = qr/^\Q$opt{basename}\E\.\d{$opt{digits}}$/;
opendir(DIR, $opt{spool}) or die "can’t open spool: $!";
my @qf = grep { /$re/ && -f "$opt{spool}/$_" } readdir(DIR);
closedir DIR;

# ensure order and continuity
@qf = sort @qf;
my ($head) = ($qf[0] =~ /(\d+)$/);
my ($tail) = ($qf[-1] =~ /(\d+)$/);
$head += 0;
$tail += 0;
if ($tail-$head+1 != @qf || $tail > $iMaxFiles) {
  die "broken queue: missing file(s) or wrong tail\n";
}

# collect some counters about the queue, assuming all are unprocessed entries.
my $sizeOnDisk = 0;
my $iQueueSize = 0;
chdir($opt{spool}) or die "can't chdir to spool: $!";
print STDERR "traversing ". @qf ." files, please wait...\n";
for (@qf) {
  open FH, "<", $_ or die "can't read queue file $_\n";
  $sizeOnDisk += (stat FH)[7];
  while (<FH>) {
    $iQueueSize++ if /^<Obj/;  # runtime/msg.c: MsgSerialize()
  }
  close FH;
}
# happen to reuse last stat
my $iCurrOffs_Write = (stat(_))[7];

# runtime/queue.c: qqueuePersist()
my $qqueue = Rsyslog::OPB->new("qqueue",1);
$qqueue->property("iQueueSize", "INT", $iQueueSize);
$qqueue->property("tVars.disk.sizeOnDisk", "INT64", $sizeOnDisk);
$qqueue->property("tVars.disk.bytesRead", "INT64", 0);

# runtime/stream.h: strmType_t
my $STREAMTYPE_FILE_CIRCULAR = 1;
# runtime/stream.h: strmMode_t
my $STREAMMODE_READ = 1;
my $STREAMMODE_WRITE_APPEND = 4;

# runtime/stream.c: strmSerialize()
# write to end
my $strm_Write = Rsyslog::Obj->new("strm",1);
$strm_Write->property(      "iCurrFNum",  "INT",                     $tail);
$strm_Write->property(       "pszFName",  "PSZ",            $opt{basename});
$strm_Write->property(      "iMaxFiles",  "INT",                $iMaxFiles);
$strm_Write->property( "bDeleteOnClose",  "INT",                         0);
$strm_Write->property(          "sType",  "INT", $STREAMTYPE_FILE_CIRCULAR);
$strm_Write->property("tOperationsMode",  "INT",  $STREAMMODE_WRITE_APPEND);
$strm_Write->property(      "tOpenMode",  "INT",                      0600);
$strm_Write->property(      "iCurrOffs","INT64",          $iCurrOffs_Write);
# read from head
my $strm_ReadDel = Rsyslog::Obj->new("strm",1);
$strm_ReadDel->property(      "iCurrFNum",  "INT",                     $head);
$strm_ReadDel->property(       "pszFName",  "PSZ",            $opt{basename});
$strm_ReadDel->property(      "iMaxFiles",  "INT",                $iMaxFiles);
$strm_ReadDel->property( "bDeleteOnClose",  "INT",                         1);
$strm_ReadDel->property(          "sType",  "INT", $STREAMTYPE_FILE_CIRCULAR);
$strm_ReadDel->property("tOperationsMode",  "INT",          $STREAMMODE_READ);
$strm_ReadDel->property(      "tOpenMode",  "INT",                      0600);
$strm_ReadDel->property(      "iCurrOffs","INT64",                         0);

# .qi
print $qqueue->serialize();
print $strm_Write->serialize();
print $strm_ReadDel->serialize();

exit;
#-----------------------------------------------------------------------------

package Rsyslog::Serializable;
# runtime/obj.c 
sub COOKIE_OBJLINE   { '<' }
sub COOKIE_PROPLINE  { '+' }
sub COOKIE_ENDLINE   { '>' }
sub COOKIE_BLANKLINE { '.' }
# VARTYPE(short_ptype)
sub VARTYPE {
  my ($t) = @_;
  # runtime/obj-types.h: propType_t
  my $ptype = "PROPTYPE_".$t;
  # runtime/var.h: varType_t
  my %vm = ( 
    VARTYPE_NONE       => 0,
    VARTYPE_STR        => 1,
    VARTYPE_NUMBER     => 2,
    VARTYPE_SYSLOGTIME => 3,
  );
  # runtime/obj.c: SerializeProp()
  my %p2v = (
    #PROPTYPE_NONE  => "",
    PROPTYPE_PSZ   => "VARTYPE_STR",
    PROPTYPE_SHORT => "VARTYPE_NUMBER",
    PROPTYPE_INT   => "VARTYPE_NUMBER",
    PROPTYPE_LONG  => "VARTYPE_NUMBER",
    PROPTYPE_INT64 => "VARTYPE_NUMBER",
    PROPTYPE_CSTR  => "VARTYPE_STR",
    #PROPTYPE_SYSLOGTIME => "VARTYPE_SYSLOGTIME",
  );
  my $vtype = $p2v{$ptype};
  unless ($vtype) {
    die "property type $t is not supported!\n";
  }
  return $vm{$vtype};
}
sub serialize {
  my $self = shift;
  # runtime/obj.c: objSerializeHeader()
  my $x = COOKIE_OBJLINE();
  $x .= join(":", $self->type(), $self->cver(), $self->id(), $self->version());
  $x .= ":\n";
  for ( values %{$self->{props}} ) {
    # runtime/obj.c: SerializeProp()
    $x .= COOKIE_PROPLINE();
    $x .= join(":",
            $_->{name},
            VARTYPE($_->{type}),
            length($_->{value}),
            $_->{value});
    $x .= ":\n";
  }
  # runtime/obj.c: EndSerialize()
  $x .= COOKIE_ENDLINE() . "End\n";
  $x .= COOKIE_BLANKLINE() . "\n";
}
# constructor: new(id,version)
sub new {
  my ($class, $id, $version) = @_;
  $class = ref $class if ref $class;
  bless {
    id => $id,
    version => $version,
    props => {},
  }, $class;
}
sub id {
  my $self = shift;
  if (@_) {
    my $x = $self->{id};
    $self->{id} = shift;
    return $x;
  }
  return $self->{id};
}
sub version {
  my $self = shift;
  if (@_) {
    my $x = $self->{version};
    $self->{version} = shift;
    return $x;
  }
  return $self->{version};
}
# property(name, type, value)
sub property {
  my $self = shift;
  my $name = shift;
  if (@_) {
    my $x = $self->{props}{$name};
    $self->{props}{$name}{name} = $name;
    $self->{props}{$name}{type} = shift;
    $self->{props}{$name}{value} = shift;
    return $x;
  }
  return $self->{props}{$name};
}
1;
package Rsyslog::OPB;
use base qw(Rsyslog::Serializable);
sub type { 'OPB' }
sub cver { 1 }
sub new { shift->SUPER::new(@_) }
1;
package Rsyslog::Obj;
use base qw(Rsyslog::Serializable);
sub type { 'Obj' }
sub cver { 1 }
sub new { shift->SUPER::new(@_) }
1;
