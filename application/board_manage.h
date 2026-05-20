#pragma once
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "offline_manage_task.h"

#define ENCODER_FAKE_DATA_MODE 0
extern IWDG_HandleTypeDef hiwdg1;
#define FEED_SYS_WATCHDOG() HAL_IWDG_Refresh(&hiwdg1)

void board_get_mac_address(uint8_t *mac);
void board_system_reset_force(void);
void board_setup_components(void);
void board_create_user_tasks(void);
