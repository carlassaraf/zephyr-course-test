/* `sensor` shell command for the custom LED sensor (CONFIG_CUSTOM_LED_SHELL). */
#include <stdlib.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/shell/shell.h>

#include <custom_led/custom_led.h>

static const struct device *const led_dev = DEVICE_DT_GET_ONE(course_custom_led);

static int cmd_fetch(const struct shell *sh, size_t argc, char **argv)
{
	int ret = sensor_sample_fetch(led_dev);

	if (ret < 0) {
		shell_error(sh, "sensor_sample_fetch() failed: %d", ret);
		return ret;
	}
	shell_print(sh, "fetched: LED on");
	return 0;
}

static int cmd_read(const struct shell *sh, size_t argc, char **argv)
{
	struct sensor_value val = {0};
	int ret = sensor_channel_get(led_dev, SENSOR_CHAN_ALL, &val);

	if (ret < 0) {
		shell_error(sh, "sensor_channel_get() failed: %d", ret);
		return ret;
	}
	shell_print(sh, "read: %d.%06d (LED off)", val.val1, val.val2);
	return 0;
}

static int cmd_info(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "%s: %s, period %u ms", led_dev->name,
		    device_is_ready(led_dev) ? "ready" : "not ready", custom_led_get_period(led_dev));
	return 0;
}

static int cmd_set(const struct shell *sh, size_t argc, char **argv)
{
	char *end;
	long period = strtol(argv[1], &end, 10);

	if (end == argv[1] || *end != '\0') {
		shell_error(sh, "'%s' is not a number", argv[1]);
		return -EINVAL;
	}
	if (period < CUSTOM_LED_PERIOD_MIN_MS || period > CUSTOM_LED_PERIOD_MAX_MS) {
		shell_error(sh, "%ld is out of range (%d-%d ms)", period, CUSTOM_LED_PERIOD_MIN_MS,
			    CUSTOM_LED_PERIOD_MAX_MS);
		return -EINVAL;
	}
	custom_led_set_period(led_dev, (uint32_t)period);
	shell_print(sh, "period set to %ld ms", period);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
	SHELL_CMD_ARG(fetch, NULL, "Turn the LED on (sensor_sample_fetch)", cmd_fetch, 1, 0),
	SHELL_CMD_ARG(read, NULL, "Turn the LED off (sensor_channel_get)", cmd_read, 1, 0),
	SHELL_CMD_ARG(info, NULL, "Device name and ready state", cmd_info, 1, 0),
	SHELL_CMD_ARG(set, NULL, "Set the blink period: set <100-5000 ms>", cmd_set, 2, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(sensor, &sub_sensor, "Custom LED sensor commands", NULL);
