#!/bin/bash
#This script gives 3 information
#1. Gives list of symbomls from 'romctl.txt' which are not excluded from ROM
#2. Gives list of files from 'romctl.txt' which are not excluded from ROM
#3. IOVAR modules list
#3.a Gives list of IOVAR's which are present in rom but not enabled for iovar patching
#3.b Gives list of IOVAR's which are not added to rom

echo ""; echo "------------ Checking for ISSUES in ROMLIB -------------"

# INITIALIZATIONS
export CHIP=$1
export PWD=$(dirname $0)
export LPATH=${PWD}/../../build/dongle/romlsym/${CHIP}/$2/ROM_ISSUES
export RPATH=${PWD}/../chips/images/roml/${CHIP}
export RESULT_FILE=${LPATH}/../rom_issues.txt

if [ -d "${LPATH}" ]; then rm -rf ${LPATH}; fi;
mkdir -p ${LPATH}
cp ${RPATH}/roml.map ${LPATH}/roml_cp.map

# REMOVE ram symbols from roml.map by seraching __ramfnptr
grep __ramfnptr ${RPATH}/roml.map > ${LPATH}/roml_ramfn
cat ${LPATH}/roml_ramfn | sed 's/^.*ramfnptr_//g' > ${LPATH}/roml_ramfn_temp; mv ${LPATH}/roml_ramfn_temp ${LPATH}/roml_ramfn
for i in `cat ${LPATH}/roml_ramfn`; do cat ${LPATH}/roml_cp.map | grep -v $i > ${LPATH}/roml_cp_tmp.map; mv ${LPATH}/roml_cp_tmp.map ${LPATH}/roml_cp.map; done

#1. Find symbols which are not excluded in ROM from romctl
cat ${RPATH}/romctl.txt | sed '/^\s*#/d;/^\s*$/d' > ${LPATH}/romctl.txt
grep exclude_sym ${LPATH}/romctl.txt | awk '{ print $1 }' > ${LPATH}/romctl_syms.txt
cat ${LPATH}/romctl_syms.txt | sed 's/^.*ramfnptr_//g' > ${LPATH}/romctl_syms_temp.txt; mv ${LPATH}/romctl_syms_temp.txt ${LPATH}/romctl_syms.txt
for i in `cat ${LPATH}/romctl_syms.txt`; do echo $i | awk -F'$' '{ if ($2)  print $2; else print $1}' >> ${LPATH}/roml_syms_temp.txt; done;
mv ${LPATH}/roml_syms_temp.txt ${LPATH}/romctl_syms.txt
echo "1. SYMBOLS NOT EXCLUDED FROM ROM" > ${RESULT_FILE}; echo "================================" >> ${RESULT_FILE};
for i in `cat ${LPATH}/romctl_syms.txt` ; do if [ `grep -w $i ${LPATH}/roml_cp.map | wc -l ` -ne 0 ]; then echo $i >> ${RESULT_FILE}; fi;done

#2. Find files which are not excluded in ROM from romctl
grep exclude_file ${RPATH}/romctl.txt | awk '{ print $1 }' > ${LPATH}/romctl_file.txt
sed 's/\.o/\.c/' ${LPATH}/romctl_file.txt > ${LPATH}/romctl_file_c.txt
echo "" >> ${RESULT_FILE}; echo "2. FILES NOT EXCLUDED FROM ROMMING" >> ${RESULT_FILE}; echo "==================================" >> ${RESULT_FILE};
for i in `cat ${LPATH}/romctl_file_c.txt`; do if [ `find . -name $i | wc -l ` -ne 0 ]; then echo "$i" >> ${RESULT_FILE}; fi; done

#3.a. Find IOVAR which are not enabled for patching
# Get iovar list from roml.map
cat ${RPATH}/roml.map | grep doiovar$ | sort > ${LPATH}/rom_iovars.txt
cat  ${LPATH}/rom_iovars.txt | awk '{ print $3 }' > ${LPATH}/rom_iovars_temp.txt; mv ${LPATH}/rom_iovars_temp.txt ${LPATH}/rom_iovars.txt

# GET IOVAR LIST MEANT FOR PATCHING FROM CFG FILE
grep iovar ${RPATH}/autopatch_roml.cfg | awk ' {  print $3 } ' | sort > ${LPATH}/romcfg_iovars.txt

# Remove file names and '$' symbols
for i in `cat ${LPATH}/roml_ramfn`; do echo $i | awk -F'$' '{ if ($2)  print $2; else print $1}' >> ${LPATH}/roml_ramfn_temp; done;
mv ${LPATH}/roml_ramfn_temp ${LPATH}/roml_ramfn

for i in `cat ${LPATH}/romcfg_iovars.txt`; do echo $i | awk -F'$' '{ if ($2)  print $2; else print $1}' >> ${LPATH}/romcfg_iovars_tmp.txt; done;
mv ${LPATH}/romcfg_iovars_tmp.txt ${LPATH}/romcfg_iovars.txt

for i in `cat ${LPATH}/rom_iovars.txt`; do echo $i | awk -F'$' '{ if ($2)  print $2; else print $1}' >> ${LPATH}/rom_iovars_tmp.txt; done;
mv ${LPATH}/rom_iovars_tmp.txt ${LPATH}/rom_iovars.txt

# Find modules not enabled for iovar patching
for i in `cat ${LPATH}/romcfg_iovars.txt`;
	do
		if [ `grep $i ${LPATH}/rom_iovars.txt | wc -l ` -ne 0 ];
			then cat ${LPATH}/romcfg_iovars.txt | grep -v $i > ${LPATH}/romcfg_iovars_temp.txt;
			mv ${LPATH}/romcfg_iovars_temp.txt ${LPATH}/romcfg_iovars.txt;
		fi;
		cat ${LPATH}/rom_iovars.txt | grep -w -v $i > ${LPATH}/rom_iovars_tmp.txt;
		mv ${LPATH}/rom_iovars_tmp.txt ${LPATH}/rom_iovars.txt;
	done

# REMOVE blank lines from files
sed '/^$/d' ${LPATH}/romcfg_iovars.txt > ${LPATH}/romcfg_iovars_tmp.txt;mv ${LPATH}/romcfg_iovars_tmp.txt ${LPATH}/romcfg_iovars.txt
sed '/^$/d' ${LPATH}/rom_iovars.txt > ${LPATH}/rom_iovars_tmp.txt;mv ${LPATH}/rom_iovars_tmp.txt ${LPATH}/rom_iovars.txt
sed '/^$/d' ${LPATH}/roml_ramfn > ${LPATH}/roml_ramfn_temp; mv ${LPATH}/roml_ramfn_temp ${LPATH}/roml_ramfn

# Make a list of IOVAR MODULES which are not enabled for iovar patching and which are not in ROMLIB at all
echo "" >> ${RESULT_FILE}; echo "3 IOVARS NOT ENABLED FOR PATCHING" >> ${RESULT_FILE}; echo "==================================" >> ${RESULT_FILE};
cat ${LPATH}/rom_iovars.txt >>  ${RESULT_FILE}

#Dump list of ROM issues to console
cat ${RESULT_FILE}

# Remove Local files
rm -rf ${LPATH}
