#!/bin/sh
# usage: gen_emmc_linux_raw_img.sh linux_fit_binary rootfs_binary metadata_binary linux_raw_img


die() { echo "$*" >&2; exit 2; }  # complain to STDERR and exit with error
needs_arg() { if [ -z "$OPTARG" ]; then die "No arg for --$OPT option"; fi; }

while getopts r:b:-: OPT; do
  # support long options: https://stackoverflow.com/a/28466267/519360
  if [ "$OPT" = "-" ]; then   # long option: reformulate OPT and OPTARG
    OPT="${OPTARG%%=*}"       # extract long option name
    OPTARG="${OPTARG#$OPT}"   # extract long option argument (may be empty)
    OPTARG="${OPTARG#=}"      # if long option argument, remove assigning `=`
  fi
  case "$OPT" in
    b | boot )     needs_arg; bootmb="$OPTARG" ;;
    r | root )     needs_arg; rootmb="$OPTARG" ;;
    ??* )          die "Illegal option --$OPT" ;;  # bad long option
    ? )            exit 2 ;;  # bad short option (error reported via getopts)
  esac
done
shift $((OPTIND-1)) # remove parsed options and args from $@ list

linux_fit=$1;
rootfs_bin=$2;
metadata_bin=$3;
linux_raw_img=$4;

# Partition sizes in MB. Tweak these to get different partition sizes for your implementation
data_len_mb=100;
defaults_len_mb=20;
overhead_len_mb=10;
metadata_len_kb=256;

# Calulate lengths in MB
bootfs_len=`stat -c%s $linux_fit`;
rootfs_len=`stat -c%s $rootfs_bin`;
bootfs_len_mb=`expr $bootfs_len / 1048576 + $overhead_len_mb`;
rootfs_len_mb=`expr $rootfs_len / 1048576 + $overhead_len_mb`;

if [ -n "$bootmb" ]
then
  bootfs_len_mb=$bootmb
fi 

if [ -n "$rootmb" ]
then
  rootfs_len_mb=$rootmb
fi 

# Partition size strings in KB and MB
metadata1_offs_kb=256;
metadata2_offs_kb=512;
bootfs1_offs_mb=1;
rootfs1_offs_mb=`expr $bootfs1_offs_mb + $bootfs_len_mb + 1`;

# Calculate total and truncated image lengths
trunc_len_mb=`expr $rootfs1_offs_mb + $rootfs_len_mb + 1`;  
tot_len_mb=`expr $trunc_len_mb + $rootfs_len_mb + $bootfs_len_mb + $data_len_mb + $defaults_len_mb + $overhead_len_mb`;

#echo "$bootfs1_offs_mb $rootfs_len_mb $rootfs1_offs_mb $bootfs_len_mb $tot_len_mb $trunc_len_mb";

# Generate temp full userdata image
rm -f ${linux_raw_img}.temp
dd if=/dev/zero of=${linux_raw_img}.temp bs=1M count=$tot_len_mb; 

# Create partitions on full binary
sgdisk -a 512 -n 1:${metadata1_offs_kb}KiB:+${metadata_len_kb}KiB -c 1:metadata1 -n 2:${metadata2_offs_kb}KiB:+${metadata_len_kb}KiB -c 2:metadata2 -n 3:${bootfs1_offs_mb}MiB:+${bootfs_len_mb}MiB -c 3:bootfs1 -n 4:${rootfs1_offs_mb}MiB:+${rootfs_len_mb}MiB -c 4:rootfs1 -n 5::+${bootfs_len_mb}MiB -c 5:bootfs2 -n 6::+${rootfs_len_mb}MiB -c 6:rootfs2 -n 7::+${data_len_mb}MiB  -c 7:data -n 8::+${defaults_len_mb}MiB -c 8:defaults ${linux_raw_img}.temp;

# Copy over metadata
dd if=$metadata_bin of=${linux_raw_img}.temp bs=1KiB seek=${metadata1_offs_kb} count=${metadata_len_kb} conv=notrunc;
dd if=$metadata_bin of=${linux_raw_img}.temp bs=1KiB seek=${metadata2_offs_kb} count=${metadata_len_kb} conv=notrunc;

# Copy over bootfs
dd if=$linux_fit of=${linux_raw_img}.temp bs=1MiB seek=${bootfs1_offs_mb} conv=notrunc; 

# Copy over rootfs
dd if=$rootfs_bin of=${linux_raw_img}.temp bs=1MiB seek=${rootfs1_offs_mb} conv=notrunc; 

# Get truncated userdata 
rm -f ${linux_raw_img}
dd if=${linux_raw_img}.temp of=${linux_raw_img} bs=1MiB count=${trunc_len_mb} conv=notrunc; 
rm -f ${linux_raw_img}.temp


