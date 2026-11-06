#ifndef CUSTOM_LED_CUSTOM_LED_H_
#define CUSTOM_LED_CUSTOM_LED_H_

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/* The driver's sensor API (l6-task2 adds the blink period). */
struct custom_led_driver_api {
	struct sensor_driver_api sensor;
};

#endif /* CUSTOM_LED_CUSTOM_LED_H_ */
