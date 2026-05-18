#include "ota_ymodem_protocol.h"

#include <string.h>

#define LOGD(...) printf(__VA_ARGS__)

#define YMODEM_SOH                0x01U
#define YMODEM_STX                0x02U
#define YMODEM_EOT                0x04U
#define YMODEM_ACK                0x06U
#define YMODEM_NAK                0x15U
#define YMODEM_CAN                0x18U
#define YMODEM_CRC_CHAR           0x43U
#define YMODEM_SOH_DATA_LEN       128U
#define YMODEM_STX_DATA_LEN       1024U
#define YMODEM_FRAME_OVERHEAD     5U

static uint16_t ymodem_crc16_ccitt(const uint8_t *buf, uint32_t len)
{
	uint16_t crc = 0U;

	for (uint32_t i = 0U; i < len; ++i)
	{
		crc ^= (uint16_t)buf[i] << 8;
		for (uint8_t j = 0U; j < 8U; ++j)
		{
			if ((crc & 0x8000U) != 0U)
			{
				crc = (uint16_t)((crc << 1U) ^ 0x1021U);
			}
			else
			{
				crc <<= 1U;
			}
		}
	}

	return crc;
}

static int ymodem_send_byte(const ota_ymodem_callbacks_t *cb, uint8_t val)
{
	if ((cb == NULL) || (cb->send == NULL))
	{
		return OTA_YMODEM_ERR_IO;
	}
    
	vTaskDelay(60);
    LOGD("Ymodem send byte: 0x%02X\r\n", val);
	return cb->send(&val, 1U, cb->user);
}

static void ymodem_notify_error(ota_ymodem_ctx_t *ctx, int err)
{
	if ((ctx != NULL) && (ctx->cb.on_error != NULL))
	{
		ctx->cb.on_error(err, ctx->cb.user);
	}
}

static int ymodem_handle_header(ota_ymodem_ctx_t *ctx, const uint8_t *data, uint16_t len)
{
	if ((ctx == NULL) || (data == NULL) || (len == 0U))
	{
		return OTA_YMODEM_ERR_PARAM;
	}
     
    // 获取文件名
	const char *filename = (const char *)data;
    // 传输最终确认包
	if (filename[0] == '\0')
	{
		(void)ymodem_send_byte(&ctx->cb, YMODEM_ACK);
		ctx->state = OTA_YMODEM_STATE_DONE;
		if (ctx->cb.on_finish != NULL)
		{
			(void)ctx->cb.on_finish(ctx->file_size, ctx->cb.user);
		}
		return OTA_YMODEM_OK;
	}
    
    // 首包信息包
    // 获取文件大小
	const char *size_str = filename + strlen(filename) + 1U;
	uint32_t size_val = 0U;
	while ((*size_str >= '0') && (*size_str <= '9'))
	{
		size_val = (size_val * 10U) + (uint32_t)(*size_str - '0');
		size_str++;
	}
	ctx->file_size = size_val;
    
    // 让上层应用准备接收环境（擦除 Flash）
	if (ctx->cb.on_begin != NULL)
	{
		int ret = ctx->cb.on_begin(filename, size_val, ctx->cb.user);
		if (ret != 0)
		{
			ymodem_notify_error(ctx, ret);
			return ret;
		}
	}
    
    // 发送 ACK + CRC 请求数据包，进入数据接收状态
	(void)ymodem_send_byte(&ctx->cb, YMODEM_ACK);
	(void)ymodem_send_byte(&ctx->cb, YMODEM_CRC_CHAR);
	ctx->state = OTA_YMODEM_STATE_RECV_DATA;
	ctx->block_num = 1U;
	ctx->eot_count = 0U;
	return OTA_YMODEM_OK;
}

static int ymodem_handle_data(ota_ymodem_ctx_t *ctx, const uint8_t *data, uint16_t len)
{
	if ((ctx == NULL) || (data == NULL) || (len == 0U))
	{
		return OTA_YMODEM_ERR_PARAM;
	}

	if (ctx->cb.on_data != NULL)
	{
		int ret = ctx->cb.on_data(data, len, ctx->cb.user);
		if (ret != 0)
		{
			ymodem_notify_error(ctx, ret);
			return ret;
		}
	}

	(void)ymodem_send_byte(&ctx->cb, YMODEM_ACK);
	return OTA_YMODEM_OK;
}

static int ymodem_process_frame(ota_ymodem_ctx_t *ctx)
{
    // 值检查：防御性编程，防止空指针解引用和无效操作
	if ((ctx == NULL) || (ctx->frame_len < YMODEM_FRAME_OVERHEAD))
	{
		return OTA_YMODEM_ERR_PARAM;
	}
    
    // 检查帧长
	uint8_t head = ctx->frame_buf[0];
	uint16_t data_len = (head == YMODEM_SOH) ? YMODEM_SOH_DATA_LEN : YMODEM_STX_DATA_LEN;
	if (ctx->frame_len != (uint16_t)(data_len + YMODEM_FRAME_OVERHEAD))
	{
		return OTA_YMODEM_ERR_PARAM;
	}
    
    // 检查帧序号
	uint8_t blk = ctx->frame_buf[1];
	uint8_t blk_inv = ctx->frame_buf[2];
	if ((uint8_t)(blk + blk_inv) != 0xFFU)
	{
		(void)ymodem_send_byte(&ctx->cb, YMODEM_NAK);
		return OTA_YMODEM_ERR_SEQ;
	}
    
    // 检验CRC
	const uint8_t *data = &ctx->frame_buf[3];
	uint16_t crc_rx = (uint16_t)((uint16_t)ctx->frame_buf[3 + data_len] << 8) | ctx->frame_buf[4 + data_len];
	uint16_t crc_calc = ymodem_crc16_ccitt(data, data_len);
	if (crc_rx != crc_calc)
	{
		(void)ymodem_send_byte(&ctx->cb, YMODEM_NAK);
		return OTA_YMODEM_ERR_CRC;
	}
    
    // 处理首包（信息包）
	if (blk == 0U)
	{
		if ((ctx->state == OTA_YMODEM_STATE_WAIT_HEADER) || (ctx->state == OTA_YMODEM_STATE_WAIT_LAST_EMPTY))
		{
			return ymodem_handle_header(ctx, data, data_len);
		}

		(void)ymodem_send_byte(&ctx->cb, YMODEM_NAK);
		return OTA_YMODEM_ERR_SEQ;
	}

	if (ctx->state != OTA_YMODEM_STATE_RECV_DATA)
	{
		(void)ymodem_send_byte(&ctx->cb, YMODEM_NAK);
		return OTA_YMODEM_ERR_SEQ;
	}
 
    // 处理数据包
	if (blk == ctx->block_num)
	{
		int ret = ymodem_handle_data(ctx, data, data_len);
		if (ret == OTA_YMODEM_OK)
		{
			ctx->block_num++;
		}
		return ret;
	}
    
    // 处理重包
	if (blk == (uint8_t)(ctx->block_num - 1U))
	{
		(void)ymodem_send_byte(&ctx->cb, YMODEM_ACK);
		return OTA_YMODEM_OK;
	}

	(void)ymodem_send_byte(&ctx->cb, YMODEM_NAK);
	return OTA_YMODEM_ERR_SEQ;
}

static int ymodem_handle_control(ota_ymodem_ctx_t *ctx, uint8_t val)
{
	if (ctx == NULL)
	{
		return OTA_YMODEM_ERR_PARAM;
	}
    
    // 正常传输结束：两次确认（NAK → ACK），RECV_DATA → WAIT_EOT → WAIT_LAST_EMPTY → DONE
	if (val == YMODEM_EOT)
	{
		if (ctx->state == OTA_YMODEM_STATE_RECV_DATA)
		{
			(void)ymodem_send_byte(&ctx->cb, YMODEM_NAK);
			ctx->state = OTA_YMODEM_STATE_WAIT_EOT;
			ctx->eot_count = 1U;
			return OTA_YMODEM_OK;
		}

		if (ctx->state == OTA_YMODEM_STATE_WAIT_EOT)
		{
			(void)ymodem_send_byte(&ctx->cb, YMODEM_ACK);
			(void)ymodem_send_byte(&ctx->cb, YMODEM_CRC_CHAR);
			ctx->state = OTA_YMODEM_STATE_WAIT_LAST_EMPTY;
			return OTA_YMODEM_OK;
		}
	}
    
    // 异常中止：无握手，立即生效，任意状态 → ABORTED
	if (val == YMODEM_CAN)
	{
		ctx->state = OTA_YMODEM_STATE_ABORTED;
		ymodem_notify_error(ctx, OTA_YMODEM_ERR_ABORTED);
		return OTA_YMODEM_ERR_ABORTED;
	}

	return OTA_YMODEM_OK;
}

void ota_ymodem_init(ota_ymodem_ctx_t *ctx, const ota_ymodem_callbacks_t *cb)
{
	if (ctx == NULL)
	{
		return;
	}

	memset(ctx, 0, sizeof(*ctx));
	if (cb != NULL)
	{
		ctx->cb = *cb;
	}
	ctx->state = OTA_YMODEM_STATE_WAIT_HEADER;
}

void ota_ymodem_reset(ota_ymodem_ctx_t *ctx)
{
	if (ctx == NULL)
	{
		return;
	}

	ctx->state = OTA_YMODEM_STATE_WAIT_HEADER;
	ctx->frame_len = 0U;
	ctx->frame_expected = 0U;
	ctx->block_num = 1U;
	ctx->eot_count = 0U;
	ctx->file_size = 0U;
}

ota_ymodem_state_e ota_ymodem_get_state(ota_ymodem_ctx_t *ctx)
{
	if (ctx == NULL)
	{
		return OTA_YMODEM_STATE_IDLE;
	}
	return ctx->state;
}

// 设计思想：状态机 + 增量解析，支持分片接收，采用流式处理方式，能够处理不完整帧、多帧拼接和跨包边界的情况
int ota_ymodem_feed(ota_ymodem_ctx_t *ctx, const uint8_t *buf, uint16_t len)
{
	if ((ctx == NULL) || (buf == NULL) || (len == 0U))
	{
		return OTA_YMODEM_ERR_PARAM;
	}

	uint16_t idx = 0U;
	while (idx < len)
	{
		if (ctx->frame_expected == 0U)
		{
			uint16_t search = idx;
			while (search < len)
			{
				uint8_t head = buf[search];
				if ((head == YMODEM_SOH) || (head == YMODEM_STX))
				{
					uint16_t data_len = (head == YMODEM_SOH) ? YMODEM_SOH_DATA_LEN : YMODEM_STX_DATA_LEN;
					ctx->frame_expected = (uint16_t)(data_len + YMODEM_FRAME_OVERHEAD);
					ctx->frame_len = 0U;
					idx = search;
					break;
				}
				if ((head == YMODEM_EOT) || (head == YMODEM_CAN))
				{
					(void)ymodem_handle_control(ctx, head);
				}
				search++;
			}

			if (ctx->frame_expected == 0U)
			{
				break;
			}
		}

		uint16_t to_copy = (uint16_t)(ctx->frame_expected - ctx->frame_len);
		if (to_copy > (uint16_t)(len - idx))
		{
			to_copy = (uint16_t)(len - idx);
		}
        
        // 增量累积机制：将接收到的数据逐步填充到帧缓冲区中，直到达到预期的帧长度后进行处理。
        // 实现断点续传：即使数据分多次到达，也能正确拼接
		memcpy(&ctx->frame_buf[ctx->frame_len], &buf[idx], to_copy);
		ctx->frame_len = (uint16_t)(ctx->frame_len + to_copy);
		idx = (uint16_t)(idx + to_copy);

		if (ctx->frame_len >= ctx->frame_expected)
		{
			int ret = ymodem_process_frame(ctx);
			if (ret == OTA_YMODEM_OK)
			{
				ctx->frame_expected = 0U;
				ctx->frame_len = 0U;
			}
			else
			{
				uint16_t resync = 1U;
				while (resync < ctx->frame_len)
				{
					uint8_t head = ctx->frame_buf[resync];
					if ((head == YMODEM_SOH) || (head == YMODEM_STX))
					{
						uint16_t data_len = (head == YMODEM_SOH) ? YMODEM_SOH_DATA_LEN : YMODEM_STX_DATA_LEN;
						uint16_t remaining = (uint16_t)(ctx->frame_len - resync);
						memmove(ctx->frame_buf, &ctx->frame_buf[resync], remaining);
						ctx->frame_len = remaining;
						ctx->frame_expected = (uint16_t)(data_len + YMODEM_FRAME_OVERHEAD);
						break;
					}
					resync++;
				}

				if (resync >= ctx->frame_len)
				{
					ctx->frame_expected = 0U;
					ctx->frame_len = 0U;
				}
			}
		}
	}

	return OTA_YMODEM_OK;
}

int ota_ymodem_request_start(ota_ymodem_ctx_t *ctx)
{
	if ((ctx == NULL) || (ctx->cb.send == NULL))
	{
		return OTA_YMODEM_ERR_IO;
	}
    
    return ymodem_send_byte(&ctx->cb, YMODEM_CRC_CHAR);
}
