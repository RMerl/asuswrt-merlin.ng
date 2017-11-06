#!/bin/bash

ssh-add

if [ "$1" == "beta" ]
then
	SUFFIXE="Beta/"
	SUFFIXESF="Beta/"
else
	SUFFIXE=""
	SUFFIXESF="Release/"
fi

cd /media/sf_Share/images/

MODELS=(RT-N66U RT-AC66U RT-AC56U RT-AC68U RT-AC87U RT-AC3200 RT-AC88U RT-AC3100 RT-AC5300 RT-AC86U)

for MODEL in "${MODELS[@]}"
do
   :
   echo "Deploying $MODEL to Onedrive..."
   cp $MODEL*.zip /media/nas/Onedrive/Asuswrt-Merlin/Releases/$MODEL/$SUFFIXE
   echo "Deploying $MODEL to Sourceforge..."
   scp $MODEL*.zip rmerlin@frs.sourceforge.net:/home/pfs/project/asuswrt-merlin/$MODEL/$SUFFIXESF
done

echo "Done deploying!"
