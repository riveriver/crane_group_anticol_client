#include "board_manage.h"

uint8_t is_usr_4g_at_command(const uint8_t *buf, uint16_t len)
{
	static const char at_prefix[] = "usr.cn#AT";
	const uint16_t at_prefix_len = (uint16_t)(sizeof(at_prefix) - 1U);
	uint16_t index = 0U;

	while (index < len)
	{
		if ((buf[index] != ' ') && (buf[index] != '\t') && (buf[index] != '\r') && (buf[index] != '\n'))
		{
			break;
		}
		index++;
	}

	if ((len - index) < at_prefix_len)
	{
		return 0U;
	}

	if (memcmp(&buf[index], at_prefix, at_prefix_len) == 0)
	{
		return 1U;
	}

	return 0U;
}

uint32_t uart_send_by_4g(uint8_t *buf, uint16_t len)
{
  (void)uart_manage_dma_send_by_name("4g", buf, len);
  return 0U;
}