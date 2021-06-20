/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <drivers/gpio.h>
#include <init.h>
#include <nrf.h>
#include <nrfx.h>

#include <nrfx_qdec.h>


#define QDEC_IRQ QDEC_IRQn  //QDEC0_IRQn for nRF53


static volatile bool m_report_ready_flag = false;
static volatile bool m_first_report_flag = true;
static volatile uint32_t m_accdblread;
static volatile int32_t m_accread;


ISR_DIRECT_DECLARE(nrfx_qdec_isr_handler)
{
	nrfx_qdec_irq_handler();
	ISR_DIRECT_PM(); /* PM done after servicing interrupt for best latency
			  */

	return 1; /* We should check if scheduling decision should be made */
}

static void qdec_nrfx_event_handler(nrfx_qdec_event_t event)
{
	
	unsigned int key;

	switch (event.type) {
	case NRF_QDEC_EVENT_REPORTRDY:
		//accumulate(&qdec_nrfx_data, event.data.report.acc);

		key = irq_lock();
		//handler = qdec_nrfx_data.data_ready_handler;
		irq_unlock(key);
		break;

	default:
		printk("unhandled event (0x%x)", event.type);
		break;
	}
}


void main(void)
{

		IRQ_DIRECT_CONNECT(QDEC_IRQ, 0,
			   nrfx_qdec_isr_handler, 0);
	/* Rest of main */

	static const nrfx_qdec_config_t config = {
		.reportper          = NRF_QDEC_REPORTPER_40,
		.sampleper          = NRF_QDEC_SAMPLEPER_2048us,
		.psela              =  0x10,
		.pselb              = 0x11,
#if DT_INST_NODE_HAS_PROP(0, led_pin)
		.pselled            = DT_INST_PROP(0, led_pin),
#else
		.pselled            = 0xFFFFFFFF, /* disabled */
#endif
		.ledpre             = 0x12,
		.ledpol             = NRF_QDEC_LEPOL_ACTIVE_HIGH,
		.interrupt_priority = NRFX_QDEC_DEFAULT_CONFIG_IRQ_PRIORITY,
		.dbfen              = 0, /* disabled */
		.sample_inten       = 0, /* disabled */
	};

	nrfx_err_t err = nrfx_qdec_init(&config, qdec_nrfx_event_handler);
	if (err != NRFX_SUCCESS) {
		printk("nrfx_gpiote_init error: %08x", err);
		return;
	}

}




