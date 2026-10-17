# SPDX-License-Identifier: GPL-2.0+

dtb-$(CONFIG_TARGET_KMCENT2) += kmcent2.dtb
dtb-$(CONFIG_TARGET_KMCOGE5NE) += kmcoge5ne.dtb
dtb-$(CONFIG_TARGET_KMETER1) += kmeter1.dtb
dtb-$(CONFIG_TARGET_KMOPTI2) += kmopti2.dtb
dtb-$(CONFIG_TARGET_KMSUPX5) += kmsupc5.dtb kmsupm5.dtb
dtb-$(CONFIG_TARGET_KMTEGR1) += kmtegr1.dtb
dtb-$(CONFIG_TARGET_KMTEPR2) += kmtepr2.dtb
dtb-$(CONFIG_TARGET_MPC837XERDB) += mpc8379erdb.dtb
dtb-$(CONFIG_TARGET_MPC8548CDS) += mpc8548cds.dtb mpc8548cds_36b.dtb
dtb-$(CONFIG_TARGET_P1010RDB_PA) += p1010rdb-pa.dtb p1010rdb-pa_36b.dtb
dtb-$(CONFIG_TARGET_P1010RDB_PB) += p1010rdb-pb.dtb p1010rdb-pb_36b.dtb
dtb-$(CONFIG_TARGET_P1020RDB_PC) += p1020rdb-pc.dtb p1020rdb-pc_36b.dtb
dtb-$(CONFIG_TARGET_P1020RDB_PD) += p1020rdb-pd.dtb
dtb-$(CONFIG_TARGET_P2020RDB) += p2020rdb-pc.dtb p2020rdb-pc_36b.dtb
dtb-$(CONFIG_TARGET_P2041RDB) += p2041rdb.dtb
dtb-$(CONFIG_TARGET_P3041DS) += p3041ds.dtb
dtb-$(CONFIG_TARGET_P4080DS) += p4080ds.dtb
dtb-$(CONFIG_TARGET_P5040DS) += p5040ds.dtb
dtb-$(CONFIG_TARGET_QEMU_PPCE500) += qemu-ppce500.dtb
dtb-$(CONFIG_TARGET_SOCRATES) += socrates.dtb
dtb-$(CONFIG_TARGET_T1024RDB) += t1024rdb.dtb
dtb-$(CONFIG_TARGET_T1042D4RDB) += t1042d4rdb.dtb
dtb-$(CONFIG_TARGET_T2080QDS) += t2080qds.dtb
dtb-$(CONFIG_TARGET_T2080RDB) += t2080rdb.dtb
dtb-$(CONFIG_TARGET_T4240RDB) += t4240rdb.dtb
dtb-$(CONFIG_TARGET_TUGE1) += kmtuge1.dtb
dtb-$(CONFIG_TARGET_TUXX1) += kmtuxa1.dtb
dtb-$(CONFIG_TARGET_MCR3000) += mcr3000.dtb
dtb-$(CONFIG_TARGET_GAZERBEAM) += gazerbeam.dtb
dtb-$(CONFIG_TARGET_CMPC885) += cmpc885.dtb
dtb-$(CONFIG_TARGET_CMPCPRO) += cmpcpro.dtb

include $(srctree)/scripts/Makefile.dts

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
