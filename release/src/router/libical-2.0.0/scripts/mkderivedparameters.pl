#!/usr/bin/env perl
################################################################################
# (C) COPYRIGHT 2000, Eric Busboom <eric@softwarestudio.org>
#     http://www.softwarestudio.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of either:
#
#   The LGPL as published by the Free Software Foundation, version
#   2.1, available at: http://www.gnu.org/licenses/lgpl-2.1.txt
#
# Or:
#
#   The Mozilla Public License Version 1.0. You may obtain a copy of
#   the License at http://www.mozilla.org/MPL/
################################################################################

require "readvaluesfile.pl";

use Getopt::Std;
getopts('chspi:');

%no_xname = (RELATED => 1, RANGE => 1, RSVP => 1, XLICERRORTYPE => 1, XLICCOMPARETYPE => 1);

%params = read_parameters_file($ARGV[0]);

# Write the file inline by copying everything before a demarcation
# line, and putting the generated data after the demarcation

if ($opt_i) {

  open(IN, $opt_i) || die "Can't open input file $opt_i";

  while (<IN>) {
    if (/<insert_code_here>/) {
      $autogenMsg = "of section of machine generated code (mkderivedparameters.pl). Do not edit.";
      if ($opt_p) {
        $startComment = "#";
        $endComment   = "";
      } else {
        $startComment = "/*";
        $endComment   = " */";
      }
      print $startComment. " START " . $autogenMsg . $endComment . "\n\n";

      insert_code();

      print $startComment. " END   " . $autogenMsg . $endComment . "\n\n";
    } else {
      print;
    }

  }

}

sub insert_code
{

  # Write parameter enumerations and datatypes

  if ($opt_h) {
    my $enumConst = $params{'ANY'}->{"kindEnum"};
    print "typedef enum icalparameter_kind {\n    ICAL_ANY_PARAMETER = " . $enumConst . ",\n";
    $enumVal = 1;
    foreach $param (sort keys %params) {

      next if !$param;

      next if $param eq 'NO' or $param eq 'ANY';

      my $uc = join("", map {uc($_);} split(/-/, $param));

      $enumConst = $params{$param}->{"kindEnum"};

      print "    ICAL_${uc}_PARAMETER = " . $enumConst . ", \n";

    }
    $enumConst = $params{'NO'}->{"kindEnum"};
    print "    ICAL_NO_PARAMETER = " . $enumConst . "\n} icalparameter_kind;\n\n";

    # Now create enumerations for parameter values
    $lastidx = $idx = 20000;

    print "#define ICALPARAMETER_FIRST_ENUM $idx\n\n";

    foreach $param (sort keys %params) {

      next if !$param;

      next if $param eq 'NO' or $param eq 'ANY';

      my $type  = $params{$param}->{"C"};
      my $ucv   = join("", map {uc(lc($_));} split(/-/, $param));
      my @enums = @{$params{$param}->{'enums'}};

      if (@enums) {

        print "typedef enum $type {\n";
        my $first = 1;

        foreach $e (@enums) {
          if (!$first) {
            print ",\n";
          } else {
            $first = 0;
          }

          $e =~ /([a-zA-Z0-9\-]+)=?([0-9]+)?/;
          $e = $1;
          if ($2) {
            $idx = $2;
          } else {
            $idx++;
          }
          if ($idx > $lastidx) {
            $lastidx = $idx;
          }

          my $uce = join("", map {uc(lc($_));} split(/-/, $e));

          print "    ICAL_${ucv}_${uce} = $idx";
        }
        $c_type =~ s/enum //;

        print "\n} $type;\n\n";
      }
    }

    $lastidx++;
    print "#define ICALPARAMETER_LAST_ENUM $lastidx\n\n";

  }

  if ($opt_c) {

    # Create the icalparameter_value to icalvalue_kind conversion table
    my $count = 0;
    my $out;

    foreach $enum (@{$params{'VALUE'}->{'enums'}}) {
      $enum =~ /([a-zA-Z0-9\-]+)=?([0-9]+)?/;
      $enum = $1;

      next if $enum eq 'X'  or $enum eq 'NONE';
      next if $enum eq 'NO' or $enum eq 'ERROR';
      $uc = join("", map {uc(lc($_));} split(/-/, $enum));
      $out .= "    {ICAL_VALUE_${uc},ICAL_${uc}_VALUE},\n";
      $count++;
    }

    $count += 2;
    print "static const struct icalparameter_value_kind_map value_kind_map[$count] = {\n";
    print $out;
    print "    {ICAL_VALUE_X,ICAL_X_VALUE},\n";
    print "    {ICAL_VALUE_NONE,ICAL_NO_VALUE}\n};\n\n";

    #Create the parameter Name map

    $out   = "";
    $count = 0;
    foreach $param (sort keys %params) {

      next if !$param;

      next if $param eq 'NO' or $param eq 'ANY';

      my $lc = join("", map {lc($_);} split(/-/, $param));
      my $uc = join("", map {uc(lc($_));} split(/-/, $param));

      $count++;
      $out .= "    {ICAL_${uc}_PARAMETER,\"$param\"},\n";

    }
    $count += 1;
    print "static const struct icalparameter_kind_map parameter_map[$count] = { \n";
    print $out;
    print "    { ICAL_NO_PARAMETER, \"\"}\n};\n\n";

    # Create the parameter value map
    $out   = "";
    $count = 0;
    foreach $param (sort keys %params) {

      next if !$param;

      next if $param eq 'NO' or $param eq 'ANY';

      my $type  = $params{$param}->{"C"};
      my $uc    = join("", map {uc(lc($_));} split(/-/, $param));
      my @enums = @{$params{$param}->{'enums'}};

      if (@enums) {

        foreach $e (@enums) {
          $e =~ /([a-zA-Z0-9\-]+)=?([0-9]+)?/;
          $e = $1;

          next if $e eq 'X' or $e eq 'NONE';

          my $uce = join("", map {uc(lc($_));} split(/-/, $e));

          $count++;
          $out .= "    {ICAL_${uc}_PARAMETER,ICAL_${uc}_${uce},\"$e\"},\n";
        }

      }
    }

    $count += 3;
    print "static const struct icalparameter_map icalparameter_map[] = {\n";
    print "{ICAL_ANY_PARAMETER,0,\"\"},\n";
    print $out;
    print "    {ICAL_NO_PARAMETER,0,\"\"}};\n\n";

  }

  foreach $param (sort keys %params) {

    next if $param eq 'NO' or $param eq 'ANY';

    my $type = $params{$param}->{'C'};

    my $ucf = join("", map {ucfirst(lc($_));} split(/-/, $param));

    my $lc = lc($ucf);
    my $uc = uc($lc);

    my $charorenum;
    my $set_code;
    my $pointer_check;
    my $pointer_check_v;
    my $xrange;

    if ($type =~ /char/) {

      $charorenum =
        "    icalerror_check_arg_rz( (param!=0), \"param\");\n    return param->string;";

      $set_code = "((struct icalparameter_impl*)param)->string = icalmemory_strdup(v);";

      $pointer_check   = "icalerror_check_arg_rz( (v!=0),\"v\");";
      $pointer_check_v = "icalerror_check_arg_rv( (v!=0),\"v\");";

    } elsif ($type =~ /int/) {

      $charorenum =
        "    icalerror_check_arg( (param!=0), \"param\");\n$xrange\nreturn param->data;";

      $set_code = "((struct icalparameter_impl*)param)->data = v;";

    } else {

      $xrange = "     if (param->string != 0){\n        return ICAL_${uc}_X;\n        }\n"
        if !exists $no_xname{$uc};

      $charorenum =
        "icalerror_check_arg( (param!=0), \"param\");\n$xrange\nreturn ($type)(param->data);";

      $pointer_check =
"icalerror_check_arg_rz(v >= ICAL_${uc}_X,\"v\");\n    icalerror_check_arg_rz(v < ICAL_${uc}_NONE,\"v\");";

      $pointer_check_v =
"icalerror_check_arg_rv(v >= ICAL_${uc}_X,\"v\");\n    icalerror_check_arg_rv(v < ICAL_${uc}_NONE,\"v\");";

      $set_code = "((struct icalparameter_impl*)param)->data = (int)v;";

    }

    if ($opt_c) {

      print <<EOM;
/* $param */
icalparameter* icalparameter_new_${lc}($type v)
{
   struct icalparameter_impl *impl;
   icalerror_clear_errno();
   $pointer_check
   impl = icalparameter_new_impl(ICAL_${uc}_PARAMETER);
   if (impl == 0) {
      return 0;
   }

   icalparameter_set_${lc}((icalparameter*) impl,v);
   if (icalerrno != ICAL_NO_ERROR) {
      icalparameter_free((icalparameter*) impl);
      return 0;
   }

   return (icalparameter*) impl;
}

${type} icalparameter_get_${lc}(const icalparameter* param)
{
   icalerror_clear_errno();
$charorenum
}

void icalparameter_set_${lc}(icalparameter* param, ${type} v)
{
   $pointer_check_v
   icalerror_check_arg_rv( (param!=0), "param");
   icalerror_clear_errno();

   if (param->string != NULL)
      free ((void*)param->string);
   $set_code
}

EOM

    } elsif ($opt_h) {

      print <<EOM;
/* $param */
LIBICAL_ICAL_EXPORT icalparameter* icalparameter_new_${lc}($type v);
LIBICAL_ICAL_EXPORT ${type} icalparameter_get_${lc}(const icalparameter* value);
LIBICAL_ICAL_EXPORT void icalparameter_set_${lc}(icalparameter* value, ${type} v);

EOM

    }

    if ($opt_p) {

      print <<EOM;

# $param

package Net::ICal::Parameter::${ucf};
\@ISA=qw(Net::ICal::Parameter);

sub new
{
   my \$self = [];
   my \$package = shift;
   my \$value = shift;

   bless \$self, \$package;

   my \$p;

   if (\$value) {
      \$p = Net::ICal::icalparameter_new_from_string(\$Net::ICal::ICAL_${uc}_PARAMETER,\$value);
   } else {
      \$p = Net::ICal::icalparameter_new(\$Net::ICal::ICAL_${uc}_PARAMETER);
   }

   \$self->[0] = \$p;

   return \$self;
}

sub get
{
   my \$self = shift;
   my \$impl = \$self->_impl();

   return Net::ICal::icalparameter_as_ical_string(\$impl);

}

sub set
{
   # This is hard to implement, so I've punted for now.
   die "Set is not implemented";
}

EOM

    }

  }

  if ($opt_h) {

    print <<EOM;
#endif /*ICALPARAMETER_H*/

EOM
  }

}
