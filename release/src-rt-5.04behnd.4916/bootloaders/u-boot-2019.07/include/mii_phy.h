#ifndef _MII_PHY_H_
#define _MII_PHY_H_

void mii_discover_phy(void);
unsigned short mii_phy_read(unsigned short reg);
void mii_phy_write(unsigned short reg, unsigned short val);

#endif
