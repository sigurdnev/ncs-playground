#include <nrf9160.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/i2c.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


#include <drivers/gpio.h>

#define I2C_ACCEL_WRITE_ADDR 0x32
#define I2C_ACCEL_READ_ADDR 0x75

struct device * i2c_accel;
uint8_t WhoAmI = 0u;


#ifdef CONFIG_SOC_NRF9160
#define I2C_DEV "I2C_3"
#else
#define I2C_DEV "I2C_1"
#endif


void main(void)
{
	struct device *i2c_dev;
	
	k_sleep(K_SECONDS(1));

	printk("Starting i2c scanner...\n");

	i2c_dev = device_get_binding(I2C_DEV);
	if (!i2c_dev) {
		printk("I2C: Device driver not found.\n");
		return;
	}
	
	uint8_t error = 0u;
	
	i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD));


	printk("Value of NRF_TWIM3_NS->PSEL.SCL: %d \n",NRF_TWIM3_NS->PSEL.SCL);
	printk("Value of NRF_TWIM3_NS->PSEL.SDA: %d \n",NRF_TWIM3_NS->PSEL.SDA);
	//printk("Value of NRF_TWIM3_NS->FREQUENCY: %d \n",NRF_TWIM3_NS->FREQUENCY);
	//printk("26738688 -> 100k\n");
	
	
	
	for (uint8_t i = 4; i <= 0x77; i++) {
		struct i2c_msg msgs[1];
		uint8_t dst = 1;

		/* Send the address to read from */
		msgs[0].buf = &dst;
		msgs[0].len = 1U;
		msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;
		
		error = i2c_transfer(i2c_dev, &msgs[0], 1, i);
		if (error == 0) {
			printk("0x%2x FOUND\n", i);
		}
		else {
			//printk("error %d \n", error);
		}
		
		
	}
	printk("Scanning done\n");
	

	
	
}