/* EEPROM init functions for Cyrus */


void init_eeprom(int bus_num, int addr, int addr_len);
void mac_read_from_fixed_id(void);
int mac_read_from_eeprom_common(void);
