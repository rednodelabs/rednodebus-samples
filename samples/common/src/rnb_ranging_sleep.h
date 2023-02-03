/*! ----------------------------------------------------------------------------
 * @file    rnb_ranging_sleep.h
 * @brief   platform dependent sleep implementation
 *
 * @attention
 *
 * Copyright 2023 (c) RedNodeLabs.
 *
 * All rights reserved.
 *
 * @author RedNodeLabs.
 */

#ifndef _RNB_RANGING_SLEEP_H_
#define _RNB_RANGING_SLEEP_H_

#ifdef __cplusplus
extern "C"
{
#endif

	/*! ------------------------------------------------------------------------------------------------------------------
	 * Function: deca_sleep()
	 *
	 * Wait for a given amount of time.
	 * /!\ This implementation is designed for a single threaded application and is blocking.
	 *
	 * param  time_ms  time to wait in milliseconds
	 */
	void deca_sleep(unsigned int time_ms);

	void deca_usleep(unsigned long time_us);

#ifdef __cplusplus
}
#endif

#endif /* _RNB_RANGING_SLEEP_H_ */
