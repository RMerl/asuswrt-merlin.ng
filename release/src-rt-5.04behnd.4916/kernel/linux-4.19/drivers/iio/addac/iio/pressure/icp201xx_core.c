#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/jiffies.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/version.h>

#include "icp201xx.h"
#include "icp201xx_core.h"
#include "icp201xx_i2c.h"

#define ICP201XX_MODE4_CONFIG_HFOSC_EN          0
#define ICP201XX_MODE4_CONFIG_DVDD_EN           0
#define ICP201XX_MODE4_CONFIG_IIR_EN            0
#define ICP201XX_MODE4_CONFIG_FIR_EN            0
#define ICP201XX_MODE4_CONFIG_IIR_K_FACTOR      1


int icp201xx_mode_update(struct icp201xx_state *st, uint8_t mask, uint8_t pos, uint8_t new_value)
{
	int status;
	uint8_t reg_value = 0;
	uint8_t status_value = 0;

	status = icp201xx_reg_read(st, ICP201XX_REG_MODE_SELECT, &reg_value);
	if (status)
		return status;

	do {
		status = icp201xx_reg_read(st, ICP201XX_REG_DEVICE_STATUS, &status_value);

		if ( status )
			return status;

		if (status_value & BIT_DEVICE_STATUS_MODE_SYNC_STATUS_MASK)
			break;

		udelay(500);
	}while(1);

	reg_value = ( reg_value & (~mask) ) | ( new_value << pos) ;
	status = icp201xx_reg_write(st, ICP201XX_REG_MODE_SELECT, reg_value);

	return status;

}

int icp201xx_read_mode4_val(struct icp201xx_state *st, uint8_t reg_addr, uint8_t mask, uint8_t pos, uint8_t *value)
{
	int status;
	uint8_t reg_value = 0;
	status = icp201xx_reg_read(st, reg_addr, &reg_value);

	if(status)
		return status;

	*value = (reg_value & mask) >> pos;

	return status;
}

int icp201xx_get_mode4_config(struct icp201xx_state *st, uint8_t *pres_osr, uint8_t *temp_osr, uint16_t *odr, uint8_t *HFOSC_on,
			      uint8_t *DVDD_on , uint8_t *IIR_filter_en, uint8_t *FIR_filter_en, uint16_t *IIR_k,
			      uint8_t *pres_bs,uint8_t *temp_bs, uint16_t *press_gain)
{
	int status = 0;
	uint8_t temp1,temp2;
	/* OSR */
	status |= icp201xx_reg_read(st, ICP201XX_REG_MODE4_OSR_PRESS, pres_osr);
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_CONFIG1, BIT_MODE4_CONFIG1_OSR_TEMP_MASK, 0, temp_osr);

	/* ODR */
	status |= icp201xx_reg_read(st, ICP201XX_REG_MODE4_ODR_LSB, &temp1);
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_CONFIG2, BIT_MODE4_CONFIG2_ODR_MSB_MASK, 0, &temp2);
	*odr = (uint16_t) ((temp2 << 8) | temp1 );

	/* IIR */
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_CONFIG1, BIT_MODE4_CONFIG1_IIR_EN_MASK, BIT_MODE4_CONFIG1_IIR_EN_POS, IIR_filter_en);
	status |= icp201xx_reg_read(st, ICP201XX_REG_IIR_K_FACTOR_LSB, &temp1);
	status |= icp201xx_reg_read(st, ICP201XX_REG_IIR_K_FACTOR_MSB, &temp2);
	*IIR_k = (uint16_t) ((temp2 << 8) | temp1 );
	/* FIR */
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_CONFIG1, BIT_MODE4_CONFIG1_FIR_EN_MASK, BIT_MODE4_CONFIG1_FIR_EN_POS, FIR_filter_en);

	/* dvdd */
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_CONFIG2, BIT_MODE4_CONFIG2_DVDD_ON_MASK, BIT_MODE4_CONFIG2_DVDD_ON_POS, DVDD_on);

	/* dfosc */
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_CONFIG2, BIT_MODE4_CONFIG2_HFOSC_ON_MASK, BIT_MODE4_CONFIG2_HFOSC_ON_POS, HFOSC_on);

	/* Barrel Shifter */
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_BS_VALUE, BIT_MODE4_BS_VALUE_PRESS, 0, pres_bs);
	status |= icp201xx_read_mode4_val(st, ICP201XX_REG_MODE4_BS_VALUE, BIT_MODE4_BS_VALUE_TEMP, 4, temp_bs);

	/* Gain Factor */
	icp201xx_reg_read(st, ICP201XX_REG_MODE4_PRESS_GAIN_FACTOR_LSB, &temp1);
	icp201xx_reg_read(st, ICP201XX_REG_MODE4_PRESS_GAIN_FACTOR_MSB, &temp2);
	*press_gain = (uint16_t) ((temp2 << 8) | temp1 );	

	return status;
}

int icp201xx_set_mode4_config(struct icp201xx_state *st, uint8_t pres_osr, uint8_t temp_osr, uint16_t odr, uint8_t HFOSC_on, 
			      uint8_t DVDD_on , uint8_t IIR_filter_en, uint8_t FIR_filter_en, uint16_t IIR_k,
			      uint8_t pres_bs,uint8_t temp_bs, uint16_t press_gain)
{
	int status = 0;

	status |= icp201xx_mode_update(st, BIT_POWER_MODE_MASK, BIT_FORCED_POW_MODE_POS, ICP201XX_POWER_MODE_ACTIVE);
	msleep(2);

	/* OSR */
	status |= icp201xx_reg_write(st, ICP201XX_REG_MODE4_OSR_PRESS, pres_osr);
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_CONFIG1, BIT_MODE4_CONFIG1_OSR_TEMP_MASK, 0, temp_osr);

	/* ODR */
	status |= icp201xx_reg_write(st, ICP201XX_REG_MODE4_ODR_LSB, (uint8_t)(0xFF & odr));
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_CONFIG2, BIT_MODE4_CONFIG2_ODR_MSB_MASK, 0, (uint8_t)(0x1F & (odr >> 8))); 

	/* IIR */
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_CONFIG1, BIT_MODE4_CONFIG1_IIR_EN_MASK, BIT_MODE4_CONFIG1_IIR_EN_POS, IIR_filter_en);
	status |= icp201xx_reg_write(st, ICP201XX_REG_IIR_K_FACTOR_LSB, (uint8_t)(IIR_k & 0xFF));
	status |= icp201xx_reg_write(st, ICP201XX_REG_IIR_K_FACTOR_MSB, (uint8_t)((IIR_k >>8) & 0xFF));

	/* FIR */
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_CONFIG1, BIT_MODE4_CONFIG1_FIR_EN_MASK, BIT_MODE4_CONFIG1_FIR_EN_POS, FIR_filter_en);

	/* dvdd */
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_CONFIG2, BIT_MODE4_CONFIG2_DVDD_ON_MASK, BIT_MODE4_CONFIG2_DVDD_ON_POS, DVDD_on);

	/* dfosc */
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_CONFIG2, BIT_MODE4_CONFIG2_HFOSC_ON_MASK, BIT_MODE4_CONFIG2_HFOSC_ON_POS, HFOSC_on);

	/* Barrel Shifter */
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_BS_VALUE, BIT_MODE4_BS_VALUE_PRESS, 0, pres_bs);
	status |= icp201xx_reg_update(st, ICP201XX_REG_MODE4_BS_VALUE, BIT_MODE4_BS_VALUE_TEMP, 4, temp_bs);

	/* Pressure gain factor */
	status |= icp201xx_reg_write(st, ICP201XX_REG_MODE4_PRESS_GAIN_FACTOR_LSB, (uint8_t)( press_gain & 0xFF ));
	status |= icp201xx_reg_write(st, ICP201XX_REG_MODE4_PRESS_GAIN_FACTOR_MSB, (uint8_t)( (press_gain >> 8) & 0xFF ));

	return status;
}

int icp201xx_flush_fifo(struct icp201xx_state *st)
{
	int status;
	uint8_t read_val = 0;

	status = icp201xx_reg_read(st, ICP201XX_REG_FIFO_FILL, &read_val);
	if ( status )
		return status;

	read_val |= 0x80;
	status = icp201xx_reg_write(st, ICP201XX_REG_FIFO_FILL, read_val);

	return status;
}

int icp201xx_get_fifo_count(struct icp201xx_state *st, uint8_t *fifo_cnt)
{
	int status;
	uint8_t read_val = 0;

	status = icp201xx_reg_read(st, ICP201XX_REG_FIFO_FILL, &read_val);
	if ( status )
		return status;

	*fifo_cnt = (uint8_t)( read_val & BIT_FIFO_LEVEL_MASK ) ;
	/*Max value for fifo level is 0x10 for any values higher than 0x10 function sthould return error */
	if (( *fifo_cnt & 0x10 )  && ( *fifo_cnt & 0x0F) )
		status = -1;

	return status;
}
EXPORT_SYMBOL_GPL(icp201xx_get_fifo_count);

int icp201xx_get_fifo_data(struct icp201xx_state *st,uint8_t req_packet_cnt, uint8_t *data)
{
	uint8_t fifo_read_offset = ((st->fifo_readout_mode == ICP201XX_FIFO_READOUT_MODE_PRES_ONLY )||
				    (st->fifo_readout_mode == ICP201XX_FIFO_READOUT_MODE_TEMP_ONLY)) ? 3 : 0;
	uint8_t packet_cnt = req_packet_cnt * 2 * 3 ;

	return icp201xx_reg_read_n(st, (ICP201XX_REG_PRESS_DATA_0 + fifo_read_offset), packet_cnt, data);
}
EXPORT_SYMBOL_GPL(icp201xx_get_fifo_data);

int icp201xx_read_dummy_data(struct icp201xx_state *st)
{
	uint8_t dummy_data;

	return icp201xx_reg_read_n(st, 0x00, 1, &dummy_data);
}
EXPORT_SYMBOL_GPL(icp201xx_read_dummy_data);

int icp201xx_set_press_notification_config(struct icp201xx_state *st, uint8_t press_int_mask,int16_t press_abs , int16_t press_delta)
{
	uint8_t reg_value = 0;
	int status = 0;

	status |= icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_MASK, &reg_value);

	reg_value =  (reg_value |  (ICP201XX_INT_MASK_PRESS_ABS | ICP201XX_INT_MASK_PRESS_DELTA)) & ~press_int_mask;

	status |= icp201xx_reg_write(st, ICP201XX_REG_INTERRUPT_MASK, reg_value);

	if (press_int_mask & ICP201XX_INT_MASK_PRESS_ABS) {
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_ABS_LSB, (uint8_t)(press_abs & 0xff));
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_ABS_MSB, (uint8_t)((press_abs >> 8) & 0xff));
	}
	else {
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_ABS_LSB, 0x00);
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_ABS_MSB, 0x00);
	}

	if (press_int_mask & ICP201XX_INT_MASK_PRESS_DELTA) {
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_DELTA_LSB, (uint8_t)(press_delta & 0xff));
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_DELTA_MSB, (uint8_t)((press_delta >> 8) & 0xff));
	}
	else {
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_DELTA_LSB, 0);
		status |= icp201xx_reg_write(st, ICP201XX_REG_PRESS_DELTA_MSB, 0);
	}

	return status;
}

int icp201xx_get_press_notification_config(struct icp201xx_state *st, uint8_t *press_int_mask,int16_t *press_abs , int16_t *press_delta)
{
	uint8_t reg_value = 0,lsb=0,msb=0;
	int status = 0;

	status |= icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_MASK, &reg_value);
	*press_int_mask = reg_value & (ICP201XX_INT_MASK_PRESS_ABS | ICP201XX_INT_MASK_PRESS_DELTA);
	status |= icp201xx_reg_read(st, ICP201XX_REG_PRESS_ABS_LSB, &lsb);
	status |= icp201xx_reg_read(st, ICP201XX_REG_PRESS_ABS_MSB, &msb);
	*press_abs = (msb << 8) | lsb ;
	status |= icp201xx_reg_read(st, ICP201XX_REG_PRESS_DELTA_LSB, &lsb);
	status |= icp201xx_reg_read(st, ICP201XX_REG_PRESS_DELTA_MSB, &msb);
	*press_delta = (msb << 8) | lsb ;

	return status;
}

int icp201xx_set_fifo_notification_config(struct icp201xx_state *st, uint8_t fifo_int_mask, uint8_t fifo_wmk_high,uint8_t fifo_wmk_low)
{
	uint8_t reg_value = 0;
	int status = 0;

	if ( fifo_wmk_high > 0xf || fifo_wmk_low > 0xf )
		return 0;

	/** FIFO config **/
	reg_value = (fifo_wmk_high << 4) | fifo_wmk_low;
	status |= icp201xx_reg_write(st, ICP201XX_REG_FIFO_CONFIG, reg_value);
	status |= icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_MASK, &reg_value);
	reg_value =  (reg_value |  (ICP201XX_INT_MASK_FIFO_WMK_HIGH | ICP201XX_INT_MASK_FIFO_OVER_FLOW |ICP201XX_INT_MASK_FIFO_WMK_LOW |ICP201XX_INT_MASK_FIFO_UNDER_FLOW)) & ~fifo_int_mask;
	status |= icp201xx_reg_write(st, ICP201XX_REG_INTERRUPT_MASK, reg_value);

	return status;
}

int icp201xx_get_fifo_notification_config(struct icp201xx_state *st,uint8_t *fifo_int_mask, uint8_t *fifo_wmk_high,uint8_t *fifo_wmk_low)
{
	uint8_t reg_value = 0;
	int status = 0;

	status |= icp201xx_reg_read(st, ICP201XX_REG_FIFO_CONFIG, &reg_value);

	*fifo_wmk_low = (reg_value ) & 0x0f ;
	*fifo_wmk_high = (reg_value >> 4) & 0x0f ;

	status |= icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_MASK, &reg_value);

	*fifo_int_mask = reg_value  & (ICP201XX_INT_MASK_FIFO_WMK_HIGH | ICP201XX_INT_MASK_FIFO_OVER_FLOW |ICP201XX_INT_MASK_FIFO_WMK_LOW |ICP201XX_INT_MASK_FIFO_UNDER_FLOW ) ;

	return status;
}

int icp201xx_soft_reset(struct icp201xx_state *st)
{
	int status = 0;
	uint8_t int_status;

	status |= icp201xx_mode_update(st, 0xFF, 0, 0);
	msleep(2);

	status |= icp201xx_flush_fifo(st);
	status |= icp201xx_set_fifo_notification_config(st ,0,0,0);

	status |= icp201xx_reg_write(st, ICP201XX_REG_INTERRUPT_MASK, 0xFF);
	status |= icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_STATUS, &int_status);
	if (int_status )
		status |= icp201xx_reg_write(st, ICP201XX_REG_INTERRUPT_STATUS, int_status);

	return status;
}
EXPORT_SYMBOL_GPL(icp201xx_soft_reset);

int icp201xx_app_pre_start_config(struct icp201xx_state *st, icp201xx_op_mode_t op_mode, icp201xx_meas_mode_t meas_mode, uint16_t odr_hz)
{
	uint8_t pres_bs = 0, temp_bs = 0;
	uint16_t press_gain;
	uint32_t temp1, temp2, press_gain_32bit;
	uint8_t fir_en = 0;
	uint16_t odr_set = 0;
	uint8_t max_idx = 0;
	int i = 0;
	int icp201xx_mode4_config_press_osr_val = 0;

	uint8_t m4_default_pres_osr, m4_default_temp_osr, m4_default_HFOSC_on, m4_default_DVDD_on, m4_default_IIR_filter_en, m4_default_FIR_filter_en,m4_default_pres_bs,m4_default_temp_bs;
	uint16_t m4_default_odr =0,m4_default_IIR_k = 0, m4_default_press_gain =0;
	int pres_bs_cond[] = {8192, 5793, 4096, 2896, 2048, 1448, 1024, 724, 512, 362, 256, 181, 128, 91, 64, 45, 32};

	if (op_mode != ICP201XX_OP_MODE4)
		return -1;

	if (st->version == ICP201XX_VERSION_A)
		max_idx = 8;
	else
		max_idx = 15;

	icp201xx_get_mode4_config(st, &m4_default_pres_osr, &m4_default_temp_osr, &m4_default_odr, &m4_default_HFOSC_on, &m4_default_DVDD_on , 
				  &m4_default_IIR_filter_en, &m4_default_FIR_filter_en, &m4_default_IIR_k,&m4_default_pres_bs,&m4_default_temp_bs,&m4_default_press_gain);

	/** calculate gain factor from default m4 config **/
	icp201xx_mode4_config_press_osr_val = (ICP201XX_MODE4_CONFIG_PRESS_OSR + 1 ) << 5;
	do {
		for (i = max_idx; i >= 0; i--) {
			if (icp201xx_mode4_config_press_osr_val <= pres_bs_cond[i]) {
				pres_bs = i;
				break;
			}
		}
	} while(0);

	if (ICP201XX_MODE4_CONFIG_TEMP_OSR == 0x31) {
		if (st->version == ICP201XX_VERSION_A)
			temp_bs = 6;
		else
			temp_bs = 7;
	} else {
		if (st->version == ICP201XX_VERSION_A)
			temp_bs = 8;
		else
			temp_bs = 9;
	}

	temp1 = (uint32_t)((m4_default_pres_osr + 1) * (m4_default_pres_osr + 1) * (1 << m4_default_pres_bs));
	temp2 = (uint32_t)((ICP201XX_MODE4_CONFIG_PRESS_OSR + 1) * (ICP201XX_MODE4_CONFIG_PRESS_OSR + 1) * (1 << pres_bs));
	press_gain_32bit = ((temp1<<14) / temp2);
	press_gain_32bit *= m4_default_press_gain;
	press_gain_32bit >>= 14;

	press_gain = (uint16_t)press_gain_32bit;

	odr_set = (8000 / odr_hz) - 1;

	// set Mode4 config
	return icp201xx_set_mode4_config(st,
			 ICP201XX_MODE4_CONFIG_PRESS_OSR,
			 ICP201XX_MODE4_CONFIG_TEMP_OSR,
			 odr_set,//ICP201XX_MODE4_CONFIG_ODR_SETTING,
			 ICP201XX_MODE4_CONFIG_HFOSC_EN,
			 ICP201XX_MODE4_CONFIG_DVDD_EN,
			 ICP201XX_MODE4_CONFIG_IIR_EN,
			 fir_en,
			 ICP201XX_MODE4_CONFIG_IIR_K_FACTOR,pres_bs,temp_bs,press_gain);
}

/*ICP201xx warm up.
 *If FIR filter is enabled, it will cause a stettling effect on the first 14 pressure values.
 *Therefore the first 14 pressure output values are discarded.
 **/
static void icp201xx_app_warmup(struct icp201xx_state *st, icp201xx_op_mode_t op_mode, icp201xx_meas_mode_t meas_mode)
{
	uint8_t fifo_packets = 0;
	uint8_t fifo_packets_to_skip = 14;
	uint8_t status;
	uint8_t pres_osr, temp_osr,  HFOSC_on,	DVDD_on , IIR_filter_en, FIR_filter_en,pres_bs,temp_bs;
	uint16_t odr =0,IIR_k = 0, press_gain =0;

	if ( op_mode == ICP201XX_OP_MODE4)
	{
		icp201xx_get_mode4_config(st, &pres_osr, &temp_osr, &odr, &HFOSC_on, &DVDD_on , &IIR_filter_en, &FIR_filter_en, &IIR_k,&pres_bs,&temp_bs,&press_gain);
		if(!FIR_filter_en)
			fifo_packets_to_skip = 1;
	}

	do{
		fifo_packets = 0;
		if ( !icp201xx_get_fifo_count(st, &fifo_packets) && ( fifo_packets >= fifo_packets_to_skip ) ) {
			icp201xx_flush_fifo(st);
			icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_STATUS, &status);
			if (status)
				icp201xx_reg_write(st, ICP201XX_REG_INTERRUPT_STATUS, status); // Clear interrupt status
			break;
		}
		udelay(2);
	} while (1);
}

int icp201xx_config(struct icp201xx_state *st, icp201xx_op_mode_t op_mode, icp201xx_FIFO_readout_mode_t fifo_read_mode)
{
	uint8_t reg_value = 0;
	int status = 0;

	if ( op_mode >= ICP201XX_OP_MODE_MAX )
		return -1;

	status |= icp201xx_mode_update(st, 0xFF, 0, reg_value);
	status |= icp201xx_flush_fifo(st);
	status |= icp201xx_reg_read(st, ICP201XX_REG_INTERRUPT_STATUS, &reg_value);
	if ( reg_value )
		status |= icp201xx_reg_write(st, ICP201XX_REG_INTERRUPT_STATUS, reg_value);

	/** FIFO config **/
	status |= icp201xx_mode_update(st, BIT_MEAS_MODE_MASK, BIT_FORCED_MEAS_MODE_POS, (uint8_t)ICP201XX_MEAS_MODE_CONTINUOUS);
	status |= icp201xx_mode_update(st, BIT_FORCED_MEAS_TRIGGER_MASK, BIT_FORCED_MEAS_TRIGGER_POS, (uint8_t)ICP201XX_FORCE_MEAS_STANDBY);
	status |= icp201xx_mode_update(st, BIT_POWER_MODE_MASK, BIT_FORCED_POW_MODE_POS, ICP201XX_POWER_MODE_NORMAL);
	status |= icp201xx_mode_update(st, BIT_FIFO_READOUT_MODE_MASK, 0, fifo_read_mode);
	status |= icp201xx_mode_update(st, BIT_MEAS_CONFIG_MASK, BIT_MEAS_CONFIG_POS, (uint8_t)op_mode);

	st->fifo_readout_mode = fifo_read_mode;

	return status;
}
EXPORT_SYMBOL_GPL(icp201xx_config);

void inv_run_icp201xx_in_polling(struct icp201xx_state *st, icp201xx_op_mode_t op_mode, uint16_t odr_hz)
{
	int status;

	status = icp201xx_soft_reset(st);
	if ( status )
		pr_info("Soft Reset Error %d",status);

	icp201xx_app_pre_start_config(st,op_mode,ICP201XX_MEAS_MODE_CONTINUOUS, odr_hz);
	/*Configure for polling **/
	icp201xx_set_fifo_notification_config(st, 0 ,0,0);

	status = icp201xx_config(st,op_mode, ICP201XX_FIFO_READOUT_MODE_PRES_TEMP);
	if ( status ) {
		pr_info("ICP201xx config to run %d mode Error %d",op_mode,status);
		return ;
	}

	pr_info("### Starting in Polling Mode for ODR_HZ %d stec Op Mode:%d ###",odr_hz, op_mode);
	icp201xx_app_warmup(st, op_mode,ICP201XX_MEAS_MODE_CONTINUOUS);
	pr_info("Polling Mode Set Done");
}
EXPORT_SYMBOL_GPL(inv_run_icp201xx_in_polling);

static int icp201xx_enable_write_switch_OTP_read(struct icp201xx_state *st)
{
	int status = 0;

	status |= icp201xx_mode_update(st, 0xFF, 0, 0x00);
	msleep(2);

	status |= icp201xx_mode_update(st, 0xFF, 0, BIT_POWER_MODE_MASK);
	msleep(4);

	status |= icp201xx_reg_write(st, ICP201XX_REG_MASTER_LOCK, 0x1f);
	status |= icp201xx_reg_update(st, ICP201XX_REG_OTP_MTP_OTP_CFG1, BIT_OTP_CONFIG1_OTP_ENABLE_MASK, 0, 0x01);
	status |= icp201xx_reg_update(st, ICP201XX_REG_OTP_MTP_OTP_CFG1, BIT_OTP_CONFIG1_WRITE_SWITCH_MASK, BIT_OTP_CONFIG1_WRITE_SWITCH_POS, 0x01);
	udelay(10);

	status |= icp201xx_reg_update(st, ICP201XX_REG_OTP_DEBUG2, BIT_OTP_DBG2_RESET_MASK, BIT_OTP_DBG2_RESET_POS, 0x01);
	udelay(10);

	status |= icp201xx_reg_update(st, ICP201XX_REG_OTP_DEBUG2, BIT_OTP_DBG2_RESET_MASK, BIT_OTP_DBG2_RESET_POS, 0x00);
	udelay(10);

	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_MRA_LSB, 0x04);
	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_MRA_MSB, 0x04);
	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_MRB_LSB, 0x21);
	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_MRB_MSB, 0x20);
	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_MR_LSB, 0x10);
	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_MR_MSB, 0x80);

	return status;
}

static int icp201xx_disable_write_switch_OTP_read(struct icp201xx_state *st)
{
	int status = 0;

	status |= icp201xx_reg_update(st, ICP201XX_REG_OTP_MTP_OTP_CFG1, BIT_OTP_CONFIG1_OTP_ENABLE_MASK, 0, 0x00);
	status |= icp201xx_reg_update(st, ICP201XX_REG_OTP_MTP_OTP_CFG1, BIT_OTP_CONFIG1_WRITE_SWITCH_MASK, BIT_OTP_CONFIG1_WRITE_SWITCH_POS, 0x00);
	status |= icp201xx_mode_update(st, 0xFF, 0, 0x00);

	return status;
}

static int icp201xx_read_otp_data(struct icp201xx_state *st, uint8_t otp_addr, uint8_t *val)
{
	uint8_t otp_status;
	int status = 0;

	icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_OTP_ADDR, otp_addr);
	icp201xx_reg_write(st, ICP201XX_REG_OTP_MTP_OTP_CMD, 0x10);

	do {
		icp201xx_reg_read(st, ICP201XX_REG_OTP_MTP_OTP_STATUS, &otp_status);
		if (status)
			return -1;

		if (otp_status == 0)
			break;

		udelay(1);
	}while(1);

	return icp201xx_reg_read(st, ICP201XX_REG_OTP_MTP_RD_DATA, val);
}

int icp201xx_OTP_bootup_cfg(struct icp201xx_state *st)
{
	int status = 0;
	uint8_t offset = 0, gain = 0,Hfosc = 0;
	icp201xx_enable_write_switch_OTP_read(st);

	status |= icp201xx_read_otp_data(st, 0xF8, &offset);
	status |= icp201xx_read_otp_data(st, 0xF9, &gain);
	status |= icp201xx_read_otp_data(st, 0xFA, &Hfosc);

	/** Updating main reg */
	status |= icp201xx_reg_update(st, ICP201XX_REG_TRIM1_MSB, BIT_PEFE_OFFSET_TRIM_MASK, 0, offset & 0x3F);
	status |= icp201xx_reg_update(st, ICP201XX_REG_TRIM2_MSB, BIT_PEFE_GAIN_TRIM_MASK, BIT_PEFE_GAIN_TRIM_POS, gain & 0x07);
	status |= icp201xx_reg_update(st, ICP201XX_REG_TRIM2_LSB, BIT_HFOSC_OFFSET_TRIM_MASK, 0, Hfosc & 0x7F);

	status |= icp201xx_disable_write_switch_OTP_read(st);
	status |= icp201xx_reg_write(st, ICP201XX_REG_OTP_STATUS2, 0x01);

	return status;
}
EXPORT_SYMBOL_GPL(icp201xx_OTP_bootup_cfg);

MODULE_AUTHOR("Invensense Corporation");
MODULE_DESCRIPTION("Invensense ICP201XX driver");
MODULE_LICENSE("GPL");
