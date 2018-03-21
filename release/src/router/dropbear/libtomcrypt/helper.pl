#!/usr/bin/env perl

use strict;
use warnings;

use Getopt::Long;
use File::Find 'find';
use File::Basename 'basename';
use File::Glob 'bsd_glob';

sub read_file {
  my $f = shift;
  open my $fh, "<", $f or die "FATAL: read_rawfile() cannot open file '$f': $!";
  binmode $fh;
  return do { local $/; <$fh> };
}

sub write_file {
  my ($f, $data) = @_;
  die "FATAL: write_file() no data" unless defined $data;
  open my $fh, ">", $f or die "FATAL: write_file() cannot open file '$f': $!";
  binmode $fh;
  print $fh $data or die "FATAL: write_file() cannot write to '$f': $!";
  close $fh or die "FATAL: write_file() cannot close '$f': $!";
  return;
}

sub check_source {
  my @all_files = (bsd_glob("makefile*"), bsd_glob("*.sh"), bsd_glob("*.pl"));
  find({ wanted=>sub { push @all_files, $_ if -f $_ }, no_chdir=>1 }, qw/src tests demos/);

  my $fails = 0;
  for my $file (sort @all_files) {
    next unless $file =~ /\.(c|h|pl|py|sh)$/ || basename($file) =~ /^makefile/i;
    my $troubles = {};
    my $lineno = 1;
    my $content = read_file($file);
    push @{$troubles->{crlf_line_end}}, '?' if $content =~ /\r/;
    for my $l (split /\n/, $content) {
      push @{$troubles->{merge_conflict}},   $lineno if $l =~ /^(<<<<<<<|=======|>>>>>>>)([^<=>]|$)/;
      push @{$troubles->{trailing_space}},   $lineno if $l =~ / $/;
      push @{$troubles->{tab}},              $lineno if $l =~ /\t/ && basename($file) !~ /^makefile/i;
      push @{$troubles->{non_ascii_char}},   $lineno if $l =~ /[^[:ascii:]]/;
      push @{$troubles->{cpp_comment}},      $lineno if $file =~ /\.(c|h)$/ && ($l =~ /\s\/\// || $l =~ /\/\/\s/);
      # in ./src we prefer using XMEMCPY, XMALLOC, XFREE ...
      push @{$troubles->{unwanted_memcpy}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bmemcpy\s*\(/;
      push @{$troubles->{unwanted_malloc}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bmalloc\s*\(/;
      push @{$troubles->{unwanted_realloc}}, $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\brealloc\s*\(/;
      push @{$troubles->{unwanted_calloc}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bcalloc\s*\(/;
      push @{$troubles->{unwanted_free}},    $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bfree\s*\(/;
      push @{$troubles->{unwanted_memset}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bmemset\s*\(/;
      push @{$troubles->{unwanted_memcpy}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bmemcpy\s*\(/;
      push @{$troubles->{unwanted_memmove}}, $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bmemmove\s*\(/;
      push @{$troubles->{unwanted_memcmp}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bmemcmp\s*\(/;
      push @{$troubles->{unwanted_strcmp}},  $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bstrcmp\s*\(/;
      push @{$troubles->{unwanted_clock}},   $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bclock\s*\(/;
      push @{$troubles->{unwanted_qsort}},   $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bqsort\s*\(/;
      push @{$troubles->{sizeof_no_brackets}}, $lineno if $file =~ /^src\/.*\.c$/ && $l =~ /\bsizeof\s*[^\(]/;
      if ($file =~ m|src/.*\.c$| &&
          $file !~ m|src/ciphers/.*\.c$| &&
          $file !~ m|src/hashes/.*\.c$| &&
          $file !~ m|src/math/.+_desc.c$| &&
          $file !~ m|src/stream/sober128/sober128_stream.c$| &&
          $l =~ /^static(\s+[a-zA-Z0-9_]+)+\s+([^_][a-zA-Z0-9_]+)\s*\(/) {
        push @{$troubles->{staticfunc_name}}, "$lineno($2)";
      }
      $lineno++;
    }
    for my $k (sort keys %$troubles) {
      warn "[$k] $file line:" . join(",", @{$troubles->{$k}}) . "\n";
      $fails++;
    }
  }

  warn( $fails > 0 ? "check-source:    FAIL $fails\n" : "check-source:    PASS\n" );
  return $fails;
}

sub check_defines {
  my $fails = 0;
  my $cust_h = read_file("src/headers/tomcrypt_custom.h");
  my $cryp_c = read_file("src/misc/crypt/crypt.c");
  $cust_h =~ s|/\*.*?\*/||sg; # remove comments
  $cryp_c =~ s|/\*.*?\*/||sg; # remove comments
  my %def = map { $_ => 1 } map { my $x = $_; $x =~ s/^\s*#define\s+(LTC_\S+).*$/$1/; $x } grep { /^\s*#define\s+LTC_\S+/ } split /\n/, $cust_h;
  for my $d (sort keys %def) {
    next if $d =~ /^LTC_(DH\d+|ECC\d+|ECC_\S+|MPI|MUTEX_\S+\(x\)|NO_\S+)$/;
    warn "$d missing in src/misc/crypt/crypt.c\n" and $fails++ if $cryp_c !~ /\Q$d\E/;
  }
  warn( $fails > 0 ? "check-defines:   FAIL $fails\n" : "check-defines:   PASS\n" );
  return $fails;
}

sub check_descriptor {
  my $which = shift;
  my $what = shift;
  my @src;
  my @descriptors;
  find({ wanted => sub { push @src, $_ if $_ =~ /\.c$/ }, no_chdir=>1 }, "./src/${which}/");
  for my $f (@src) {
    my @n = map { my $x = $_; $x =~ s/^.*?ltc_${what}_descriptor\s+(\S+).*$/$1/; $x } grep { $_ =~ /ltc_${what}_descriptor/ } split /\n/, read_file($f);
    push @descriptors, @n if @n;
  }
  my $fails = 0;
  for my $d (@descriptors) {
    for my $f ("./src/misc/crypt/crypt_register_all_${which}.c") {
      my $txt = read_file($f);
      warn "$d missing in $f\n" and $fails++ if $txt !~ /\Q$d\E/;
    }
  }
  for my $d (@descriptors) {
    for my $f ("./tests/test.c") {
      my $txt = read_file($f);
      warn "$d missing in $f\n" and $fails++ if $txt !~ /\Q$d\E/;
    }
  }
  my $name = sprintf("%-17s", "check-${which}:");
  warn( $fails > 0 ? "${name}FAIL $fails\n" : "${name}PASS\n" );
  return $fails;
}

sub check_descriptors {
  my $fails = 0;
  $fails = $fails + check_descriptor("ciphers", "cipher");
  $fails = $fails + check_descriptor("hashes", "hash");
  $fails = $fails + check_descriptor("prngs", "prng");
  return $fails;
}

sub check_comments {
  my $fails = 0;
  my $first_comment = <<'MARKER';
/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
MARKER
  my $last_comment = <<'MARKER';
/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
MARKER
  my @all_files;
  find({ wanted=> sub { push @all_files, $_ if $_ =~ /\.(c|h)$/ }, no_chdir=>1 }, 'demos', 'src', 'tests');
  for my $f (@all_files) {
    my $txt = read_file($f);
    if ($txt !~ /^\Q$first_comment\E/s) {
      warn "[first_comment] $f\n";
      $fails++;
    }
    if ($txt !~ /\Q$last_comment\E\s*$/s) {
      warn "[last_comment] $f\n";
      $fails++;
    }
  }
  warn( $fails > 0 ? "check-comments:  FAIL $fails\n" : "check-comments:  PASS\n" );
  return $fails;
}

sub prepare_variable {
  my ($varname, @list) = @_;
  my $output = "$varname=";
  my $len = length($output);
  foreach my $obj (sort @list) {
    $len = $len + length $obj;
    $obj =~ s/\*/\$/;
    if ($len > 100) {
      $output .= "\\\n";
      $len = length $obj;
    }
    $output .= $obj . ' ';
  }
  $output =~ s/ $//;
  return $output;
}

sub prepare_msvc_files_xml {
  my ($all, $exclude_re, $targets) = @_;
  my $last = [];
  my $depth = 2;

  # sort files in the same order as visual studio (ugly, I know)
  my @parts = ();
  for my $orig (@$all) {
    my $p = $orig;
    $p =~ s|/|/~|g;
    $p =~ s|/~([^/]+)$|/$1|g;
    # now we have: 'src/pk/rsa/rsa_verify_hash.c' > 'src/~pk/~rsa/rsa_verify_hash.c'
    my @l = map { sprintf "% -99s", $_ } split /\//, $p;
    push @parts, [ $orig, join(':', @l) ];
  }
  my @sorted = map { $_->[0] } sort { $a->[1] cmp $b->[1] } @parts;

  my $files = "<Files>\r\n";
  for my $full (@sorted) {
    my @items = split /\//, $full; # split by '/'
    $full =~ s|/|\\|g;             # replace '/' bt '\'
    shift @items; # drop first one (src)
    pop @items;   # drop last one (filename.ext)
    my $current = \@items;
    if (join(':', @$current) ne join(':', @$last)) {
      my $common = 0;
      $common++ while ($last->[$common] && $current->[$common] && $last->[$common] eq $current->[$common]);
      my $back = @$last - $common;
      if ($back > 0) {
        $files .= ("\t" x --$depth) . "</Filter>\r\n" for (1..$back);
      }
      my $fwd = [ @$current ]; splice(@$fwd, 0, $common);
      for my $i (0..scalar(@$fwd) - 1) {
        $files .= ("\t" x $depth) . "<Filter\r\n";
        $files .= ("\t" x $depth) . "\tName=\"$fwd->[$i]\"\r\n";
        $files .= ("\t" x $depth) . "\t>\r\n";
        $depth++;
      }
      $last = $current;
    }
    $files .= ("\t" x $depth) . "<File\r\n";
    $files .= ("\t" x $depth) . "\tRelativePath=\"$full\"\r\n";
    $files .= ("\t" x $depth) . "\t>\r\n";
    if ($full =~ $exclude_re) {
      for (@$targets) {
        $files .= ("\t" x $depth) . "\t<FileConfiguration\r\n";
        $files .= ("\t" x $depth) . "\t\tName=\"$_\"\r\n";
        $files .= ("\t" x $depth) . "\t\tExcludedFromBuild=\"true\"\r\n";
        $files .= ("\t" x $depth) . "\t\t>\r\n";
        $files .= ("\t" x $depth) . "\t\t<Tool\r\n";
        $files .= ("\t" x $depth) . "\t\t\tName=\"VCCLCompilerTool\"\r\n";
        $files .= ("\t" x $depth) . "\t\t\tAdditionalIncludeDirectories=\"\"\r\n";
        $files .= ("\t" x $depth) . "\t\t\tPreprocessorDefinitions=\"\"\r\n";
        $files .= ("\t" x $depth) . "\t\t/>\r\n";
        $files .= ("\t" x $depth) . "\t</FileConfiguration>\r\n";
      }
    }
########### aes_enc "hack" disabled - discussion: https://github.com/libtom/libtomcrypt/pull/158
#    if ($full eq 'src\ciphers\aes\aes.c') { #hack
#      my %cmd = (
#        'Debug|Win32'   => [ 'Debug/aes.obj;Debug/aes_enc.obj', 'cl /nologo /MLd /W3 /Gm /GX /ZI /Od /I &quot;src\headers&quot; /I &quot;..\libtommath&quot; /D &quot;_DEBUG&quot; /D &quot;LTM_DESC&quot; /D &quot;WIN32&quot; /D &quot;_MBCS&quot; /D &quot;_LIB&quot; /D &quot;LTC_SOURCE&quot; /D &quot;USE_LTM&quot; /Fp&quot;Debug/libtomcrypt.pch&quot; /YX /Fo&quot;Debug/&quot; /Fd&quot;Debug/&quot; /FD /GZ /c $(InputPath)&#x0D;&#x0A;cl /nologo /DENCRYPT_ONLY /MLd /W3 /Gm /GX /ZI /Od /I &quot;src\headers&quot; /I &quot;..\libtommath&quot; /D &quot;_DEBUG&quot; /D &quot;LTM_DESC&quot; /D &quot;WIN32&quot; /D &quot;_MBCS&quot; /D &quot;_LIB&quot; /D &quot;LTC_SOURCE&quot; /D &quot;USE_LTM&quot; /Fp&quot;Debug/libtomcrypt.pch&quot; /YX /Fo&quot;Debug/aes_enc.obj&quot; /Fd&quot;Debug/&quot; /FD /GZ /c $(InputPath)&#x0D;&#x0A;' ],
#        'Release|Win32' => [ 'Release/aes.obj;Release/aes_enc.obj', 'cl /nologo /MLd /W3 /Gm /GX /ZI /Od /I &quot;src\headers&quot; /I &quot;..\libtommath&quot; /D &quot;_DEBUG&quot; /D &quot;LTM_DESC&quot; /D &quot;WIN32&quot; /D &quot;_MBCS&quot; /D &quot;_LIB&quot; /D &quot;LTC_SOURCE&quot; /D &quot;USE_LTM&quot; /Fp&quot;Release/libtomcrypt.pch&quot; /YX /Fo&quot;Release/&quot; /Fd&quot;Release/&quot; /FD /GZ /c $(InputPath)&#x0D;&#x0A;cl /nologo /DENCRYPT_ONLY /MLd /W3 /Gm /GX /ZI /Od /I &quot;src\headers&quot; /I &quot;..\libtommath&quot; /D &quot;_DEBUG&quot; /D &quot;LTM_DESC&quot; /D &quot;WIN32&quot; /D &quot;_MBCS&quot; /D &quot;_LIB&quot; /D &quot;LTC_SOURCE&quot; /D &quot;USE_LTM&quot; /Fp&quot;Release/libtomcrypt.pch&quot; /YX /Fo&quot;Release/aes_enc.obj&quot; /Fd&quot;Release/&quot; /FD /GZ /c $(InputPath)&#x0D;&#x0A;' ],
#      );
#      for (@$targets) {
#        next unless $cmd{$_};
#        $files .= ("\t" x $depth) . "\t<FileConfiguration\r\n";
#        $files .= ("\t" x $depth) . "\t\tName=\"$_\"\r\n";
#        $files .= ("\t" x $depth) . "\t\t>\r\n";
#        $files .= ("\t" x $depth) . "\t\t<Tool\r\n";
#        $files .= ("\t" x $depth) . "\t\t\tName=\"VCCustomBuildTool\"\r\n";
#        $files .= ("\t" x $depth) . "\t\t\tCommandLine=\"$cmd{$_}[1]\"\r\n";
#        $files .= ("\t" x $depth) . "\t\t\tOutputs=\"$cmd{$_}[0]\"\r\n";
#        $files .= ("\t" x $depth) . "\t\t/>\r\n";
#        $files .= ("\t" x $depth) . "\t</FileConfiguration>\r\n";
#      }
#    }
    $files .= ("\t" x $depth) . "</File>\r\n";
  }
  $files .= ("\t" x --$depth) . "</Filter>\r\n" for (@$last);
  $files .= "\t</Files>";
  return $files;
}

sub patch_file {
  my ($content, @variables) = @_;
  for my $v (@variables) {
    if ($v =~ /^([A-Z0-9_]+)\s*=.*$/si) {
      my $name = $1;
      $content =~ s/\n\Q$name\E\b.*?[^\\]\n/\n$v\n/s;
    }
    else {
      die "patch_file failed: " . substr($v, 0, 30) . "..";
    }
  }
  return $content;
}

sub version_from_tomcrypt_h {
  my $h = read_file(shift);
  if ($h =~ /\n#define\s*SCRYPT\s*"([0-9]+)\.([0-9]+)\.([0-9]+)(.*)"/s) {
    return "VERSION_PC=$1.$2.$3", "VERSION_LT=1:1", "VERSION=$1.$2.$3$4", "PROJECT_NUMBER=$1.$2.$3$4";
  }
  else {
    die "#define SCRYPT not found in tomcrypt.h";
  }
}

sub process_makefiles {
  my $write = shift;
  my $changed_count = 0;
  my @c = ();
  find({ no_chdir => 1, wanted => sub { push @c, $_ if -f $_ && $_ =~ /\.c$/ && $_ !~ /tab.c$/ } }, 'src');
  my @h = ();
  find({ no_chdir => 1, wanted => sub { push @h, $_ if -f $_ && $_ =~ /\.h$/ && $_ !~ /dh_static.h$/ } }, 'src');
  my @all = ();
  find({ no_chdir => 1, wanted => sub { push @all, $_ if -f $_ && $_ =~ /\.(c|h)$/  } }, 'src');
  my @t = qw();
  find({ no_chdir => 1, wanted => sub { push @t, $_ if $_ =~ /(common|no_prng|_tests?|test).c$/ } }, 'tests');

  my @o = sort ('src/ciphers/aes/aes_enc.o', map { my $x = $_; $x =~ s/\.c$/.o/; $x } @c);
  my $var_o = prepare_variable("OBJECTS", @o);
  my $var_h = prepare_variable("HEADERS", (sort @h));
  (my $var_obj = $var_o) =~ s/\.o\b/.obj/sg;

  my $var_to = prepare_variable("TOBJECTS", sort map { my $x = $_; $x =~ s/\.c$/.o/; $x } @t);
  (my $var_tobj = $var_to) =~ s/\.o\b/.obj/sg;

  my @ver_version = version_from_tomcrypt_h("src/headers/tomcrypt.h");

  # update MSVC project files
  my $msvc_files = prepare_msvc_files_xml(\@all, qr/tab\.c$/, ['Debug|Win32', 'Release|Win32', 'Debug|x64', 'Release|x64']);
  for my $m (qw/libtomcrypt_VS2008.vcproj/) {
    my $old = read_file($m);
    my $new = $old;
    $new =~ s|<Files>.*</Files>|$msvc_files|s;
    if ($old ne $new) {
      write_file($m, $new) if $write;
      warn "changed: $m\n";
      $changed_count++;
    }
  }

  # update OBJECTS + HEADERS in makefile*
  for my $m (qw/ makefile makefile.shared makefile.unix makefile.mingw makefile.msvc makefile_include.mk doc\/Doxyfile /) {
    my $old = read_file($m);
    my $new = $m eq 'makefile.msvc' ? patch_file($old, $var_obj, $var_h, $var_tobj, @ver_version)
                                    : patch_file($old, $var_o, $var_h, $var_to, @ver_version);
    if ($old ne $new) {
      write_file($m, $new) if $write;
      warn "changed: $m\n";
      $changed_count++;
    }
  }

  if ($write) {
    return 0; # no failures
  }
  else {
    warn( $changed_count > 0 ? "check-makefiles: FAIL $changed_count\n" : "check-makefiles: PASS\n" );
    return $changed_count;
  }
}

sub die_usage {
  die <<"MARKER";
usage: $0 -s   OR   $0 --check-source
       $0 -c   OR   $0 --check-descriptors
       $0 -d   OR   $0 --check-defines
       $0 -o   OR   $0 --check-comments
       $0 -m   OR   $0 --check-makefiles
       $0 -a   OR   $0 --check-all
       $0 -u   OR   $0 --update-makefiles
       $0 --fixupind crypt.ind
MARKER
}

GetOptions( "s|check-source"        => \my $check_source,
            "c|check-descriptors"   => \my $check_descriptors,
            "d|check-defines"       => \my $check_defines,
            "o|check-comments"      => \my $check_comments,
            "m|check-makefiles"     => \my $check_makefiles,
            "a|check-all"           => \my $check_all,
            "u|update-makefiles"    => \my $update_makefiles,
            "f|fixupind=s"          => \my $fixupind,
            "h|help"                => \my $help
          ) or die_usage;

if ($fixupind) {
  my $txt = read_file($fixupind);
  $txt =~ s/^([^\n]*\n)/$1\n\\addcontentsline{toc}{chapter}{Index}\n/s;
  write_file($fixupind, $txt);
  exit 0;
}

my $failure;
$failure ||= check_source()       if $check_all || $check_source;
$failure ||= check_defines()      if $check_all || $check_defines;
$failure ||= check_descriptors()  if $check_all || $check_descriptors;
$failure ||= check_comments()     if $check_all || $check_comments;
$failure ||= process_makefiles(0) if $check_all || $check_makefiles;
$failure ||= process_makefiles(1) if $update_makefiles;

die_usage unless defined $failure;
exit $failure ? 1 : 0;
