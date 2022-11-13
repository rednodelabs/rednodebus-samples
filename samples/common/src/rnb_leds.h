/**
 * @file
 * @defgroup rnb_leds RedNodeBus leds API
 * @{
 */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __RNB_LEDS_H__
#define __RNB_LEDS_H__

int rnb_leds_init();
void rnb_leds_set_tx(bool active);
void rnb_leds_set_rx(bool active);

#endif

/**
 * @}
 */
