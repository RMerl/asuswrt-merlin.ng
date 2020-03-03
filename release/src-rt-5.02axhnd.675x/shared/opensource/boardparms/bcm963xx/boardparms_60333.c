#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"
extern BpCmdElem moca6802InitSeq[];


static char g_obsoleteStr[] = "(obsolete)";

static bp_elem_t g_bcm960333plc_dut[] = {
  {bp_cpBoardId,               .u.cp = "960333PLC_DUT"},
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_5_AH},

  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
  {bp_ulPortMap,               .u.ul = 0x07},
  {bp_ulPhyId0,                .u.ul = 0},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_EXT_PHY},
  {bp_ucPhyDevName,            .u.cp = "eth0"},
  {bp_ulPhyId1,                .u.ul = 1},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_INT_PHY},
  {bp_ucPhyDevName,            .u.cp = "eth1"},
  {bp_ulPhyId2,                .u.ul = 2},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_PLC},
  {bp_ucPhyDevName,            .u.cp = "plc0"},

  {bp_last}
};

static bp_elem_t g_bcm960333plc_ref[] = {
  {bp_cpBoardId,               .u.cp = "960333PLC_REF"},
  {bp_usGpioUart2Sdin,         .u.us = BP_GPIO_4_AH},
  {bp_usGpioUart2Sdout,        .u.us = BP_GPIO_5_AH},

  /* NOTE: This is the only option for this signal.
   *       power LED is driven by AON and is not assigned to any GPIO */
  {bp_usGpioLedBlPowerOn,          .u.us = BP_PIN_AON_POWER},

  /*
   * ON and OFF time values for Power LED blinking (in ms).
   * If undefined, the Power LED will stay solid ON.
   */
  /*
  {bp_ulLedBlPowerOnBlinkTimeOn,   .u.ul = 350},
  {bp_ulLedBlPowerOnBlinkTimeOff,  .u.ul = 650},
  */

  {bp_ucPhyType0,              .u.uc = BP_ENET_NO_PHY},
  {bp_ulPortMap,               .u.ul = 0x03},
  {bp_ulPhyId0,                .u.ul = 1},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_INT_PHY},
  {bp_ucPhyDevName,            .u.cp = "eth0"},
  {bp_usGpioLedLan,            .u.us = BP_GPIO_6_AH},
  {bp_ulPhyId1,                .u.ul = 2},
  {bp_usPhyConnType,           .u.us = PHY_CONN_TYPE_PLC},
  {bp_ucPhyDevName,            .u.cp = "plc0"},
  {bp_last}
};

static bp_elem_t g_bcm960500wifi_obsolete[] = {
  {bp_cpBoardId,               .u.cp = "960500PLC_WIFI"},
  {bp_cpComment,               .u.cp = g_obsoleteStr},  

  // buttons for 201
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,          .u.us = BP_BTN_ACTION_PLC_UKE | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,          .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S },
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL },
  {bp_usGpio_Intr,             .u.us = BP_GPIO_11_AL},
  {    bp_usButtonAction,          .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,       .u.ptr = (void*)"Button 1 Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,          .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S },
  {bp_usCfeResetToDefaultBtnIdx, .u.us = 1 },
  {bp_elemTemplate,            .u.bp_elemp = g_bcm960333plc_ref},
  {bp_last}
};

static bp_elem_t g_bcm960500wifi[] = {
  {bp_cpBoardId,               .u.cp = "960500WIFI"}, // works for P104 or P201
  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL },
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PLC_UKE | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S },
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_2 | BP_EXT_INTR_TYPE_IRQ_HIGH_LEVEL },
  {bp_usGpio_Intr,             .u.us = BP_GPIO_11_AH},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,    .u.ptr = (void*)"Button 1 Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S },
  {bp_usButtonIdx,             .u.us = 2},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,    .u.ptr = (void*)"Button 2 Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S },
  {bp_usCfeResetToDefaultBtnIdx, .u.us = 1 },    
  {bp_elemTemplate,            .u.bp_elemp = g_bcm960333plc_ref},
  {bp_last}
};

static bp_elem_t g_bcm960500wifi_ubus167[] = {
  {bp_cpBoardId,               .u.cp = "960500_UBUS167"},  // reduced list for P201

  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PLC_UKE | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_11_AL},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,    .u.ptr = (void*)"Button 1 Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S },
  {bp_usCfeResetToDefaultBtnIdx, .u.us = 1 },
  {bp_elemTemplate,            .u.bp_elemp = g_bcm960333plc_ref},
  {bp_last}
};


static bp_elem_t g_bcm960500wifi_p201[] = {
  {bp_cpBoardId,               .u.cp = "960500WIFI_P201"},  // reduced list for P201

  {bp_usButtonIdx,             .u.us = 0},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_1 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_1_AL},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PLC_UKE | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_SES | BP_BTN_TRIG_RELEASE | BP_BTN_TRIG_0S  },
  {bp_usButtonIdx,             .u.us = 1},
  {  bp_usButtonExtIntr,       .u.us = BP_EXT_INTR_0 | BP_EXT_INTR_TYPE_IRQ_LOW_LEVEL},
  {bp_usGpio_Intr,             .u.us = BP_GPIO_0_AL},
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_PRINT | BP_BTN_TRIG_PRESS },
  {      bp_ulButtonActionParm,    .u.ptr = (void*)"Button 1 Press -- Hold for 5s to do restore to default" },
  {    bp_usButtonAction,        .u.us = BP_BTN_ACTION_RESTORE_DEFAULTS | BP_BTN_TRIG_HOLD | BP_BTN_TRIG_5S },
  {bp_usCfeResetToDefaultBtnIdx, .u.us = 1 },
  {bp_elemTemplate,            .u.bp_elemp = g_bcm960333plc_ref},
  {bp_last}
};

bp_elem_t * g_BoardParms[] = {g_bcm960333plc_dut, g_bcm960333plc_ref, g_bcm960500wifi_obsolete, g_bcm960500wifi, g_bcm960500wifi_ubus167, g_bcm960500wifi_p201, 0};


