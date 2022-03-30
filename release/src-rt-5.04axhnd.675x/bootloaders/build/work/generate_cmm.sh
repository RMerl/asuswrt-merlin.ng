

#objdir=obj/
#ubootobjdir=${objdir}/uboot/
#BRCM_CHIP=63158
#UBOOT_CROSS_COMPILE=/opt/toolchains/crosstools-aarch64-gcc-8.2-linux-4.19-glibc-2.28-binutils-2.31.1/bin/aarch64-linux-

objdir=$1
ubootobjdir=${objdir}/uboot/
dtbdir=${objdir}/binaries/linux/
BRCM_CHIP=$2
UBOOT_CROSS_COMPILE=$3
spl_path=obj/nand_spl/spl/
if [ ! "x$4" = "x" ]; then
spl_path=$4
fi

dpfe_fix_load_offset=0x7000

echo "Generating spl jtag ddr init cmm files"
cmm_out=${objdir}/cmm/${BRCM_CHIP}
mkdir -p ${cmm_out}
cp ${spl_path}/u-boot-spl ${cmm_out}/u-boot-spl
cp ${spl_path}/u-boot-spl.bin ${cmm_out}/u-boot-spl.bin
ddr_mode=bcmbca_ddr
register=r9

CONFIG_ARM64=`grep CONFIG_ARM64= ${ubootobjdir}/.config   |sed -e 's/.*=//'`;

if [ "x${CONFIG_ARM64}" == "xy" ]; then
	register=x9
	cp build/support/jtag/misc/armv8_disable_mmu.bin ${cmm_out}/.
fi


for i in `ls ${objdir}|grep DDR`;
do 

	CONFIG_BCMBCA_DDR_LOADADDR=`grep CONFIG_BCMBCA_DDR_LOADADDR ${ubootobjdir}/.config   |sed -e 's/.*=//'`;
	CONFIG_SPL_TEXT_BASE=`grep CONFIG_SPL_TEXT_BASE ${ubootobjdir}/.config   |sed -e 's/.*=//'`;
	CONFIG_BCMBCA_DPFE=`grep CONFIG_BCMBCA_DPFE ${spl_path}/../.config   |sed -e 's/.*=//'`;
	CONFIG_BOOT_BLOB_JTAG_LOAD_MAX_DDR_SIZE=`grep CONFIG_BOOT_BLOB_JTAG_LOAD_MAX_DDR_SIZE ${spl_path}/../.config   |sed -e 's/.*=//'`;

	mcb_offset=${CONFIG_BOOT_BLOB_JTAG_LOAD_MAX_DDR_SIZE};

	if [ "x${CONFIG_BCMBCA_DPFE}" == "xy" ]; then 
		ddr_mode=bcmbca_dpfe
	fi
	cp ${objdir}/$i/arch/arm/mach-bcmbca/${ddr_mode}/bcm_ddr.bin  ${cmm_out}/${i}_bcm_ddr.bin
	#build/work/generate_hashes generates all the jtag*${BRCM_CHIP}*${i}* in the following format
	#mcb selector (4byts) + padding (4 bytes) + MCB binary associated with the mcb selector
	for j in `ls ${objdir}/binaries/jtag*${i}* 2> /dev/null`;
	do
		mcb_sel=`od -A n -t x -N 4 ${j}|sed -e 's/^ //'`;

		fileout=${cmm_out}/${BRCM_CHIP}_${i}_${mcb_sel}
		cmm_file=${fileout}.cmm
		gdb_file=${fileout}.gdb
		fsize=`stat -c %s ${j}`;
		let addr=${CONFIG_BCMBCA_DDR_LOADADDR}+${mcb_offset};
		hex_addr=$(printf "%X" ${addr})
		echo d.load.elf ~~~~/u-boot-spl /nocode > ${cmm_file}
		echo d.load.binary ~~~~/u-boot-spl.bin ${CONFIG_SPL_TEXT_BASE} /noclear >> ${cmm_file}
		echo B::R.S R9 0xd0deed >> ${cmm_file}
		echo B::R.S PC ${CONFIG_SPL_TEXT_BASE} >> ${cmm_file}
		echo break spl_ddrinit >> ${cmm_file}
		echo go >> ${cmm_file}
		echo "wait !run()" >> ${cmm_file}
		echo d.load.binary ~~~~/${j##*/} ${hex_addr} /noclear >> ${cmm_file}
		echo d.load.binary ~~~~/${i}_bcm_ddr.bin ${CONFIG_BCMBCA_DDR_LOADADDR} /noclear >> ${cmm_file}
		echo break.set jtag_spl_done >> ${cmm_file}


		echo file u-boot-spl  > ${gdb_file}
		echo restore u-boot-spl.bin binary ${CONFIG_SPL_TEXT_BASE} >> ${gdb_file}
		echo set \$${register}=0xd0deed >> ${gdb_file}
		echo set \$pc=${CONFIG_SPL_TEXT_BASE} >> ${gdb_file}
		echo break spl_ddrinit >> ${gdb_file}
		echo c >> ${gdb_file}
		echo restore ${j##*/} binary 0x${hex_addr} >> ${gdb_file}
		echo restore ${i}_bcm_ddr.bin binary ${CONFIG_BCMBCA_DDR_LOADADDR}  >> ${gdb_file}
		echo break spl_ddrinit:jtag_spl_done >> ${gdb_file}

		if [ "x${CONFIG_BCMBCA_DPFE}" == "xy" ]; then 
			#make a copy of base cmm so we can extend it to load  dpfe
			dpfe_cmm=${cmm_out}/dpfe_${BRCM_CHIP}_${i}_${mcb_sel}.cmm
			dpfe_gdb=${cmm_out}/dpfe_${BRCM_CHIP}_${i}_${mcb_sel}.gdb
			cp -rf ${cmm_file} ${dpfe_cmm}
			cp -rf ${gdb_file} ${dpfe_gdb}
		fi

		echo go >> ${cmm_file}
		echo c >> ${gdb_file}
		cp ${j}  ${cmm_out}/.


		if [ "x${CONFIG_BCMBCA_DPFE}" == "xy" ]; then 
			# there is no dpfe for ddr3
			for file in `ls build/work/dpfe/${BRCM_CHIP}/dpfe_${BRCM_CHIP}_* |grep -i ${i} 2> /dev/null` 
			do
				chmod +w ${cmm_out}/${file##*/}	2> /dev/null
				#copy over the dpfe stage file to cmm folder
				cp ${file} ${cmm_out}/${file##*/}
				# enable a breakpoint at the stub function that replaces real dpfe segment loader
				echo break load_dpfe_segment_stub >> ${dpfe_cmm}
				echo go >> ${dpfe_cmm}
				echo "wait !run()" >> ${dpfe_cmm}
				#when the t32 stops at load_dpfe_segment_stub, load the next dpfe stage
				echo d.load.binary ~~~~/${file##*/} ${CONFIG_BCMBCA_DDR_LOADADDR}+${dpfe_fix_load_offset} /noclear >> ${dpfe_cmm}


				# enable a breakpoint at the stub function that replaces real dpfe segment loader
				echo break load_dpfe_segment_stub >> ${dpfe_gdb}
				echo c >> ${dpfe_gdb}
				#when the t32 stops at load_dpfe_segment_stub, load the next dpfe stage
				echo restore ${file##*/} binary ${CONFIG_BCMBCA_DDR_LOADADDR}+${dpfe_fix_load_offset} >> ${dpfe_gdb}

				
			done
			echo break load_dpfe_segment_stub >> ${dpfe_cmm}
			# when we reach the last stage, set the last_stage flag to 1 so the ddr dpfe library knows
			echo step  >> ${dpfe_cmm}
			echo r.s r0 0x01 >> ${dpfe_cmm}
			echo go >> ${dpfe_cmm}

			# when we reach the last stage, set the last_stage flag to 1 so the ddr dpfe library knows
			echo step  >> ${dpfe_gdb}
			echo set \$r0=0x01 >> ${dpfe_gdb}
			echo step  >> ${dpfe_gdb}
			echo set \$r0=0x01 >> ${dpfe_gdb}
			echo c >> ${dpfe_gdb}
		fi

	done;
done;
for i in ${dtbdir}/*.dtb
do
    cat ${ubootobjdir}/u-boot-nodtb.bin $i > ${cmm_out}/u-boot-`basename $i .dtb`.bin
    mcb=`obj/uboot/scripts/dtc/dtc  -O dts $i | grep memcfg | perl -ne '$_=~m/^.*<(.*)>.*$/; printf("%08x",hex($1));'`

    dpfe_prefix=
    if [ "x${CONFIG_BCMBCA_DPFE}" == "xy" ];then
        dpfe_prefix=dpfe_
    fi
    ddrfile=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${mcb}.cmm 2> /dev/null`
    ddrgdb=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${mcb}.gdb 2> /dev/null` 
    if [ "x${ddrfile}" == "x" ]; then
	m=$(printf "%d" 0x${mcb})
        let m=${m}+0x20000
	m=$(printf "%x" ${m})
        ddrfile=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m}.cmm 2> /dev/null`
        ddrgdb=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m}.gdb 2> /dev/null` 
    fi
    if [ "x${ddrfile}" == "x" ]; then
	m_temp=`echo ${mcb}|sed -e 's/^4//'`
        ddrfile=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m_temp}.cmm 2> /dev/null`
        ddrgdb=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m_temp}.gdb 2> /dev/null`
    fi 
    if [ "x${ddrfile}" == "x" ]; then
	m_temp=`echo ${mcb}|sed -e 's/^4//'`
	m=$(printf "%d" 0x${m_temp})
        let m=${m}+0x20000
	m=$(printf "%x" ${m})
        ddrfile=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m}.cmm 2> /dev/null`
        ddrgdb=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m}.gdb 2> /dev/null` 
    fi
    if [ "x${ddrfile}" == "x" ]; then
	m_temp=`echo ${mcb}|sed -e 's/^7/3/'`
	m=$(printf "%d" 0x${m_temp})
        let m=${m}+0x20000
	m=$(printf "%x" ${m})
        ddrfile=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m}.cmm 2> /dev/null`
        ddrgdb=`cd ${cmm_out} && ls ${dpfe_prefix}${BRCM_CHIP}_DDR*${m}.gdb 2> /dev/null` 
    fi
    if [ "x${ddrfile}" == "x" ]; then
        echo "[$i] Missing MCB (kernel dts entry) binary "  $mcb
    fi
    boardcmm=${cmm_out}/`basename $i .dtb`.cmm
    boardgdb=${cmm_out}/`basename $i .dtb`.gdb
    boardbin=u-boot-`basename $i .dtb`.bin
    if [ -f "${cmm_out}/${ddrfile}" ]
    then
        echo 'do attach.cmm' > ${boardcmm}
        echo 'BREAK' >> ${boardcmm}
        echo "do ${ddrfile}" >> ${boardcmm}
	echo "wait !run()" >> ${boardcmm}
        echo "data.load.binary ${boardbin} 0x1000000" >> ${boardcmm}
        echo 'r.s pc 0x1000000' >> ${boardcmm}
        echo 'go' >> ${boardcmm}
    fi

    if [ -f "${cmm_out}/${ddrgdb}" ]
    then
	cat <<- ENDENDEND > ${boardgdb}
		source ${ddrgdb}
		restore ${boardbin} binary 0x1000000
		set \$pc=0x1000000
		c
		ENDENDEND
    fi

done
cp ${ubootobjdir}/u-boot-dtb.bin ${cmm_out} # should not be needed

cp build/support/jtag/${BRCM_CHIP}/* ${cmm_out}

rm .temp.bin 2> /dev/null;

exit 0 

