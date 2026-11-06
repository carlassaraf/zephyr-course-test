#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

static const struct device *const led = DEVICE_DT_GET(DT_ALIAS(ledr));

int main(void)
{
	if (!device_is_ready(led)) {
		return 0;
	}
	while (true) {
		struct sensor_value val;

		sensor_sample_fetch(led);
		k_msleep(500);
		sensor_channel_get(led, SENSOR_CHAN_ALL, &val);
		k_msleep(500);
	}
	return 0;
}
