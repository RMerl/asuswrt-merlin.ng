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

sub sanitize_comments {
  my($content) = @_;
  $content =~ s{/\*(.*?)\*/}{my $x=$1; $x =~ s/\w/x/g; "/*$x*/";}egs;
  return $content;
}

sub check_source {
  my @all_files = (
        bsd_glob("Makefile*"),
        bsd_glob("*.{h,c,sh,pl}"),
        bsd_glob("*/*.{h,c,sh,pl}"),
  );

  my $fails = 0;
  for my $file (sort @all_files) {
    my $troubles = {};
    my $lineno = 1;
    my $content = read_file($file);
    $content = sanitize_comments $content;
    push @{$troubles->{crlf_line_end}}, '?' if $content =~ /\r/;
    for my $l (split /\n/, $content) {
      push @{$troubles->{merge_conflict}},     $lineno if $l =~ /^(<<<<<<<|=======|>>>>>>>)([^<=>]|$)/;
      push @{$troubles->{trailing_space}},     $lineno if $l =~ / $/;
      push @{$troubles->{tab}},                $lineno if $l =~ /\t/ && basename($file) !~ /^makefile/i;
      push @{$troubles->{non_ascii_char}},     $lineno if $l =~ /[^[:ascii:]]/;
      push @{$troubles->{cpp_comment}},        $lineno if $file =~ /\.(c|h)$/ && ($l =~ /\s\/\// || $l =~ /\/\/\s/);
      # we prefer using XMALLOC, XFREE, XREALLOC, XCALLOC ...
      push @{$troubles->{unwanted_malloc}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bmalloc\s*\(/;
      push @{$troubles->{unwanted_realloc}},   $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\brealloc\s*\(/;
      push @{$troubles->{unwanted_calloc}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bcalloc\s*\(/;
      push @{$troubles->{unwanted_free}},      $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bfree\s*\(/;
      # and we probably want to also avoid the following
      push @{$troubles->{unwanted_memcpy}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bmemcpy\s*\(/;
      push @{$troubles->{unwanted_memset}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bmemset\s*\(/;
      push @{$troubles->{unwanted_memcpy}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bmemcpy\s*\(/;
      push @{$troubles->{unwanted_memmove}},   $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bmemmove\s*\(/;
      push @{$troubles->{unwanted_memcmp}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bmemcmp\s*\(/;
      push @{$troubles->{unwanted_strcmp}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bstrcmp\s*\(/;
      push @{$troubles->{unwanted_strcpy}},    $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bstrcpy\s*\(/;
      push @{$troubles->{unwanted_strncpy}},   $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bstrncpy\s*\(/;
      push @{$troubles->{unwanted_clock}},     $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bclock\s*\(/;
      push @{$troubles->{unwanted_qsort}},     $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bqsort\s*\(/;
      push @{$troubles->{sizeof_no_brackets}}, $lineno if $file =~ /^[^\/]+\.c$/ && $l =~ /\bsizeof\s*[^\(]/;
      if ($file =~ m|^[^\/]+\.c$| && $l =~ /^static(\s+[a-zA-Z0-9_]+)+\s+([a-zA-Z0-9_]+)\s*\(/) {
        my $funcname = $2;
        # static functions should start with s_
        push @{$troubles->{staticfunc_name}}, "$lineno($funcname)" if $funcname !~ /^s_/;
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

sub check_comments {
  my $fails = 0;
  my $first_comment = <<'MARKER';
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
MARKER
  #my @all_files = (bsd_glob("*.{h,c}"), bsd_glob("*/*.{h,c}"));
  my @all_files = (bsd_glob("*.{h,c}"));
  for my $f (@all_files) {
    my $txt = read_file($f);
    if ($txt !~ /\Q$first_comment\E/s) {
      warn "[first_comment] $f\n";
      $fails++;
    }
  }
  warn( $fails > 0 ? "check-comments:  FAIL $fails\n" : "check-comments:  PASS\n" );
  return $fails;
}

sub check_doc {
  my $fails = 0;
  my $tex = read_file('doc/bn.tex');
  my $tmh = read_file('tommath.h');
  my @functions = $tmh =~ /\n\s*[a-zA-Z0-9_* ]+?(mp_[a-z0-9_]+)\s*\([^\)]+\)\s*;/sg;
  my @macros    = $tmh =~ /\n\s*#define\s+([a-z0-9_]+)\s*\([^\)]+\)/sg;
  for my $n (sort @functions) {
    (my $nn = $n) =~ s/_/\\_/g; # mp_sub_d >> mp\_sub\_d
    if ($tex !~ /index\Q{$nn}\E/) {
      warn "[missing_doc_for_function] $n\n";
      $fails++
    }
  }
  for my $n (sort @macros) {
    (my $nn = $n) =~ s/_/\\_/g; # mp_iszero >> mp\_iszero
    if ($tex !~ /index\Q{$nn}\E/) {
      warn "[missing_doc_for_macro] $n\n";
      $fails++
    }
  }
  warn( $fails > 0 ? "check_doc:       FAIL $fails\n" : "check-doc:       PASS\n" );
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

sub process_makefiles {
  my $write = shift;
  my $changed_count = 0;
  my @o = map { my $x = $_; $x =~ s/\.c$/.o/; $x } bsd_glob("*.c");
  my @all = bsd_glob("*.{c,h}");

  my $var_o = prepare_variable("OBJECTS", @o);
  (my $var_obj = $var_o) =~ s/\.o\b/.obj/sg;

  # update OBJECTS + HEADERS in makefile*
  for my $m (qw/ Makefile.in /) {
    my $old = read_file($m);
    my $new = $m eq 'makefile.msvc' ? patch_file($old, $var_obj)
                                    : patch_file($old, $var_o);
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

sub draw_func
{
   my ($deplist, $depmap, $out, $indent, $funcslist) = @_;
   my @funcs = split ',', $funcslist;
   # try this if you want to have a look at a minimized version of the callgraph without all the trivial functions
   #if ($deplist =~ /$funcs[0]/ || $funcs[0] =~ /BN_MP_(ADD|SUB|CLEAR|CLEAR_\S+|DIV|MUL|COPY|ZERO|GROW|CLAMP|INIT|INIT_\S+|SET|ABS|CMP|CMP_D|EXCH)_C/) {
   if ($deplist =~ /$funcs[0]/) {
      return $deplist;
   } else {
      $deplist = $deplist . $funcs[0];
   }
   if ($indent == 0) {
   } elsif ($indent >= 1) {
      print {$out} '|   ' x ($indent - 1) . '+--->';
   }
   print {$out} $funcs[0] . "\n";
   shift @funcs;
   my $olddeplist = $deplist;
   foreach my $i (@funcs) {
      $deplist = draw_func($deplist, $depmap, $out, $indent + 1, ${$depmap}{$i}) if exists ${$depmap}{$i};
   }
   return $olddeplist;
}

sub update_dep
{
    #open class file and write preamble
    open(my $class, '>', 'tommath_class.h') or die "Couldn't open tommath_class.h for writing\n";
    print {$class} << 'EOS';
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#if !(defined(LTM1) && defined(LTM2) && defined(LTM3))
#define LTM_INSIDE
#if defined(LTM2)
#   define LTM3
#endif
#if defined(LTM1)
#   define LTM2
#endif
#define LTM1
#if defined(LTM_ALL)
EOS

    foreach my $filename (glob 'bn*.c') {
        my $define = $filename;

        print "Processing $filename\n";

        # convert filename to upper case so we can use it as a define
        $define =~ tr/[a-z]/[A-Z]/;
        $define =~ tr/\./_/;
        print {$class} "#   define $define\n";

        # now copy text and apply #ifdef as required
        my $apply = 0;
        open(my $src, '<', $filename);
        open(my $out, '>', 'tmp');

        # first line will be the #ifdef
        my $line = <$src>;
        if ($line =~ /include/) {
            print {$out} $line;
        } else {
            print {$out} << "EOS";
#include "tommath_private.h"
#ifdef $define
/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */
$line
EOS
            $apply = 1;
        }
        while (<$src>) {
            if ($_ !~ /tommath\.h/) {
                print {$out} $_;
            }
        }
        if ($apply == 1) {
            print {$out} "#endif\n";
        }
        close $src;
        close $out;

        unlink $filename;
        rename 'tmp', $filename;
    }
    print {$class} "#endif\n#endif\n";

    # now do classes
    my %depmap;
    foreach my $filename (glob 'bn*.c') {
        my $content;
        if ($filename =~ "bn_deprecated.c") {
            open(my $src, '<', $filename) or die "Can't open source file!\n";
            read $src, $content, -s $src;
            close $src;
        } else {
            my $cc = $ENV{'CC'} || 'gcc';
            $content = `$cc -E -x c -DLTM_ALL $filename`;
            $content =~ s/^# 1 "$filename".*?^# 2 "$filename"//ms;
        }

        # convert filename to upper case so we can use it as a define
        $filename =~ tr/[a-z]/[A-Z]/;
        $filename =~ tr/\./_/;

        print {$class} "#if defined($filename)\n";
        my $list = $filename;

        # strip comments
        $content =~ s{/\*.*?\*/}{}gs;

        # scan for mp_* and make classes
        my @deps = ();
        foreach my $line (split /\n/, $content) {
            while ($line =~ /(fast_)?(s_)?mp\_[a-z_0-9]*((?=\;)|(?=\())|(?<=\()mp\_[a-z_0-9]*(?=\()/g) {
                my $a = $&;
                next if $a eq "mp_err";
                $a =~ tr/[a-z]/[A-Z]/;
                $a = 'BN_' . $a . '_C';
                push @deps, $a;
            }
        }
        @deps = sort(@deps);
        foreach my $a (@deps) {
            if ($list !~ /$a/) {
                print {$class} "#   define $a\n";
            }
            $list = $list . ',' . $a;
        }
        $depmap{$filename} = $list;

        print {$class} "#endif\n\n";
    }

    print {$class} << 'EOS';
#ifdef LTM_INSIDE
#undef LTM_INSIDE
#ifdef LTM3
#   define LTM_LAST
#endif

#include "tommath_superclass.h"
#include "tommath_class.h"
#else
#   define LTM_LAST
#endif
EOS
    close $class;

    #now let's make a cool call graph...

    open(my $out, '>', 'callgraph.txt');
    foreach (sort keys %depmap) {
        draw_func("", \%depmap, $out, 0, $depmap{$_});
        print {$out} "\n\n";
    }
    close $out;

    return 0;
}

sub generate_def {
    my @files = split /\n/, `git ls-files`;
    @files = grep(/\.c/, @files);
    @files = map { my $x = $_; $x =~ s/^bn_|\.c$//g; $x; } @files;
    @files = grep(!/mp_radix_smap/, @files);

    push(@files, qw(mp_set_int mp_set_long mp_set_long_long mp_get_int mp_get_long mp_get_long_long mp_init_set_int));

    my $files = join("\n    ", sort(grep(/^mp_/, @files)));
    write_file "tommath.def", "; libtommath
;
; Use this command to produce a 32-bit .lib file, for use in any MSVC version
;   lib -machine:X86 -name:libtommath.dll -def:tommath.def -out:tommath.lib
; Use this command to produce a 64-bit .lib file, for use in any MSVC version
;   lib -machine:X64 -name:libtommath.dll -def:tommath.def -out:tommath.lib
;
EXPORTS
    $files
";
    return 0;
}

sub die_usage {
  die <<"MARKER";
usage: $0 -s   OR   $0 --check-source
       $0 -o   OR   $0 --check-comments
       $0 -m   OR   $0 --check-makefiles
       $0 -a   OR   $0 --check-all
       $0 -u   OR   $0 --update-files
MARKER
}

GetOptions( "s|check-source"        => \my $check_source,
            "o|check-comments"      => \my $check_comments,
            "m|check-makefiles"     => \my $check_makefiles,
            "d|check-doc"           => \my $check_doc,
            "a|check-all"           => \my $check_all,
            "u|update-files"        => \my $update_files,
            "h|help"                => \my $help
          ) or die_usage;

my $failure;
$failure ||= check_source()       if $check_all || $check_source;
$failure ||= check_comments()     if $check_all || $check_comments;
$failure ||= check_doc()          if $check_doc; # temporarily excluded from --check-all
$failure ||= process_makefiles(0) if $check_all || $check_makefiles;
$failure ||= process_makefiles(1) if $update_files;
$failure ||= update_dep()         if $update_files;
$failure ||= generate_def()       if $update_files;

die_usage unless defined $failure;
exit $failure ? 1 : 0;
