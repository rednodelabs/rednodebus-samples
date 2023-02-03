/*! ----------------------------------------------------------------------------
 * @file    rnb_ranging_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2023 (c) RedNodeLabs.
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
#include <nrfx_spi.h>
#include <zephyr/drivers/pinctrl.h>
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
static bool spi_initialized;

struct spi_nrfx_config {
	nrfx_spi_t	  spi;
	nrfx_spi_config_t def_config;
	void (*irq_connect)(void);
#ifdef CONFIG_PINCTRL
	const struct pinctrl_dev_config *pcfg;
#endif
};

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

//static struct spi_cs_control cs_ctrl;

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
 *                              DW3000 SPI section
 *
 *****************************************************************************
 */

int update_spi_config(void)
{
	const struct spi_nrfx_config *dev_config = spi->config;
	nrfx_spi_config_t config;
	nrfx_err_t result;

	pinctrl_apply_state(dev_config->pcfg, PINCTRL_STATE_DEFAULT);

	config = dev_config->def_config;
	config.frequency = get_nrf_spi_frequency(spi_cfg->frequency);
	config.mode = get_nrf_spi_mode(spi_cfg->operation);
	config.bit_order = get_nrf_spi_bit_order(spi_cfg->operation);

	if (spi_initialized)
	{
		nrfx_spi_uninit(&dev_config->spi);
		spi_initialized = false;
	}

	result = nrfx_spi_init(&dev_config->spi, &config,
			       NULL, NULL);
	if (result != NRFX_SUCCESS)
	{
		LOG_ERR("Failed to initialize nrfx driver: %08x", result);
		return -EIO;
	}

	spi_initialized = true;

	return 0;
}

/*
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(void)
{
	const struct spi_nrfx_config *dev_config;

	gpio_pin_configure_dt(&dwm_cs, GPIO_OUTPUT_INACTIVE);

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

	set_spi_speed_slow();

	dev_config = spi->config;
	NRFX_IRQ_DISABLE(nrfx_get_irq_number((&dev_config->spi)->p_reg));
        nrf_spi_int_disable((&dev_config->spi)->p_reg, NRF_SPI_ALL_INTS_MASK);

	return 0;
}

void set_spi_speed_slow(void)
{
	if (spi_cfg != &spi_cfgs[0])
	{
		spi_cfg = &spi_cfgs[0];
		update_spi_config();
	}
}

void set_spi_speed_fast(void)
{
	if (spi_cfg != &spi_cfgs[1])
	{
		spi_cfg = &spi_cfgs[1];
		update_spi_config();
	}
}

/*
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
	const struct spi_nrfx_config *dev_config = spi->config;

	spi_cfg = NULL;

	if (spi_initialized)
	{
		nrfx_spi_uninit(&dev_config->spi);
		spi_initialized = false;
	}

	gpio_pin_configure_dt(&dwm_cs, GPIO_DISCONNECTED);
	pinctrl_apply_state(dev_config->pcfg, PINCTRL_STATE_SLEEP);

	return 0;
}

/*
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success
 */
int writetospi(uint16_t headerLength,
	       const uint8_t *headerBuffer,
	       uint16_t bodyLength,
	       const uint8_t *bodyBuffer)
{
	const struct spi_nrfx_config *dev_config = spi->config;
	nrfx_spi_xfer_desc_t xfer;
	nrfx_err_t result;

	gpio_pin_set_dt(&dwm_cs, true);

	xfer.p_tx_buffer = headerBuffer;
	xfer.tx_length = headerLength;
	xfer.p_rx_buffer = NULL;
	xfer.rx_length = 0;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		gpio_pin_set_dt(&dwm_cs, false);
		LOG_ERR("SPI transfer failed: %08x", result);
		return -EIO;
	}

	xfer.p_tx_buffer = bodyBuffer;
	xfer.tx_length = bodyLength;
	xfer.p_rx_buffer = NULL;
	xfer.rx_length = 0;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		gpio_pin_set_dt(&dwm_cs, false);
		LOG_ERR("SPI transfer failed: %08x", result);
		return -EIO;
	}

	gpio_pin_set_dt(&dwm_cs, false);

	return 0;
}

int writetospiwithcrc(uint16_t headerLength,
		      const uint8_t *headerBuffer,
		      uint16_t bodylength,
		      const uint8_t *bodyBuffer,
		      uint8_t crc8)
{
	return 0;
}

/*
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data
 * may be found, or returns 0
 */
int readfromspi(uint16_t headerLength,
		const uint8_t *headerBuffer,
		uint16_t readLength,
		uint8_t *readBuffer)
{
	const struct spi_nrfx_config *dev_config = spi->config;
	nrfx_spi_xfer_desc_t xfer;
	nrfx_err_t result;

    	gpio_pin_set_dt(&dwm_cs, true);

	xfer.p_tx_buffer = headerBuffer;
	xfer.tx_length = headerLength;
	xfer.p_rx_buffer = NULL;
	xfer.rx_length = 0;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		gpio_pin_set_dt(&dwm_cs, false);
		LOG_ERR("SPI transfer failed: %08x", result);
		return -EIO;
	}

	xfer.p_tx_buffer = NULL;
	xfer.tx_length = 0;
	xfer.p_rx_buffer = readBuffer;
	xfer.rx_length = readLength;
	result = nrfx_spi_xfer(&dev_config->spi, &xfer, 0);

	if (result != NRFX_SUCCESS)
	{
		gpio_pin_set_dt(&dwm_cs, false);
		LOG_ERR("SPI transfer failed: %08x", result);
		return -EIO;
	}

	gpio_pin_set_dt(&dwm_cs, false);

	return 0;
}
