#include "uart_manage.h"
#include <string.h>
#include "am_modbus.h"
#include "at_protocol_handler.h"
#include "ota_service_task.h"

#define LOG_D(...) // printf(__VA_ARGS__)
#define LOG_I(...) printf(__VA_ARGS__)
#define LOG_E(...) printf(__VA_ARGS__)

/* DMA buffer placement */
#if defined(__GNUC__)
#define DMA_BUFFER __attribute__((section(".dma_buffer"), aligned(32)))
#else
#define DMA_BUFFER
#endif

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
static uint8_t uart1_send_buff[256U] DMA_BUFFER;
static uint8_t uart1_send_fifo_buff[256U] DMA_BUFFER;
static uint8_t uart1_recv_buff[512U] DMA_BUFFER;
static uint8_t uart1_process_buff[1024U] DMA_BUFFER;

extern UART_HandleTypeDef huart8;
extern DMA_HandleTypeDef hdma_uart8_rx;
static uint8_t uart8_send_buff[256U] DMA_BUFFER;
static uint8_t uart8_send_fifo_buff[256U] DMA_BUFFER;
static uint8_t uart8_recv_buff[2048U] DMA_BUFFER;
static uint8_t uart8_process_buff[2048U] DMA_BUFFER;

int shell_inform_send(uint8_t *buf, uint16_t len)
{
	(void)uart_manage_dma_send_by_name("shell", buf, len);
	return 0U;
}

int mqtt_inform_send(uint8_t *buf, uint16_t len)
{
	static const uint8_t prefix[] = "1,";
	const uint16_t prefix_len = (uint16_t)(sizeof(prefix) - 1U);
	(void)uart_manage_dma_send_by_name("4g", (uint8_t *)prefix, prefix_len);
	(void)uart_manage_dma_send_by_name("4g", buf, len);
	return 0U;
}

static int32_t uart_shell_recv_callback(uint8_t *buf, uint16_t len)
{
	int32_t ret;

	/* AT command packets should be consumed immediately after a prefix match. */
	ret = craner_at_handler(buf, len, shell_inform_send);
	if (ret != AT_PREFIX_NOT_MATCH)
	{
		return (ret < 0) ? -1 : 0;
	}

	ret = usr_at_handler(buf, len);
	if (ret != AT_PREFIX_NOT_MATCH)
	{
		return (ret < 0) ? -1 : 0;
	}

	{
		static const uint8_t prefix[] = "[SHELL] ";
		const uint16_t prefix_len = (uint16_t)(sizeof(prefix) - 1U);
		(void)uart_manage_dma_send_by_name("shell", (uint8_t *)prefix, prefix_len);
		(void)uart_manage_dma_send_by_name("shell", buf, len);
		return 0U;
	}
}

static int32_t uart_4g_recv_callback(uint8_t *buf, uint16_t len)
{
	if ((buf == NULL) || (len == 0U))
	{
		return -1;
	}

	if ((len >= 2U) && (buf[0] == '1') && (buf[1] == ','))
	{
		int32_t ret;

		ret = craner_at_handler(&buf[2], len - 2U, mqtt_inform_send);
		if (ret != AT_PREFIX_NOT_MATCH)
		{
			return (ret < 0) ? -1 : 0;
		}
		ret = usr_at_handler(&buf[2], len - 2U);
		if (ret != AT_PREFIX_NOT_MATCH)
		{
			return (ret < 0) ? -1 : 0;
		}
		return 0U;
	}
	else if ((len >= 2U) && (buf[0] == '2') && (buf[1] == ','))
	{

		uint16_t data_len = len - 2U;
		if (data_len == 0U)
		{
			return -1;
		}

		if (g_ota.rx_queue == NULL)
		{
			return -1;
		}
		
		static ota_rx_chunk_t msg;
		if (data_len > sizeof(msg.data))
		{
			data_len = sizeof(msg.data);
		}

		msg.len = data_len;
		memcpy(msg.data, &buf[2], data_len);
		(void)osMessageQueuePut(g_ota.rx_queue, &msg, 0U, 0U);
		return 0U;
	}

	{
		shell_inform_send(buf, len);
		mqtt_inform_send(buf, len);
		return 0U;
	}
}

const uart_inferface_t uart_manage_table[] = {
  {
    .name = "shell",
    .uart_h = &huart1,
    .dma_h = &hdma_usart1_rx,
    .recv_buffer = uart1_recv_buff,
    .recv_buffer_size = sizeof(uart1_recv_buff),
    .process_buffer = uart1_process_buff,
    .process_buffer_size = sizeof(uart1_process_buff),
    .recv_callback = uart_shell_recv_callback,
    .send_buffer = uart1_send_buff,
    .send_buffer_size = sizeof(uart1_send_buff),
    .send_fifo_buffer = uart1_send_fifo_buff,
    .send_fifo_size = sizeof(uart1_send_fifo_buff),
    .send_callback = NULL,
  },
  {
    .name = "4g",
    .uart_h = &huart8,
    .dma_h = &hdma_uart8_rx,
    .recv_buffer = uart8_recv_buff,
    .recv_buffer_size = sizeof(uart8_recv_buff),
    .process_buffer = uart8_process_buff,
    .process_buffer_size = sizeof(uart8_process_buff),
	.recv_callback = uart_4g_recv_callback,
    .send_buffer = uart8_send_buff,
    .send_buffer_size = sizeof(uart8_send_buff),
    .send_fifo_buffer = uart8_send_fifo_buff,
    .send_fifo_size = sizeof(uart8_send_fifo_buff),
    .send_callback = NULL,
  },
};

const uint16_t uart_manage_table_size =
  (uint16_t)(sizeof(uart_manage_table) / sizeof(uart_manage_table[0]));

void setup_uart_service(void)
{
  (void)uart_manage_init_table(uart_manage_table, uart_manage_table_size);
  uart_manage_enable_dma_recv_by_name("shell");
  uart_manage_enable_dma_recv_by_name("4g");
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
/* support am_modbus begin */
#if  ENABLE_USART_DMA ==  1
 for (int i = 0; i < numberHandlers; i++ )
 {
    	if (mHandlers[i]->port == huart  )
    	{

    		if(mHandlers[i]->xTypeHW == USART_HW_DMA)
    		{
    			while(HAL_UARTEx_ReceiveToIdle_DMA(mHandlers[i]->port, mHandlers[i]->xBufferRX.uxBuffer, MAX_BUFFER) != HAL_OK)
    		    {
    					HAL_UART_DMAStop(mHandlers[i]->port);
   				}
				__HAL_DMA_DISABLE_IT(mHandlers[i]->port->hdmarx, DMA_IT_HT); // we don't need half-transfer interrupt

    		}

    		break;
    	}
   }
#endif
/* support am_modbus end */

  (void)uart_manage_enable_dma_recv(huart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	/* support am_modbus begin*/
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	for (int i = 0; i < numberHandlers; i++ )
	{
	   	if (mHandlers[i]->port == huart  )
	   	{
	   		// notify the end of TX
	   		xTaskNotifyFromISR(mHandlers[i]->myTaskModbusAHandle, 0, eNoAction, &xHigherPriorityTaskWoken);
	   		break;
	   	}

	}
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	/* support am_modbus end*/

	uart_manage_send_completed_hook(huart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	/* support am_modbus begin*/
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    for (int i = 0; i < numberHandlers; i++ )
    {
    	if (mHandlers[i]->port == huart)
    	{

    		if(mHandlers[i]->xTypeHW == USART_HW)
    		{
    			RingAdd(&mHandlers[i]->xBufferRX, mHandlers[i]->dataRX);
    			HAL_UART_Receive_IT(mHandlers[i]->port, &mHandlers[i]->dataRX, 1);
    			xTimerResetFromISR(mHandlers[i]->xTimerT35, &xHigherPriorityTaskWoken);
    		}
    		break;
    	}
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	/* support am_modbus end*/

    (void *)huart;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{

/* support am_modbus begin */
#if  ENABLE_USART_DMA ==  1
	    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
	    int i;
	    for (i = 0; i < numberHandlers; i++ )
	    {
	    	if (mHandlers[i]->port == huart  )
	    	{


	    		if(mHandlers[i]->xTypeHW == USART_HW_DMA)
	    		{
	    			if(size) //check if we have received any byte
	    			{
		    				mHandlers[i]->xBufferRX.u8available = size;
		    				mHandlers[i]->xBufferRX.overflow = false;

		    				while(HAL_UARTEx_ReceiveToIdle_DMA(mHandlers[i]->port, mHandlers[i]->xBufferRX.uxBuffer, MAX_BUFFER) != HAL_OK)
		    				{
		    					HAL_UART_DMAStop(mHandlers[i]->port);

		    				}
		    				__HAL_DMA_DISABLE_IT(mHandlers[i]->port->hdmarx, DMA_IT_HT); // we don't need half-transfer interrupt

		    				xTaskNotifyFromISR(mHandlers[i]->myTaskModbusAHandle, 0 , eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
	    			}
	    		}

	    		break;
	    	}
	    }
	    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#endif
/* support am_modbus end */

  uart_inferface_t *m_obj = uart_manage_get_obj(huart);

  if (m_obj != NULL)
  {
    if (size > 0U)
    {
      (void)uart_manage_recv_idle_hook(m_obj, INTERRUPT_TYPE_UART, size);
    }
    (void)uart_manage_enable_dma_recv(huart);
  }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  (void *)huart;
}



