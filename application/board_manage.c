#include "board_manage.h"
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#include "modbus_register_interface.h"
#include "modbus_register_database.h"
#include "printf_redirect.h"
#include "ota_service_task.h"
#include "uart_manage_port.h"
#include "eth_manage_task.h"
#include "offline_manage_task.h"
#include "read_swing_encoder_task.h"
#include "read_luffing_encoder_task.h"
#include "encoder_forward_server.h"
#include "report_atc_data_task.h"

#define LOG_D(fmt, ...) printf("[D][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_I(fmt, ...) printf("[I][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[E][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)    

extern UART_HandleTypeDef huart1;

void board_get_mac_address(uint8_t *mac);

void board_system_reset_force(void)
{
    __disable_irq();
    __DSB();
    NVIC_SystemReset();

}

void board_setup_components(void) {
    specify_redirect_uart(&huart1);
    init_uart_service();
    encoder_forward_init_server();
    (void)ota_init_service(0x08020000U, 896U * 1024U);
}

static const osThreadAttr_t offline_manage_attr = {
    .name = "OfflineManageTask",
    .stack_size = 4096 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t eth_manage_attr = {
    .name = "EthManageTask",
    .stack_size = 4096 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t read_swing_encoder_attr = {
    .name = "ReadSwingEncoderTask",
    .stack_size = 4096 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

static const osThreadAttr_t read_luffing_encoder_attr = {
    .name = "ReadLuffingEncoderTask",
    .stack_size = 4096 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

void board_create_user_tasks(void) {
    
    osThreadId_t tid;
    
    tid = osThreadNew(offline_manage_task, NULL, &offline_manage_attr);
    if (tid == NULL) {
        LOG_E("Failed to create OfflineManageTask\n");
    } else {
        LOG_I("OfflineManageTask created successfully\n");
    }

    tid = osThreadNew(eth_manage_task, NULL, &eth_manage_attr);
    if (tid == NULL) {
        LOG_E("Failed to create EthManageTask\n");
    } else {
        LOG_I("EthManageTask created successfully\n");
    }

    tid = osThreadNew(read_swing_encoder_thread, NULL, &read_swing_encoder_attr);
    if (tid == NULL) {
        LOG_E("Failed to create ReadSwingEncoderTask\n");
    } else {
        LOG_I("ReadSwingEncoderTask created successfully\n");
    }

    tid = osThreadNew(read_luffing_encoder_thread, NULL, &read_luffing_encoder_attr);
    if (tid == NULL) {
        LOG_E("Failed to create ReadLuffingEncoderTask\n");
    } else {
        LOG_I("ReadLuffingEncoderTask created successfully\n"); 
    }

    ota_create_consumer_task();

    const osThreadAttr_t attr = {
        .name = "neighbor_report",
        .stack_size = 1024 * 4,
        .priority = (osPriority_t)osPriorityNormal,
    };
    osThreadNew(report_atc_data_task, NULL, &attr);
    
}

void board_get_mac_address(uint8_t *mac)
{
  /* STM32H7 Unique ID at 0x1FF1E800 */
  uint32_t uid[3];
  uid[0] = *(uint32_t*)(0x1FF1E800);
  uid[1] = *(uint32_t*)(0x1FF1E804);
  uid[2] = *(uint32_t*)(0x1FF1E808);
  
  /* Generate MAC address from Unique ID */
  mac[0] = (uid[0] >> 8) & 0xFF;
  mac[1] = (uid[0] >> 0) & 0xFF;
  mac[2] = (uid[1] >> 24) & 0xFF;
  mac[3] = (uid[1] >> 16) & 0xFF;
  mac[4] = (uid[1] >> 8) & 0xFF;
  mac[5] = (uid[1] >> 0) & 0xFF;
}
