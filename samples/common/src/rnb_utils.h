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

#if defined(CONFIG_REDNODEBUS)

#define REDNODEBUS_UTILS_STACK_SIZE 2048
#define REDNODEBUS_UTILS_EVENT_BUFFER_SIZE 8
#define REDNODEBUS_UTILS_THREAD_PRIORITY 3

int init_rnb(void);
bool is_rnb_connected(void);

#endif

#endif

/**
 * @}
 */
