#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include <custom_led/custom_led.h>

static const struct device *const led = DEVICE_DT_GET(DT_ALIAS(ledr));

int main(void)
{
	if (!device_is_ready(led)) {
		return 0;
	}
	custom_led_set_period(led, 500);
	while (true) {
		struct sensor_value val;

		sensor_sample_fetch(led);
		k_msleep(custom_led_get_period(led));
		sensor_channel_get(led, SENSOR_CHAN_ALL, &val);
		k_msleep(custom_led_get_period(led));
	}
	return 0;
}
