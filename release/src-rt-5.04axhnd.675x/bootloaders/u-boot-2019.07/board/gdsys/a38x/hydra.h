struct ihs_fpga {
	u32 reflection_low;		/* 0x0000 */
	u32 versions;			/* 0x0004 */
	u32 fpga_version;		/* 0x0008 */
	u32 fpga_features;		/* 0x000c */
	u32 reserved0[4];		/* 0x0010 */
	u32 control;			/* 0x0020 */
	u32 reserved1[375];		/* 0x0024 */
	u32 qsgmii_port_state[80];	/* 0x0600 */
};

void print_hydra_version(uint index);
void hydra_initialize(void);
struct ihs_fpga *get_fpga(void);
