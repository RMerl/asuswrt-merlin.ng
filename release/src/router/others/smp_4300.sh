#!/bin/sh

#=============================================================================
# smp_affinity: 1 = CPU1, 2 = CPU2, 3 = CPU3, 4 = CPU4
# rps_cpus: wxyz = CPU3 CPU2 CPU1 CPU0 (ex:0xd = 0'b1101 = CPU1, CPU3, CPU4)
#=============================================================================

if [ ! -n "$1" ]; then
  echo "insufficient arguments!"
  echo "Usage: $0 <mode>"
  exit 0
fi

OPTIMIZED_FOR="$1"
LIST=`cat /proc/interrupts | sed -n '1p'`
NUM_OF_CPU=0; for i in $LIST; do NUM_OF_CPU=`expr $NUM_OF_CPU + 1`; done;

if [ $OPTIMIZED_FOR == "wifi" ]; then

	if [ $NUM_OF_CPU == "4" ]; then
		echo 2 > /proc/irq/3/smp_affinity  #GMAC
		echo 4 > /proc/irq/4/smp_affinity  #PCIe0
		echo 8 > /proc/irq/24/smp_affinity #PCIe1
		echo 8 > /proc/irq/25/smp_affinity #PCIe2
		echo 8 > /proc/irq/19/smp_affinity #VPN
		echo 8 > /proc/irq/20/smp_affinity #SDXC
		echo 8 > /proc/irq/22/smp_affinity #USB

		echo 3 > /sys/class/net/ra0/queues/rx-0/rps_cpus
		echo 3 > /sys/class/net/rai0/queues/rx-0/rps_cpus
		if [ -d "/sys/class/net/rai0" ]; then
			echo 5 > /sys/class/net/eth2/queues/rx-0/rps_cpus
			echo 5 > /sys/class/net/eth3/queues/rx-0/rps_cpus
			echo "eth2/eth3 RPS: CPU0/2"
		else
			echo 3 > /sys/class/net/eth2/queues/rx-0/rps_cpus
			echo 3 > /sys/class/net/eth3/queues/rx-0/rps_cpus
			echo "eth2/eth3 RPS: CPU0/1"
		fi
	elif [ $NUM_OF_CPU == "2" ]; then
		echo 2 > /proc/irq/3/smp_affinity  #GMAC
		echo 1 > /proc/irq/4/smp_affinity  #PCIe0
		echo 2 > /proc/irq/24/smp_affinity #PCIe1
		echo 2 > /proc/irq/25/smp_affinity #PCIe2
		echo 1 > /proc/irq/19/smp_affinity #VPN
		echo 1 > /proc/irq/20/smp_affinity #SDXC
		echo 1 > /proc/irq/22/smp_affinity #USB

		echo 2 > /sys/class/net/ra0/queues/rx-0/rps_cpus
		echo 1 > /sys/class/net/rai0/queues/rx-0/rps_cpus
		echo 2 > /sys/class/net/eth2/queues/rx-0/rps_cpus
		echo 2 > /sys/class/net/eth3/queues/rx-0/rps_cpus

	fi

elif [ $OPTIMIZED_FOR == "storage" ]; then

	if [ $NUM_OF_CPU == "4" ]; then
		echo 2 > /proc/irq/3/smp_affinity  #GMAC
		echo 4 > /proc/irq/4/smp_affinity  #PCIe0
		echo 4 > /proc/irq/24/smp_affinity #PCIe1
		echo 8 > /proc/irq/25/smp_affinity #PCIe2
		echo 8 > /proc/irq/19/smp_affinity #VPN
		echo 8 > /proc/irq/20/smp_affinity #SDXC
		echo 8 > /proc/irq/22/smp_affinity #USB

		echo f > /sys/class/net/ra0/queues/rx-0/rps_cpus
		echo f > /sys/class/net/rai0/queues/rx-0/rps_cpus
		echo c > /sys/class/net/eth2/queues/rx-0/rps_cpus
		echo c > /sys/class/net/eth3/queues/rx-0/rps_cpus
	elif [ $NUM_OF_CPU == "2" ]; then
		echo 1 > /proc/irq/3/smp_affinity  #GMAC
		echo 1 > /proc/irq/4/smp_affinity  #PCIe0
		echo 1 > /proc/irq/24/smp_affinity #PCIe1
		echo 1 > /proc/irq/25/smp_affinity #PCIe2
		echo 1 > /proc/irq/19/smp_affinity #VPN
		echo 2 > /proc/irq/20/smp_affinity #SDXC
		echo 2 > /proc/irq/22/smp_affinity #USB

		echo 1 > /sys/class/net/ra0/queues/rx-0/rps_cpus
		echo 1 > /sys/class/net/rai0/queues/rx-0/rps_cpus
		echo 1 > /sys/class/net/eth2/queues/rx-0/rps_cpus
		echo 1 > /sys/class/net/eth3/queues/rx-0/rps_cpus

	fi

else

	echo "unknow arguments!"
	exit 0

fi
