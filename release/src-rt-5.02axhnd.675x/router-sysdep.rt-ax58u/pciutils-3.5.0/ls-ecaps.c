/*
 *	The PCI Utilities -- Show Extended Capabilities
 *
 *	Copyright (c) 1997--2010 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>

#include "lspci.h"

static void
cap_tph(struct device *d, int where)
{
  u32 tph_cap;
  printf("Transaction Processing Hints\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_TPH_CAPABILITIES, 4))
    return;

  tph_cap = get_conf_long(d, where + PCI_TPH_CAPABILITIES);

  if (tph_cap & PCI_TPH_INTVEC_SUP)
    printf("\t\tInterrupt vector mode supported\n");
  if (tph_cap & PCI_TPH_DEV_SUP)
    printf("\t\tDevice specific mode supported\n");
  if (tph_cap & PCI_TPH_EXT_REQ_SUP)
    printf("\t\tExtended requester support\n");

  switch (tph_cap & PCI_TPH_ST_LOC_MASK) {
  case PCI_TPH_ST_NONE:
    printf("\t\tNo steering table available\n");
    break;
  case PCI_TPH_ST_CAP:
    printf("\t\tSteering table in TPH capability structure\n");
    break;
  case PCI_TPH_ST_MSIX:
    printf("\t\tSteering table in MSI-X table\n");
    break;
  default:
    printf("\t\tReserved steering table location\n");
    break;
  }
}

static u32
cap_ltr_scale(u8 scale)
{
  return 1 << (scale * 5);
}

static void
cap_ltr(struct device *d, int where)
{
  u32 scale;
  u16 snoop, nosnoop;
  printf("Latency Tolerance Reporting\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_LTR_MAX_SNOOP, 4))
    return;

  snoop = get_conf_word(d, where + PCI_LTR_MAX_SNOOP);
  scale = cap_ltr_scale((snoop >> PCI_LTR_SCALE_SHIFT) & PCI_LTR_SCALE_MASK);
  printf("\t\tMax snoop latency: %lldns\n",
	 ((unsigned long long)snoop & PCI_LTR_VALUE_MASK) * scale);

  nosnoop = get_conf_word(d, where + PCI_LTR_MAX_NOSNOOP);
  scale = cap_ltr_scale((nosnoop >> PCI_LTR_SCALE_SHIFT) & PCI_LTR_SCALE_MASK);
  printf("\t\tMax no snoop latency: %lldns\n",
	 ((unsigned long long)nosnoop & PCI_LTR_VALUE_MASK) * scale);
}

static void
cap_dsn(struct device *d, int where)
{
  u32 t1, t2;
  if (!config_fetch(d, where + 4, 8))
    return;
  t1 = get_conf_long(d, where + 4);
  t2 = get_conf_long(d, where + 8);
  printf("Device Serial Number %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
	t2 >> 24, (t2 >> 16) & 0xff, (t2 >> 8) & 0xff, t2 & 0xff,
	t1 >> 24, (t1 >> 16) & 0xff, (t1 >> 8) & 0xff, t1 & 0xff);
}

static void
cap_aer(struct device *d, int where)
{
  u32 l;

  printf("Advanced Error Reporting\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_ERR_UNCOR_STATUS, 24))
    return;

  l = get_conf_long(d, where + PCI_ERR_UNCOR_STATUS);
  printf("\t\tUESta:\tDLP%c SDES%c TLP%c FCP%c CmpltTO%c CmpltAbrt%c UnxCmplt%c RxOF%c "
	"MalfTLP%c ECRC%c UnsupReq%c ACSViol%c\n",
	FLAG(l, PCI_ERR_UNC_DLP), FLAG(l, PCI_ERR_UNC_SDES), FLAG(l, PCI_ERR_UNC_POISON_TLP),
	FLAG(l, PCI_ERR_UNC_FCP), FLAG(l, PCI_ERR_UNC_COMP_TIME), FLAG(l, PCI_ERR_UNC_COMP_ABORT),
	FLAG(l, PCI_ERR_UNC_UNX_COMP), FLAG(l, PCI_ERR_UNC_RX_OVER), FLAG(l, PCI_ERR_UNC_MALF_TLP),
	FLAG(l, PCI_ERR_UNC_ECRC), FLAG(l, PCI_ERR_UNC_UNSUP), FLAG(l, PCI_ERR_UNC_ACS_VIOL));
  l = get_conf_long(d, where + PCI_ERR_UNCOR_MASK);
  printf("\t\tUEMsk:\tDLP%c SDES%c TLP%c FCP%c CmpltTO%c CmpltAbrt%c UnxCmplt%c RxOF%c "
	"MalfTLP%c ECRC%c UnsupReq%c ACSViol%c\n",
	FLAG(l, PCI_ERR_UNC_DLP), FLAG(l, PCI_ERR_UNC_SDES), FLAG(l, PCI_ERR_UNC_POISON_TLP),
	FLAG(l, PCI_ERR_UNC_FCP), FLAG(l, PCI_ERR_UNC_COMP_TIME), FLAG(l, PCI_ERR_UNC_COMP_ABORT),
	FLAG(l, PCI_ERR_UNC_UNX_COMP), FLAG(l, PCI_ERR_UNC_RX_OVER), FLAG(l, PCI_ERR_UNC_MALF_TLP),
	FLAG(l, PCI_ERR_UNC_ECRC), FLAG(l, PCI_ERR_UNC_UNSUP), FLAG(l, PCI_ERR_UNC_ACS_VIOL));
  l = get_conf_long(d, where + PCI_ERR_UNCOR_SEVER);
  printf("\t\tUESvrt:\tDLP%c SDES%c TLP%c FCP%c CmpltTO%c CmpltAbrt%c UnxCmplt%c RxOF%c "
	"MalfTLP%c ECRC%c UnsupReq%c ACSViol%c\n",
	FLAG(l, PCI_ERR_UNC_DLP), FLAG(l, PCI_ERR_UNC_SDES), FLAG(l, PCI_ERR_UNC_POISON_TLP),
	FLAG(l, PCI_ERR_UNC_FCP), FLAG(l, PCI_ERR_UNC_COMP_TIME), FLAG(l, PCI_ERR_UNC_COMP_ABORT),
	FLAG(l, PCI_ERR_UNC_UNX_COMP), FLAG(l, PCI_ERR_UNC_RX_OVER), FLAG(l, PCI_ERR_UNC_MALF_TLP),
	FLAG(l, PCI_ERR_UNC_ECRC), FLAG(l, PCI_ERR_UNC_UNSUP), FLAG(l, PCI_ERR_UNC_ACS_VIOL));
  l = get_conf_long(d, where + PCI_ERR_COR_STATUS);
  printf("\t\tCESta:\tRxErr%c BadTLP%c BadDLLP%c Rollover%c Timeout%c NonFatalErr%c\n",
	FLAG(l, PCI_ERR_COR_RCVR), FLAG(l, PCI_ERR_COR_BAD_TLP), FLAG(l, PCI_ERR_COR_BAD_DLLP),
	FLAG(l, PCI_ERR_COR_REP_ROLL), FLAG(l, PCI_ERR_COR_REP_TIMER), FLAG(l, PCI_ERR_COR_REP_ANFE));
  l = get_conf_long(d, where + PCI_ERR_COR_MASK);
  printf("\t\tCEMsk:\tRxErr%c BadTLP%c BadDLLP%c Rollover%c Timeout%c NonFatalErr%c\n",
	FLAG(l, PCI_ERR_COR_RCVR), FLAG(l, PCI_ERR_COR_BAD_TLP), FLAG(l, PCI_ERR_COR_BAD_DLLP),
	FLAG(l, PCI_ERR_COR_REP_ROLL), FLAG(l, PCI_ERR_COR_REP_TIMER), FLAG(l, PCI_ERR_COR_REP_ANFE));
  l = get_conf_long(d, where + PCI_ERR_CAP);
  printf("\t\tAERCap:\tFirst Error Pointer: %02x, GenCap%c CGenEn%c ChkCap%c ChkEn%c\n",
	PCI_ERR_CAP_FEP(l), FLAG(l, PCI_ERR_CAP_ECRC_GENC), FLAG(l, PCI_ERR_CAP_ECRC_GENE),
	FLAG(l, PCI_ERR_CAP_ECRC_CHKC), FLAG(l, PCI_ERR_CAP_ECRC_CHKE));

}

static void cap_dpc(struct device *d, int where)
{
  u16 l;

  printf("Downstream Port Containment\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_DPC_CAP, 8))
    return;

  l = get_conf_word(d, where + PCI_DPC_CAP);
  printf("\t\tDpcCap:\tINT Msg #%d, RPExt%c PoisonedTLP%c SwTrigger%c RP PIO Log %d, DL_ActiveErr%c\n",
    PCI_DPC_CAP_INT_MSG(l), FLAG(l, PCI_DPC_CAP_RP_EXT), FLAG(l, PCI_DPC_CAP_TLP_BLOCK),
    FLAG(l, PCI_DPC_CAP_SW_TRIGGER), PCI_DPC_CAP_RP_LOG(l), FLAG(l, PCI_DPC_CAP_DL_ACT_ERR));

  l = get_conf_word(d, where + PCI_DPC_CTL);
  printf("\t\tDpcCtl:\tTrigger:%x Cmpl%c INT%c ErrCor%c PoisonedTLP%c SwTrigger%c DL_ActiveErr%c\n",
    PCI_DPC_CTL_TRIGGER(l), FLAG(l, PCI_DPC_CTL_CMPL), FLAG(l, PCI_DPC_CTL_INT),
    FLAG(l, PCI_DPC_CTL_ERR_COR), FLAG(l, PCI_DPC_CTL_TLP), FLAG(l, PCI_DPC_CTL_SW_TRIGGER),
    FLAG(l, PCI_DPC_CTL_DL_ACTIVE));

  l = get_conf_word(d, where + PCI_DPC_STATUS);
  printf("\t\tDpcSta:\tTrigger%c Reason:%02x INT%c RPBusy%c TriggerExt:%02x RP PIO ErrPtr:%02x\n",
    FLAG(l, PCI_DPC_STS_TRIGGER), PCI_DPC_STS_REASON(l), FLAG(l, PCI_DPC_STS_INT),
    FLAG(l, PCI_DPC_STS_RP_BUSY), PCI_DPC_STS_TRIGGER_EXT(l), PCI_DPC_STS_PIO_FEP(l));

  l = get_conf_word(d, where + PCI_DPC_SOURCE);
  printf("\t\tSource:\t%04x\n", l);
}

static void
cap_acs(struct device *d, int where)
{
  u16 w;

  printf("Access Control Services\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_ACS_CAP, 4))
    return;

  w = get_conf_word(d, where + PCI_ACS_CAP);
  printf("\t\tACSCap:\tSrcValid%c TransBlk%c ReqRedir%c CmpltRedir%c UpstreamFwd%c EgressCtrl%c "
	"DirectTrans%c\n",
	FLAG(w, PCI_ACS_CAP_VALID), FLAG(w, PCI_ACS_CAP_BLOCK), FLAG(w, PCI_ACS_CAP_REQ_RED),
	FLAG(w, PCI_ACS_CAP_CMPLT_RED), FLAG(w, PCI_ACS_CAP_FORWARD), FLAG(w, PCI_ACS_CAP_EGRESS),
	FLAG(w, PCI_ACS_CAP_TRANS));
  w = get_conf_word(d, where + PCI_ACS_CTRL);
  printf("\t\tACSCtl:\tSrcValid%c TransBlk%c ReqRedir%c CmpltRedir%c UpstreamFwd%c EgressCtrl%c "
	"DirectTrans%c\n",
	FLAG(w, PCI_ACS_CTRL_VALID), FLAG(w, PCI_ACS_CTRL_BLOCK), FLAG(w, PCI_ACS_CTRL_REQ_RED),
	FLAG(w, PCI_ACS_CTRL_CMPLT_RED), FLAG(w, PCI_ACS_CTRL_FORWARD), FLAG(w, PCI_ACS_CTRL_EGRESS),
	FLAG(w, PCI_ACS_CTRL_TRANS));
}

static void
cap_ari(struct device *d, int where)
{
  u16 w;

  printf("Alternative Routing-ID Interpretation (ARI)\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_ARI_CAP, 4))
    return;

  w = get_conf_word(d, where + PCI_ARI_CAP);
  printf("\t\tARICap:\tMFVC%c ACS%c, Next Function: %d\n",
	FLAG(w, PCI_ARI_CAP_MFVC), FLAG(w, PCI_ARI_CAP_ACS),
	PCI_ARI_CAP_NFN(w));
  w = get_conf_word(d, where + PCI_ARI_CTRL);
  printf("\t\tARICtl:\tMFVC%c ACS%c, Function Group: %d\n",
	FLAG(w, PCI_ARI_CTRL_MFVC), FLAG(w, PCI_ARI_CTRL_ACS),
	PCI_ARI_CTRL_FG(w));
}

static void
cap_ats(struct device *d, int where)
{
  u16 w;

  printf("Address Translation Service (ATS)\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_ATS_CAP, 4))
    return;

  w = get_conf_word(d, where + PCI_ATS_CAP);
  printf("\t\tATSCap:\tInvalidate Queue Depth: %02x\n", PCI_ATS_CAP_IQD(w));
  w = get_conf_word(d, where + PCI_ATS_CTRL);
  printf("\t\tATSCtl:\tEnable%c, Smallest Translation Unit: %02x\n",
	FLAG(w, PCI_ATS_CTRL_ENABLE), PCI_ATS_CTRL_STU(w));
}

static void
cap_pri(struct device *d, int where)
{
  u16 w;
  u32 l;

  printf("Page Request Interface (PRI)\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_PRI_CTRL, 0xc))
    return;

  w = get_conf_word(d, where + PCI_PRI_CTRL);
  printf("\t\tPRICtl: Enable%c Reset%c\n",
	FLAG(w, PCI_PRI_CTRL_ENABLE), FLAG(w, PCI_PRI_CTRL_RESET));
  w = get_conf_word(d, where + PCI_PRI_STATUS);
  printf("\t\tPRISta: RF%c UPRGI%c Stopped%c\n",
	FLAG(w, PCI_PRI_STATUS_RF), FLAG(w, PCI_PRI_STATUS_UPRGI),
	FLAG(w, PCI_PRI_STATUS_STOPPED));
  l = get_conf_long(d, where + PCI_PRI_MAX_REQ);
  printf("\t\tPage Request Capacity: %08x, ", l);
  l = get_conf_long(d, where + PCI_PRI_ALLOC_REQ);
  printf("Page Request Allocation: %08x\n", l);
}

static void
cap_pasid(struct device *d, int where)
{
  u16 w;

  printf("Process Address Space ID (PASID)\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_PASID_CAP, 4))
    return;

  w = get_conf_word(d, where + PCI_PASID_CAP);
  printf("\t\tPASIDCap: Exec%c Priv%c, Max PASID Width: %02x\n",
	FLAG(w, PCI_PASID_CAP_EXEC), FLAG(w, PCI_PASID_CAP_PRIV),
	PCI_PASID_CAP_WIDTH(w));
  w = get_conf_word(d, where + PCI_PASID_CTRL);
  printf("\t\tPASIDCtl: Enable%c Exec%c Priv%c\n",
	FLAG(w, PCI_PASID_CTRL_ENABLE), FLAG(w, PCI_PASID_CTRL_EXEC),
	FLAG(w, PCI_PASID_CTRL_PRIV));
}

static void
cap_sriov(struct device *d, int where)
{
  u16 b;
  u16 w;
  u32 l;
  int i;

  printf("Single Root I/O Virtualization (SR-IOV)\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + PCI_IOV_CAP, 0x3c))
    return;

  l = get_conf_long(d, where + PCI_IOV_CAP);
  printf("\t\tIOVCap:\tMigration%c, Interrupt Message Number: %03x\n",
	FLAG(l, PCI_IOV_CAP_VFM), PCI_IOV_CAP_IMN(l));
  w = get_conf_word(d, where + PCI_IOV_CTRL);
  printf("\t\tIOVCtl:\tEnable%c Migration%c Interrupt%c MSE%c ARIHierarchy%c\n",
	FLAG(w, PCI_IOV_CTRL_VFE), FLAG(w, PCI_IOV_CTRL_VFME),
	FLAG(w, PCI_IOV_CTRL_VFMIE), FLAG(w, PCI_IOV_CTRL_MSE),
	FLAG(w, PCI_IOV_CTRL_ARI));
  w = get_conf_word(d, where + PCI_IOV_STATUS);
  printf("\t\tIOVSta:\tMigration%c\n", FLAG(w, PCI_IOV_STATUS_MS));
  w = get_conf_word(d, where + PCI_IOV_INITIALVF);
  printf("\t\tInitial VFs: %d, ", w);
  w = get_conf_word(d, where + PCI_IOV_TOTALVF);
  printf("Total VFs: %d, ", w);
  w = get_conf_word(d, where + PCI_IOV_NUMVF);
  printf("Number of VFs: %d, ", w);
  b = get_conf_byte(d, where + PCI_IOV_FDL);
  printf("Function Dependency Link: %02x\n", b);
  w = get_conf_word(d, where + PCI_IOV_OFFSET);
  printf("\t\tVF offset: %d, ", w);
  w = get_conf_word(d, where + PCI_IOV_STRIDE);
  printf("stride: %d, ", w);
  w = get_conf_word(d, where + PCI_IOV_DID);
  printf("Device ID: %04x\n", w);
  l = get_conf_long(d, where + PCI_IOV_SUPPS);
  printf("\t\tSupported Page Size: %08x, ", l);
  l = get_conf_long(d, where + PCI_IOV_SYSPS);
  printf("System Page Size: %08x\n", l);

  for (i=0; i < PCI_IOV_NUM_BAR; i++)
    {
      u32 addr;
      int type;
      u32 h;
      l = get_conf_long(d, where + PCI_IOV_BAR_BASE + 4*i);
      if (l == 0xffffffff)
	l = 0;
      if (!l)
	continue;
      printf("\t\tRegion %d: Memory at ", i);
      addr = l & PCI_ADDR_MEM_MASK;
      type = l & PCI_BASE_ADDRESS_MEM_TYPE_MASK;
      if (type == PCI_BASE_ADDRESS_MEM_TYPE_64)
	{
	  i++;
	  h = get_conf_long(d, where + PCI_IOV_BAR_BASE + (i*4));
	  printf("%08x", h);
	}
      printf("%08x (%s-bit, %sprefetchable)\n",
	addr,
	(type == PCI_BASE_ADDRESS_MEM_TYPE_32) ? "32" : "64",
	(l & PCI_BASE_ADDRESS_MEM_PREFETCH) ? "" : "non-");
    }

  l = get_conf_long(d, where + PCI_IOV_MSAO);
  printf("\t\tVF Migration: offset: %08x, BIR: %x\n", PCI_IOV_MSA_OFFSET(l),
	PCI_IOV_MSA_BIR(l));
}

static void
cap_vc(struct device *d, int where)
{
  u32 cr1, cr2;
  u16 ctrl, status;
  int evc_cnt;
  int arb_table_pos;
  int i, j;
  static const char ref_clocks[][6] = { "100ns" };
  static const char arb_selects[8][7] = { "Fixed", "WRR32", "WRR64", "WRR128", "??4", "??5", "??6", "??7" };
  static const char vc_arb_selects[8][8] = { "Fixed", "WRR32", "WRR64", "WRR128", "TWRR128", "WRR256", "??6", "??7" };
  char buf[8];

  printf("Virtual Channel\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + 4, 0x1c - 4))
    return;

  cr1 = get_conf_long(d, where + PCI_VC_PORT_REG1);
  cr2 = get_conf_long(d, where + PCI_VC_PORT_REG2);
  ctrl = get_conf_word(d, where + PCI_VC_PORT_CTRL);
  status = get_conf_word(d, where + PCI_VC_PORT_STATUS);

  evc_cnt = BITS(cr1, 0, 3);
  printf("\t\tCaps:\tLPEVC=%d RefClk=%s PATEntryBits=%d\n",
    BITS(cr1, 4, 3),
    TABLE(ref_clocks, BITS(cr1, 8, 2), buf),
    1 << BITS(cr1, 10, 2));

  printf("\t\tArb:");
  for (i=0; i<8; i++)
    if (arb_selects[i][0] != '?' || cr2 & (1 << i))
      printf("%c%s%c", (i ? ' ' : '\t'), arb_selects[i], FLAG(cr2, 1 << i));
  arb_table_pos = BITS(cr2, 24, 8);

  printf("\n\t\tCtrl:\tArbSelect=%s\n", TABLE(arb_selects, BITS(ctrl, 1, 3), buf));
  printf("\t\tStatus:\tInProgress%c\n", FLAG(status, 1));

  if (arb_table_pos)
    {
      arb_table_pos = where + 16*arb_table_pos;
      printf("\t\tPort Arbitration Table [%x] <?>\n", arb_table_pos);
    }

  for (i=0; i<=evc_cnt; i++)
    {
      int pos = where + PCI_VC_RES_CAP + 12*i;
      u32 rcap, rctrl;
      u16 rstatus;
      int pat_pos;

      printf("\t\tVC%d:\t", i);
      if (!config_fetch(d, pos, 12))
	{
	  printf("<unreadable>\n");
	  continue;
	}
      rcap = get_conf_long(d, pos);
      rctrl = get_conf_long(d, pos+4);
      rstatus = get_conf_word(d, pos+10);

      pat_pos = BITS(rcap, 24, 8);
      printf("Caps:\tPATOffset=%02x MaxTimeSlots=%d RejSnoopTrans%c\n",
	pat_pos,
	BITS(rcap, 16, 6) + 1,
	FLAG(rcap, 1 << 15));

      printf("\t\t\tArb:");
      for (j=0; j<8; j++)
	if (vc_arb_selects[j][0] != '?' || rcap & (1 << j))
	  printf("%c%s%c", (j ? ' ' : '\t'), vc_arb_selects[j], FLAG(rcap, 1 << j));

      printf("\n\t\t\tCtrl:\tEnable%c ID=%d ArbSelect=%s TC/VC=%02x\n",
	FLAG(rctrl, 1 << 31),
	BITS(rctrl, 24, 3),
	TABLE(vc_arb_selects, BITS(rctrl, 17, 3), buf),
	BITS(rctrl, 0, 8));

      printf("\t\t\tStatus:\tNegoPending%c InProgress%c\n",
	FLAG(rstatus, 2),
	FLAG(rstatus, 1));

      if (pat_pos)
	printf("\t\t\tPort Arbitration Table <?>\n");
    }
}

static void
cap_rclink(struct device *d, int where)
{
  u32 esd;
  int num_links;
  int i;
  static const char elt_types[][9] = { "Config", "Egress", "Internal" };
  char buf[8];

  printf("Root Complex Link\n");
  if (verbose < 2)
    return;

  if (!config_fetch(d, where + 4, PCI_RCLINK_LINK1 - 4))
    return;

  esd = get_conf_long(d, where + PCI_RCLINK_ESD);
  num_links = BITS(esd, 8, 8);
  printf("\t\tDesc:\tPortNumber=%02x ComponentID=%02x EltType=%s\n",
    BITS(esd, 24, 8),
    BITS(esd, 16, 8),
    TABLE(elt_types, BITS(esd, 0, 8), buf));

  for (i=0; i<num_links; i++)
    {
      int pos = where + PCI_RCLINK_LINK1 + i*PCI_RCLINK_LINK_SIZE;
      u32 desc;
      u32 addr_lo, addr_hi;

      printf("\t\tLink%d:\t", i);
      if (!config_fetch(d, pos, PCI_RCLINK_LINK_SIZE))
	{
	  printf("<unreadable>\n");
	  return;
	}
      desc = get_conf_long(d, pos + PCI_RCLINK_LINK_DESC);
      addr_lo = get_conf_long(d, pos + PCI_RCLINK_LINK_ADDR);
      addr_hi = get_conf_long(d, pos + PCI_RCLINK_LINK_ADDR + 4);

      printf("Desc:\tTargetPort=%02x TargetComponent=%02x AssocRCRB%c LinkType=%s LinkValid%c\n",
	BITS(desc, 24, 8),
	BITS(desc, 16, 8),
	FLAG(desc, 4),
	((desc & 2) ? "Config" : "MemMapped"),
	FLAG(desc, 1));

      if (desc & 2)
	{
	  int n = addr_lo & 7;
	  if (!n)
	    n = 8;
	  printf("\t\t\tAddr:\t%02x:%02x.%d  CfgSpace=%08x%08x\n",
	    BITS(addr_lo, 20, n),
	    BITS(addr_lo, 15, 5),
	    BITS(addr_lo, 12, 3),
	    addr_hi, addr_lo);
	}
      else
	printf("\t\t\tAddr:\t%08x%08x\n", addr_hi, addr_lo);
    }
}

static void
cap_evendor(struct device *d, int where)
{
  u32 hdr;

  printf("Vendor Specific Information: ");
  if (!config_fetch(d, where + PCI_EVNDR_HEADER, 4))
    {
      printf("<unreadable>\n");
      return;
    }

  hdr = get_conf_long(d, where + PCI_EVNDR_HEADER);
  printf("ID=%04x Rev=%d Len=%03x <?>\n",
    BITS(hdr, 0, 16),
    BITS(hdr, 16, 4),
    BITS(hdr, 20, 12));
}

static void
cap_l1pm(struct device *d, int where)
{
  u32 l1_cap;
  int power_on_scale;

  printf("L1 PM Substates\n");

  if (verbose < 2)
    return;

  if (!config_fetch(d, where + 4, 4))
    {
      printf("\t\t<unreadable>\n");
      return;
    }

  l1_cap = get_conf_long(d, where + 4);
  printf("\t\tL1SubCap: ");
  printf("PCI-PM_L1.2%c PCI-PM_L1.1%c ASPM_L1.2%c ASPM_L1.1%c L1_PM_Substates%c\n",
    FLAG(l1_cap, 1),
    FLAG(l1_cap, 2),
    FLAG(l1_cap, 4),
    FLAG(l1_cap, 8),
    FLAG(l1_cap, 16));

  if (BITS(l1_cap, 0, 1) || BITS(l1_cap, 2, 1))
    {
      printf("\t\t\t  PortCommonModeRestoreTime=%dus ",
	BITS(l1_cap, 8,8));

      power_on_scale = BITS(l1_cap, 16, 2);

      printf("PortTPowerOnTime=");
      switch (power_on_scale)
	{
	  case 0:
	    printf("%dus\n", BITS(l1_cap, 19, 5) * 2);
	    break;
	  case 1:
	    printf("%dus\n", BITS(l1_cap, 19, 5) * 10);
	    break;
	  case 2:
	    printf("%dus\n", BITS(l1_cap, 19, 5) * 100);
	    break;
	  default:
	    printf("<error>\n");
	    break;
	}
    }
}

static void
cap_ptm(struct device *d, int where)
{
  u32 buff;
  u16 clock;

  printf("Precision Time Measurement\n");

  if (verbose < 2)
    return;

  if (!config_fetch(d, where + 4, 8))
    {
      printf("\t\t<unreadable>\n");
      return;
    }

  buff = get_conf_long(d, where + 4);
  printf("\t\tPTMCap: ");
  printf("Requester:%c Responder:%c Root:%c\n",
    FLAG(buff, 0x1),
    FLAG(buff, 0x2),
    FLAG(buff, 0x4));

  clock = BITS(buff, 8, 8);
  printf("\t\tPTMClockGranularity: ");
  switch (clock)
    {
      case 0x00:
        printf("Unimplemented\n");
        break;
      case 0xff:
        printf("Greater than 254ns\n");
        break;
      default:
        printf("%huns\n", clock);
    }

  buff = get_conf_long(d, where + 8);
  printf("\t\tPTMControl: ");
  printf("Enabled:%c RootSelected:%c\n",
    FLAG(buff, 0x1),
    FLAG(buff, 0x2));

  clock = BITS(buff, 8, 8);
  printf("\t\tPTMEffectiveGranularity: ");
  switch (clock)
    {
      case 0x00:
        printf("Unknown\n");
        break;
      case 0xff:
        printf("Greater than 254ns\n");
        break;
      default:
        printf("%huns\n", clock);
    }
}

void
show_ext_caps(struct device *d)
{
  int where = 0x100;
  char been_there[0x1000];
  memset(been_there, 0, 0x1000);
  do
    {
      u32 header;
      int id, version;

      if (!config_fetch(d, where, 4))
	break;
      header = get_conf_long(d, where);
      if (!header)
	break;
      id = header & 0xffff;
      version = (header >> 16) & 0xf;
      printf("\tCapabilities: [%03x", where);
      if (verbose > 1)
	printf(" v%d", version);
      printf("] ");
      if (been_there[where]++)
	{
	  printf("<chain looped>\n");
	  break;
	}
      switch (id)
	{
	  case PCI_EXT_CAP_ID_AER:
	    cap_aer(d, where);
	    break;
	  case PCI_EXT_CAP_ID_DPC:
	    cap_dpc(d, where);
	    break;
	  case PCI_EXT_CAP_ID_VC:
	  case PCI_EXT_CAP_ID_VC2:
	    cap_vc(d, where);
	    break;
	  case PCI_EXT_CAP_ID_DSN:
	    cap_dsn(d, where);
	    break;
	  case PCI_EXT_CAP_ID_PB:
	    printf("Power Budgeting <?>\n");
	    break;
	  case PCI_EXT_CAP_ID_RCLINK:
	    cap_rclink(d, where);
	    break;
	  case PCI_EXT_CAP_ID_RCILINK:
	    printf("Root Complex Internal Link <?>\n");
	    break;
	  case PCI_EXT_CAP_ID_RCECOLL:
	    printf("Root Complex Event Collector <?>\n");
	    break;
	  case PCI_EXT_CAP_ID_MFVC:
	    printf("Multi-Function Virtual Channel <?>\n");
	    break;
	  case PCI_EXT_CAP_ID_RBCB:
	    printf("Root Bridge Control Block <?>\n");
	    break;
	  case PCI_EXT_CAP_ID_VNDR:
	    cap_evendor(d, where);
	    break;
	  case PCI_EXT_CAP_ID_ACS:
	    cap_acs(d, where);
	    break;
	  case PCI_EXT_CAP_ID_ARI:
	    cap_ari(d, where);
	    break;
	  case PCI_EXT_CAP_ID_ATS:
	    cap_ats(d, where);
	    break;
	  case PCI_EXT_CAP_ID_SRIOV:
	    cap_sriov(d, where);
	    break;
	  case PCI_EXT_CAP_ID_PRI:
	    cap_pri(d, where);
	    break;
	  case PCI_EXT_CAP_ID_TPH:
	    cap_tph(d, where);
	    break;
	  case PCI_EXT_CAP_ID_LTR:
	    cap_ltr(d, where);
	    break;
	  case PCI_EXT_CAP_ID_PASID:
	    cap_pasid(d, where);
	    break;
	  case PCI_EXT_CAP_ID_L1PM:
	    cap_l1pm(d, where);
	    break;
	  case PCI_EXT_CAP_ID_PTM:
	    cap_ptm(d, where);
	    break;
	  default:
	    printf("#%02x\n", id);
	    break;
	}
      where = (header >> 20) & ~3;
    } while (where);
}
