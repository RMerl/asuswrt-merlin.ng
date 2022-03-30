#!/bin/sh
echo "Mounting filesystems..."
/bin/mount -a

# Mount /dev/pts as devpts
mkdir /dev/pts
mount -t devpts devpts /dev/pts

# Initialize and run mdev to crete dynamic device nodes
echo ">>>>> Starting mdev <<<<<"
/sbin/mdev -s

# Create static device nodes
/etc/make_static_devnodes.sh
mknod /var/fuse c 10 229
chmod a+rw /var/fuse
mkdir -p /var/log /var/run /var/state/dhcp /var/ppp /var/udhcpd /var/zebra /var/siproxd /var/cache /var/tmp /var/samba /var/samba/share /var/samba/homes /var/samba/private /var/samba/locks
ln -s /var/log/log /dev/log


echo ">>>>> Starting bcmlibs <<<<<"
insmod /lib/modules/4.19.151/extra/bcmlibs.ko
echo ">>>>> Starting bdmf <<<<<"
insmod /lib/modules/4.19.151/extra/bdmf.ko 

echo ">>>>> Starting rdpa_gpl <<<<<"
insmod /lib/modules/4.19.134/extra/rdpa_gpl.ko
echo ">>>>> Starting rdpa_gpl_ext <<<<<"
insmod /lib/modules/4.19.134/extra/rdpa_gpl_ext.ko
echo ">>>>> Starting rdpa <<<<<"
insmod /lib/modules/4.19.134/extra/rdpa.ko 
echo ">>>>> Starting rdpa_usr <<<<<"
insmod /lib/modules/4.19.134/extra/rdpa_usr.ko
echo ">>>>> Starting rdpa_mw <<<<<"
insmod /lib/modules/4.19.134/extra/rdpa_mw.ko
echo ">>>>> Starting rdpa_cmd <<<<<"
insmod /lib/modules/4.19.134/extra/rdpa_cmd.ko
echo ">>>>> Starting pktflow <<<<<"
insmod /lib/modules/4.19.134/extra/pktflow.ko 
echo ">>>>> Starting cmdlist <<<<<"
insmod /lib/modules/4.19.134/extra/cmdlist.ko
echo ">>>>> Starting pktrunner.ko <<<<<"
insmod /lib/modules/4.19.134/extra/pktrunner.ko
echo ">>>>> Starting bcm_enet <<<<<"
insmod /lib/modules/4.19.134/extra/bcm_enet.ko

echo ">>>>> INSMOD done <<<<<"

/etc/rdpa_init.sh


#bs /b/new ip_class 
#bs /bdmf/configure ip_class key_type=six_tuple
#bs /bdmf/configure ip_class routed_mac[0]={00:00:00:00:0a:00}
#bs /bdmf/configure ip_class routed_mac[1]={00:00:00:00:0b:00}
#bs /bdmf/attr/add ip_class flow string {key={src_ip=10.0.0.1,dst_ip=10.0.0.2,prot=17,src_port=100,dst_port=200,dir=us,ingress_if=lan0},result={qos_method=flow,action=forward,trap_reason=conn_trap0,dscp_value=35,nat_ip=136.38.102.215,port=wan0,queue_id=0,wan_flow=0,opbit_action=outer_copy,ipbit_action=inner_copy,l2_offset=-16,l2_head_size=30,action_vec=nat+ttl+dscp+opbit,l2_header=01020305080d00000c1122ab810000aa810000bb88641100988ea75a0021}}
#bs /bdmf/attr/add ip_class flow string key={dir=ds,src_ip=10.0.0.1,dst_ip=10.0.0.2,prot=17,src_port=100,dst_port=200,ingress_if=wan0},result={qos_method=flow,action=forward,trap_reason=conn_trap0,dscp_value=0,port=lan0,queue_id=0,ovid_offset=offset_12,l2_offset=0,l2_head_size=18,l2_num_tags=1,action_vec=0,l2_header=00000000000a00000000000b810004000800,wl_metadata=0x0,service_queue_id=disable,drop_eligibility=0}
echo ">>>FINISH CONFIGURE IP CLASS<<<"





