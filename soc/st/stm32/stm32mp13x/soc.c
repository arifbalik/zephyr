/*
 * Copyright (c) 2019 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System/hardware module for STM32MP13 processor
 */

#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <soc.h>
#include <stm32_ll_bus.h>

#include <cmsis_core.h>

/**
 * @brief Perform basic hardware initialization at boot.
 *
 * This needs to be run from the very beginning.
 * So the init priority has to be 0 (zero).
 *
 * @return 0
 */
static int stm32a7_init(void)
{
	/*HW semaphore Clock enable*/
	/* Update CMSIS SystemCoreClock variable (HCLK) */
	SystemCoreClock = 209000000;

	return 0;
}
static const struct arm_mmu_region mmu_regions[] = {

	// MMU_REGION_FLAT_ENTRY("GIC", DT_REG_ADDR_BY_IDX(DT_NODELABEL(gic), 0),
	// 		      DT_REG_SIZE_BY_IDX(DT_NODELABEL(gic), 0),
	// 		      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("GIC", 0xC0000000, 0x20000000,
			      MPERM_R | MPERM_W | MPERM_X)

	// MMU_REGION_DT_COMPAT_FOREACH_FLAT_ENTRY(nxp_mbox_imx_mu,
	// 					(MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS))

	// 	MMU_REGION_DT_COMPAT_FOREACH_FLAT_ENTRY(nxp_kinetis_lpuart,
	// 						(MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS))

};

const struct arm_mmu_config mmu_config = {
	.num_regions = ARRAY_SIZE(mmu_regions),
	.mmu_regions = mmu_regions,
};

SYS_INIT(stm32a7_init, PRE_KERNEL_1, 0);
