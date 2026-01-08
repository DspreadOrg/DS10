#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>
#include <stdio.h>

#define CLOCK_CONF_SECOND 1000
typedef uint32_t clock_time_t;

/**
 * A second, measured in system clock time.
 *
 * \hideinitializer
 */
#ifdef CLOCK_CONF_SECOND
#define CLOCK_SECOND CLOCK_CONF_SECOND
#else
#define CLOCK_SECOND (clock_time_t)32
#endif

/**
 * Initialize the clock library.
 *
 * This function initializes the clock library and should be called
 * from the main() function of the system.
 *
 */
void clock_init(void);

/**
 * Get the current clock time.
 *
 * This function returns the current system clock time.
 *
 * \return The current clock time, measured in system ticks.
 */
clock_time_t clock_time(void);

/**
 * Get the current value of the platform seconds.
 *
 * This could be the number of seconds since startup, or
 * since a standard epoch.
 *
 * \return The value.
 */
unsigned long clock_seconds(void);

/**
 * Set the value of the platform seconds.
 * \param sec   The value to set.
 *
 */
void clock_set_seconds(unsigned long sec);

/**
 * Wait for a given number of ticks.
 * \param t   How many ticks.
 *
 */
void clock_wait(clock_time_t t);

/**
 * Wait for a given number of ms, at lease 1 tick.
 * \param t   How many ms.
 *
 */
void clock_wait_ms(unsigned ms);

/**
 * Delay a given number of microseconds.
 * \param dt   How many microseconds to delay.
 *
 * \note Interrupts could increase the delay by a variable amount.
 */
void clock_delay_usec(uint16_t dt);

/**
 * Deprecated platform-specific routines.
 *
 */
int clock_fine_max(void);
unsigned short clock_fine(void);
void clock_delay(unsigned int delay);

#endif /* CLOCK_H_ */

/** @} */
/** @} */

