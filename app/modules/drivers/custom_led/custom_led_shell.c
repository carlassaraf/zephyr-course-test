/* `sensor` shell command for the custom LED sensor (CONFIG_CUSTOM_LED_SHELL). */
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
	shell_print(sh, "%s: %s", led_dev->name, device_is_ready(led_dev) ? "ready" : "not ready");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
	SHELL_CMD_ARG(fetch, NULL, "Turn the LED on (sensor_sample_fetch)", cmd_fetch, 1, 0),
	SHELL_CMD_ARG(read, NULL, "Turn the LED off (sensor_channel_get)", cmd_read, 1, 0),
	SHELL_CMD_ARG(info, NULL, "Device name and ready state", cmd_info, 1, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(sensor, &sub_sensor, "Custom LED sensor commands", NULL);
