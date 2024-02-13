/*! ----------------------------------------------------------------------------
 * @file    rnb_ranging_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2024 (c) RedNodeLabs.
 *
 * All rights reserved.
 *
 * @author RedNodeLabs.
 */

#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <drivers/gpio.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/pm/device.h>

#include "rnb_ranging_spi.h"
#include "rnb_ranging_port.h"

#define LOG_MODULE_NAME rnb_ranging_spi
#if defined(CONFIG_REDNODERANGING_DRIVER_LOG_LEVEL)
#define LOG_LEVEL CONFIG_REDNODERANGING_DRIVER_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_NONE
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define SPI_CFGS_COUNT 2

static const struct device *spi;
static struct spi_config *spi_cfg;
static struct spi_config spi_cfgs[SPI_CFGS_COUNT] = {0};
static bool driver_initialized = false;
static bool spi_initialized = false;
static bool spi_busy = false;

static inline nrf_spi_frequency_t get_nrf_spi_frequency(uint32_t frequency)
{
	/* Get the highest supported frequency not exceeding the requested one.
	 */
	if (frequency < 250000)
	{
		return NRF_SPI_FREQ_125K;
	}
	else if (frequency < 500000)
	{
		return NRF_SPI_FREQ_250K;
	}
	else if (frequency < 1000000)
	{
		return NRF_SPI_FREQ_500K;
	}
	else if (frequency < 2000000)
	{
		return NRF_SPI_FREQ_1M;
	}
	else if (frequency < 4000000)
	{
		return NRF_SPI_FREQ_2M;
	}
	else if (frequency < 8000000)
	{
		return NRF_SPI_FREQ_4M;
	}
	else
	{
		return NRF_SPI_FREQ_8M;
	}
}

static inline nrf_spi_mode_t get_nrf_spi_mode(uint16_t operation)
{
	if (SPI_MODE_GET(operation) & SPI_MODE_CPOL)
	{
		if (SPI_MODE_GET(operation) & SPI_MODE_CPHA)
		{
			return NRF_SPI_MODE_3;
		}
		else
		{
			return NRF_SPI_MODE_2;
		}
	}
	else
	{
		if (SPI_MODE_GET(operation) & SPI_MODE_CPHA)
		{
			return NRF_SPI_MODE_1;
		}
		else
		{
			return NRF_SPI_MODE_0;
		}
	}
}

static inline nrf_spi_bit_order_t get_nrf_spi_bit_order(uint16_t operation)
{
	if (operation & SPI_TRANSFER_LSB)
	{
		return NRF_SPI_BIT_ORDER_LSB_FIRST;
	}
	else
	{
		return NRF_SPI_BIT_ORDER_MSB_FIRST;
	}
}

/*
 *****************************************************************************
 *
 *                              DeviceTree Information
 *
 *****************************************************************************
 */

#if defined(CONFIG_IEEE802154_DW1000)
#define DWM_SPI DT_INST(0, decawave_dw1000)
#elif defined(CONFIG_IEEE802154_DW3000)
#define DWM_SPI DT_INST(0, qorvo_dw3000)
#endif
static const struct gpio_dt_spec dwm_cs = SPI_CS_GPIOS_DT_SPEC_GET(DWM_SPI);

/*
 *****************************************************************************
 *
 *                              SPI section
 *
 *****************************************************************************
 */

int openspi(void)
{
	const struct spi_nrfx_config *dev_config;
	nrfx_spi_config_t config;
	nrfx_err_t result;

	if (spi_busy)
	{
		return -EIO;
	}

	if (!driver_initialized)
	{
		for (uint8_t i = 0; i < SPI_CFGS_COUNT; i++)
		{
			spi_cfgs[i].operation = SPI_WORD_SET(8);
		}

		spi_cfgs[0].frequency = 2000000;
		spi_cfgs[1].frequency = DT_PROP(DWM_SPI, spi_max_frequency);

		spi = DEVICE_DT_GET(DT_PARENT(DWM_SPI));
		if (!spi)
		{
			LOG_ERR("%s: SPI binding failed.\n", __func__);
			return -EIO;
		}

		pm_device_busy_set(spi);

		driver_initialized = true;
	}

	if (spi_initialized)
	{
		LOG_ERR("Failed accessing SPI");
		return -EIO;
	}

	dev_config = spi->config;

	spi_cfg = &spi_cfgs[0];

	config = dev_config->def_config;
	config.frequency = get_nrf_spi_frequency(spi_cfg->frequency);
	config.mode = get_nrf_spi_mode(spi_cfg->operation);
	config.bit_order = get_nrf_spi_bit_order(spi_cfg->operation);

	config.skip_gpio_cfg = true;
	config.skip_psel_cfg = true;
	config.ss_pin = NRFX_SPI_PIN_NOT_USED;

	result = nrfx_spi_init(&dev_config->spi, &config,
			       NULL, NULL);
	if (result != NRFX_SUCCESS)
	{
		LOG_ERR("Failed to initialize nrfx driver: %08x", result);
		return -EIO;
	}

	gpio_pin_configure_dt(&dwm_cs, GPIO_OUTPUT_INACTIVE);
	pinctrl_apply_state(dev_config->pcfg, PINCTRL_STATE_DEFAULT);

	NRFX_IRQ_DISABLE(nrfx_get_irq_number((&dev_config->spi)->p_reg));
	nrf_spi_int_disable((&dev_config->spi)->p_reg, NRF_SPI_ALL_INTS_MASK);

	spi_initialized = true;

	return 0;
}

void set_spi_speed_slow(void)
{
	const struct spi_nrfx_config *dev_config = spi->config;

	if (!spi_initialized || spi_busy)
	{
		LOG_ERR("Failed accessing SPI");
		return;
	}

	if (spi_cfg != &spi_cfgs[0])
	{
		spi_cfg = &spi_cfgs[0];
		nrf_spi_frequency_set((&dev_config->spi)->p_reg, get_nrf_spi_frequency(spi_cfg->frequency));
	}
}

void set_spi_speed_fast(void)
{
	const struct spi_nrfx_config *dev_config = spi->config;

	if (!spi_initialized || spi_busy)
	{
		LOG_ERR("Failed accessing SPI");
		return;
	}

	if (spi_cfg != &spi_cfgs[1])
	{
		spi_cfg = &spi_cfgs[1];
		nrf_spi_frequency_set((&dev_config->spi)->p_reg, get_nrf_spi_frequency(spi_cfg->frequency));
	}
}

int closespi(void)
{
	const struct spi_nrfx_config *dev_config = spi->config;

	if (spi_busy)
	{
		return -EIO;
	}

	if (spi_initialized)
	{
		nrfx_spi_uninit(&dev_config->spi);

		gpio_pin_configure_dt(&dwm_cs, GPIO_DISCONNECTED);
		pinctrl_apply_state(dev_config->pcfg, PINCTRL_STATE_SLEEP);

		spi_initialized = false;
	}

	return 0;
}

int writetospi(uint16_t headerLength,
	       const uint8_t *headerBuffer,
	       uint16_t bodyLength,
	       const uint8_t *bodyBuffer)
{
	const struct spi_nrfx_config *dev_config = spi->config;
	nrfx_spi_xfer_desc_t xfer;
	nrfx_err_t result;
	int ret = 0;

	__disable_irq();

	if (!spi_initialized || spi_busy)
	{
		LOG_ERR("Failed accessing SPI");
		__enable_irq();
		return -EIO;
	}

	spi_busy = true;

	__enable_irq();

	gpio_pin_set_dt(&dwm_cs, true);

	xfer.p_tx_buffer = headerBuffer;
	xfer.tx_length = headerLength;
	xfer.p_rx_buffer = NULL;
	xfer.rx_length = 0;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		LOG_ERR("SPI transfer failed: %08x", result);
		ret = -EIO;
		goto exit;
	}

	xfer.p_tx_buffer = bodyBuffer;
	xfer.tx_length = bodyLength;
	xfer.p_rx_buffer = NULL;
	xfer.rx_length = 0;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		LOG_ERR("SPI transfer failed: %08x", result);
		ret = -EIO;
		goto exit;
	}

exit:
	gpio_pin_set_dt(&dwm_cs, false);
	spi_busy = false;
	return ret;
}

int writetospiwithcrc(uint16_t headerLength,
		      const uint8_t *headerBuffer,
		      uint16_t bodylength,
		      const uint8_t *bodyBuffer,
		      uint8_t crc8)
{
	return 0;
}

int activate_cs()
{
	if (!spi_initialized || spi_busy)
	{
		LOG_ERR("Failed accessing SPI");
		return -EIO;
	}

	gpio_pin_set_dt(&dwm_cs, true);

	return 0;
}

int deactivate_cs()
{
	if (!spi_initialized || spi_busy)
	{
		LOG_ERR("Failed accessing SPI");
		return -EIO;
	}

	gpio_pin_set_dt(&dwm_cs, false);

	return 0;
}

int readfromspi(uint16_t headerLength,
		const uint8_t *headerBuffer,
		uint16_t readLength,
		uint8_t *readBuffer)
{
	const struct spi_nrfx_config *dev_config = spi->config;
	nrfx_spi_xfer_desc_t xfer;
	nrfx_err_t result;
	int ret = 0;

	__disable_irq();

	if (!spi_initialized || spi_busy)
	{
		LOG_ERR("Failed accessing SPI");
		__enable_irq();
		return -EIO;
	}

	spi_busy = true;

	__enable_irq();

	gpio_pin_set_dt(&dwm_cs, true);

	xfer.p_tx_buffer = headerBuffer;
	xfer.tx_length = headerLength;
	xfer.p_rx_buffer = NULL;
	xfer.rx_length = 0;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		LOG_ERR("SPI transfer failed: %08x", result);
		ret = -EIO;
		goto exit;
	}

	xfer.p_tx_buffer = NULL;
	xfer.tx_length = 0;
	xfer.p_rx_buffer = readBuffer;
	xfer.rx_length = readLength;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		LOG_ERR("SPI transfer failed: %08x", result);
		ret = -EIO;
		goto exit;
	}

exit:
	gpio_pin_set_dt(&dwm_cs, false);
	spi_busy = false;
	return ret;
}
