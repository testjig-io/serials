#include "lightserial.h"

#define LS_PKT_CMD_IDX	0
#define LS_PKT_ARG_IDX	1
#define LS_PKT_DATA_IDX	2

void _ls_pkt_init(ls_pkt_t *pkt);
void _ls_pkt_send(ls_t *instance, ls_pkt_t *pkt);
void _ls_send_byte(ls_t *instance, uint8_t data);
void _ls_send_data(ls_t *instance, uint8_t *data, uint32_t count);

void ls_init(ls_t *instance, ls_init_t *init)
{
	instance->rxpkt_ready = false;
	instance->rx_cnt = 0;
	instance->bs_esc = false;

	_ls_pkt_init(&(instance->rx_pkt));
	instance->rx_buf = init->rx_buf;
	instance->rx_buf_len = init->rx_buf_len;

	instance->fn_send_byte = init->fn_send_byte;
	instance->cb_rxpkt_ready = init->cb_rxpkt_ready;
}

void ls_parse_bytes(ls_t *instance, uint8_t *data, uint32_t count)
{
	uint32_t i;
	for(i = 0; i < count; i++)
	{
		uint8_t b = data[i];

		if(b == LS_BS_ESC && !instance->bs_esc)
		{
			instance->bs_esc = true;
			continue;
		}

		if(b == LS_BS_BEGIN && !instance->bs_esc)
		{
			_ls_pkt_init(&(instance->rx_pkt));
			instance->rx_cnt = 0;
			instance->rxpkt_ready = false;
			continue;
		}

		if(b == LS_BS_END && !instance->bs_esc)
		{
			if(instance->cb_rxpkt_ready != 0)
			{
				instance->cb_rxpkt_ready();
			}
			else
			{
				instance->rxpkt_ready = true;
			}
			continue;
		}

		uint32_t rx_i = instance->rx_cnt;
		instance->rx_buf[rx_i] = b;
		ls_pkt_t *rx_pkt = &(instance->rx_pkt);

		switch(rx_i)
		{
		case LS_PKT_CMD_IDX:
			rx_pkt->cmd = (ls_cmd_t) b;
			break;
		case LS_PKT_ARG_IDX:
			rx_pkt->arg = b;
			break;
		case LS_PKT_DATA_IDX:
			rx_pkt->data = instance->rx_buf + LS_PKT_DATA_IDX;
			rx_pkt->data_count = 1;
			break;
		default:
		    rx_pkt->data_count += 1;
		}

		instance->rx_cnt += 1;
		instance->bs_esc = false;
	}
}

bool ls_rxpkt_ready(ls_t *instance)
{
	return instance->rxpkt_ready;
}

void ls_rxpkt_ready_clear(ls_t *instance)
{
	instance->rxpkt_ready = false;
}

ls_pkt_t* ls_rxpkt(ls_t *instance)
{
    return &(instance->rx_pkt);
}

void _ls_send_byte(ls_t *instance, uint8_t data)
{
	instance->fn_send_byte(data);
}
void _ls_send_data(ls_t *instance, uint8_t *data, uint32_t count)
{
	uint32_t i;
	for(i = 0; i < count; i++)
	{
		if(data[i] == LS_BS_BEGIN || data[i] == LS_BS_END || data[i] == LS_BS_ESC)
		{
			_ls_send_byte(instance, LS_BS_ESC);
		}
		_ls_send_byte(instance, data[i]);
	}
}

void _ls_pkt_send(ls_t *instance, ls_pkt_t *pkt)
{
	_ls_send_byte(instance, LS_BS_BEGIN);
	_ls_send_data(instance, (uint8_t*)&(pkt->cmd), 1);
	_ls_send_data(instance, &(pkt->arg), 1);
	if(pkt->data != 0 && pkt->data_count > 0)
		_ls_send_data(instance, pkt->data, pkt->data_count);
	_ls_send_byte(instance, LS_BS_END);
}

void _ls_pkt_init(ls_pkt_t *pkt)
{
	pkt->arg = LS_NO_ARG;
	pkt->data = 0;
	pkt->data_count = 0;
}

void ls_send_ack(ls_t *instance)
{
	ls_pkt_t pkt;
	_ls_pkt_init(&pkt);
	pkt.cmd = LS_CMD_ACK;
	_ls_pkt_send(instance, &pkt);
}

void ls_send_err(ls_t *instance, uint8_t err_code)
{
	ls_pkt_t pkt;
	_ls_pkt_init(&pkt);
	pkt.cmd = LS_CMD_ERR;
	pkt.arg = err_code;
	_ls_pkt_send(instance, &pkt);
}

void ls_send_bytes(ls_t *instance, uint8_t *data, uint32_t count)
{
	ls_pkt_t pkt;
	_ls_pkt_init(&pkt);
	pkt.cmd = LS_CMD_BYTES;
	pkt.data = data;
	pkt.data_count = count;
	_ls_pkt_send(instance, &pkt);
}

