#!/bin/bash

if [[ $# != 2 ]]; then
   echo "$0 console_output_file_name data_path_<chip_id>.c"
   exit -1
fi
dos2unix $1
cat <<EOT > $2
/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include "access_logging.h"

const access_log_tuple_t init_data[] = {
EOT

grep '^>>>' $1 | awk '{print "\t{", $2, ",", $3,"},"}' >> $2

cat <<EOT >> $2
    { (ACCESS_LOG_OP_STOP << 24), 0 }
};
EOT
