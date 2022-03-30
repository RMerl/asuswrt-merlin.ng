#ifndef _EXYNOS5_DT_H_
#define _EXYNOS5_DT_H_

enum {
	EXYNOS5_BOARD_GENERIC,

	EXYNOS5_BOARD_ODROID_XU3,
	EXYNOS5_BOARD_ODROID_XU3_REV01,
	EXYNOS5_BOARD_ODROID_XU3_REV02,
	EXYNOS5_BOARD_ODROID_XU4_REV01,
	EXYNOS5_BOARD_ODROID_HC1_REV01,
	EXYNOS5_BOARD_ODROID_HC2_REV01,
	EXYNOS5_BOARD_ODROID_UNKNOWN,

	EXYNOS5_BOARD_COUNT,
};

struct odroid_rev_info {
	int board_type;
	int board_rev;
	int adc_val;
	const char *name;
};

bool board_is_generic(void);
bool board_is_odroidxu3(void);
bool board_is_odroidxu4(void);
bool board_is_odroidhc1(void);
bool board_is_odroidhc2(void);

#endif
