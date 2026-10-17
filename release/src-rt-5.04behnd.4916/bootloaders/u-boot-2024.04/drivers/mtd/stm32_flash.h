struct stm32_flash_regs {
	u32 acr;
	u32 key;
	u32 optkeyr;
	u32 sr;
	u32 cr;
	u32 optcr;
	u32 optcr1;
};

#define STM32_FLASH_KEY1	0x45670123
#define STM32_FLASH_KEY2	0xCDEF89AB

#define STM32_FLASH_SR_BSY		(1 << 16)

#define STM32_FLASH_CR_PG		(1 << 0)
#define STM32_FLASH_CR_SER		(1 << 1)
#define STM32_FLASH_CR_STRT		(1 << 16)
#define STM32_FLASH_CR_LOCK		(1 << 31)
#define STM32_FLASH_CR_SNB_OFFSET	3
#define STM32_FLASH_CR_SNB_MASK		(15 << STM32_FLASH_CR_SNB_OFFSET)

/* Flash ACR: Access control register */
#define FLASH_ACR_WS(n)		n
#define FLASH_ACR_PRFTEN	(1 << 8)
#define FLASH_ACR_ICEN		(1 << 9)
#define FLASH_ACR_DCEN		(1 << 10)
