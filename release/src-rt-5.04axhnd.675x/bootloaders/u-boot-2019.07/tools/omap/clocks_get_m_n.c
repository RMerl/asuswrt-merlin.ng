// SPDX-License-Identifier: GPL-2.0+
/*
 * Program for finding M & N values for DPLLs
 * To be run on Host PC
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#include <stdlib.h>
#include <stdio.h>
typedef unsigned int u32;
#define MAX_N	127

/*
 * get_m_n_optimized() - Finds optimal DPLL multiplier(M) and divider(N)
 * values based on the reference frequency, required output frequency,
 * maximum tolerance for output frequency etc.
 *
 * target_freq_khz - output frequency required in KHz
 * ref_freq_khz - reference(input) frequency in KHz
 * m - pointer to computed M value
 * n - pointer to computed N value
 * tolerance_khz - tolerance for the output frequency. When the algorithm
 * succeeds in finding vialble M and N values the corresponding output
 * frequency will be in the range:
 *	[target_freq_khz - tolerance_khz, target_freq_khz]
 *
 * Formula:
 *	Fdpll = (2 * M * Fref) / (N + 1)
 *
 * Considerations for lock-time:
 *	- Smaller the N, better lock-time, especially lock-time will be
 *	- For acceptable lock-times:
 *		Fref / (M + 1) >= 1 MHz
 *
 * Considerations for power:
 *	- The difference in power for different N values giving the same
 *	  output is negligible. So, we optimize for lock-time
 *
 * Hard-constraints:
 *	- N can not be greater than 127(7 bit field for representing N)
 *
 * Usage:
 *	$ gcc clocks_get_m_n.c
 *	$ ./a.out
 */
int get_m_n_optimized(u32 target_freq_khz, u32 ref_freq_khz, u32 *M, u32 *N)
{
	u32 freq = target_freq_khz;
	u32 m_optimal, n_optimal, freq_optimal = 0, freq_old;
	u32 m, n;
	n = 1;
	while (1) {
		m = target_freq_khz / ref_freq_khz / 2 * n;
		freq_old = 0;
		while (1) {
			freq = ref_freq_khz * 2 * m / n;
			if (freq > target_freq_khz) {
				freq = freq_old;
				m--;
				break;
			}
			m++;
			freq_old = freq;
		}
		if (freq > freq_optimal) {
			freq_optimal = freq;
			m_optimal = m;
			n_optimal = n;
		}
		n++;
		if ((freq_optimal == target_freq_khz) ||
			((ref_freq_khz / n) < 1000)) {
			break;
		}
	}
	n--;
	*M = m_optimal;
	*N = n_optimal - 1;
	printf("ref %d m %d n %d target %d locked %d\n", ref_freq_khz,
		m_optimal, n_optimal - 1, target_freq_khz, freq_optimal);
	return 0;
}

void main(void)
{
	u32 m, n;
	printf("\nMPU - 2000000\n");
	get_m_n_optimized(2000000, 12000, &m, &n);
	get_m_n_optimized(2000000, 13000, &m, &n);
	get_m_n_optimized(2000000, 16800, &m, &n);
	get_m_n_optimized(2000000, 19200, &m, &n);
	get_m_n_optimized(2000000, 26000, &m, &n);
	get_m_n_optimized(2000000, 27000, &m, &n);
	get_m_n_optimized(2000000, 38400, &m, &n);

	printf("\nMPU - 1200000\n");
	get_m_n_optimized(1200000, 12000, &m, &n);
	get_m_n_optimized(1200000, 13000, &m, &n);
	get_m_n_optimized(1200000, 16800, &m, &n);
	get_m_n_optimized(1200000, 19200, &m, &n);
	get_m_n_optimized(1200000, 26000, &m, &n);
	get_m_n_optimized(1200000, 27000, &m, &n);
	get_m_n_optimized(1200000, 38400, &m, &n);

	printf("\nMPU - 1584000\n");
	get_m_n_optimized(1584000, 12000, &m, &n);
	get_m_n_optimized(1584000, 13000, &m, &n);
	get_m_n_optimized(1584000, 16800, &m, &n);
	get_m_n_optimized(1584000, 19200, &m, &n);
	get_m_n_optimized(1584000, 26000, &m, &n);
	get_m_n_optimized(1584000, 27000, &m, &n);
	get_m_n_optimized(1584000, 38400, &m, &n);

	printf("\nCore 1600000\n");
	get_m_n_optimized(1600000, 12000, &m, &n);
	get_m_n_optimized(1600000, 13000, &m, &n);
	get_m_n_optimized(1600000, 16800, &m, &n);
	get_m_n_optimized(1600000, 19200, &m, &n);
	get_m_n_optimized(1600000, 26000, &m, &n);
	get_m_n_optimized(1600000, 27000, &m, &n);
	get_m_n_optimized(1600000, 38400, &m, &n);

	printf("\nPER 1536000\n");
	get_m_n_optimized(1536000, 12000, &m, &n);
	get_m_n_optimized(1536000, 13000, &m, &n);
	get_m_n_optimized(1536000, 16800, &m, &n);
	get_m_n_optimized(1536000, 19200, &m, &n);
	get_m_n_optimized(1536000, 26000, &m, &n);
	get_m_n_optimized(1536000, 27000, &m, &n);
	get_m_n_optimized(1536000, 38400, &m, &n);

	printf("\nIVA 1862000\n");
	get_m_n_optimized(1862000, 12000, &m, &n);
	get_m_n_optimized(1862000, 13000, &m, &n);
	get_m_n_optimized(1862000, 16800, &m, &n);
	get_m_n_optimized(1862000, 19200, &m, &n);
	get_m_n_optimized(1862000, 26000, &m, &n);
	get_m_n_optimized(1862000, 27000, &m, &n);
	get_m_n_optimized(1862000, 38400, &m, &n);

	printf("\nIVA Nitro - 1290000\n");
	get_m_n_optimized(1290000, 12000, &m, &n);
	get_m_n_optimized(1290000, 13000, &m, &n);
	get_m_n_optimized(1290000, 16800, &m, &n);
	get_m_n_optimized(1290000, 19200, &m, &n);
	get_m_n_optimized(1290000, 26000, &m, &n);
	get_m_n_optimized(1290000, 27000, &m, &n);
	get_m_n_optimized(1290000, 38400, &m, &n);

	printf("\nABE 196608 sys clk\n");
	get_m_n_optimized(196608, 12000, &m, &n);
	get_m_n_optimized(196608, 13000, &m, &n);
	get_m_n_optimized(196608, 16800, &m, &n);
	get_m_n_optimized(196608, 19200, &m, &n);
	get_m_n_optimized(196608, 26000, &m, &n);
	get_m_n_optimized(196608, 27000, &m, &n);
	get_m_n_optimized(196608, 38400, &m, &n);

	printf("\nABE 196608 32K\n");
	get_m_n_optimized(196608000/4, 32768, &m, &n);

	printf("\nUSB 1920000\n");
	get_m_n_optimized(1920000, 12000, &m, &n);
	get_m_n_optimized(1920000, 13000, &m, &n);
	get_m_n_optimized(1920000, 16800, &m, &n);
	get_m_n_optimized(1920000, 19200, &m, &n);
	get_m_n_optimized(1920000, 26000, &m, &n);
	get_m_n_optimized(1920000, 27000, &m, &n);
	get_m_n_optimized(1920000, 38400, &m, &n);

	printf("\nCore ES1 1523712\n");
	get_m_n_optimized(1524000, 12000, &m, &n);
	get_m_n_optimized(1524000, 13000, &m, &n);
	get_m_n_optimized(1524000, 16800, &m, &n);
	get_m_n_optimized(1524000, 19200, &m, &n);
	get_m_n_optimized(1524000, 26000, &m, &n);
	get_m_n_optimized(1524000, 27000, &m, &n);

	/* exact recommendation for SDPs */
	get_m_n_optimized(1523712, 38400, &m, &n);

}
