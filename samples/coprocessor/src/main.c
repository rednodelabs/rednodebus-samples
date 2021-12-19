/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(coprocessor_sample, CONFIG_OT_COPROCESSOR_LOG_LEVEL);

#define WELCOME_TEXT                                                           \
	"\n\r"                                                                 \
	"\n\r"                                                                 \
	"=========================================================\n\r"        \
	"OpenThread Coprocessor application is now running on NCS.\n\r"        \
	"=========================================================\n\r"

void main(void)
{
	LOG_INF(WELCOME_TEXT);

	// uint32_t counter = 0;
	//
	// while (1)
	// {
	// 	LOG_INF("RedNodeBus %u", counter);
	//
	// 	counter++;
	//
	// 	k_sleep(K_MSEC(800));
	// }
}
