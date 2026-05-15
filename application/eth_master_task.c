#include "eth_master_task.h"
#include "main.h"
#include "lwip/ip4_addr.h"
#include <stdbool.h>
#include "lwip/netif.h"
/* UDP receive and parse for load weight */
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <stdint.h>
#include <string.h>

#define LOG_D(fmt, ...) printf("[D][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_I(fmt, ...) printf("[I][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[E][%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define ETH_STATIC_IP_ADDR "192.168.1.33"
#define ETH_STATIC_NETMASK "255.255.255.0"
#define ETH_STATIC_GATEWAY "192.168.1.1"

int eth_link_up_timeout_ms = 60 * 1000;


void eth_master_task(void *argument) {

    HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_RESET);
    osDelay(100);
    HAL_GPIO_WritePin(ETH_RESET_GPIO_Port, ETH_RESET_Pin, GPIO_PIN_SET);
    osDelay(100);

    ip4_addr_t static_ip, static_netmask, static_gateway;
    ip4addr_aton(ETH_STATIC_IP_ADDR, &static_ip);
    ip4addr_aton(ETH_STATIC_NETMASK, &static_netmask);
    ip4addr_aton(ETH_STATIC_GATEWAY, &static_gateway);
    lwip_set_net_param(false, static_ip.addr, static_netmask.addr, static_gateway.addr);
    MX_LWIP_Init();

    extern struct netif gnetif;
    {
      uint32_t start_tick = osKernelGetTickCount();
      while (!netif_is_link_up(&gnetif) || !netif_is_up(&gnetif))
      {
        if ((osKernelGetTickCount() - start_tick) > eth_link_up_timeout_ms)
        {
            LOG_E("Network interface is not up after %d ms\n", eth_link_up_timeout_ms);
            break;
        }
        osDelay(100);
      }
    }

    while (1) 
    {
      osDelay(10);
    }
}