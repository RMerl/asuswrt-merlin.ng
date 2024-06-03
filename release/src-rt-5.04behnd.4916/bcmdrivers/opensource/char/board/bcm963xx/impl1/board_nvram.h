int set_uboot_env_flash(const char *string, int strLen, int offset);
unsigned char *get_uboot_env_area(int current_size);
int get_uboot_env_area_size(void);

#define BRCM_UBOOT_PROP "uboot_env"
