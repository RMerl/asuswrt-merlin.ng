#!/bin/bash

# We need to down eth0 prior to starting runner's data path init
ifconfig eth0 down &> /dev/null

# Initialize bdmf shell
bdmf_shell -c init | while read a b ; do echo $b ; done > /var/bdmf_sh_id
alias bs="bdmf_shell -c `cat /var/bdmf_sh_id` -cmd "
