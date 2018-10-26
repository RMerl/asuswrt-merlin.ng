#!/usr/bin/env python
import argparse
import os.path
import os
import subprocess
import sys
import re

# Settings mirrored from source header files will be replaced with actual
# values from header file if header file is specified on command line
num_images = 2
num_mdata = 2
sgdisk_sector_alignment = 2048

# Default Settings
cferom_offsetkB = 64
gpt_overheadMB = 10
sgdisk_seperator = " "

########################### FUNCTIONS ###########################

# Gets the value of a 'C' '#define' from a header file
def get_define_val( define_name, file_content_list ):
	def_regex = re.compile('#define +' + define_name + ' +((0x)?[0-9]+).*')
	for line in file_content_list:
		result = def_regex.match(line)
		if( result ):
			define_val = result.group(1)
			if( result.group(2) ):
				return int(define_val, 16)
			else:
				return int(define_val)
	return None

# Sets global variables by reading emmc header files
def set_emmc_globals( header_file ):
	global num_images
	global num_mdata 
	global sgdisk_sector_alignment

	# Open and read file into a list
	fd = open(header_file, 'r')
	if( fd ):
		file_contents = list(fd)
		num_images = get_define_val('EMMC_NUM_IMGS', file_contents)
		num_mdata = get_define_val('EMMC_NUM_MDATA', file_contents)
		sgdisk_sector_alignment = get_define_val('EMMC_PART_ALIGN_LB', file_contents)

# Creates GPT partitions and copies imgdata
# Partition list = [ partition name, partition size, partition binary data ]
# img_file_name = name of final raw image file
def create_gpt_partitions( partition_list, img_file_name ):
	partition_num = 1

	# Align all partitions to 2048 X 512Byte sectors = 1Mbytes
	# DONOT Change this alignment without changing CFE source
	# variable EMMC_PART_ALIGN_LB
	sgdisk_cmd = "sgdisk --set-alignment=" + str(sgdisk_sector_alignment)
	for x in partition_list:
		# Specify partition size and offset
		sgdisk_cmd += " -n" + sgdisk_seperator + str(partition_num) + ":0:+" + x[1] + "M"
		# Label partition
		sgdisk_cmd += " -c" + sgdisk_seperator + str(partition_num) + ":" + x[0]
		partition_num += 1
	sgdisk_cmd += " " + img_file_name

	# Create partitions
	print(sgdisk_cmd)
	FNULL = open(os.devnull, 'w')
	subprocess.call(sgdisk_cmd, stdout=FNULL, stderr=subprocess.STDOUT, shell=True)
	sgdisk_partition_cfg = subprocess.check_output( 'sgdisk -p ' + img_file_name , shell=True );

	# Get sector size
	sector_size = sgdisk_partition_cfg.split("sector size: ", 1)[1].split(" bytes",1)[0]

	# Get a list of sgdisk partition info rows
	partition_info_list = sgdisk_partition_cfg.split("Name\n", 1)[1].split("\n")

	# Get sector offsets for each partition and write img data to proper locations
	for x,y in zip(partition_list,partition_info_list):
		partition_info = y.split()
		partition_offset = partition_info[1]
		# Only copy over partition data if it is specified
		if( x[2] != None ):
			cmd = "dd if=%s of=%s bs=%s seek=%s conv=nocreat,notrunc" % (x[2], img_file_name, sector_size, partition_offset)
			print(cmd)
			subprocess.call(cmd, shell=True)
			img_write_byte_offset = int(x[1]) + (int(sector_size) * int(partition_offset))

	# Only copy parts of the img_file that have valid written data
	# This will truncate the image to the last partition data written
	# and will result in us ommitting the backup GPT table which is
	# supposed to be at the end of flash. We do this to reduce the size
	# of the raw image, CFE ram should detect this and re-write the proper
	# backup GPT table
	cmd = "truncate -s %sM %s" % (str(img_write_byte_offset/(1024*1024) + 1), img_file_name)
	#cmd = "dd if=%s of=%s bs=1M count=%s" % (img_file_name, img_file_name, str(img_write_byte_offset/(1024*1024) + 1))
	print(cmd + ' ' + str(img_write_byte_offset))
	subprocess.call(cmd, shell=True)

	return;

# Gets filesize in MB
def get_filesize_MB( filename ):
	align_overhead = 0
	file_size = os.stat(filename).st_size
	if( file_size%(1024*1024) ):
		align_overhead = 1
	sizeMB = str(file_size/(1024*1024) + align_overhead)
	return sizeMB

# Main function	
def main():
	# Globals
	global cferom_offsetkB

	# Argument parser 
	parser = argparse.ArgumentParser(description='This is a emmc raw image generation script.')

	# full image related options
	parser.add_argument('--nvram_file',  help='nvram  binary ', required=True)
	parser.add_argument('--bootfs_file', help='bootfs binary ', required=True)
	parser.add_argument('--rootfs_file', help='rootfs binary ')
	parser.add_argument('--mdata_file',  help='mdata  binary ', required=True) 
	parser.add_argument('--data_sizeMB',   help='data  partition size ') 
	parser.add_argument('--rawfullimg_file', help='output full raw image (no cferom) ', required=True) 
	parser.add_argument('--nvram_sizeMB',  help='nvram  partition size ')
	parser.add_argument('--bootfs_sizeMB', help='bootfs partition size ')
	parser.add_argument('--rootfs_sizeMB', help='rootfs partition size ')
	parser.add_argument('--misc1_sizeMB',  help='misc1  partition size ') 
	parser.add_argument('--misc2_sizeMB',  help='misc2  partition size ') 
	parser.add_argument('--misc3_sizeMB',  help='misc3  partition size ') 
	parser.add_argument('--misc4_sizeMB',  help='misc4  partition size ') 
	parser.add_argument('--emmcdefs_file', help='Header file containing emmc base defines ', required=True) 

	# cferom related options
	parser.add_argument('--cferom_file'  ,help='cferom binary') 
	parser.add_argument('--cferom_offsetkB',help='Offset of cferom from start of boot partition') 
	parser.add_argument('--rawcfeimg_file',  help='output cferom only raw image ') 

	args = parser.parse_args()

	# Check whether we need to include rootfs
	if( (args.rootfs_file != None) and os.path.isfile(args.rootfs_file) ):
		include_rootfs = True
	else:
		include_rootfs = False 

	# show values 
	print ("\n\n#### eMMC raw image generation utility ####")

        # Get emmc default values 
	if ( os.path.isfile(args.emmcdefs_file) ):
		set_emmc_globals(args.emmcdefs_file)
	else:
		print("Invalid emmc header file!! ")
		parser.print_help()
		sys.exit()

	# cfe raw binary image generation
	if ( (args.cferom_file != None) and (args.rawcfeimg_file != None) and os.path.isfile(args.cferom_file)):
		print("\n[Generating eMMC physical BOOT partition raw image]")
		print ("Input files: " + args.cferom_file )
		print ("Output file: " + args.rawcfeimg_file )
		# Get cferom offset
		if( (args.cferom_offsetkB != None) ):
			cferom_offsetkB = int(args.cferom_offsetkB)

		# Determine cfe size
		cferom_sizeMB = get_filesize_MB(args.cferom_file)
		
		# Generate raw binary
		cmd = 'dd if=/dev/zero of=%s bs=1M count=%s' % ( args.rawcfeimg_file, cferom_sizeMB )
		print(cmd)
		subprocess.call(cmd, shell=True)

		# Copy over img data
		cmd = 'dd if=%s of=%s bs=1K seek=%s conv=nocreat,notrunc' % (args.cferom_file, args.rawcfeimg_file, str(cferom_offsetkB))
		print(cmd)
		subprocess.call(cmd, shell=True)

	# raw full image generation
	if ( os.path.isfile(args.nvram_file) and os.path.isfile(args.bootfs_file) and os.path.isfile(args.mdata_file)):
		print("\n[Generating eMMC physical DATA partition raw image]")
		print ("Input files: " + args.nvram_file + " " + args.bootfs_file + " " + args.mdata_file)
		if( include_rootfs ):
			print ("             " + " " + args.rootfs_file)
		print ("Output file: " + args.rawfullimg_file )

		# Setup partition list
		partition_list = []

		# Add nvram partition
		if ( (args.nvram_sizeMB == None) ):
			args.nvram_sizeMB = get_filesize_MB(args.nvram_file)
			partition_list.append(['nvram', args.nvram_sizeMB, args.nvram_file])

		# Add bootfs/rootfs and mdata partitions
		if ( (args.bootfs_sizeMB == None) ):
			args.bootfs_sizeMB = get_filesize_MB(args.bootfs_file)

		# Only add the rootfs partition if specified
		if( include_rootfs ):
			if ( (args.rootfs_sizeMB == None) ):
				args.rootfs_sizeMB = get_filesize_MB(args.rootfs_file)

		args.mdata_sizeMB = get_filesize_MB(args.mdata_file)

		for x in range(1,num_images+1):
			# Add bootfs
			partition_list.append(['bootfs'+str(x), args.bootfs_sizeMB, args.bootfs_file if x== 1 else None ])
			# Add rootfs
			if( include_rootfs ):
				partition_list.append(['rootfs'+str(x), args.rootfs_sizeMB, args.rootfs_file if x== 1 else None ])
			# Add mdata
			for y in range(1,num_mdata+1):
				partition_list.append(['mdata'+str(x)+'_'+str(y), args.mdata_sizeMB, args.mdata_file if x== 1 else None])

		# Full image contains 1 nvram, 2 bootfs, 2 rootfs and 4 mdata partitions. We add extra 10MB for GPT overhead
		full_img_size = (int(args.nvram_sizeMB) 
				+ num_images*int(args.bootfs_sizeMB) 
				+ num_images*num_mdata*int(args.mdata_sizeMB) 
				+ gpt_overheadMB)
		if( include_rootfs ):
				full_img_size += num_images*int(args.rootfs_sizeMB) 

		# Add data partition
		if ( (args.data_sizeMB != None) ):
			full_img_size += int(args.data_sizeMB)
			partition_list.append(['data', args.data_sizeMB, None])

		# Add misc partitions
		if ( (args.misc1_sizeMB != None) ):
			full_img_size += int(args.misc1_sizeMB)
			partition_list.append(['misc1', args.misc1_sizeMB, None])
		if ( (args.misc2_sizeMB != None) ):
			full_img_size += int(args.misc2_sizeMB)
			partition_list.append(['misc2', args.misc2_sizeMB, None])
		if ( (args.misc3_sizeMB != None) ):
			full_img_size += int(args.misc3_sizeMB)
			partition_list.append(['misc3', args.misc3_sizeMB, None])
		if ( (args.misc4_sizeMB != None) ):
			full_img_size += int(args.misc4_sizeMB)
			partition_list.append(['misc4', args.misc4_sizeMB, None])

		# Add more custom partitions here

		#print('Sizes(MB) %s= %s %s %s %s %s %s %s %s %d' %( str(full_img_size), args.nvram_sizeMB, args.bootfs_sizeMB, args.rootfs_sizeMB, args.mdata_sizeMB,
		#		args.misc1_sizeMB, args.misc2_sizeMB, args.misc3_sizeMB, args.misc4_sizeMB, gpt_overheadMB ))

		# generate empty binary
		cmd = 'dd if=/dev/zero of=%s bs=1M count=%s' % ( args.rawfullimg_file, str(full_img_size))
		print(cmd)
		subprocess.call(cmd, shell=True)

		# Create partitions and copy over img data
		create_gpt_partitions(partition_list, args.rawfullimg_file)
	else:
		print("Invalid input img binaries!! ")
		parser.print_help()

	print ("#### eMMC raw image generation complete ####\n\n")


######################## SCRIPT START ###########################

main()
