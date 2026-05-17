#include "am_modbus.h"
#include "modbus_register_database.h"
#include "modbus_register_interface.h"
#include  "board_manage.h"

#define LOG_D(...) // printf(__VA_ARGS__)
#define LOG_I(...) printf(__VA_ARGS__)
#define LOG_E(...) printf(__VA_ARGS__)


extern UART_HandleTypeDef huart3; 

#define MB_RTU_INTERFACE huart3
#define MB_TIMEOUT_MS 200

#define MB_SLAVE_ADDR 0x0001
#define MB_FUNCTION_CODE MB_FC_READ_REGISTERS
#define MB_READ_ADDR 0x0002
#define MB_READ_LEN 2

#define ENCODER_NAME "swing"
#define ENCODER_REG_ADDR REG_SWING_ENCODER_COUNT

static modbusHandler_t client;
static modbus_t telegram;
#define RECV_BUFF_SIZE 10
static uint16_t recv_buff[RECV_BUFF_SIZE] = {0};

void read_swing_encoder_thread(void *argument){

    (void)argument;

    memset(recv_buff, 0, sizeof(recv_buff));
    client.uModbusType = MB_MASTER;
    client.u8id = 0;  // Master ID = 0
    client.port = &MB_RTU_INTERFACE;
    client.EN_Port = NULL; 
    client.EN_Pin = 0;
    client.u16regs = recv_buff;
    client.u16regsize = RECV_BUFF_SIZE;
    client.u16timeOut = MB_TIMEOUT_MS;
    client.xTypeHW = USART_HW;
    ModbusInit(&client);
    ModbusStart(&client);

    telegram.u8id = MB_SLAVE_ADDR;
    telegram.u8fct = MB_FUNCTION_CODE;
    telegram.u16RegAdd = MB_READ_ADDR;
    telegram.u16CoilsNo = MB_READ_LEN;
    telegram.u16reg = recv_buff;

    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    for(;;)
    {
        vTaskDelay(xFrequency);
        offline_manage_update_event(OFFLINE_READ_SWING_TASK);

        int err = ModbusQueryV2(&client, telegram);
        if (err != OP_OK_QUERY){
            LOG_E("E(%s,%d)\r\n", ENCODER_NAME,err);
            mb_reg_write_bit(REG_DATA_VAILD, REG_DATA_VAILD_BIT_SWING_ENCODER, false);
        }else{
            LOG_D("%s:", ENCODER_NAME);
            for (int i = 0; i < telegram.u16CoilsNo; i++) {
            LOG_D(" %d", telegram.u16reg[i]);
            }
            LOG_D("\r\n");
            mb_reg_write_u32(ENCODER_REG_ADDR, telegram.u16reg[0], telegram.u16reg[1]);
            
            /* Set data valid bit for swing encoder */
            mb_reg_write_bit(REG_DATA_VAILD, REG_DATA_VAILD_BIT_SWING_ENCODER, true);
        }
        
    }
}