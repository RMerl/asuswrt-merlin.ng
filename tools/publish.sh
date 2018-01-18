#!/bin/bash

if [ "$1" == "beta" ]
then
	SUFFIXE="Beta/"
	SUFFIXESF="Beta/"
else
	SUFFIXE=""
	SUFFIXESF="Release/"
fi

read -p "Deploy to $SUFFIXESF (y/n)?" choice
case "$choice" in
  y|Y ) echo "Deploying to $SUFFIXESF";;
  * ) echo "Aborting."; exit;;
esac

eval $(ssh-agent -s)
ssh-add

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

echo "Uploading documentation..."
cp README-merlin.txt /media/nas/Onedrive/Asuswrt-Merlin/Documentation/
scp README-merlin.txt rmerlin@frs.sourceforge.net:/home/pfs/project/asuswrt-merlin/Documentation/

cp Changelog*.txt /media/nas/Onedrive/Asuswrt-Merlin/Documentation/
scp Changelog*.txt rmerlin@frs.sourceforge.net:/home/pfs/project/asuswrt-merlin/Documentation/

# Only upload SHA256 checksums for non-beta releases
if [ "$SUFFIXE" == "Beta/" ]
then
   mv sha256sums.txt sha256sums-beta.txt
   mv sha256sums-ng.txt sha256sums-ng-beta.txt
fi

cp sha256sums*.txt /media/nas/Onedrive/Asuswrt-Merlin/Documentation/
scp sha256sums*.txt rmerlin@frs.sourceforge.net:/home/pfs/project/asuswrt-merlin/Documentation/

echo "Done deploying!"

eval $(ssh-agent -k)

