# Crane Group Anti-collision Client - Release Notes

---

## 功能清单 (Release v0.1.0)

### 1. 串行通信
- ✅ **UART 驱动**
  - UART1、UART2、UART3（波特率115200）
  - UART7、UART8（波特率9600）
  - DMA 传输支持（UART1/7、UART2/3/8）

- ✅ **Printf 重定向**
  - 标准输出重定向到 USART1
  - 带时间戳的日志宏：`LOG_D()`, `LOG_I()`, `LOG_E()`
  - 支持函数名和行号打印

### 2. 系统监控与反馈
- ✅ **LED 心跳**
  - LED1 每秒闪烁一次（默认任务）
  - 位置：GPIO PE9

- ✅ **独立看门狗 (IWDG)**
  - 看门狗周期性喂狗防止复位
  - 在默认任务循环中每秒喂狗一次

- ✅ **MAC 地址获取与显示**
  - 从 STM32H743 唯一 ID 区域生成 MAC 地址
  - 在启动日志中打印：`MAC:XX:XX:XX:XX:XX:XX`

### 3. 用户任务框架
- ✅ **任务创建接口**
  - `board_create_user_tasks()` - 集中管理用户任务创建
  - 支持动态任务生成

- ✅ **板卡组件管理**
  - `board_setup_components()` - 硬件初始化
  - `board_get_mac_addr()` - MAC 地址查询

---

## 提交

| 提交哈希 | 描述 |
|---------|------|
| `179cec8` | feat: add DMA support for UART8 and USART3 |
| `5286566` | chore: remove unused includes from board_manage.c |
| `21def9e` | feat: implement printf redirection to usart1 |
| `5d60def` | feat: implement heartbeat led in StartDefaultTask function |
| `d9aba8b` | feat: feed watchdog in StartDefaultTask function |
| `1c945a6` | feat: init project |
| `9b504e7` | chore: Add. gitignore file to exclude debugging and configuration files |

---

## 系统架构

```
┌─────────────────────────────────────────────────────┐
│         FreeRTOS RTOS (CMSIS-RTOS V2)               │
├─────────────────────────────────────────────────────┤
│  defaultTask (LED/看门狗)                            │ 
└─────────────────────────────────────────────────────┘
         ▲                   
         │                  
    ┌────┴────┐        
    │   HAL   │        
    └─────────┘        
         │                   
    ┌────┴──────────────┬
    │   STM32H743       │
    │   (UART/GPIO)     │
    └───────────────────┴
```

## 许可证

© 2025 All Rights Reserved
