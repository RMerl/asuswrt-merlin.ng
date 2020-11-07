#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];

static bp_elem_t g_bcm947189ref[] = {
  /* Unmanaged switch in RGMII 0 */
  {bp_cpBoardId,               .u.cp = "947189REF"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_ulPortMap,             .u.ul = 0x1},
    {bp_ucPhyAddress,          .u.uc = 0x1e},
    /* Only one port definition: unmanaged switch */
    {bp_ulPhyId0,              .u.ul = 0x1e | MAC_IF_RGMII},
      {bp_usPhyConnType,       .u.us = PHY_CONN_TYPE_EXT_SW},
      {bp_ucPhyDevName,        .u.cp = "eth0"},
      {bp_usGpioPhyReset,      .u.us = 0x02},
  /* Need to set TRFC_160NS for 2Gb DDR chip */
  /* {bp_ulMemoryConfig,		   .u.ul = BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TRFC_160NS}, */
  {bp_last}
};

static bp_elem_t g_bcm947189ref2[] = {
  /* GPHY in RGMII 0, PLC in RGMII 1 */
  {bp_cpBoardId,               .u.cp = "947189REF2"},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_ulPortMap,             .u.ul = 0x3},
    {bp_ulPhyId0,              .u.ul = 25 | MAC_IF_RGMII},
      {bp_usPhyConnType,       .u.us = PHY_CONN_TYPE_EXT_PHY},
      {bp_ucPhyDevName,        .u.cp = "eth0"},
      {bp_usGpioPhyReset,      .u.us = 0x02},
    {bp_ulPhyId1,              .u.ul = BP_PHY_ID_2 | MAC_IF_RGMII},
      {bp_usPhyConnType,       .u.us = PHY_CONN_TYPE_PLC},
      {bp_ucPhyDevName,        .u.cp = "plc0"},
  {bp_last}
};

static bp_elem_t g_bcm947189acnrm[] = {
  /* managed switch in RGMII 0 */
  {bp_cpBoardId,               .u.cp = "947189acnrm"},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL },
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_10_AH},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_ulPortMap,             .u.ul = 0x1},
    {bp_ucPhyAddress,          .u.uc = 0x0},
    /* Only one port definition: managed switch */
    {bp_ulPhyId0,              .u.ul = RGMII_DIRECT | EXTSW_CONNECTED},
    {bp_usGpioPhyReset,        .u.us = 0x02},
  {bp_ucPhyType1,              .u.uc = BP_ENET_EXTERNAL_SWITCH},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_usConfigType,            .u.us = BP_ENET_CONFIG_MDIO},
  {bp_ucPhyAddress,            .u.uc = 0x0},
  {bp_ulPortMap,               .u.ul = 0x1f},
  {bp_ulPhyId0,                .u.ul = 0x00 | CONNECTED_TO_EXTERN_SW},
  {bp_ulPhyId1,                .u.ul = 0x01 | CONNECTED_TO_EXTERN_SW},
  {bp_ulPhyId2,                .u.ul = 0x02 | CONNECTED_TO_EXTERN_SW},
  {bp_ulPhyId3,                .u.ul = 0x03 | CONNECTED_TO_EXTERN_SW},
  {bp_ulPhyId4,                .u.ul = 0x04 | CONNECTED_TO_EXTERN_SW},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_8_AL},
  {bp_ulMemoryConfig,		   .u.ul = BP_DDR_DEVICE_WIDTH_16 | BP_DDR_TOTAL_WIDTH_32BIT},
  {bp_last}
};

static bp_elem_t g_bcm947189acnrh[] = {
  /* managed switch in RGMII 0 */
  {bp_cpBoardId,               .u.cp = "947189acnrh"},
  {bp_elemTemplate,            .u.bp_elemp = g_bcm947189acnrm},
  {bp_last}
};


static bp_elem_t g_bcm947189acdbmr[] = {
  {bp_cpBoardId,               .u.cp = "947189acdbmr"},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL },
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0}, 
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
  {bp_usGpioMocaReset,         .u.us = BP_GPIO_2_AH},
  {bp_usGpioSpiClk,            .u.us = BP_GPIO_21_AH}, 
  {bp_usGpioSpiCs,             .u.us = BP_GPIO_24_AH}, 
  {bp_usGpioSpiMiso,           .u.us = BP_GPIO_22_AH},
  {bp_usGpioSpiMosi,           .u.us = BP_GPIO_23_AH}, 
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_10_AH},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_ulPortMap,             .u.ul = 0x1},
    {bp_ucPhyAddress,          .u.uc = 0x0},
    {bp_ulPhyId0,              .u.ul = 10 | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL},	
      {bp_usPhyConnType,       .u.us = PHY_CONN_TYPE_MOCA_ETH},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_8_AL},
  {bp_last}
};

static bp_elem_t g_bcm947452eapl[] = {
  {bp_cpBoardId,               .u.cp = "947452eapl"},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_7_AL },
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_9_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_10_AH},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_ulPortMap,             .u.ul = 0x1},
    {bp_ulPhyId0,              .u.ul = 25 | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
      {bp_usPhyConnType,       .u.us = PHY_CONN_TYPE_EXT_PHY},
      {bp_ucPhyDevName,        .u.cp = "eth0"},
      {bp_usGpioPhyReset,      .u.us = 0x02},
  {bp_last}
};

static bp_elem_t g_bcm947189acr[] = {
  {bp_cpBoardId,               .u.cp = "947189acr"},
  {bp_usExtIntrResetToDefault, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_4_AL },
  {bp_usExtIntrSesBtnWireless, .u.us = BP_EXT_INTR_SHARED | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL | BP_EXT_INTR_0},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_5_AL},
  {bp_usGpioLedSesWireless,    .u.us = BP_GPIO_7_AH},
  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
    {bp_ulPortMap,             .u.ul = 0x1},
    {bp_ulPhyId0,              .u.ul = 24 | MAC_IF_RGMII | PHY_INTEGRATED_VALID | PHY_EXTERNAL | (ADVERTISE_ALL_GMII | PHY_ADV_CFG_VALID)},
      {bp_usPhyConnType,       .u.us = PHY_CONN_TYPE_EXT_PHY},
      {bp_ucPhyDevName,        .u.cp = "eth0"},
      {bp_usGpioPhyReset,      .u.us = 0x09},
  {bp_usUsbPwrOn0,             .u.us = BP_GPIO_2_AL},
  {bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm947189ref, g_bcm947189ref2, g_bcm947189acnrm, g_bcm947189acnrh, g_bcm947189acdbmr, g_bcm947452eapl, g_bcm947189acr, 0};


