#ifndef CUSTOM_LED_CUSTOM_LED_H_
#define CUSTOM_LED_CUSTOM_LED_H_

#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/* Blink period limits, in milliseconds (the `sensor set` shell command enforces them). */
#define CUSTOM_LED_PERIOD_MIN_MS 100
#define CUSTOM_LED_PERIOD_MAX_MS 5000

/* Extension API: the sensor API plus the blink period. */
typedef int (*custom_led_set_period_t)(const struct device *dev, uint32_t period_ms);
typedef uint32_t (*custom_led_get_period_t)(const struct device *dev);

struct custom_led_driver_api {
	struct sensor_driver_api sensor;
	custom_led_set_period_t set_period;
	custom_led_get_period_t get_period;
};

static inline int custom_led_set_period(const struct device *dev, uint32_t period_ms)
{
	const struct custom_led_driver_api *api = (const struct custom_led_driver_api *)dev->api;

	return api->set_period(dev, period_ms);
}

static inline uint32_t custom_led_get_period(const struct device *dev)
{
	const struct custom_led_driver_api *api = (const struct custom_led_driver_api *)dev->api;

	return api->get_period(dev);
}

#endif /* CUSTOM_LED_CUSTOM_LED_H_ */
