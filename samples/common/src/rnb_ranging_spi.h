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

#ifdef __cplusplus
extern "C"
{
#endif

#define DECA_MAX_SPI_HEADER_LENGTH (3) // max number of bytes in header (for formating & sizing)

	/*! ------------------------------------------------------------------------------------------------------------------
	 * Function: openspi()
	 *
	 * Low level abstract function to open and initialise access to the SPI device.
	 * returns 0 for success, or -1 for error
	 */
	int openspi(void);

	/*! ------------------------------------------------------------------------------------------------------------------
	 * Function: closespi()
	 *
	 * Low level abstract function to close the the SPI device.
	 * returns 0 for success, or -1 for error
	 */
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
