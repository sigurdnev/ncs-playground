#include <stdio.h>
#include <zephyr/kernel.h>

#include <zephyr/irq.h>
#include <nrfx_gpiote.h>
#include <nrfx_timer.h>
#include <nrfx_rtc.h>
#include <helpers/nrfx_gppi.h>

static const nrfx_gpiote_t gpiote_instance = NRFX_GPIOTE_INSTANCE(0);
static const nrfx_rtc_t    rtc_instance = NRFX_RTC_INSTANCE(0);


#define TEST_PIN 4


static uint32_t gpiote_setup(void)
{
	uint8_t gpiote_ch;
	nrfx_err_t err;

	err = nrfx_gpiote_channel_alloc(&gpiote_instance, &gpiote_ch);
	if (err != NRFX_SUCCESS) {
		printf("failed to allocate gpiote\n");
		return 0;
	}

	nrfx_gpiote_task_config_t task_config = {
		.task_ch = gpiote_ch,
		.polarity = NRF_GPIOTE_POLARITY_TOGGLE,
		.init_val = NRF_GPIOTE_INITIAL_VALUE_LOW
	};
	nrfx_gpiote_output_config_t out_config = {
		.drive = NRF_GPIO_PIN_S0S1,
		.input_connect = NRF_GPIO_PIN_INPUT_DISCONNECT,
		.pull = NRF_GPIO_PIN_NOPULL
	};

	err = nrfx_gpiote_output_configure(&gpiote_instance, TEST_PIN, &out_config, &task_config);
	if (err != NRFX_SUCCESS) {
		printf("failed to configure pin\n");
		return 0;
	}

	nrfx_gpiote_out_task_enable(&gpiote_instance, TEST_PIN);

	uint32_t addr = nrfx_gpiote_out_task_address_get(&gpiote_instance, TEST_PIN);

	return addr;
}

static void rtc_isr_handler(nrfx_rtc_int_type_t int_type)
{
	
}


void rtc_setup(void)
{
	nrfx_err_t err;
	nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;
	config.prescaler = 0;

	err = nrfx_rtc_init(&rtc_instance, &config, &rtc_isr_handler);
	if (err != NRFX_SUCCESS) {
		printf("failed to nrfx_rtc_init\n");
	}
	nrfx_rtc_tick_enable(&rtc_instance, false);
}

void rtc_start(void)
{
	nrfx_rtc_enable(&rtc_instance);
}

void setup_reference_timer_ppi(void)
{
	uint32_t addr = gpiote_setup();

	if (addr == 0) {
		printf("gpiote setup failed\n");
		return;
	}

	
	rtc_setup();
	uint32_t evt_addr = nrfx_rtc_event_address_get(&rtc_instance,NRF_RTC_EVENT_TICK );

	uint8_t ch;
	nrfx_err_t err = nrfx_gppi_channel_alloc(&ch);
	if (err != NRFX_SUCCESS) {
		printf("failed to allocate gppi\n");
		return;
	}

	nrfx_gppi_channel_endpoints_setup(ch, evt_addr, addr);

	rtc_start();

	nrfx_gppi_channels_enable(BIT(ch));

}

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);
	setup_reference_timer_ppi();

	return 0;
}
