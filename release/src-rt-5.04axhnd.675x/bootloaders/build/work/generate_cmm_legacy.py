#!/usr/bin/env python

import sys
import os


objdir=sys.argv[1]
chip=sys.argv[2]

if chip.find('63138') >= 0  or chip.find('63148') >= 0:
	cross_compile=sys.argv[3]
	spl_dir=sys.argv[4]
	cmm_out=objdir+"/cmm/"+chip+"/"
	dtbdir=objdir+"/binaries/linux/"
	os.system("cp build/support/jtag/misc/ddrinit/"+"ddrinit" +  " " + cmm_out) 

	for entry in os.listdir(dtbdir):
	    if (entry.endswith(".dtb")):
		bid=entry.rstrip(".dtb")
		gdb_contents='''load ddrinit\nfile ddrinit\nb done\nc\nrestore u-boot-''' + bid + '''.bin binary 0x1000000\nset $pc=0x1000000\nc\n''' 
		text_file = open(cmm_out+bid+".gdb", "w")
		text_file.write(gdb_contents)
		text_file.close()
		cmm_contents='''do ~~~~/attach.cmm\nbreak\nd.load.elf ~~~~/ddrinit\nb done\ngo\nwait !run()\nd.load.binary  ~~~~/u-boot-''' + bid + '''.bin 0x1000000\nr.s pc 0x1000000\ngo\n''' 
		text_file = open(cmm_out+bid+".cmm", "w")
		text_file.write(cmm_contents)
		text_file.close()

