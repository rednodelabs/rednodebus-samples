/* main.c - RedNodeBus node */

/*
 * Copyright (c) 2022 RedNodeLabs.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(rednodebus_node, LOG_LEVEL_INF);

#ifdef CONFIG_REDNODEBUS
#include "rnb_utils.h"
#endif /* CONFIG_REDNODEBUS */

void main(void)
{
    LOG_INF("RedNodeBus node sample");

#ifdef CONFIG_REDNODEBUS
    init_rnb();
#endif /* CONFIG_REDNODEBUS */
}
