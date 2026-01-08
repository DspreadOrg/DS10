/*
 * (C) Copyright 2011
 * Marvell Inc, <www.marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _I2C_H_
#define _I2C_H_

#if defined(BOARD_CRANE)||defined(BOARD_CRANEW_MCU)

#define HAVE_I2C_SPEED_T

typedef enum {
  I2C_SPEED_STANDARD = 0,           /* standard mode:    ~100kbit/s */
  I2C_SPEED_FAST,                   /* fast mode:         ~400kbit/sj */
  I2C_SPEED_HIGH,                   /* high speed mode:   ~3.4Mbit/s */
} i2c_speed_t;

#ifdef BOARD_CRANEW_MCU
#define I2C_MASTER_PI2C  0
#define I2C_MASTER_MAX   1
#else
#define I2C_MASTER_CI2C0 0
#define I2C_MASTER_CI2C1 1
#define I2C_MASTER_PI2C  2
#define I2C_MASTER_CI2C2 3
#define I2C_MASTER_MAX   4
#endif

struct i2c_conf_t {
  i2c_speed_t speed;
};

/**
 * @brief   Default I2C device access macro
 * @{
 */
#ifndef I2C_DEV
#define I2C_DEV(x)          (x)
#endif
/** @} */

/**
 * @brief   Default I2C undefined value
 * @{
 */
#ifndef I2C_UNDEF
#define I2C_UNDEF           (UINT_MAX)
#endif
/** @} */

/**
 * @brief   Default i2c_t type definition
 * @{
 */
#ifndef HAVE_I2C_T
typedef unsigned int i2c_t;
#endif
/**  @} */

/**
 * @brief   Read bit needs to be set when reading
 */
#define I2C_READ            (0x0001)

/**
 * @brief   Special bit pattern indicating a 10 bit address is used
 *
 * Should only be used internally in CPU driver implementations, this is not
 * intended to be used by applications.
 *
 * @see https://www.i2c-bus.org/addressing/10-bit-addressing/
 */
#define I2C_10BIT_MAGIC     (0xF0u)

/**
 * @brief   Default mapping of I2C bus speed values
 * @{
 */
#ifndef HAVE_I2C_SPEED_T
typedef enum {
  I2C_SPEED_LOW = 0,        /**< low speed mode:     ~10 kbit/s */
  I2C_SPEED_NORMAL,         /**< normal mode:       ~100 kbit/s */
  I2C_SPEED_FAST,           /**< fast mode:         ~400 kbit/s */
  I2C_SPEED_FAST_PLUS,      /**< fast plus mode:   ~1000 kbit/s */
  I2C_SPEED_HIGH,           /**< high speed mode:  ~3400 kbit/s */
} i2c_speed_t;
#endif
/** @} */

/**
 * @brief   I2C transfer flags
 * @{
 */
#ifndef HAVE_I2C_FLAGS_T
typedef enum {
  I2C_ADDR10  = 0x01,       /**< use 10-bit device addressing */
  I2C_REG16   = 0x02,       /**< use 16-bit register addressing */
  I2C_NOSTOP  = 0x04,       /**< do not issue a STOP condition after transfer */
  I2C_NOSTART = 0x08,       /**< skip START sequence, ignores address field */
} i2c_flags_t;
#endif
/** @} */

/**
 * @brief   Initialize the given I2C bus
 *
 * The given I2C device will be initialized with the parameters as specified in
 * the boards periph_conf.h, using the pins and the speed value given there.
 *
 * The bus MUST not be acquired before initializing it, as this is handled
 * internally by the i2c_init function!
 *
 * @param[in] dev       the device to initialize
 */
void i2c_init(i2c_t dev);

/**
 * @brief   Get mutually exclusive access to the given I2C bus
 *
 * In case the I2C device is busy, this function will block until the bus is
 * free again.
 *
 * @param[in] dev           I2C device to access
 *
 * @return                  0 on success, -1 on error
 */
int i2c_acquire(i2c_t dev);

/**
 * @brief   Release the given I2C device to be used by others
 *
 * @param[in] dev           I2C device to release
 *
 * @return                  0 on success, -1 on error
 */
int i2c_release(i2c_t dev);

/**
 * @brief   Convenience function for reading one byte from a given register
 *          address
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  flags        optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */

int i2c_read_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                 void *data, uint8_t flags);

/**
 * @brief   Convenience function for reading several bytes from a given
 *          register address
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to read into @p data
 * @param[in]  flags        optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */
int i2c_read_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                  void *data, size_t len, uint8_t flags);

/**
 * @brief   Convenience function for reading one byte from a device
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  flags        optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */

int i2c_read_byte(i2c_t dev, uint16_t addr, void *data, uint8_t flags);

/**
 * @brief   Convenience function for reading bytes from a device
 *
 * @note    This function is using a repeated start sequence for reading from
 *          the specified register address.
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to read into @p data
 * @param[in]  flags        optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */

int i2c_read_bytes(i2c_t dev, uint16_t addr,
                   void *data, size_t len, uint8_t flags);

/**
 * @brief   Convenience function for writing a single byte onto the bus
 *
 * @param[in] dev           I2C peripheral device
 * @param[in] addr          7-bit or 10-bit device address (right-aligned)
 * @param[in] data          byte to write to the device
 * @param[in] flags         optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */
int i2c_write_byte(i2c_t dev, uint16_t addr, uint8_t data, uint8_t flags);

/**
 * @brief   Convenience function for writing several bytes onto the bus
 *
 * @param[in] dev           I2C peripheral device
 * @param[in] addr          7-bit or 10-bit device address (right-aligned)
 * @param[in] data          array holding the bytes to write to the device
 * @param[in] len           the number of bytes to write
 * @param[in] flags         optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */
int i2c_write_bytes(i2c_t dev, uint16_t addr, const void *data,
                    size_t len, uint8_t flags);

/**
 * @brief   Convenience function for writing one byte to a given
 *          register address
 *
 * @note    This function is using a repeated start sequence for writing to the
 *          specified register address.
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[in]  data         byte to write
 * @param[in]  flags        optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */
int i2c_write_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                  uint8_t data, uint8_t flags);

/**
 * @brief   Convenience function for writing data to a given register address
 *
 * @note    This function is using a repeated start sequence for writing to the
 *          specified register address.
 *
 * @param[in]  dev          I2C peripheral device
 * @param[in]  reg          register address to read from (8- or 16-bit,
 *                          right-aligned)
 * @param[in]  addr         7-bit or 10-bit device address (right-aligned)
 * @param[out] data         memory location to store received data
 * @param[in]  len          the number of bytes to write
 * @param[in]  flags        optional flags (see @ref i2c_flags_t)
 *
 * @return                  0 When success
 * @return                  -EIO When slave device doesn't ACK the byte
 * @return                  -ENXIO When no devices respond on the address sent on the bus
 * @return                  -ETIMEDOUT  When timeout occurs before device's response
 * @return                  -EINVAL When an invalid argument is given
 * @return                  -EOPNOTSUPP When MCU driver doesn't support the flag operation
 * @return                  -EAGAIN When a lost bus arbitration occurs
 */
int i2c_write_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                   const void *data, size_t len, uint8_t flags);

int pi2c_init(unsigned addr);
int pi2c_read_reg(unsigned reg, unsigned char *pval);
int pi2c_write_reg(unsigned reg, unsigned char val);
void pi2c_reset(void);
void pi2c_bus_reset(void);

#endif
#endif
