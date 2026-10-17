/*
 * This header provides constants for binding nvidia,tegra186-hsp.
 *
 * The number with HSP_DB_MASTER prefix indicates the bit that is
 * associated with a master ID in the doorbell registers.
 */

#ifndef _DT_BINDINGS_MAILBOX_TEGRA186_HSP_H
#define _DT_BINDINGS_MAILBOX_TEGRA186_HSP_H

#define HSP_MBOX_TYPE_DB 0x0
#define HSP_MBOX_TYPE_SM 0x1
#define HSP_MBOX_TYPE_SS 0x2
#define HSP_MBOX_TYPE_AS 0x3

#define HSP_DB_MASTER_CCPLEX 17
#define HSP_DB_MASTER_BPMP 19

#endif
