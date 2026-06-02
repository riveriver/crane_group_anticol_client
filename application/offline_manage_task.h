#ifndef OFFLINE_MANAGE_TASK_H
#define OFFLINE_MANAGE_TASK_H

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

typedef void (*offline_t)(void);

typedef enum
{
    NO_OFFLINE = 0, /*!< system is normal */
	OFFLINE_ETH_MANAGE,
    OFFLINE_READ_SWING_ENCODER,
    OFFLINE_READ_LUFFING_ENCODER,
    OFFLINE_READ_LOAD_WEIGHT,
    OFFLINE_PUBLISH_MQTT,
    OFFLINE_EVENT_MAX_NUM,
} offline_event;

struct offline_manage_obj
{
    offline_event event;
    volatile uint8_t enable;
    volatile uint8_t online_state;
    volatile uint8_t last_state;
    uint8_t error_level;
    offline_t offline_first_func;
    offline_t offline_func;
    offline_t online_first_func;
    offline_t online_func;
    uint8_t group_id;
    volatile uint32_t last_time;
    uint32_t offline_time;
};

void offline_manage_enable_event(offline_event event);
void offline_manage_disable_event(offline_event event);
void offline_manage_task(void *argument);
void offline_manage_disable_all_event();

#endif