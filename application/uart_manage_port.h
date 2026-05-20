// filepath: /home/river/crane_ws/crane_group_anticol_slave/application/uart_manage_port.h
#ifndef UART_MANAGE_PORT_H_
#define UART_MANAGE_PORT_H_

#include <stdint.h>
#include "uart_manage.h"

#ifdef __cplusplus
extern "C" {
#endif

void setup_uart_service(void);
int shell_inform_send(uint8_t *buf, uint16_t len);
int mqtt_inform_send(uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* UART_MANAGE_PORT_H_ */