#include "am_modbus.h"

#include "board_manage.h"
#include "modbus_register_database.h"
#include "modbus_register_interface.h"

#define LOG_D(...) printf(__VA_ARGS__)
#define LOG_I(...) printf(__VA_ARGS__)
#define LOG_E(...) printf(__VA_ARGS__)

extern UART_HandleTypeDef huart7; 

#define MB_RTU_INTERFACE  huart7
#define MB_TIMEOUT_MS 200
#define MB_RTU_UID 0x0001

static modbusHandler_t encoder_forward_server;

void encoder_forward_init_server(void){

    uint32_t err = init_modbus_register(holding_reg_data, REG_HOLDING_SIZE);
    if (err != ERR_OK) {
        LOG_E("Failed to initialize Modbus register interface: %lu\n", (unsigned long)err);
    } else {
        LOG_I("Modbus register interface initialized successfully\n");
    }

    memset(holding_reg_data, 0, REG_HOLDING_SIZE * sizeof(uint16_t));
    mb_reg_write_u16(REG_SYSTEM_VERSION, 0x0100); 
    mb_reg_write_u16(REG_DATA_VAILD, 0);

    encoder_forward_server.uModbusType = MB_SLAVE;
    encoder_forward_server.u8id = MB_RTU_UID;
    encoder_forward_server.port = &MB_RTU_INTERFACE;
    encoder_forward_server.EN_Port = NULL;  // 无RS485控制引脚
    encoder_forward_server.EN_Pin = 0;
    encoder_forward_server.u16regs = holding_reg_data;
    encoder_forward_server.u16regsize = REG_HOLDING_SIZE;
    encoder_forward_server.u16timeOut = MB_TIMEOUT_MS; 
    encoder_forward_server.xTypeHW = USART_HW;
    ModbusInit(&encoder_forward_server);
    ModbusStart(&encoder_forward_server);

}
