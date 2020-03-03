/*
 * <:label-BRCM:2012:NONE:standard
 *
 * :>
 * */
#ifndef __BCM63xx_POTP_SOTP_UTIL__
#define  __BCM63xx_POTP_SOTP_UTIL__
/**********************************************************************
 *  static int read_potp(uint32_t row, uint32_t *pRval)
 *  
 *  Input parameters: 
 *      row   - row number of potp to read
 *      pRval - pointer to uint32_t container to be loaded with data
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int read_potp(uint32_t row, uint32_t *pRval);

/**********************************************************************
 *  static int burn_potp(uint32_t row, uint32_t val)
 *  
 *  Input parameters: 
 *      row   - row number of potp to fuse
 *      pRval - pointer to uint32_t data containing bits to fuse
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int burn_potp(uint32_t row, uint32_t val);

/**********************************************************************
 *  static int burn_potp_mid(uint16_t mid)
 *  
 *  Input parameters: 
 *      mid - 16 bit market identifier to be fused
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int burn_potp_mid(uint16_t mid);


/**********************************************************************
 *  static int read_sotp(uint32_t section)
 *  
 *  Input parameters: 
 *      section - section of sotp to be read
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int read_sotp(uint32_t section);


/**********************************************************************
 *  static int burn_sotp(uint32_t section, uint32_t *sotpKey)
 *  
 *  Input parameters: 
 *      section - section of sotp to be fused
 *      sotpKey - pointer to array of uint32 words to be fused
 *      
 *  Return value:
 *      0 - everything worked as expected
 *      1 - not good
 *
 ********************************************************************* */
int burn_sotp(uint32_t section, uint32_t *sotpKey);
int potp_set_brom_mode(void);
int potp_set_brom_opmode(void);
int potp_set_oid(uint16_t oid);
int ui_init_otp_cmds(void);
int read_potp_oid(uint16_t *oid);
int read_potp_mid(uint16_t *mid);

#endif
