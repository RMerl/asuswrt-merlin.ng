#include "bp_defs.h"
#include "boardparms.h"
#include "bcmSpiRes.h"

static bp_elem_t GT_10[] = {
    {bp_cpBoardId,               .u.cp = "GT10"},
    {bp_last}
};

static bp_elem_t RT_AX9000[] = {
    {bp_cpBoardId,               .u.cp = "RTAX9000"},
    {bp_last}
};

bp_elem_t * g_BoardParms[] = { GT_10, RT_AX9000, 0 };
