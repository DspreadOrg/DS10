/*
 * Copyright (C) 2015 Freie Universit?t Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_periph_gpio GPIO
 * @ingroup     drivers_periph
 * @brief       Low-level GPIO peripheral driver
 *
 * This is a basic GPIO (General-purpose input/output) interface to allow
 * platform independent access to a MCU's input/output pins. This interface is
 * intentionally designed to be as simple as possible, to allow for easy
 * implementation and maximum portability.
 *
 * The interface provides capabilities to initialize a pin as output-,
 * input- and interrupt pin. With the API you can basically set/clear/toggle the
 * digital signal at the hardware pin when in output mode. Configured as input you can
 * read a digital value that is being applied to the pin externally. When initializing
 * an external interrupt pin, you can register a callback function that is executed
 * in interrupt context once the interrupt condition applies to the pin. Usually you
 * can react to rising or falling signal flanks (or both).
 *
 * In addition the API provides to set standard input/output circuit modes such as
 * e.g. internal push-pull configurations.
 *
 * All modern micro controllers organize their GPIOs in some form of ports,
 * often named 'PA', 'PB', 'PC'..., or 'P0', 'P1', 'P2'..., or similar. Each of
 * these ports is then assigned a number of pins, often 8, 16, or 32. A hardware
 * pin can thus be described by its port/pin tuple. To access a pin, the
 * @p GPIO_PIN(port, pin) macro should be used. For example: If your platform has
 * a pin PB22, it will be port=1 and pin=22. The @p GPIO_PIN macro should be
 * overridden by a MCU, to allow for efficient encoding of the the port/pin tuple.
 * For example, on many platforms it is possible to `OR` the pin number with the
 * corresponding ports base register address. This allows for efficient decoding
 * of pin number and base address without the need of any address lookup.
 *
 * In case the driver does not define it, the below macro definition is used to
 * simply map the port/pin tuple to the pin value. In that case, predefined GPIO
 * definitions in `RIOT/boards/ * /include/periph_conf.h` will define the selected
 * GPIO pin.
 *
 * @{
 * @file
 * @brief       Low-level GPIO peripheral driver interface definitions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef PERIPH_GPIO_H
#define PERIPH_GPIO_H

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   GPIO type identifier
 */
typedef unsigned int gpio_t;

/**
 * @brief   Convert (port, pin) tuple to @c gpio_t value
 */
/* Default GPIO macro maps port-pin tuples to the pin value */
#define GPIO_PIN(x,y)       ((gpio_t)((x & 0) | y))

/**
 * @brief   GPIO pin not defined
 */
#define GPIO_UNDEF          ((gpio_t)(UINT_MAX))

/**
 * @brief   Available pin modes
 *
 * Generally, a pin can be configured to be input or output. In output mode, a
 * pin can further be put into push-pull or open drain configuration. Though
 * this is supported by most platforms, this is not always the case, so driver
 * implementations may return an error code if a mode is not supported.
 */
typedef enum {
    GPIO_IN ,               /**< configure as input without pull resistor */
    GPIO_IN_PD,             /**< configure as input with pull-down resistor */
    GPIO_IN_PU,             /**< configure as input with pull-up resistor */
    GPIO_OUT,               /**< configure as output in push-pull mode */
    GPIO_OD,                /**< configure as output in open-drain mode without
                             *   pull resistor */
    GPIO_OD_PU              /**< configure as output in open-drain mode with
                             *   pull resistor enabled */
} gpio_mode_t;

/**
 * @brief   Definition of possible active flanks for external interrupt mode
 */
typedef enum {
    GPIO_FALLING = 0,       /**< emit interrupt on falling flank */
    GPIO_RISING = 1,        /**< emit interrupt on rising flank */
    GPIO_BOTH = 2           /**< emit interrupt on both flanks */
} gpio_flank_t;

/**
 * @brief   Initialize the given pin as general purpose input or output
 *
 * When configured as output, the pin state after initialization is undefined.
 * The output pin's state **should** be untouched during the initialization.
 * This behavior can however **not be guaranteed** by every platform.
 *
 * @param[in] pin       pin to initialize
 * @param[in] mode      mode of the pin, see @c gpio_mode_t
 *
 * @return              0 on success
 * @return              -1 on error
 */
int gpio_init(gpio_t pin, gpio_mode_t mode);

/**
 * @brief   Get the current value of the given pin
 *
 * @param[in] pin       the pin to read
 *
 * @return              0 when pin is LOW
 * @return              >0 for HIGH
 */
int gpio_read(gpio_t pin);

/**
 * @brief   Set the given pin to HIGH
 *
 * @param[in] pin       the pin to set
 */
void gpio_set(gpio_t pin);

/**
 * @brief   Set the given pin to LOW
 *
 * @param[in] pin       the pin to clear
 */
void gpio_clear(gpio_t pin);

/**
 * @brief   Toggle the value of the given pin
 *
 * @param[in] pin       the pin to toggle
 */
void gpio_toggle(gpio_t pin);

/**
 * @brief   Set the given pin to the given value
 *
 * @param[in] pin       the pin to set
 * @param[in] value     value to set the pin to, 0 for LOW, HIGH otherwise
 */
void gpio_write(gpio_t pin, int value);

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_GPIO_H */
/** @} */
