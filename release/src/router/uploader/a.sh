
make clean
make 

rm -rf /tmp/smartsync
rm -rf /tmp/.smartsync

rm -rf /tmp/router


mkdir /tmp/router
mkdir /tmp/router/alarm
mkdir /tmp/router/record
mkdir /tmp/router/event


#rm /tmp/router/mac/router_78DA38AE552A
#rm /tmp/router/system_mac_78DA38AE552A 
mkdir /tmp/router/mac
mkdir /tmp/router/mac/AiCAM_78DA38AE552A
cp system_mac_78DA38AE552A /tmp/router/


#rm /tmp/router/mac/AiCAM_78DA38AE552B
#rm /tmp/router/system_mac_78DA38AE552B 
#mkdir /tmp/router/mac/AiCAM_78DA38AE552B
#cp system_mac_78DA38AE552B /tmp/router/


cp googleToken /tmp/router/
cp input.mp4 /tmp/router/
cp uploader /tmp/router/
cp aaewsProvision /tmp/router/
cp storageProvision /tmp/router/
cp deploy /tmp/router/
cp basicCommand /tmp/router/
cp sysinfo.txt /tmp/router/
cp cam_status.txt /tmp/router/
cp aaewsRun /tmp/router/
cp aicloud /tmp/router/
cp system /tmp/router/
cp lighttpd.user /tmp/
cp 20160102_032720.370EV.txt /tmp/router/event/

/tmp/router/uploader
