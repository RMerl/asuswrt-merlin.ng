#! /bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016, Lukasz Majewski <l.majewski@majess.pl>
#

# This file extracts default envs from built u-boot
# usage: get_default_envs.sh [build dir] > u-boot-env-default.txt
set -ue

: "${OBJCOPY:=${CROSS_COMPILE:-}objcopy}"

ENV_OBJ_FILE="built-in.o"
ENV_OBJ_FILE_COPY="copy_${ENV_OBJ_FILE}"

echoerr() { echo "$@" 1>&2; }

if [ "$#" -eq 1 ]; then
    path=${1}
else
    path=$(readlink -f $0)
    path=${path%/scripts*}
fi

env_obj_file_path=$(find ${path} -path "*/env/*" -not -path "*/spl/*" \
			 -not -path "*/tools/*" -name "${ENV_OBJ_FILE}")
[ -z "${env_obj_file_path}" ] && \
    { echoerr "File '${ENV_OBJ_FILE}' not found!"; exit 1; }

cp ${env_obj_file_path} ${ENV_OBJ_FILE_COPY}

# NOTE: objcopy saves its output to file passed in
# (copy_${ENV_OBJ_FILE} in this case)

${OBJCOPY} -O binary -j ".rodata.default_environment" ${ENV_OBJ_FILE_COPY}

# Replace default '\0' with '\n' and sort entries
tr '\0' '\n' < ${ENV_OBJ_FILE_COPY} | sort -u

rm ${ENV_OBJ_FILE_COPY}

exit 0
