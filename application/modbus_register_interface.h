#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Error codes */
#define ERR_OK               0x0000
#define ERR_INIT_FAILED      0x0001
#define ERR_INVALID_PARAM    0x0002
#define ERR_INVALID_ADDR     0x0003
#define ERR_LOCK_FAILED      0x0004
#define ERR_BUFF_OVERFLOW    0x0005
#define ERR_NOT_MASTER       0x0006

/**
 * Initialize modbus register interface with external database
 * @param reg_buffer: Pointer to external register database (from project's modbus_register_database)
 * @param buffer_size: Number of 16-bit registers in the database
 * @return ERR_OK on success, error code on failure
 * 
 * This function decouples interface from database. Different projects can have
 * different database implementations and sizes while sharing the same interface code.
 * Database is registered at initialization, not embedded in the interface.
 */
uint32_t init_modbus_register(uint16_t *reg_buffer, uint16_t buffer_size);

/**
 * Read single 16-bit register by address
 * @param addr Register address
 * @param value Pointer to store the read value
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_read_u16(uint16_t addr, uint16_t *value);

/**
 * Write single 16-bit register by address
 * @param addr Register address
 * @param value Value to write
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_write_u16(uint16_t addr, uint16_t value);

/**
 * Read 32-bit value from two consecutive registers (HI/LO pair)
 * Typically used for sensor position data (HI at addr_hi, LO at addr_hi+1)
 * @param addr_hi Address of high 16-bit register
 * @param value_hi Pointer to store high 16 bits
 * @param value_lo Pointer to store low 16 bits
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_read_u32(uint16_t addr_hi, uint16_t *value_hi, uint16_t *value_lo);

/**
 * Write 32-bit value to two consecutive registers (HI/LO pair)
 * @param addr_hi Address of high 16-bit register
 * @param value_hi High 16 bits to write
 * @param value_lo Low 16 bits to write
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_write_u32(uint16_t addr_hi, uint16_t value_hi, uint16_t value_lo);

/**
 * Write float value to two consecutive registers (HI/LO pair)
 * IEEE754 bits are stored as high 16 bits at addr_hi and low 16 bits at addr_hi+1.
 * @param addr_hi Address of high 16-bit register
 * @param value Float value to write
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_write_float(uint16_t addr_hi, float value);

/**
 * Read multiple consecutive registers
 * @param addr Starting register address
 * @param count Number of registers to read
 * @param buffer Buffer to store register values
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_read_n(uint16_t addr, uint16_t count, uint16_t *buffer);

/**
 * Read multiple non-consecutive registers in one lock operation
 * @param addr_list Array of register addresses to read
 * @param count Number of addresses to read
 * @param values Output array for register values (same order as addr_list)
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_read_multi_u16(const uint16_t *addr_list, uint16_t count, uint16_t *values);

/**
 * Write multiple consecutive registers
 * @param addr Starting register address
 * @param count Number of registers to write
 * @param buffer Buffer containing register values
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_write_n(uint16_t addr, uint16_t count, const uint16_t *buffer);

/**
 * Write a single bit in a register
 * @param addr Register address
 * @param bit_pos Bit position (0-15)
 * @param bit_value Boolean bit value
 * @return ERR_OK on success, error code on failure
 */
uint32_t mb_reg_write_bit(uint16_t addr, uint8_t bit_pos, bool bit_value);

/**
 * Get pointer to holding register buffer (for Modbus stack direct access)
 * Use with caution - no mutex protection!
 * @return Pointer to g_holding_registers array
 */
uint16_t* mb_reg_get_buffer(void);

#ifdef __cplusplus
}
#endif
