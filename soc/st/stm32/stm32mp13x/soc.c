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

#include "stm32mp13xx_hal.h"

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
	SystemCoreClock = 1000000000U;
	__ASM volatile(
		"MRC     p15, 0, R0, c1, c0, 0                   \n" /* Read CP15 System Control register */
		"BIC     R0, R0, #(0x1 << 30)                    \n" /* Clear TE bit to take exceptions in Thumb mode to fix the DDR init*/
	    	"MCR     p15, 0, R0, c1, c0, 0                   \n" /* Write value back to CP15 System Control register */
		"ISB                                             \n"
		"LDR     R0, =_vector_table                      \n"
		"MCR     p15, 0, R0, c12, c0, 0                  \n"
	);
	return 0;
}
static const struct arm_mmu_region mmu_regions[] = {

	// MMU_REGION_FLAT_ENTRY("GIC", DT_REG_ADDR_BY_IDX(DT_NODELABEL(gic), 0),
	// 		      DT_REG_SIZE_BY_IDX(DT_NODELABEL(gic), 0),
	// 		      MT_DEVICE_nGnRnE | MT_P_RW_U_NA | MT_NS),

	MMU_REGION_FLAT_ENTRY("EXEC", 0xC0000000, 0x20000000,
			    	MPERM_R | MPERM_W | MPERM_X),

	MMU_REGION_FLAT_ENTRY("GIC", 0xA0021000, 0x7000,
			    	MPERM_R | MPERM_W | MPERM_X),

	MMU_REGION_FLAT_ENTRY("STGEN", 0x5C008000, 0x1000,
					MPERM_R | MPERM_W | MPERM_X),

	MMU_REGION_FLAT_ENTRY("GPIOA", 0x50002000, 0x1000,
					MPERM_R | MPERM_W | MPERM_X | MT_DEVICE),
	//MMU_REGION_FLAT_ENTRY("RCC", 0x50000000, 0x1000, MPERM_R | MPERM_W | MPERM_X),
};

const struct arm_mmu_config mmu_config = {
	.num_regions = ARRAY_SIZE(mmu_regions),
	.mmu_regions = mmu_regions,
};

SYS_INIT(stm32a7_init, PRE_KERNEL_1, 0);
