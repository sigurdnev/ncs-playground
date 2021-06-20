/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <drivers/gpio.h>
#include <dk_buttons_and_leds.h>
#include <timing/timing.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

/*
https://docs.zephyrproject.org/latest/reference/timing_functions/index.html
The timing functions can be used to obtain execution time
of a section of code to aid in analysis and optimization.
*/

void main(void)
{
    LOG_INF("Main start");
    timing_t start_time, end_time = 0;
    uint64_t total_cycles = 0;
    uint64_t total_ns = 0;
	uint32_t freq = 0;
	
	freq = timing_freq_get_mhz();
	LOG_INF("Timing results: Clock frequency: %u MHz", freq);
  
    timing_init();
    timing_start();

	start_time = timing_counter_get();

	k_busy_wait(7000);

	end_time = timing_counter_get();
	
	total_cycles = timing_cycles_get(&start_time, &end_time);
    total_ns = timing_cycles_to_ns(total_cycles);
	
	LOG_INF("Timing results: total_cycles: %llu", total_cycles);
	LOG_INF("Timing results: total_ns: %llu", total_ns);
	
	timing_stop();
}