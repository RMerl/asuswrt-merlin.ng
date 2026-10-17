#ifndef __MPC8308_H_
#define __MPC8308_H_

void setbits_gpio0_out(u32 mask);
void clrbits_gpio0_out(u32 mask);

/* functions to be provided by board implementation */
void mpc8308_init(void);
void mpc8308_set_fpga_reset(unsigned state);
void mpc8308_setup_hw(void);
int mpc8308_get_fpga_done(unsigned fpga);

#endif /* __MPC8308_H_ */
