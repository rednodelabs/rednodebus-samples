/**
 * @file
 * @defgroup rnb_utils RedNodeBus utilities API
 * @{
 */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __RNB_UTILS_H__
#define __RNB_UTILS_H__

int init_rnb(void);
bool is_rnb_connected(void);
int64_t rnb_get_unix_time(void);
void rnb_utils_get_euid(uint64_t *euid);
void rnb_utils_start();
void rnb_utils_stop();

#endif

/**
 * @}
 */
