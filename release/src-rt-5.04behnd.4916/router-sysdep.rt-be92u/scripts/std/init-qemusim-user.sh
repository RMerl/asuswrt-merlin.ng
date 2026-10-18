#!/bin/sh
bs /d/rdd/cso off # cso : not supported by simulator
if [ -f /data/init-qemusim-user.sh ] ; then
  echo exec /data/init-qemusim-user.sh
  /data/init-qemusim-user.sh
fi
