#include <common.h>
#include <malloc.h>
#include <exports.h>
#include <spi.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

__attribute__((unused)) static void dummy(void)
{
}

unsigned long get_version(void)
{
	return XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

#ifndef CONFIG_PHY_AQUANTIA
# define mdio_get_current_dev		dummy
# define phy_find_by_mask		dummy
# define mdio_phydev_for_ethname	dummy
# define miiphy_set_current_dev		dummy
#endif

void jumptable_init(void)
{
	gd->jt = malloc(sizeof(struct jt_funcs));
#include <_exports.h>
}
