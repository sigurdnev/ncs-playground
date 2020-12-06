/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>


#include <logging/log.h>

LOG_MODULE_REGISTER(pwm_sample, LOG_LEVEL_INF);

static void pwm_init(void)
{
    nrf_drv_pwm_config_t config =
    {
        // These are the common configuration options we use for all PWM
        // instances.
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .count_mode   = NRF_PWM_MODE_UP,
        .step_mode    = NRF_PWM_STEP_AUTO,
    };

    config.base_clock = NRF_PWM_CLK_1MHz;
    config.top_value  = 1000;
    config.load_mode  = NRF_PWM_LOAD_COMMON;

    config.output_pins[0] = PWM_PIN;
    config.output_pins[1] = NRF_DRV_PWM_PIN_NOT_USED;
    config.output_pins[2] = NRF_DRV_PWM_PIN_NOT_USED;
    config.output_pins[3] = NRF_DRV_PWM_PIN_NOT_USED;
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config, NULL));
}

static void start_pwm_demo()
{
    // Sequence 0:   1ms high level, 100ms low level
    static nrf_pwm_values_common_t seq0_values[] =
    {
       0x8000, 0,0x8000
    };

    nrf_pwm_sequence_t const pwm0_seq0 =
    {
        .values.p_common = seq0_values,    
        .length          = NRF_PWM_VALUES_LENGTH(seq0_values),
        .repeats         = 0,
        .end_delay       = 100
    };



    // Sequence 1 : 2ms high, then low.
   static nrf_pwm_values_common_t seq1_values[2] = {0,0};

    nrf_pwm_sequence_t const pwm0_seq1 =
    {
        .values.p_common = seq1_values,
        .length          = NRF_PWM_VALUES_LENGTH(seq1_values),
        .repeats         = 0,
        .end_delay       = 0
    };

    (void)nrf_drv_pwm_complex_playback(&m_pwm0, &pwm0_seq0, &pwm0_seq1, 1,
                                       NRF_DRV_PWM_FLAG_STOP);


}

void main(void)
{
	int err;
	
    /* Connect GPIOTE_0 IRQ to nrfx_gpiote_irq_handler */
	IRQ_CONNECT(DT_IRQN(DT_NODELABEL(gpiote)),
		        DT_IRQ(DT_NODELABEL(gpiote), priority),
		        nrfx_isr, nrfx_gpiote_irq_handler, 0);


	while (1) {

	}
}
