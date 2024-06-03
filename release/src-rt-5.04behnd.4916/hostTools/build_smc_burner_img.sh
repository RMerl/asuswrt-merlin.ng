#!/bin/bash

help () {
    echo "Usage: $(basename ${0%.*}).sh -p profile -f flash_type -e env_conf -r rootfs_type [-b board_id] [-v]"
    echo "            -p          profile name as it is in make PROFILE parameter"
    echo "            -f          flash type: emmc, nand128 or nand256"
    echo "            -e          uboot env source file"
    echo "            -r          ext4 or squashfs, in case of NAND could be omitted to produce bootstrap image without FS"
    echo "            -b          board id to be added to environment, optional"
    echo "            -v          explain what is being done"
    exit 0
}

fdump () {
    filesize=$(stat -c "%s" $2)
    if [[ $filesize -gt $3 ]]; then
        echo "ERROR: image $2 ($filesize) is larger then reserved ($3)"
        exit 1
    fi
    if [ "$verbose" != "" ]; then
        x=$( printf "%06x" $current )
        if [[ $1 -gt 0 ]]; then
            echo "$2 >> $output ($1 @ 0x$x)"
        else
            y=$( printf "%06x" $(stat -c%s "$2") )
        echo "$2 >> $output 0x$y @ 0x$x)"
        fi
        current=$(($current + $1))
        dd if=$2 1>> $output
    else
        dd if=$2 1>> $output 2>&-
    fi
    padsize=$1
    if [[ $padsize -gt 0 ]]; then
        padcount=$((padsize - filesize))
        < /dev/zero tr '\000' '\377' | head -c "$padcount" 1>> $output
    fi
}

create_ini () {
cat <<EOF > $profile/bcm${profile}_dyn_burn.ini
[03-lun]
image=$profile/bcm${profile}_uboot_linux.itb
lun_id=3
lun_name=bootfs1
[04-lun]
image=$rootfs
lun_id=4
lun_name=rootfs1
EOF
}

while getopts "hvp:f:e:b:r:" options; do
    case "${options}" in
        p)
            profile=${OPTARG}
            ;;
        f)
            ftype=${OPTARG}
            ;;
        e)
            env=${OPTARG}
            ;;
        b)
            bid=${OPTARG}
            ;;
        r)
            rfs=${OPTARG}
            ;;
        h)
            help
            ;;
        v)
            verbose="-v"
            ;;
    esac
done

cd `dirname -- "$0"`/../targets

if [[ $profile == *"68880"* ]]; then
    chip=68880B0
elif [[ $profile == *"6837"* ]]; then
    chip=6837A0
else
    echo "ERROR: wrong profile '$profile'"
    exit 1
fi

BL_MAX=0x040000
OS_MAX=0x1A0000
ENV_MAX=0x004008
MEM_MAX=0x100000
TPL_MAX=0x100000

current=0
if [[ $ftype == *"emmc"* ]]; then
    DYN_MAX=0x10000000
    BL_PAD=0x040000
    OS_PAD=0x1A0000
    ENV_PAD=0x020000
    MEM_PAD=0x100000
    TPL_PAD=0x340000
    DYN_START=0x480000 
    output=$profile/bcm${profile}_emmc_boot_partition.bin
    if [[ $rfs == *"ex"* ]]; then
        output2=$profile/bcm${profile}_emmc_user_partition_ext4.bin
        rootfs=$profile/rootfs.ext4
        echo "Building EMMC flash image with ext4 rootfs..."
    elif [[ $rfs == *"sq"* ]]; then
        output2=$profile/bcm${profile}_emmc_user_partition_squash.bin
        rootfs=$profile/rootfs.img
        echo "Building EMMC flash image with squash rootfs..."
    else
        echo "ERROR: wrong FS type '$rfs' (could be ext4 or squash only)"
        exit 1
    fi
else
    output2=""
    if [[ $rfs == *"ex"* ]]; then
        pkgtb=../bootloaders/obj/binaries/bcm${profile}_ext4_update.pkgti
        msg="with ext4 rootfs" 
    elif [[ $rfs == *"sq"* ]]; then
        pkgtb=../bootloaders/obj/binaries/bcm${profile}_squashfs_update.pkgti
        msg="with squash rootfs" 
    else
        rfs=bstrap
        pkgtb=$profile/bcm${profile}_bstrap.pkgtb
    fi
    PKG_MAX=0x8000000
    if [[ $ftype == *"nand1"* ]]; then
        BL_PAD=0x060000
        OS_PAD=0x400000
        ENV_PAD=0x040000
        MEM_PAD=0x120000
        TPL_PAD=0x360000
        output=$profile/bcm${profile}_nand_128_${rfs}_img.bin
        echo "Building NAND flash image $msg(128KB erase blocksize)..."
    elif [[ $ftype == *"nand2"* ]]; then
        BL_PAD=0x080000
        OS_PAD=0x500000
        ENV_PAD=0x080000
        MEM_PAD=0x140000
        TPL_PAD=0x3C0000
        output=$profile/bcm${profile}_nand_256_${rfs}_img.bin
        echo "Building NAND flash image $msg(256KB erase blocksize)..."
    else
        echo "ERROR: wrong flash type '$ftype' (could be emmc, nand128 or nand256 only"
        exit 1
    fi
fi

if [ ! -f ../bootloaders/build/configs/$env ]; then
    echo "ERROR: No uboot env source file $s exists in bootloaders/build/configs"
    exit 1
fi

#create uboot environment
cp --no-preserve=mode ../bootloaders/build/configs/$env $profile/tmp
if [[ $ftype == *"emmc"* ]]; then
    sed -Ei 's/once=(.*)/once=sdk metadata 1 1;setenv once true;saveenv/' $profile/tmp
else
    echo vf_1_boot=`stat -c %s $pkgtb` >> $profile/tmp
    echo vf_whole_img=true >> $profile/tmp
fi
if [ "$bid" != "" ]; then
    echo boardid=$bid >> $profile/tmp
fi
cat $profile/tmp | ../bootloaders/obj/uboot/tools/mkenvimage --bootmagic -s 16384 -o $profile/env.bin
rm $profile/tmp

if test -f $output; then
    rm $output
fi

fdump $BL_PAD smc/smc_bootl-$chip.prodkey.hsm_signed.flash $BL_MAX
fdump $BL_PAD smc/smc_bootl-$chip.prodkey.hsm_signed.flash $BL_MAX
fdump $OS_PAD smc/smc_os-$chip.bin.prodkey.hsm_signed.flash $OS_MAX

if [ "$output2" != "" ]; then
    echo "Created $output image"
    current=0
    output=$output2
    if test -f $output; then
        rm $output
    fi
fi

fdump $ENV_PAD $profile/env.bin $ENV_MAX
fdump $ENV_PAD $profile/env.bin $ENV_MAX
fdump $MEM_PAD $profile/bcm${profile}_meminit_lun.bin $MEM_MAX 
fdump $TPL_PAD $profile/bcm${profile}_armbl_lun.bin  $TPL_MAX

rm $profile/env.bin

if [[ $ftype == *"emmc"* ]]; then
    if test -f $profile/bcm${profile}_dyn_burn.ini; then
        rm $profile /bcm${profile}_dyn_burn.ini
    fi
    
    create_ini
    ../hostTools/vfbnize $verbose -o $profile/bcm${profile}_dyn_burn.bin -s $DYN_START $profile/bcm${profile}_dyn_burn.ini
    rm $profile/bcm${profile}_dyn_burn.ini
    fdump 0x000000 $profile/bcm${profile}_dyn_burn.bin $DYN_MAX
    rm $profile/bcm${profile}_dyn_burn.bin
else
    fdump 0x000000 $pkgtb $PKG_MAX
fi

echo "Created $output image"
cd - >& /dev/null
