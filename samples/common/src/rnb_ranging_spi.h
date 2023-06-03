/*! ----------------------------------------------------------------------------
 * @file	rnb_ranging_spi.h
 * @brief	SPI access functions
 *
 * @attention
 *
 * Copyright 2023 (c) RedNodeLabs.
 *
 * All rights reserved.
 *
 * @author RedNodeLabs.
 */

#ifndef _RNB_RANGING_SPI_H_
#define _RNB_RANGING_SPI_H_

#include <nrfx_spi.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct spi_nrfx_config
{
	nrfx_spi_t spi;
	nrfx_spi_config_t def_config;
	void (*irq_connect)(void);
	const struct pinctrl_dev_config *pcfg;
};

	int openspi(void);
	int closespi(void);

	void set_spi_speed_slow();
	void set_spi_speed_fast();

	int readfromspi(uint16_t headerLength,
			const uint8_t *headerBuffer,
			uint16_t readLength,
			uint8_t *readBuffer);

#ifdef __cplusplus
}
#endif

#endif /* _RNB_RANGING_SPI_H_ */
