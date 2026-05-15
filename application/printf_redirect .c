// https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-f030R8/system/src/retarget/retarget.c

#include "printf_redirect.h"

#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include "task.h"

#if !defined(OS_USE_SEMIHOSTING)

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

UART_HandleTypeDef *printf_uart;

void specify_redirect_uart(UART_HandleTypeDef *huart) {
  printf_uart = huart;
  /* Disable I/O buffering for STDOUT stream, so that
   * chars are sent out as soon as they are printed. */
  setvbuf(stdout, NULL, _IONBF, 0);
}

int _write(int fd, char* ptr, int len) {
  
  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {

    if(printf_uart == NULL) {
      errno = EBADF;
      return -1;
    }

    HAL_StatusTypeDef hstatus;
    hstatus = HAL_UART_Transmit(printf_uart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    if (hstatus == HAL_OK)
      return len;
    else
      return EIO;
  }
  errno = EBADF;
  return -1;
}

int _read(int fd, char* ptr, int len) {
  HAL_StatusTypeDef hstatus;

  if (fd == STDIN_FILENO) {
    hstatus = HAL_UART_Receive(printf_uart, (uint8_t *) ptr, 1, 1000);
    if (hstatus == HAL_OK)
      return 1;
    else
      return EIO;
  }
  errno = EBADF;
  return -1;
}


#endif //#if !defined(OS_USE_SEMIHOSTING)
