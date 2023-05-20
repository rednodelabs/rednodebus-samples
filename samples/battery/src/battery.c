/*
 * Copyright (c) 2018-2019 Peter Bigot Consulting, LLC
 * Copyright (c) 2019-2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/zephyr.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "battery.h"

LOG_MODULE_REGISTER(BATTERY, CONFIG_ADC_LOG_LEVEL);

#define VBATT DT_PATH(vbatt)

/* This board uses a divider that reduces max voltage to
 * reference voltage (600 mV).
 */
#define BATTERY_ADC_GAIN ADC_GAIN_1

struct io_channel_config
{
	uint8_t channel;
};

struct divider_config
{
	struct io_channel_config io_channel;
	struct gpio_dt_spec power_gpios;
	/* output_ohm is used as a flag value: if it is nonzero then
	 * the battery is measured through a voltage divider;
	 * otherwise it is assumed to be directly connected to Vdd.
	 */
	uint32_t output_ohm;
	uint32_t full_ohm;
};

static const struct divider_config divider_config = {
    .io_channel = {
	DT_IO_CHANNELS_INPUT(VBATT),
    },
    .power_gpios = GPIO_DT_SPEC_GET_OR(VBATT, power_gpios, {}),
    .output_ohm = DT_PROP(VBATT, output_ohms),
    .full_ohm = DT_PROP(VBATT, full_ohms),
};

struct divider_data
{
	const struct device *adc;
	struct adc_channel_cfg adc_cfg;
	struct adc_sequence adc_seq;
	int16_t raw;
};
static struct divider_data divider_data = {
    .adc = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(VBATT)),
};

static const struct gpio_dt_spec batt_stat = GPIO_DT_SPEC_GET_OR(DT_ALIAS(battery_stat), gpios, {0});
static const struct gpio_dt_spec pwr_good = GPIO_DT_SPEC_GET_OR(DT_ALIAS(power_good), gpios, {0});
static struct gpio_dt_spec batt_stat_led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2_blue), gpios, {0});
static struct gpio_callback batt_stat_cb_data;
static struct gpio_callback pwr_good_cb_data;
static bool battery_ok;

void batt_stat_changed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int batt_stat_val = gpio_pin_get_dt(&batt_stat);
	int pwr_good_val = gpio_pin_get_dt(&pwr_good);

	if (batt_stat_led.port && batt_stat_val >= 0 && pwr_good_val >= 0)
	{
		if (pwr_good_val == 0)
		{
			// OFF
			gpio_pin_set_dt(&batt_stat_led, 0);
		}
		else
		{
			gpio_pin_set_dt(&batt_stat_led, batt_stat_val);
		}
	}
}

void pwr_good_changed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int val = gpio_pin_get_dt(&pwr_good);

	LOG_INF("power good changed %d", val);
}

static int divider_setup(void)
{
	const struct divider_config *cfg = &divider_config;
	const struct io_channel_config *iocp = &cfg->io_channel;
	const struct gpio_dt_spec *gcp = &cfg->power_gpios;
	struct divider_data *ddp = &divider_data;
	struct adc_sequence *asp = &ddp->adc_seq;
	struct adc_channel_cfg *accp = &ddp->adc_cfg;
	int rc;

	if (!device_is_ready(ddp->adc))
	{
		LOG_ERR("ADC device is not ready %s", ddp->adc->name);
		return -ENOENT;
	}

	if (gcp->port)
	{
		if (!device_is_ready(gcp->port))
		{
			LOG_ERR("%s: device not ready", gcp->port->name);
			return -ENOENT;
		}
		rc = gpio_pin_configure_dt(gcp, GPIO_OUTPUT_INACTIVE);
		if (rc != 0)
		{
			LOG_ERR("Failed to control feed %s.%u: %d",
				gcp->port->name, gcp->pin, rc);
			return rc;
		}
	}

	*asp = (struct adc_sequence){
	    .channels = BIT(0),
	    .buffer = &ddp->raw,
	    .buffer_size = sizeof(ddp->raw),
	    .oversampling = 8,
	    .calibrate = true,
	};

#ifdef CONFIG_ADC_NRFX_SAADC
	*accp = (struct adc_channel_cfg){
	    .gain = BATTERY_ADC_GAIN,
	    .reference = ADC_REF_INTERNAL,
	    .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
	};

	if (cfg->output_ohm != 0)
	{
		accp->input_positive = SAADC_CH_PSELP_PSELP_AnalogInput0 + iocp->channel;
	}
	else
	{
		accp->input_positive = SAADC_CH_PSELP_PSELP_VDD;
	}

	asp->resolution = 14;
#else /* CONFIG_ADC_var */
#error Unsupported ADC
#endif /* CONFIG_ADC_var */

	rc = adc_channel_setup(ddp->adc, accp);
	LOG_INF("Setup AIN%u got %d", iocp->channel, rc);

	return rc;
}

static int batt_stat_setup(void)
{
	int ret;

	if (!device_is_ready(batt_stat.port))
	{
		LOG_ERR("Error: BATTERY STAT device %s is not ready\n",
			batt_stat.port->name);
		return -ENOENT;
	}

	ret = gpio_pin_configure_dt(&batt_stat, GPIO_INPUT);
	if (ret != 0)
	{
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
			ret, batt_stat.port->name, batt_stat.pin);
		return -ENOENT;
	}

	ret = gpio_pin_interrupt_configure_dt(&batt_stat, GPIO_INT_EDGE_BOTH);
	if (ret != 0)
	{
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, batt_stat.port->name, batt_stat.pin);
		return ret;
	}

	gpio_init_callback(&batt_stat_cb_data, batt_stat_changed, BIT(batt_stat.pin));
	gpio_add_callback(batt_stat.port, &batt_stat_cb_data);

	if (batt_stat_led.port && !device_is_ready(batt_stat_led.port))
	{
		LOG_ERR("Error %d: BATTERY STAT LED device %s is not ready; ignoring it\n",
			ret, batt_stat_led.port->name);
		batt_stat_led.port = NULL;
	}
	if (batt_stat_led.port)
	{
		ret = gpio_pin_configure_dt(&batt_stat_led, GPIO_OUTPUT);
		if (ret != 0)
		{
			LOG_ERR("Error %d: failed to configure BATTERY STAT LED device %s pin %d\n",
				ret, batt_stat_led.port->name, batt_stat_led.pin);
			batt_stat_led.port = NULL;
		}
		else
		{
			batt_stat_changed(NULL, NULL, NULL);

			LOG_INF("Set up BATTERY STAT LED at %s pin %d\n", batt_stat_led.port->name, batt_stat_led.pin);
		}
	}

	return ret;
}

static int pwr_good_setup(void)
{
	int ret;

	if (!device_is_ready(pwr_good.port))
	{
		LOG_ERR("Error: POWER GOOD device %s is not ready\n",
			pwr_good.port->name);
		return -ENOENT;
	}

	ret = gpio_pin_configure_dt(&pwr_good, GPIO_INPUT);
	if (ret != 0)
	{
		LOG_ERR("Error %d: failed to configure %s pin %d\n",
			ret, pwr_good.port->name, pwr_good.pin);
		return -ENOENT;
	}

	ret = gpio_pin_interrupt_configure_dt(&pwr_good, GPIO_INT_EDGE_BOTH);
	if (ret != 0)
	{
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, pwr_good.port->name, pwr_good.pin);
		return ret;
	}

	gpio_init_callback(&pwr_good_cb_data, pwr_good_changed, BIT(pwr_good.pin));
	gpio_add_callback(pwr_good.port, &pwr_good_cb_data);

	return ret;
}

static int battery_setup(const struct device *arg)
{
	int ret = divider_setup();

	battery_ok = (ret == 0);
	LOG_INF("Battery setup: %d %d", ret, battery_ok);

	if (ret != 0)
	{
		return ret;
	}

	ret = pwr_good_setup();

	if (ret != 0)
	{
		return ret;
	}

	ret = batt_stat_setup();

	return ret;
}

SYS_INIT(battery_setup, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

int battery_measure_enable(bool enable)
{
	int rc = -ENOENT;

	if (battery_ok)
	{
		const struct gpio_dt_spec *gcp = &divider_config.power_gpios;

		rc = 0;
		if (gcp->port)
		{
			rc = gpio_pin_set_dt(gcp, enable);
		}
	}
	return rc;
}

int battery_sample(void)
{
	int rc = -ENOENT;

	if (battery_ok)
	{
		struct divider_data *ddp = &divider_data;
		const struct divider_config *dcp = &divider_config;
		struct adc_sequence *sp = &ddp->adc_seq;

		rc = adc_read(ddp->adc, sp);
		sp->calibrate = false;
		if (rc == 0)
		{
			int32_t val = ddp->raw;

			adc_raw_to_millivolts(adc_ref_internal(ddp->adc),
					      ddp->adc_cfg.gain,
					      sp->resolution,
					      &val);

			if (dcp->output_ohm != 0)
			{
				rc = val * (uint64_t)dcp->full_ohm / dcp->output_ohm;
				LOG_INF("raw %u ~ %u mV => %d mV\n",
					ddp->raw, val, rc);
			}
			else
			{
				rc = val;
				LOG_INF("raw %u ~ %u mV\n", ddp->raw, val);
			}
		}
	}

	return rc;
}

unsigned int battery_level_pptt(unsigned int batt_mV,
				const struct battery_level_point *curve)
{
	const struct battery_level_point *pb = curve;

	if (batt_mV >= pb->lvl_mV)
	{
		/* Measured voltage above highest point, cap at maximum. */
		return pb->lvl_pptt;
	}
	/* Go down to the last point at or below the measured voltage. */
	while ((pb->lvl_pptt > 0) && (batt_mV < pb->lvl_mV))
	{
		++pb;
	}
	if (batt_mV < pb->lvl_mV)
	{
		/* Below lowest point, cap at minimum */
		return pb->lvl_pptt;
	}

	/* Linear interpolation between below and above points. */
	const struct battery_level_point *pa = pb - 1;

	return pb->lvl_pptt + ((pa->lvl_pptt - pb->lvl_pptt) * (batt_mV - pb->lvl_mV) / (pa->lvl_mV - pb->lvl_mV));
}
