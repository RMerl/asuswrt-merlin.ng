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

echo ">>>>> Starting bcm_pcie_hcd <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcm_pcie_hcd.ko
echo ">>>>> Starting bcmlibs <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcmlibs.ko
echo ">>>>> Starting bdmf <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bdmf.ko 
echo ">>>>> Starting bpm <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcm_bpm.ko

echo ">>>>> Starting rdpa_gpl <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/rdpa_gpl.ko
#echo ">>>>> Starting rdpa_gpl_ext <<<<<"
#insmod /lib/modules/LINUX_VER_STR/extra/rdpa_gpl_ext.ko
echo ">>>>> Starting rdpa <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/rdpa.ko 
echo ">>>>> Starting rdpa_usr <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/rdpa_usr.ko
echo ">>>>> Starting rdpa_mw <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/rdpa_mw.ko
echo ">>>>> Starting bcm_ingqos <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcm_ingqos.ko
echo ">>>>> Starting rdpa_cmd <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/rdpa_cmd.ko

echo ">>>>> Starting flow cache <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/pktflow.ko
echo ">>>>> Starting runner cmdlist <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/cmdlist.ko
echo ">>>>> Starting pktrunner <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/pktrunner.ko
echo ">>>>> Starting bcmxtmrt <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcmxtmrtdrv.ko
echo ">>>>> Starting bcmxtmcfg <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcmxtmcfg.ko
echo ">>>>> Starting bcm_enet <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcm_enet.ko
echo ">>>>> Starting bcm_vlan <<<<<"
insmod /lib/modules/LINUX_VER_STR/extra/bcmvlan.ko

echo ">>>>> INSMOD done <<<<<"

/etc/rdpa_init.sh

echo "Bring up the ethernet ports..."
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ bs /b/e port"
bs /b/e port
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ bs /b/e egress_tm"
bs /b/e egress_tm
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ bs /b/e system"
bs /b/e system
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ bs /b/e cpu"
bs /b/e cpu

echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ifconfig eth0, eth1, eth2 up"
ifconfig eth0 up
ifconfig eth1 up
ifconfig eth2 up
ifconfig


echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ brctl br0 with eth2, eth1"
brctl addbr br0
brctl addif br0 eth2
brctl addif br0 eth1
ifconfig br0 up
brctl show
ifconfig br0 192.168.1.1


echo 1 > /proc/sys/net/ipv4/ip_forward
cat /proc/sys/net/ipv4/ip_forward

# --------------------------------------------------------------------------------
# UNCOMMENT BELOW TO AUTOMATE Packet Send and Flow Configuration and Config Prints
# --------------------------------------------------------------------------------

# echo "Start Emulator Packet Data..."

# NOTE:  For data without RTL Wave Forms use qdata
# bs /d/qemu qdata

# NOTE:  For data WITH RTL Wave Forms use qdataw
# bs /d/qemu qdataw








