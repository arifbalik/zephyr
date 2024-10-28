/*
 * Copyright (c) 2018 Foundries.io Ltd
 * Copyright (c) 2024 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <soc.h>
#include <stm32_ll_lptim.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_pwr.h>
#include <stm32_ll_system.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/stm32_clock_control.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>
#include <zephyr/irq.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/pm/policy.h>

#include <zephyr/spinlock.h>

#define DT_DRV_COMPAT st_stm32_lptim

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) > 1
#error Only one LPTIM instance should be enabled
#endif

#define LPTIM (LPTIM_TypeDef *) DT_INST_REG_ADDR(0)

#if DT_INST_NUM_CLOCKS(0) == 1
#warning Kconfig for LPTIM source clock (LSI/LSE) is deprecated, use device tree.
static const struct stm32_pclken lptim_clk[] = {
	STM32_CLOCK_INFO(0, DT_DRV_INST(0)),
	/* Use Kconfig to configure source clocks fields */
	/* Fortunately, values are consistent across enabled series */
#ifdef CONFIG_STM32_LPTIM_CLOCK_LSI
	{.bus = STM32_SRC_LSI, .enr = LPTIM1_SEL(1)}
#else
	{.bus = STM32_SRC_LSE, .enr = LPTIM1_SEL(3)}
#endif
};
#else
// static const struct stm32_pclken lptim_clk[] = STM32_DT_INST_CLOCKS(0);
#endif

//static const struct device *const clk_ctrl = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);

/*
 * Assumptions and limitations:
 *
 * - system clock based on an LPTIM instance, clocked by LSI or LSE
 * - prescaler is set to a 2^value from 1 (division of the LPTIM source clock by 1)
 *   to 128 (division of the LPTIM source clock by 128)
 * - using LPTIM AutoReload capability to trig the IRQ (timeout irq)
 * - when timeout irq occurs the counter is already reset
 * - the maximum timeout duration is reached with the lptim_time_base value
 * - with prescaler of 1, the max timeout (LPTIM_TIMEBASE) is 2 seconds:
 *    0xFFFF / (LSE freq (32768Hz) / 1)
 * - with prescaler of 128, the max timeout (LPTIM_TIMEBASE) is 256 seconds:
 *    0xFFFF / (LSE freq (32768Hz) / 128)
 */

//static int32_t lptim_time_base;
//static uint32_t lptim_clock_freq = CONFIG_STM32_LPTIM_CLOCK;
/* The prescaler given by the DTS and to apply to the lptim_clock_freq */
/* Minimum nb of clock cycles to have to set autoreload register correctly */
#define LPTIM_GUARD_VALUE 2

/* A 32bit value cannot exceed 0xFFFFFFFF/LPTIM_TIMEBASE counting cycles.
 * This is for example about of 65000 x 2000ms when clocked by LSI
 */
static uint32_t accumulated_lptim_cnt;
/* Next autoreload value to set */
//static uint32_t autoreload_next;
/* Indicate if the autoreload register is ready for a write */
//static bool autoreload_ready = true;

static struct k_spinlock lock;

#ifdef CONFIG_STM32_LPTIM_STDBY_TIMER

#define CURRENT_CPU \
	(COND_CODE_1(CONFIG_SMP, (arch_curr_cpu()->id), (_current_cpu->id)))

#define cycle_t uint32_t

/* This local variable indicates that the timeout was set right before
 * entering standby state.
 *
 * It is used for chips that has to use a separate standby timer in such
 * case because the LPTIM is not clocked in some low power mode state.
 */
static bool timeout_stdby;

/* Cycle counter before entering the standby state. */
static cycle_t lptim_cnt_pre_stdby;

/* Standby timer value before entering the standby state. */
static uint32_t stdby_timer_pre_stdby;

/* Standby timer used for timer while entering the standby state */
static const struct device *stdby_timer = DEVICE_DT_GET(DT_CHOSEN(st_lptim_stdby_timer));

#endif /* CONFIG_STM32_LPTIM_STDBY_TIMER */

static inline bool arrm_state_get(void)
{
	return 0;
}

// static void lptim_irq_handler(const struct device *unused)
// {

// }

// static void lptim_set_autoreload(uint32_t arr)
// {

// }

static inline uint32_t z_clock_lptim_getcounter(void)
{return 0;
}

void sys_clock_set_timeout(int32_t ticks, bool idle)
{
	
}

static uint32_t sys_clock_lp_time_get(void)
{
	return 0;
}


uint32_t sys_clock_elapsed(void)
{
return 0;
}

uint32_t sys_clock_cycle_get_32(void)
{
return 0;
}

/* Wait for the IER register of the stm32U5 ready, after any bit write operation */
void stm32_lptim_wait_ready(void)
{
#ifdef CONFIG_SOC_SERIES_STM32U5X
	while (LL_LPTIM_IsActiveFlag_DIEROK(LPTIM) == 0) {
	}
	LL_LPTIM_ClearFlag_DIEROK(LPTIM);
#else
	/* Empty : not relevant */
#endif
}

static int sys_clock_driver_init(void)
{

	return 0;
}

void stm32_clock_control_standby_exit(void)
{
#ifdef CONFIG_STM32_LPTIM_STDBY_TIMER
	if (clock_control_get_status(clk_ctrl,
				     (clock_control_subsys_t) &lptim_clk[0])
				     != CLOCK_CONTROL_STATUS_ON) {
		sys_clock_driver_init();
	}
#endif /* CONFIG_STM32_LPTIM_STDBY_TIMER */
}

void sys_clock_idle_exit(void)
{
#ifdef CONFIG_STM32_LPTIM_STDBY_TIMER
	if (timeout_stdby) {
		cycle_t missed_lptim_cnt;
		uint32_t stdby_timer_diff, stdby_timer_post, dticks;
		uint64_t stdby_timer_us;

		/* Get current value for standby timer and reset LPTIM counter value
		 * to start anew.
		 */
		LL_LPTIM_ResetCounter(LPTIM);
		counter_get_value(stdby_timer, &stdby_timer_post);

		/* Calculate how much time has passed since last measurement for standby timer */
		/* Check IDLE timer overflow */
		if (stdby_timer_pre_stdby > stdby_timer_post) {
			stdby_timer_diff =
				(counter_get_top_value(stdby_timer) - stdby_timer_pre_stdby) +
				stdby_timer_post + 1;

		} else {
			stdby_timer_diff = stdby_timer_post - stdby_timer_pre_stdby;
		}
		stdby_timer_us = counter_ticks_to_us(stdby_timer, stdby_timer_diff);

		/* Convert standby time in LPTIM cnt */
		missed_lptim_cnt = (sys_clock_hw_cycles_per_sec() * stdby_timer_us) /
				   USEC_PER_SEC;
		/* Add the LPTIM cnt pre standby */
		missed_lptim_cnt += lptim_cnt_pre_stdby;

		/* Update the cycle counter to include the cycles missed in standby */
		accumulated_lptim_cnt += missed_lptim_cnt;

		/* Announce the passed ticks to the kernel */
		dticks = (missed_lptim_cnt * CONFIG_SYS_CLOCK_TICKS_PER_SEC)
				/ lptim_clock_freq;
		sys_clock_announce(dticks);

		/* We've already performed all needed operations */
		timeout_stdby = false;
	}
#endif /* CONFIG_STM32_LPTIM_STDBY_TIMER */
}

SYS_INIT(sys_clock_driver_init, PRE_KERNEL_2,
	 CONFIG_SYSTEM_CLOCK_INIT_PRIORITY);
