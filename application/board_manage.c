#include "board_manage.h"
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "eth_master_task.h"
#include "read_load_task.h"

#define LOG_D(fmt, ...) printf("[D][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_I(fmt, ...) printf("[I][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[E][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)    

extern UART_HandleTypeDef huart1;

void board_get_mac_address(uint8_t *mac);

void board_setup_components(void) {
    specify_redirect_uart(&huart1);
}

void board_create_user_tasks(void) {

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
