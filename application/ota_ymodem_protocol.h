#ifndef OTA_YMODEM_PROTOCOL_H_
#define OTA_YMODEM_PROTOCOL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	OTA_YMODEM_STATE_IDLE = 0,
	OTA_YMODEM_STATE_WAIT_HEADER,
	OTA_YMODEM_STATE_RECV_DATA,
	OTA_YMODEM_STATE_WAIT_EOT,
	OTA_YMODEM_STATE_WAIT_LAST_EMPTY,
	OTA_YMODEM_STATE_DONE,
	OTA_YMODEM_STATE_ABORTED
} ota_ymodem_state_e;

typedef enum
{
	OTA_YMODEM_OK = 0,
	OTA_YMODEM_ERR_PARAM = -1,
	OTA_YMODEM_ERR_CRC = -2,
	OTA_YMODEM_ERR_SEQ = -3,
	OTA_YMODEM_ERR_OVERFLOW = -4,
	OTA_YMODEM_ERR_ABORTED = -5
} ota_ymodem_err_e;

typedef struct
{
	int (*send)(const uint8_t *buf, uint16_t len, void *user);
	int (*on_begin)(const char *filename, uint32_t size, void *user);
	int (*on_data)(const uint8_t *data, uint32_t len, void *user);
	int (*on_finish)(uint32_t size, void *user);
	void (*on_error)(int err, void *user);
	void *user;
} ota_ymodem_callbacks_t;

typedef struct
{
	ota_ymodem_state_e state;
	uint8_t frame_buf[1030U];
	uint16_t frame_len;
	uint16_t frame_expected;
	uint8_t block_num;
	uint8_t eot_count;
	uint32_t file_size;
	ota_ymodem_callbacks_t cb;
} ota_ymodem_ctx_t;

void ymodem_init_procotol(ota_ymodem_ctx_t *ctx, const ota_ymodem_callbacks_t *cb);

/**
 * Feed a complete Ymodem frame (no streaming/incremental parsing).
 * The frame should include header, sequence, data, and CRC bytes.
 */
int ymodem_feed_frame(ota_ymodem_ctx_t *ctx, const uint8_t *frame, uint16_t len);
/**
 * Request sender to start Ymodem transfer (send initial 'C').
 * Protocol layer performs the handshake send via provided send callback.
 */
int ymodem_request_start(ota_ymodem_ctx_t *ctx);
/**
 * Request sender to abort Ymodem transfer (send CAN CAN).
 * Protocol layer performs the abort send via provided send callback.
 */
int ymodem_request_abort(ota_ymodem_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* OTA_YMODEM_PROTOCOL_H_ */
