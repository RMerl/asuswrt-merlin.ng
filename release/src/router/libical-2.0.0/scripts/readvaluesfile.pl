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

sub read_values_file
{

  my $path = shift;
  my %h;

  open(F, $path) || die "Can't open values file $path";

  while (<F>) {

    chop;

    s/#.*$//g;
    s/\"//g;
    s/\r//g;

    next if !$_;

    @column = split(/,/, $_);

    my $value_name = $column[0];
    my $enumConst  = $column[1];

    my $c_type_str = $column[2];
    my $c_autogen  = ($c_type_str =~ /\(a\)/);

    my $c_type = $c_type_str;
    $c_type =~ s/\(.\)//;

    my $python_type = $column[3];
    my $components  = $column[4];
    my $enum_values = $column[5];

    my @components;
    if ($components ne "unitary") {
      @components = split(/;/, $components);
    } else {
      @components = ();
    }

    my @enums;
    if ($enum_values) {
      @enums = split(/;/, $enum_values);

    } else {
      @enums = ();
    }

    $h{$value_name} = {
      C          => [$c_autogen, $c_type],
      kindEnum   => $enumConst,
      perl       => $perl_type,
      python     => $python_type,
      components => [@components],
      enums      => [@enums]
    };
  }

  return %h;
}

sub read_properties_file
{

  my $path = shift;
  my %h;

  open(F, $path) || die "Can't open properties file $path";

  while (<F>) {

    chop;

    s/#.*$//g;
    s/\"//g;
    s/\r//g;

    next if !$_;

    @column = split(/,/, $_);

    my $property_name = $column[0];

    my $enumConst     = $column[1];
    my $lic_value     = $column[2];
    my $default_value = $column[3];

    $h{$property_name} = {
      lic_value     => $lic_value,
      kindEnum      => $enumConst,
      default_value => $default_value
    };
  }

  return %h;
}

sub read_parameters_file
{

  my $path = shift;
  my %h;

  open(F, $path) || die "Can't open parameters file $path";

  while (<F>) {

    chop;

    s/#.*$//g;
    s/\"//g;
    s/\r//g;

    next if !$_;

    @column = split(/\,/, $_);

    my $parameter_name = $column[0];

    my $enumConst   = $column[1];
    my $data_type   = $column[2];
    my $enum_string = $column[3];

    my @enums;
    if ($enum_string) {
      @enums = split(/;/, $enum_string);
    }

    $h{$parameter_name} = {
      C        => $data_type,
      kindEnum => $enumConst,
      enums    => [@enums]
    };
  }

  close(F);

  return %h;
}

1;
