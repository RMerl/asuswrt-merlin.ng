ifeq "$(ADSL)" "ANNEX_A"
DSL_DEFS = "\#define ADSL_ANNEXA\\n"
endif
ifeq "$(ADSL)" "ANNEX_B"
DSL_DEFS = "\#define ADSL_ANNEXB\\n"
endif
ifeq "$(ADSL)" "ANNEX_C"
DSL_DEFS = "\#define ADSL_ANNEXC\\n"
endif
ifeq "$(BRCM_PHY_CO)" "y"
DSL_DEFS := $(DSL_DEFS)"\#define PHY_CO\\n"
endif

DSL_HDR = "\#include \"../../bcmdrivers/broadcom/char/adsl/impl1/softdsl/AdsPhyMemDefs.h\""

DSL_DEFS := $(DSL_DEFS)$(DSL_HDR)

$(info $$DSL_DEFS is [${DSL_DEFS}])
