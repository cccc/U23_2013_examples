#ifndef _LEDS_H_
#define _LEDS_H_

/*! @addtogroup libsystem
 * @{ */

/*! @addtogroup LED
 * @brief This header is used define basic LED functionality
 *
 * This controls the 4 LEDs located around the MEMS Accelerometer
 * and beneath the SNES Controller Ports.
 * @{ */

#include <stm32f4xx.h>

/*!
 * @brief Initializes the LEDs
 *
 * This function is automatically called in main().
 */
void InitializeLEDs(void);

/*!
 * @brief Sets the LEDS status
 *
 * E.g. to set LED 2 and LED 4 call
 * @code
 * SetLEDs((1<<1)|(1<<3));
 * @endcode
 * @param leds Integer Bitmask that toggles one or more LEDs
 */
static inline void SetLEDs(int leds)
{
	uint16_t val=GPIOD->ODR;
	val&=~(0x0f<<12);
	val|=leds<<12;
	GPIOD->ODR=val;
}

/*! @} */
/*! @} */

#endif
