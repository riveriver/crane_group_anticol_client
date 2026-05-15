#pragma once
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "offline_manage_task.h"

void board_get_mac_address(uint8_t *mac);
void board_setup_components(void);
void board_create_user_tasks(void);
