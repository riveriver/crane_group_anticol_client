#include "modbus_register_interface.h"
#include "cmsis_os.h"
#include <string.h>

/* Pointer to external holding registers database (registered at init time) */
static uint16_t *g_holding_registers = NULL;

/* Size of the registered database (number of 16-bit registers) */
static uint16_t g_register_count = 0;

/* Mutex for thread-safe register access */
static osMutexId_t g_reg_mutex = NULL;

/* Initialize register interface with external database */
uint32_t setup_modbus_register(uint16_t *reg_buffer, uint16_t buffer_size)
{
  if (reg_buffer == NULL || buffer_size == 0) {
    return ERR_INVALID_PARAM;
  }
  
  g_holding_registers = reg_buffer;
  g_register_count = buffer_size;
  
  if (g_reg_mutex == NULL) {
    g_reg_mutex = osMutexNew(NULL);
    if (g_reg_mutex == NULL) {
      return ERR_INIT_FAILED;
    }
  }
  return ERR_OK;
}

/* Read single 16-bit register by address */
uint32_t mb_reg_read_u16(uint16_t addr, uint16_t *value)
{
  if (value == NULL) {
    return ERR_INVALID_PARAM;
  }
  
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if (addr >= g_register_count) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  *value = g_holding_registers[addr];
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Write single 16-bit register by address */
uint32_t mb_reg_write_u16(uint16_t addr, uint16_t value)
{
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if (addr >= g_register_count) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  g_holding_registers[addr] = value;
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Read 32-bit value from two consecutive 16-bit registers (HI/LO pair) */
uint32_t mb_reg_read_u32(uint16_t addr_hi, uint16_t *value_hi, uint16_t *value_lo)
{
  if ((value_hi == NULL) || (value_lo == NULL)) {
    return ERR_INVALID_PARAM;
  }
  
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if ((addr_hi >= g_register_count) || ((addr_hi + 1) >= g_register_count)) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  *value_hi = g_holding_registers[addr_hi];
  *value_lo = g_holding_registers[addr_hi + 1];
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Write 32-bit value to two consecutive 16-bit registers (HI/LO pair) */
uint32_t mb_reg_write_u32(uint16_t addr_hi, uint16_t value_hi, uint16_t value_lo)
{
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if ((addr_hi >= g_register_count) || ((addr_hi + 1) >= g_register_count)) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  g_holding_registers[addr_hi] = value_hi;
  g_holding_registers[addr_hi + 1] = value_lo;
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Write float value to two consecutive 16-bit registers (HI/LO pair) */
uint32_t mb_reg_write_float(uint16_t addr_hi, float value)
{
  uint32_t raw_bits;
  uint16_t value_hi;
  uint16_t value_lo;

  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }

  if ((addr_hi >= g_register_count) || ((addr_hi + 1) >= g_register_count)) {
    return ERR_INVALID_ADDR;
  }

  memcpy(&raw_bits, &value, sizeof(raw_bits));
  value_hi = (uint16_t)(raw_bits >> 16);
  value_lo = (uint16_t)(raw_bits & 0xFFFFU);

  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }

  g_holding_registers[addr_hi] = value_hi;
  g_holding_registers[addr_hi + 1] = value_lo;

  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Read multiple consecutive registers */
uint32_t mb_reg_read_n(uint16_t addr, uint16_t count, uint16_t *buffer)
{
  if (buffer == NULL || count == 0) {
    return ERR_INVALID_PARAM;
  }
  
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if ((addr >= g_register_count) || ((addr + count) > g_register_count)) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  memcpy(buffer, &g_holding_registers[addr], count * sizeof(uint16_t));
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Write multiple consecutive registers */
uint32_t mb_reg_write_n(uint16_t addr, uint16_t count, const uint16_t *buffer)
{
  if (buffer == NULL || count == 0) {
    return ERR_INVALID_PARAM;
  }
  
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if ((addr >= g_register_count) || ((addr + count) > g_register_count)) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  memcpy(&g_holding_registers[addr], buffer, count * sizeof(uint16_t));
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Get pointer to all holding registers (for Modbus stack use) */
uint16_t* mb_reg_get_buffer(void)
{
  return g_holding_registers;
}

/* Write a single bit in a register (thread-safe) */
uint32_t mb_reg_write_bit(uint16_t addr, uint8_t bit_pos, bool bit_value)
{
  if (bit_pos > 15) {
    return ERR_INVALID_PARAM;
  }
  
  if (g_holding_registers == NULL) {
    return ERR_INIT_FAILED;
  }
  
  if (addr >= g_register_count) {
    return ERR_INVALID_ADDR;
  }
  
  if (osMutexAcquire(g_reg_mutex, osWaitForever) != osOK) {
    return ERR_LOCK_FAILED;
  }
  
  if (bit_value) {
    g_holding_registers[addr] |= (1U << bit_pos);   /* Set bit */
  } else {
    g_holding_registers[addr] &= ~(1U << bit_pos);  /* Clear bit */
  }
  
  osMutexRelease(g_reg_mutex);
  return ERR_OK;
}

/* Get pointer to all holding registers (for Modbus stack use) */
uint16_t* mb_reg_get_buffer(void)
{
  return g_holding_registers;
}
