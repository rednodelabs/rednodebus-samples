/*! ----------------------------------------------------------------------------
 * @file    rnb_ranging_port.c
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

// zephyr includes
#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <soc.h>
#include "rnb_ranging_port.h"
#include "rnb_ranging_spi.h"
#include "rnb_ranging_sleep.h"

#define LOG_MODULE_NAME rnb_ranging_port
#if defined(CONFIG_REDNODERANGING_DRIVER_LOG_LEVEL)
#define LOG_LEVEL CONFIG_REDNODERANGING_DRIVER_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_NONE
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

/********************************************************************************
 *
 *                              DeviceTree information
 *
 *******************************************************************************/

#if defined(CONFIG_IEEE802154_DW1000)
#define IRQ_GPIO_PORT DT_PHANDLE_BY_IDX(DT_INST(0, decawave_dw1000), int_gpios, 0)
#define IRQ_GPIO_PIN DT_PHA(DT_INST(0, decawave_dw1000), int_gpios, pin)
#define IRQ_GPIO_FLAGS DT_PHA(DT_INST(0, decawave_dw1000), int_gpios, flags)
#define RESET_GPIO_PORT DT_PHANDLE_BY_IDX(DT_INST(0, decawave_dw1000), reset_gpios, 0)
#define RESET_GPIO_PIN DT_PHA(DT_INST(0, decawave_dw1000), reset_gpios, pin)
#define RESET_GPIO_FLAGS DT_PHA(DT_INST(0, decawave_dw1000), reset_gpios, flags)
#elif defined(CONFIG_IEEE802154_DW3000)
#define IRQ_GPIO_PORT DT_PHANDLE_BY_IDX(DT_INST(0, qorvo_dw3000), int_gpios, 0)
#define IRQ_GPIO_PIN DT_PHA(DT_INST(0, qorvo_dw3000), int_gpios, pin)
#define IRQ_GPIO_FLAGS DT_PHA(DT_INST(0, qorvo_dw3000), int_gpios, flags)
#define RESET_GPIO_PORT DT_PHANDLE_BY_IDX(DT_INST(0, qorvo_dw3000), reset_gpios, 0)
#define RESET_GPIO_PIN DT_PHA(DT_INST(0, qorvo_dw3000), reset_gpios, pin)
#define RESET_GPIO_FLAGS DT_PHA(DT_INST(0, qorvo_dw3000), reset_gpios, flags)
#endif

/********************************************************************************
 *
 *******************************************************************************/

static const struct device *reset_dev = NULL;
static const struct device *irq_dev = NULL;

/********************************************************************************
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    portGetTickCnt
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
unsigned long portGetTickCnt(void)
{
	return 0;
}

/********************************************************************************
 *
 *                              END OF Time section
 *
 *******************************************************************************/

/********************************************************************************
 *
 *                              Configuration section
 *
 *******************************************************************************/

/* @fn    peripherals_init
 * */
int peripherals_init(void)
{
	/* Reset */
	reset_dev = DEVICE_DT_GET(RESET_GPIO_PORT);
	if (!reset_dev)
	{
		LOG_ERR("error: \"%s\" not found", DT_NODE_PATH(RESET_GPIO_PORT));
		return -EIO;
	}

	if (gpio_pin_configure(reset_dev,
			       RESET_GPIO_PIN,
			       GPIO_DISCONNECTED))
	{
		LOG_ERR("error: unable to configure \"%s\"", DT_NODE_PATH(RESET_GPIO_PORT));
		return -EINVAL;
	}

	return 0;
}

/* @fn    spi_peripheral_init
 * */
void spi_peripheral_init()
{
	openspi();
}

/* @fn    spi_peripheral_deinit
 * */
void spi_peripheral_deinit()
{
	closespi();
}

/********************************************************************************
 *
 *                          End of configuration section
 *
 *******************************************************************************/

/********************************************************************************
 *
 *                          DW3000 port section
 *
 *******************************************************************************/

/* @fn      reset_DWIC
 * @brief   DW_RESET pin on DW3000 has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW3000 by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DWIC(void)
{
	LOG_DBG("%s", __func__);

#if 1
	/*
	 * Use RSTn gpio pin to reset DWM3000.
	 */
	gpio_pin_configure(reset_dev,
			   RESET_GPIO_PIN,
			   GPIO_OUTPUT | GPIO_OPEN_DRAIN);

	gpio_pin_set(reset_dev, RESET_GPIO_PIN, 0);
	deca_usleep(100);

	gpio_pin_configure(reset_dev,
			   RESET_GPIO_PIN,
			   GPIO_DISCONNECTED);
#else
	/*
	 *  Use Soft Reset api to reset DWM3000
	 *  SPI bus must be <= 7MHz, e.g. slowrate
	 */
	port_set_dw_ic_spi_slowrate();

	dwt_softreset();

	/* Set SPI bus to working rate: fastrate. */
	port_set_dw_ic_spi_fastrate();
#endif
}

/* @fn      setup_DW3000RSTnIRQ
 * @brief   setup the DW_RESET pin mode
 *          0 - output Open collector mode
 *          !0 - input mode with connected EXTI0 IRQ
 * */
void setup_DWRSTnIRQ(int enable)
{
	if (enable)
	{
		/* Enable GPIO used as DECA RESET for interrupt */
		gpio_pin_configure(reset_dev, RESET_GPIO_PIN, (GPIO_OUTPUT | GPIO_OPEN_DRAIN | GPIO_INT_EDGE_RISING));
	}
	else
	{
		/* Put the pin back to tri-state, as output open-drain (not active) */
		gpio_pin_configure(reset_dev, RESET_GPIO_PIN, (GPIO_OUTPUT | GPIO_OPEN_DRAIN));
	}
}

/*
 * @fn wakeup_device_with_io()
 *
 * @brief This function wakes up the device by toggling io with a delay.
 *
 * input None
 *
 * output -None
 *
 */
void wakeup_device_with_io(void)
{
}

/*
 * @fn make_very_short_wakeup_io()
 *
 * @brief This will toggle the wakeup pin for a very short time. The device should not wakeup
 *
 * input None
 *
 * output -None
 *
 */
void make_very_short_wakeup_io(void)
{
}

/* @fn      led_off
 * @brief   switch off the led from led_t enumeration
 * */
void led_off(uint32_t led)
{
}

/* @fn      led_on
 * @brief   switch on the led from led_t enumeration
 * */
void led_on(uint32_t led)
{
}

/* @fn      port_wakeup
 * @brief   "slow" waking up of DW using DW_CS only
 * */
void port_wakeup(void)
{
#if 0
//     gpio_pin_set(wakeup_dev, WAKEUP_GPIO_PIN, 0);
#else
	uint8_t cswakeup_buf[150];
	cswakeup_buf[0] = 0;

	port_set_dw_ic_spi_slowrate();

	// Do a long read to wake up the chip (hold the chip select low)
	readfromspi(1, cswakeup_buf, sizeof(cswakeup_buf), cswakeup_buf);

	deca_sleep(2);

	port_set_dw_ic_spi_fastrate();
#endif
}

/* @fn      port_wakeup_fast
 * @brief   waking up of DW using DW_CS and DW_RESET pins.
 *          The DW_RESET signalling that the DW is in the INIT state.
 *          the total fast wakeup takes ~2.2ms and depends on crystal startup time
 * */
void port_wakeup_fast(void)
{
}

/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   set 2MHz
 * */
void port_set_dw_ic_spi_slowrate(void)
{
	set_spi_speed_slow();
}

/* @fn      port_set_dw_ic_spi_fastrate
 * @brief   set 8MHz
 * */
void port_set_dw_ic_spi_fastrate(void)
{
	set_spi_speed_fast();
}

/********************************************************************************
 *
 *                          End APP port section
 *
 *******************************************************************************/

/********************************************************************************
 *
 *                              IRQ section
 *
 *******************************************************************************/

/* @fn      port_DisableEXT_IRQ
 * @brief   wrapper to disable DW_IRQ pin IRQ
 *          in current implementation it disables all IRQ from lines 5:9
 * */
void port_DisableEXT_IRQ(void)
{
	gpio_pin_configure(irq_dev, IRQ_GPIO_PIN, GPIO_DISCONNECTED);
	gpio_pin_interrupt_configure(irq_dev, IRQ_GPIO_PIN, GPIO_INT_DISABLE);
}

/* @fn      port_EnableEXT_IRQ
 * @brief   wrapper to enable DW_IRQ pin IRQ
 *          in current implementation it enables all IRQ from lines 5:9
 * */
void port_EnableEXT_IRQ(void)
{
	gpio_pin_configure(irq_dev, IRQ_GPIO_PIN, (GPIO_INPUT | IRQ_GPIO_FLAGS));
	gpio_pin_interrupt_configure(irq_dev, IRQ_GPIO_PIN, GPIO_INT_EDGE_RISING);
}

/* @fn      port_GetEXT_IRQStatus
 * @brief   wrapper to read a DW_IRQ pin IRQ status
 * */
uint8_t port_GetEXT_IRQStatus(void)
{
	return 0;
}

/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
uint8_t port_CheckEXT_IRQ(void)
{
	return 0;
}

/* DW IRQ handler definition. */

/*! ---------------------------------------------------------------------------
 * @fn port_set_dwic_isr(struct gpio_callback *gpio_cb, gpio_callback_handler_t callback_handler)
 *
 * @brief This function is used to install the handling function for DW IRQ.
 *
 * NOTE:
 *
 * @param gpio_cb function pointer to DW interrupt handler
 * @param callback_handler function pointer to DW interrupt handler to install
 *
 * @return none
 */
void port_set_dwic_isr(struct gpio_callback *gpio_cb, gpio_callback_handler_t callback_handler)
{
	// LOG_DBG("Configure IRQ on port \"%s\" pin %d", DT_NODE_PATH(IRQ_GPIO_PORT), IRQ_GPIO_PIN);
	irq_dev = DEVICE_DT_GET(IRQ_GPIO_PORT);
	if (!irq_dev)
	{
		LOG_ERR("error: \"%s\" not found", DT_NODE_PATH(IRQ_GPIO_PORT));
		return;
	}

	/* DW interrupt */
	gpio_pin_configure(irq_dev, IRQ_GPIO_PIN, (GPIO_INPUT | IRQ_GPIO_FLAGS));

	gpio_init_callback(gpio_cb, callback_handler, BIT(IRQ_GPIO_PIN));

	gpio_add_callback(irq_dev, gpio_cb);

	port_DisableEXT_IRQ();
}

/********************************************************************************
 *
 *******************************************************************************/
