#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"

static bp_elem_t GT_10[] = {
    {bp_cpBoardId,               .u.cp = "GT10"},
    {bp_last}
};

bp_elem_t * g_BoardParms[] = { GT_10, 0 };
