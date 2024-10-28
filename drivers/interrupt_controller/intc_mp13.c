/*
 * Copyright (c) 2018 Marvell
 * Copyright (c) 2018 Lexmark International, Inc.
 * Copyright (c) 2019 Stephanos Ioannidis <root@stephanos.io>
 * Copyright 2024 NXP
 * Copyright (c) 2024 STMicroelectronics
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * NOTE: This driver implements the GICv1 and GICv2 interfaces.
 */

#include <zephyr/device.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/devicetree.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/dt-bindings/interrupt-controller/arm-gic.h>
#include <zephyr/drivers/interrupt_controller/gic.h>
#include <zephyr/sys/barrier.h>


static const uint64_t cpu_mpid_list[] = {
	DT_FOREACH_CHILD_STATUS_OKAY_SEP(DT_PATH(cpus), DT_REG_ADDR, (,))
};

BUILD_ASSERT(ARRAY_SIZE(cpu_mpid_list) >= CONFIG_MP_MAX_NUM_CPUS,
		"The count of CPU Cores nodes in dts is less than CONFIG_MP_MAX_NUM_CPUS\n");

void arm_gic_irq_enable(unsigned int irq)
{

}

void arm_gic_irq_disable(unsigned int irq)
{

}

bool arm_gic_irq_is_enabled(unsigned int irq)
{


	return 0;
}

bool arm_gic_irq_is_pending(unsigned int irq)
{

	return 0;
}

void arm_gic_irq_set_pending(unsigned int irq)
{

}

void arm_gic_irq_clear_pending(unsigned int irq)
{

}

void arm_gic_irq_set_priority(
	unsigned int irq, unsigned int prio, uint32_t flags)
{
}

unsigned int arm_gic_get_active(void)
{
	return 0;
}

void arm_gic_eoi(unsigned int irq)
{
}

void gic_raise_sgi(unsigned int sgi_id, uint64_t target_aff,
		uint16_t target_list)
{
}

static void gic_dist_init(void)
{
}

static void gic_cpu_init(void)
{
	
}

#define GIC_PARENT_IRQ 0
#define GIC_PARENT_IRQ_PRI 0
#define GIC_PARENT_IRQ_FLAGS 0

/**
 * @brief Initialize the GIC device driver
 */
int arm_gic_init(const struct device *dev)
{
	/* Init of Distributor interface registers */
	gic_dist_init();

	/* Init CPU interface registers */
	gic_cpu_init();

	return 0;
}

