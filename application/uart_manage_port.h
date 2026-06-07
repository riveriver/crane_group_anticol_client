// filepath: /home/river/crane_ws/crane_group_anticol_slave/application/uart_manage_port.h
#ifndef UART_MANAGE_PORT_H_
#define UART_MANAGE_PORT_H_

#include "board_manage.h"
#include "uart_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_uart_service(void);
int32_t shell_inform_send(uint8_t *buf, uint16_t len);
int32_t mqtt_inform_send(uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* UART_MANAGE_PORT_H_ */
