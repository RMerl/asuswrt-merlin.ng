/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __QCOM_CLK_COMMON_H__
#define __QCOM_CLK_COMMON_H__

struct platform_device;
struct regmap_config;
struct clk_regmap;
struct qcom_reset_map;
struct regmap;
struct freq_tbl;
struct clk_hw;
struct parent_map;

struct qcom_cc_desc {
	const struct regmap_config *config;
	struct clk_regmap **clks;
	size_t num_clks;
	const struct qcom_reset_map *resets;
	size_t num_resets;
};

extern const struct freq_tbl *qcom_find_freq(const struct freq_tbl *f,
					     unsigned long rate);
extern int qcom_find_src_index(struct clk_hw *hw, const struct parent_map *map,
			       u8 src);

extern struct regmap *qcom_cc_map(struct platform_device *pdev,
				  const struct qcom_cc_desc *desc);
extern int qcom_cc_really_probe(struct platform_device *pdev,
				const struct qcom_cc_desc *desc,
				struct regmap *regmap);
extern int qcom_cc_probe(struct platform_device *pdev,
			 const struct qcom_cc_desc *desc);

extern void qcom_cc_remove(struct platform_device *pdev);

#endif
