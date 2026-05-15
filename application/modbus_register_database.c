#include "modbus_register_database.h"

/* Project-specific holding register database
 * Each project defines its own database with project-specific size and register definitions.
 * The modbus_register_interface will operate on this database through registered pointers.
 */
uint16_t holding_reg_data[REG_HOLDING_SIZE] = {0};