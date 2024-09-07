/******************************************************************************

   Copyright 2023-2024 MaxLinear, Inc.

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

#ifndef MXL_GSW_FLOW_INDEX_H_
#define MXL_GSW_FLOW_INDEX_H_

/* Global Rules */
#define MPCE_RULES_INDEX		0
#define MPCE_RULES_INDEX_LAST		(MPCE_RULES_INDEX + 7)
#define EAPOL_PCE_RULE_INDEX		8
#define BPDU_PCE_RULE_INDEX		9
#define PFC_PCE_RULE_INDEX		10
#define DSA_EXT_PCE_RULE_INDEX		11
#define LLDP_PCE_RULE_INDEX		12
#define OAM_8023AH_PCE_RULE_INDEX	13
#define LACP_PCE_RULE_INDEX		OAM_8023AH_PCE_RULE_INDEX
#define PTP_PCE_RULE_INDEX		14
#define MAX_PREDEFINED_GLOBAL_PCE_RULE	15

/* Per CTP Rules */
#define FC_PCE_RULE_INDEX		0
#define MAX_PREDEFINED_PER_CTP_PCE_RULE	1

#endif /* #ifndef MXL_GSW_FLOW_INDEX_H_ */
