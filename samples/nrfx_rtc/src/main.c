/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */



#include <zephyr.h>
#include <string.h>
#include <stdlib.h>
#include <nrfx_rtc.h>
#include <nrfx_timer.h>

#include <nrfx_gpiote.h>
#include <hal/nrf_gpiote.h>
#include <hal/nrf_timer.h>


#include <logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);


#define LED_0 DT_GPIO_PIN(DT_ALIAS(led0), gpios)
#define LED_1 DT_GPIO_PIN(DT_ALIAS(led1), gpios)

#define COMPARE_COUNTERTIME 3

const nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(0);

static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
	if (int_type == NRFX_RTC_INT_COMPARE0) {
		nrf_gpio_pin_toggle(LED_0);
	} else if (int_type == NRFX_RTC_INT_COMPARE1) {
		nrf_gpio_pin_toggle(LED_1);
		nrfx_rtc_counter_clear(&rtc);
		nrfx_rtc_int_enable(&rtc,NRF_RTC_INT_COMPARE0_MASK |NRF_RTC_INT_COMPARE1_MASK);
	}
}

static void rtc_config(void)
{
	uint32_t err_code;

	nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;
	config.prescaler = 1024;
	err_code = nrfx_rtc_init(&rtc, &config, rtc_handler);
	if (err_code != NRFX_SUCCESS) {
		LOG_ERR("Failure in setup\n");
		return;
	}

	//nrfx_rtc_tick_enable(&rtc, false);
	err_code = nrfx_rtc_cc_set(&rtc, 0, COMPARE_COUNTERTIME * 8, true);
	if (err_code != NRFX_SUCCESS) {
		LOG_ERR("Failure in setup\n");
		return;
	}

	err_code = nrfx_rtc_cc_set(&rtc, 1, COMPARE_COUNTERTIME * 16, true);
	if (err_code != NRFX_SUCCESS) {
		LOG_ERR("Failure in setup\n");
		return;
	}

	nrfx_rtc_enable(&rtc);
}

static void manual_isr_setup()
{
	IRQ_DIRECT_CONNECT(RTC0_IRQn, 0, nrfx_rtc_0_irq_handler, 0);
	irq_enable(RTC0_IRQn);
}

void main(void)
{
	LOG_INF("Starting nrfx rtc sample!\n");
	nrf_gpio_cfg_output(LED_0);
	nrf_gpio_cfg_output(LED_1);
	rtc_config();
	manual_isr_setup();
}