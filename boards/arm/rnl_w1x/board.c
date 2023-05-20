/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <drivers/gpio.h>

#define RNL_W1X_LED_STRIP_EN_PIN		19
#define RNL_W1X_LED_STRIP_EN_GPIO	gpio0

void board_rnl_w1x_led_strip_enable();
void board_rnl_w1x_led_strip_disable();


static const struct device *led_strip_en;

static int board_rnl_w1x_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	led_strip_en = DEVICE_DT_GET(DT_NODELABEL(RNL_W1X_LED_STRIP_EN_GPIO));

	gpio_pin_configure(led_strip_en, RNL_W1X_LED_STRIP_EN_PIN, GPIO_OUTPUT_INACTIVE);

	return 0;
}

void board_rnl_w1x_led_strip_enable()
{
	gpio_pin_set(led_strip_en, RNL_W1X_LED_STRIP_EN_PIN, 1);
}

void board_rnl_w1x_led_strip_disable()
{
	gpio_pin_set(led_strip_en, RNL_W1X_LED_STRIP_EN_PIN, 0);
}

SYS_INIT(board_rnl_w1x_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
