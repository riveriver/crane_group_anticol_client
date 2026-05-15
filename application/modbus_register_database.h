#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "cmsis_os.h"
#include "task.h"

// 寄存器映射定义
// 保持寄存器 (Holding Registers) - 可读写
#define REG_HOLDING_SIZE    10

#define REG_SYSTEM_VERSION   0x0000  // 系统版本号
#define REG_RESERVE_01       0x0001  // 保留

#define REG_TOTAL_SENSORS 0x0002  // 传感器总数
#define REG_ERROR_CODE    0x0003  // 错误码

#define REG_SWING_ENCODER_COUNT    0x0004  // 传感器1位置高16位
#define REG_SWING_ENCODER_VALUE    0x0005  // 传感器1位置低16位

#define REG_LUFFING_ENCODER_COUNT    0x0006  // 传感器2位置高16位
#define REG_LUFFING_ENCODER_VALUE    0x0007  // 传感器2位置低16位

#define REG_HOOK_ENCODER_COUNT    0x0008  // 传感器3位置高16位
#define REG_HOOK_ENCODER_VALUE    0x0009  // 传感器3位置低16位

extern uint16_t holding_reg_data[REG_HOLDING_SIZE];
#ifdef __cplusplus
}
#endif