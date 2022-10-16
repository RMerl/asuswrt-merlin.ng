#!/usr/bin/env python
import sys
import os

def write_header_file(rpath, wpath, start_str, end_str, case):

	defaults_file = open(wpath,"a")

	if case == 0:
		defaults_file.write("#include \"calc_nvram.h\"\n")

	if os.path.isfile(rpath):
		# opening a c file
		with open(rpath, "r") as f:
			# setting flag 0:init 1:start 2:end
			search_flag = 0

			# Loop through the file line by line
			for line in f:

				if line.startswith('#') or not line.strip():
					continue

				# checking string is present in line or not
				if start_str in line:
					search_flag = 1
					if case == 1:
						line = line.replace("static ", "")
				elif search_flag == 1 and end_str in line:
					search_flag = 2

				if search_flag >= 1:
					defaults_file.write(line)

				if search_flag == 2:
					break
	else:
		defaults_file.write(start_str+end_str)

	defaults_file.close()

def main(top_path):

	# read/write path defined
	def_nv_path=top_path+"/shared/defaults.prep"
	nvpriv_path=top_path+"/shared/nvpriv.prep"
	default_header="defaults_nv.h"
	# search string
	default_start="struct nvram_tuple router_defaults[] = {"
	nvpriv_start="static char *large_nvram_list[] = {"
	str_end="};"

	# generate header file for c
	write_header_file(def_nv_path, default_header, default_start, str_end, 0)
	write_header_file(nvpriv_path, default_header, nvpriv_start, str_end, 1)

if __name__ == "__main__":
	if(len(sys.argv) != 2):
		print("Usage: python calc_nvram.py <top_path>")
		sys.exit(0)
	else:
		print("run calc_nvram.py {}".format(sys.argv[1]))
		main(sys.argv[1])
