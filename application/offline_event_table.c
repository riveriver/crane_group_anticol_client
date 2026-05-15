#include "offline_event_table.h"
#include "offline_manage_task.h"

#define LOG_D(...) // printf(__VA_ARGS__)
#define LOG_I(...) printf(__VA_ARGS__)
#define LOG_E(...) printf(__VA_ARGS__)

offline_event offline_reset_event = NO_OFFLINE;
/* ============================================================================
 * PROJECT-SPECIFIC EVENT HANDLER IMPLEMENTATIONS
 * ============================================================================ */

/* System protection: log and reset */
void system_protect_offline_first(void)
{
    LOG_E("System protect triggered by offline event: %d\r\n", offline_reset_event);
    /* wait to ensure log is flushed before reset */
    // osDelay(1000);
    // __set_FAULTMASK(1);
    // NVIC_SystemReset();
}

void system_protect_offline(void)
{
    LOG_E("system protect caused by offline event: %d\r\n", offline_reset_event);
}

/* BMS event handlers */
void bms_offline_first(void)
{
    LOG_E("BMS offline detected\r\n");
}

/* GPS event handlers */
void gps_offline_first(void)
{
    LOG_E("GPS offline detected\r\n");
}

/* Heartbeat event handlers */
void heartbeat_offline_first(void)
{
    LOG_E("Heartbeat offline detected\r\n");
}

/* ============================================================================
 * PROJECT-SPECIFIC OFFLINE EVENT TABLE
 * ============================================================================ */

struct offline_manage_obj offline_event_table[] =
{
    /* use designated initializers to avoid field-order bugs */
    {
        .event = SYSTEM_PROTECT,
        .enable = ENABLE,
        .error_level = OFFLINE_ERROR_LEVEL,
        .group_id = 0,
        .offline_time = 0,
        .offline_first_func = system_protect_offline_first,
        .offline_func = system_protect_offline,
        .online_first_func = NULL,
        .online_func = NULL
    },
    {
        .event = OFFLINE_ETH_MANAGE_TASK,
        .enable = ENABLE,
        .error_level = OFFLINE_ERROR_LEVEL,
        .group_id = 3,
        .offline_time = 1000,
        .offline_first_func = NULL,
        .offline_func = NULL,
        .online_first_func = NULL,
        .online_func = NULL
    },
    {
        .event = OFFLINE_READ_SWING_TASK,
        .enable = ENABLE,
        .error_level = OFFLINE_ERROR_LEVEL,
        .group_id = 2,
        .offline_time = 1000,
        .offline_first_func = NULL,
        .offline_func = NULL,
        .online_first_func = NULL,
        .online_func = NULL
    },
    {
        .event = OFFLINE_READ_LUFFING_TASK,
        .enable = ENABLE,
        .error_level = OFFLINE_WARNING_LEVEL,
        .group_id = 2,
        .offline_time = 1000,
        .offline_first_func = NULL,
        .offline_func = NULL,
        .online_first_func = NULL,
        .online_func = NULL
    },
};

/* Table size for runtime use (instead of sizeof calculations) */
int offline_event_table_size = sizeof(offline_event_table) / sizeof(struct offline_manage_obj);
