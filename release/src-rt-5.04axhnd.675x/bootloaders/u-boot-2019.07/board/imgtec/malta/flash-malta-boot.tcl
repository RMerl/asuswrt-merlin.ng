# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2013 Imagination Technologies
#
# Programs a MIPS Malta boot flash with a flat binary image.

proc flash-boot { binfile } {
  puts "flash monitor binary $binfile"
  config Coherent on
  config CoherencyDuringLoad on

  if {[endian]=="big"} {
    puts "CPU in BE mode"
    flash device sharp_16x32_be;
  } else {
    puts "CPU in LE mode"
    flash device sharp_16x32;
  }

  flash clear all;
  flash set 0xBE000000..0xBE0FFFFF
  flash erase sector 0xbe000000;
  flash erase sector 0xbe020000;
  flash erase sector 0xbe040000;
  flash erase sector 0xbe060000;
  flash erase sector 0xbe080000;
  flash erase sector 0xbe0a0000;
  flash erase sector 0xbe0c0000;
  flash erase sector 0xbe0e0000;
  puts "finished erasing boot flash";

  puts "programming flash, please be patient"
  load bin 0xbe000000 $binfile size4

  flash clear all
  config CoherencyDuringLoad off
  puts "finished programming boot flash";
}
