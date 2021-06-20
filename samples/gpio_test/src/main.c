/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>
#include <logging/log.h>
#include <drivers/gpio.h>
#include <modem/lte_lc.h>

LOG_MODULE_REGISTER(app);

static const struct device *gpio_dev;
static struct gpio_callback gpio_cb;

#define TEST_PIN 28

static void gpio_callback(const struct device *dev,
		     struct gpio_callback *gpio_cb, uint32_t pins)
{
	//k_work_submit_to_queue(&slm_work_q, &exit_idle_work);
	printk("gpio_callback");

	
}

void gpio_setup(void)
{
	int err;

	gpio_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	if (gpio_dev == NULL) {
		printk("GPIO_0 bind error");
		return;
	}
	err = gpio_pin_configure(gpio_dev, TEST_PIN,
				GPIO_INPUT | GPIO_PULL_UP);
	if (err) {
		printk("GPIO_0 config error: %d", err);
		return;
	}
	gpio_init_callback(&gpio_cb, gpio_callback,
			BIT(TEST_PIN));
	err = gpio_add_callback(gpio_dev, &gpio_cb);
	if (err) {
		printk("GPIO_0 add callback error: %d", err);
		return;
	}
	err = gpio_pin_interrupt_configure(gpio_dev, TEST_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (err) {
		printk("GPIO_0 enable callback error: %d", err);
	}
	
	printk("exit gpio_setup");
}

#define I2C_DEV "I2C_3"

void main(void)
{
  int err;
  
  lte_lc_power_off();
 
 
  printk("main");
 
  gpio_setup();
  
  static const struct device *i2c_dev;
  
  i2c_dev = device_get_binding(I2C_DEV);
  
  		struct i2c_msg msgs[1];
		uint8_t dst = 1;

		/* Send the address to read from */
		msgs[0].buf = &dst;
		msgs[0].len = 1U;
		msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;
		
		err = i2c_transfer(i2c_dev, &msgs[0], 1, 5);
 
 
 while(false)
 {
	k_sleep(K_MSEC(100000));
 }
 //NRF_REGULATORS->SYSTEMOFF = 1;
}
