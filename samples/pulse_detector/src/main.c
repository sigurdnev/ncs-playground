/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <logging/log.h>

#include <nrfx_gpiote.h>
#include "hal/nrf_gpio.h"

#include <nrfx_timer.h>
#include <drivers/gpio.h>
#include <helpers/nrfx_gppi.h>
#if defined(DPPI_PRESENT)
#include <nrfx_dppi.h>
#else
#include <nrfx_ppi.h>
#endif

LOG_MODULE_REGISTER(app);

// https://github.com/nrfconnect/sdk-zephyr/blob/master/samples/boards/nrf/nrfx/src/main.c
// https://devzone.nordicsemi.com/f/nordic-q-a/34547/ppi-configuration-to-count-pulses

#define INPUT_PIN 28

#define COUNT_READ_INTERVAL 2000 // Interval in ms to read the counting timer


static const nrfx_timer_t m_timer_count = NRFX_TIMER_INSTANCE(1);

static const nrfx_timer_t m_timer_read  = NRFX_TIMER_INSTANCE(2);

nrf_ppi_channel_t ppi_channel_1, ppi_channel_2;

void timer_handler_count(nrf_timer_event_t event_type, void * p_context)
{

}

void timer_handler_read(nrf_timer_event_t event_type, void * p_context)
{
    uint32_t count = nrfx_timer_capture_get(&m_timer_count, NRF_TIMER_CC_CHANNEL0);
	uint32_t avg_freq = 0;
    if(count > 0)
    {
	   avg_freq = (1000*count)/COUNT_READ_INTERVAL;
	   printk("Average freq: %d [Hz]\n",avg_freq);
    }
	printk("C: %d\n",count);
}

void gpio_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    printk("gpio_handler\n");
}


/**
 * @brief Function for configuring: INPUT_PIN pin for input sensing
 */
static void gpiote_init(void)
{
    nrfx_err_t err;
	
	/* Connect GPIOTE_0 IRQ to nrfx_gpiote_irq_handler */
	//IRQ_CONNECT(DT_IRQN(DT_NODELABEL(gpiote)),
	//	    DT_IRQ(DT_NODELABEL(gpiote), priority),
	//	    nrfx_isr, nrfx_gpiote_irq_handler, 0);
			
	/* Initialize GPIOTE (the interrupt priority passed as the parameter
	 * here is ignored, see nrfx_glue.h).
	 */
	err = nrfx_gpiote_init(0);
	if (err != NRFX_SUCCESS) {
		printk("nrfx_gpiote_init error: %08x", err);
		return;
	}
	
	nrfx_gpiote_in_config_t const in_config = {
		.sense = NRF_GPIOTE_POLARITY_LOTOHI,
		.pull = NRF_GPIO_PIN_NOPULL,
		.is_watcher = false,
		.hi_accuracy = true,
		.skip_gpio_setup = false,
	};

	err = nrfx_gpiote_in_init(INPUT_PIN, &in_config, NULL);
	if (err != NRFX_SUCCESS) {
		printk("nrfx_gpiote_in_init error: %08x", err);
		return;
	}
	
	nrfx_gpiote_in_event_enable(INPUT_PIN, false);
	

}


static void timer_init(void)
{
	// Configure m_timer_count for counting of low to high events on GPIO
	nrfx_err_t          err;
	nrfx_timer_config_t timer_cfg = {
		.mode      = NRF_TIMER_MODE_LOW_POWER_COUNTER,
		.bit_width = NRF_TIMER_BIT_WIDTH_32,
		.p_context = NULL,
	};

	err = nrfx_timer_init(&m_timer_count, &timer_cfg, timer_handler_count);
	if (err != NRFX_SUCCESS) {
		printk("nrfx_timer_init failed with: %d\n", err);
	}
	// Configure m_timer_read for reading the counter timer at a given interval COUNT_READ_INTERVAL
    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
    err = nrfx_timer_init(&m_timer_read, &timer_cfg, timer_handler_read);
    if (err != NRFX_SUCCESS) {
		printk("nrfx_timer_init failed with: %d\n", err);
	}
		
	nrfx_timer_extended_compare(&m_timer_read,
		NRF_TIMER_CC_CHANNEL0,
		nrfx_timer_ms_to_ticks(&m_timer_read, COUNT_READ_INTERVAL),
		NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
		true);
	
	
	IRQ_CONNECT(TIMER1_IRQn, NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
		nrfx_timer_1_irq_handler, NULL, 0);
	IRQ_CONNECT(TIMER2_IRQn, NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
		nrfx_timer_2_irq_handler, NULL, 0);
	
	nrfx_timer_clear(&m_timer_count);
	nrfx_timer_clear(&m_timer_read);
	
	nrfx_timer_enable(&m_timer_count);
	nrfx_timer_enable(&m_timer_read);
		
}

void ppi_init()
{
    nrfx_err_t err;
	
	/* Allocate a (D)PPI channel. */
#if defined(DPPI_PRESENT)
	uint8_t ppi_channel_1;
	uint8_t ppi_channel_2;
	
	err = nrfx_dppi_channel_alloc(&ppi_channel_1);
	err = nrfx_dppi_channel_alloc(&ppi_channel_2);
#else
	nrf_ppi_channel_t ppi_channel_1;
    nrf_ppi_channel_t ppi_channel_2;
	
	err = nrfx_ppi_channel_alloc(&ppi_channel_1);
	err = nrfx_ppi_channel_alloc(&ppi_channel_2);
#endif
	if (err != NRFX_SUCCESS) {
		printk("(D)PPI channel allocation error: %08x", err);
		return;
	}

	
	nrfx_gppi_channel_endpoints_setup(ppi_channel_1,
		nrf_gpiote_event_address_get(NRF_GPIOTE,
			nrfx_gpiote_in_event_get(INPUT_PIN)),
		nrf_timer_task_address_get(m_timer_count.p_reg,
			NRF_TIMER_TASK_COUNT));
			

	nrfx_gppi_channel_endpoints_setup(ppi_channel_2,
		nrf_timer_event_address_get(m_timer_read.p_reg,
			NRF_TIMER_EVENT_COMPARE0),
        nrf_timer_task_address_get(m_timer_count.p_reg, 
			NRF_TIMER_TASK_CAPTURE0));
	
	
    nrfx_gppi_fork_endpoint_setup(ppi_channel_2, nrf_timer_task_address_get(m_timer_count.p_reg,NRF_TIMER_TASK_CLEAR)); // Clear counter timer using (D)PPI.
 
	/* Enable (D)PPI channel. */
#if defined(DPPI_PRESENT)
	err = nrfx_dppi_channel_enable(ppi_channel_1);
	err = nrfx_dppi_channel_enable(ppi_channel_2);
#else
	err = nrfx_ppi_channel_enable(ppi_channel_1);
	err = nrfx_ppi_channel_enable(ppi_channel_2);
#endif
	if (err != NRFX_SUCCESS) {
		printk("Failed to enable (D)PPI channel, error: %08x", err);
		return;
	}
}

// Seq value for 500kHz out
//int16_t buf[] = {(1 << 15) |  12  }; // Inverse polarity (bit 15)

// Seq value for 1MHz out
int16_t buf[] = {(1 << 15) |  6  }; // Inverse polarity (bit 15)

#define PWM_PIN (27UL)
static void bare_metal_pwm_test_start(void)
{
	
  // Start accurate HFCLK (XOSC)
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) ;
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  
  // Configure PWM_PIN as output, and set it to 0
  NRF_GPIO->DIRSET = (1 << PWM_PIN);
  NRF_GPIO->OUTCLR = (1 << PWM_PIN);
  
  
  NRF_PWM0->PRESCALER   = PWM_PRESCALER_PRESCALER_DIV_1; // 1 us
  NRF_PWM0->PSEL.OUT[0] = PWM_PIN;
  NRF_PWM0->MODE        = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
  NRF_PWM0->DECODER     = (PWM_DECODER_LOAD_Common       << PWM_DECODER_LOAD_Pos) | 
                          (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
  NRF_PWM0->LOOP        = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);
  
  //Countertop for 500kHz out
  //NRF_PWM0->COUNTERTOP = 32; 

  
  //Countertop for 1 Mhz out
  NRF_PWM0->COUNTERTOP = 16;
  
  
  NRF_PWM0->SEQ[0].CNT = ((sizeof(buf) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);
  NRF_PWM0->SEQ[0].ENDDELAY = 0;
  NRF_PWM0->SEQ[0].PTR = (uint32_t)&buf[0];
  NRF_PWM0->SEQ[0].REFRESH = 0;
  NRF_PWM0->SHORTS = 0;
  
  NRF_PWM0->ENABLE = 1;
  NRF_PWM0->TASKS_SEQSTART[0] = 1;
}



static const struct device *gpio_dev;

void main(void)
{
	uint32_t err = 0;
	
	gpiote_init();
	
    timer_init();
	
    ppi_init();
    
    printk("Example start\r\n");
	
	//Start PWM output on PWM_PIN for testing. Connect pin 27 and 28 to test.
	bare_metal_pwm_test_start();
}