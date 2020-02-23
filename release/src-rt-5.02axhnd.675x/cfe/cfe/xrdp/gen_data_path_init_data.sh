#!/bin/bash

if [[ $# != 2 ]]; then
   echo $0 console_output_file_name data_path_init_data_file_name
   echo example:
   echo $0 cfe_console_output_file.txt devel/rdp/drivers/rdp_subsystem/BCM6858/data_path_init_basic_data.h
   exit -1
fi
dos2unix $1
grep '^>>>' $1 | awk '{print "{", $2, ",", $5, "," $3, ",", $4, "}," }' > $2

