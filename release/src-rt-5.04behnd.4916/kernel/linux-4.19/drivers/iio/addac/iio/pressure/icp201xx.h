#ifndef _INV_ICP201XX_H_
#define _INV_ICP201XX_H_

/********************************************
Register Name: OTP Config 1
Bank         : otp registers
Register Type: READ/WRITE
Register Address: 172 (Decimal); AC (Hex)
********************************************/
#define BIT_OTP_CONFIG1_WRITE_SWITCH_MASK			0x02
#define BIT_OTP_CONFIG1_OTP_ENABLE_MASK				0x01
#define BIT_OTP_CONFIG1_WRITE_SWITCH_POS			1

/********************************************
Register Name: OTP Debug2
Bank         : OTP registers
Register Type: READ/WRITE
Register Address:  180(Decimal); BC (Hex)
********************************************/
#define BIT_OTP_DBG2_RESET_MASK   0x80
#define BIT_OTP_DBG2_RESET_POS	  7

/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 5(Decimal); 5 (Hex)
********************************************/
#define BIT_PEFE_OFFSET_TRIM_MASK			0x3F


/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 6(Decimal); 6 (Hex)
********************************************/
#define BIT_HFOSC_OFFSET_TRIM_MASK			0x7F

/********************************************
Register Name:
Bank         : Main registers
Register Type: READ/WRITE
Register Address: 7(Decimal); 7 (Hex)
********************************************/
#define BIT_PEFE_GAIN_TRIM_MASK			0x70
#define BIT_PEFE_GAIN_TRIM_POS			4

 /********************************************
Register Name: DEVICE_STATUS
Register Type: READ
Register Address: 205 (Decimal); CD (Hex)
********************************************/
#define BIT_DEVICE_STATUS_LP_SEQ_STATE_MASK			0X06
#define BIT_DEVICE_STATUS_MODE_SYNC_STATUS_MASK     0x01
// 0 : Mode sync is going on, MODE_SELECT Reg is NOT accessible.
// 1 : MODE_SELECT Reg is accessible.

enum icp201xx_chip {
	ICP201XX,
};

enum icp201xx_version {
	ICP201XX_VERSION_A,
	ICP201XX_VERSION_B
};

enum icp201xx_mode {
	ICP201XX_MODE_0,
	ICP201XX_MODE_1,
	ICP201XX_MODE_2,
	ICP201XX_MODE_3,
	ICP201XX_MODE_4,
	ICP201XX_MODE_NB,
};

/* ICP 201XX Operation Mode */
typedef enum icp201xx_op_mode {
	ICP201XX_OP_MODE0 = 0 ,  /* Mode 0: Bw:6.25 Hz ODR: 25Hz */
	ICP201XX_OP_MODE1     ,  /* Mode 1: Bw:30 Hz ODR: 120Hz */
	ICP201XX_OP_MODE2     ,  /* Mode 2: Bw:10 Hz ODR: 40Hz */
	ICP201XX_OP_MODE3     ,  /* Mode 3: Bw:0.5 Hz ODR: 2Hz */
	ICP201XX_OP_MODE4     ,  /* Mode 4: User configurable Mode */
	ICP201XX_OP_MODE_MAX
}icp201xx_op_mode_t;

typedef enum icp201xx_forced_meas_trigger {
	ICP201XX_FORCE_MEAS_STANDBY = 0,			/* Stay in Stand by */
	ICP201XX_FORCE_MEAS_TRIGGER_FORCE_MEAS = 1	/* Trigger for forced measurements */
}icp201xx_forced_meas_trigger_t;

typedef enum icp201xx_meas_mode
{
	ICP201XX_MEAS_MODE_FORCED_TRIGGER = 0, /* Force trigger mode based on icp201xx_forced_meas_trigger_t **/
	ICP201XX_MEAS_MODE_CONTINUOUS = 1   /* Continuous measurements based on selected mode ODR settings*/
}icp201xx_meas_mode_t;

typedef enum icp201xx_power_mode
{
	ICP201XX_POWER_MODE_NORMAL = 0,  /* Normal Mode: Device is in standby and goes to active mode during the execution of a measurement */
	ICP201XX_POWER_MODE_ACTIVE = 1   /* Active Mode: Power on DVDD and enable the high frequency clock */
}icp201xx_power_mode_t;


typedef enum icp201xx_FIFO_readout_mode
{
	ICP201XX_FIFO_READOUT_MODE_PRES_TEMP = 0,   /* Pressure and temperature as pair and address wraps to the start address of the Pressure value ( pressure first ) */
	ICP201XX_FIFO_READOUT_MODE_TEMP_ONLY = 1,   /* Temperature only reporting */
	ICP201XX_FIFO_READOUT_MODE_TEMP_PRES = 2,   /* Pressure and temperature as pair and address wraps to the start address of the Temperature value ( Temperature first ) */
	ICP201XX_FIFO_READOUT_MODE_PRES_ONLY = 3    /* Pressure only reporting */
}icp201xx_FIFO_readout_mode_t;


struct icp201xx_state {
	struct i2c_client *client;
	enum icp201xx_chip chip;
	enum icp201xx_version version;
	atomic_t mode;
	struct iio_trigger *trig;
	struct hrtimer timer;
	ktime_t period;
	uint16_t frequency;
	spinlock_t period_lock;
	enum icp201xx_FIFO_readout_mode fifo_readout_mode;
};


#endif
