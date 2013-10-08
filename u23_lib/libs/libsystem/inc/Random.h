#ifndef _RANDOM_H_
#define _RANDOM_H_

/*! @addtogroup libgaming
 * @{ */

/*! @addtogroup Random
 * @brief This header is used implement the hardware RNG (Random Number Generator)
 * @{ */

#include <stdint.h>

/*!
 * @brief Initializes the RNG
 *
 * This function is automatically called in main().
 */
void InitializeRandom(void);

/*!
 * @brief Get a random integer
 *
 * This function returns 0 when the RNG has not been initialized.
 * Blocks while RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET
 * @return A random integer
 */
uint32_t GetRandomInteger(void);

/*!
 * @brief DeInitializes the hardware RNG
 */
void DeInitializeRandom(void);

/*! @} */
/*! @} */

#endif
