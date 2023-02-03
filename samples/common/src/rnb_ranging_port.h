/*! ----------------------------------------------------------------------------
 * @file    rnb_ranging_port.h
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2023 (c) RedNodeLabs.
 *
 * All rights reserved.
 *
 * @author RedNodeLabs.
 */

#ifndef _RNB_RANGING_PORT_H_
#define _RNB_RANGING_PORT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>
#include <drivers/gpio.h>
// #include "compiler.h"

	/*! ------------------------------------------------------------------------------------------------------------------
	 * @fn port_set_dwic_isr()
	 *
	 * @brief This function is used to install the handling function for DW3000 IRQ.
	 *
	 * NOTE:
	 *
	 * @param callback_handler function pointer to DW1000 interrupt handler to install
	 *
	 * @return none
	 */
	void port_set_dwic_isr(struct gpio_callback *gpio_cb, gpio_callback_handler_t callback_handler);

	/********************************************************************************************************************
	 **/

	/********************************************************************************
	 *
	 *                                 Types definitions
	 *
	 *******************************************************************************/
	typedef uint64_t uint64;
	typedef int64_t int64;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/********************************************************************************
 *
 *                              MACRO
 *
 *******************************************************************************/

/********************************************************************************
 *
 *                              MACRO function
 *
 *******************************************************************************/

// TODO
#define GPIO_ResetBits(x, y)
#define GPIO_SetBits(x, y)
#define GPIO_ReadInputDataBit(x, y)

/* NSS pin is SW controllable */
#define port_SPIx_set_chip_select()
#define port_SPIx_clear_chip_select()

/* NSS pin is SW controllable */
#define port_SPIy_set_chip_select()
#define port_SPIy_clear_chip_select()

	/********************************************************************************
	 *
	 *                              port function prototypes
	 *
	 *******************************************************************************/

	void Sleep(uint32_t Delay);
	unsigned long portGetTickCnt(void);

	void port_wakeup(void);
	void port_wakeup_fast(void);

	void port_set_dw_ic_spi_slowrate(void);
	void port_set_dw_ic_spi_fastrate(void);

	void port_DisableEXT_IRQ(void);
	void port_EnableEXT_IRQ(void);

	void process_dwRSTn_irq(void);
	void process_deca_irq(void);

	void led_on(uint32_t led);
	void led_off(uint32_t led);

	int peripherals_init(void);
	void spi_peripheral_init(void);
	void spi_peripheral_deinit(void);

	void setup_DWRSTnIRQ(int enable);

	void reset_DWIC(void);

	extern uint32_t HAL_GetTick(void);

	/*! ------------------------------------------------------------------------------------------------------------------
	 * @fn decamutexon()
	 *
	 * @brief This function should disable interrupts. This is called at the start of a critical section
	 * It returns the IRQ state before disable, this value is used to re-enable in decamutexoff call
	 *
	 * Note: The body of this function is defined in deca_mutex.c and is platform specific
	 *
	 * input parameters
	 *
	 * output parameters
	 *
	 * returns the state of the DW1000 interrupt
	 */
	void decamutexon(void);

	/*! ------------------------------------------------------------------------------------------------------------------
	 * @fn decamutexoff()
	 *
	 * @brief This function should re-enable interrupts, or at least restore their state as returned(&saved) by decamutexon
	 * This is called at the end of a critical section
	 *
	 * Note: The body of this function is defined in deca_mutex.c and is platform specific
	 *
	 * input parameters
	 *
	 * output parameters
	 *
	 * returns the state of the DW1000 interrupt
	 */
	void decamutexoff(void);

#ifdef __cplusplus
}
#endif

#endif /* _RNB_RANGING_PORT_H_ */
