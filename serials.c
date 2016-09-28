#include "serials.h"

void _serials_pkt_init(serials_pkt_t *pkt);
void _serials_pkt_send(serials_t *instance, serials_pkt_t *pkt);
void _serials_rxpkt_build(serials_t *instance);
void _serials_send_byte(serials_t *instance, uint8_t data);
void _serials_send_data(serials_t *instance, uint8_t *data, uint32_t count);

void serials_init(serials_t *instance, serials_init_t *init)
{
    instance->rxpkt_ready = false;
    instance->rx_cnt = 0;
    instance->bs_esc = false;
    instance->overflow = false;

    _serials_pkt_init(&(instance->rx_pkt));
    instance->rx_buf = init->rx_buf;
    instance->rx_buf_len = init->rx_buf_len;

    instance->fn_send_byte = init->fn_send_byte;
    instance->cb_rxpkt_ready = init->cb_rxpkt_ready;
}

void serials_parse_bytes(serials_t *instance, uint8_t *data, uint32_t count)
{
    uint32_t i;
    for(i = 0; i < count; i++)
    {
        uint8_t b = data[i];

        if(b == SERIALS_BS_ESC && !instance->bs_esc)
        {
            instance->bs_esc = true;
            continue;
        }

        if(b == SERIALS_BS_BEGIN && !instance->bs_esc)
        {
            _serials_pkt_init(&(instance->rx_pkt));
            instance->rx_cnt = 0;
            instance->rxpkt_ready = false;
            continue;
        }

        if(b == SERIALS_BS_END && !instance->bs_esc)
        {
            _serials_rxpkt_build(instance);
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

        if(instance->rx_cnt < instance->rx_buf_len)
        {
            instance->rx_buf[instance->rx_cnt] = b;
            instance->rx_cnt += 1;
        }
        else
        {
            instance->overflow = true;
        }

        instance->bs_esc = false;
    }
}

bool serials_overflow(serials_t *instance)
{
    return instance->overflow;
}

bool serials_rxpkt_ready(serials_t *instance)
{
    return instance->rxpkt_ready;
}

void serials_rxpkt_ready_clear(serials_t *instance)
{
    instance->rxpkt_ready = false;
}

serials_pkt_t* serials_rxpkt(serials_t *instance)
{
    if(instance->rxpkt_ready)
        return &(instance->rx_pkt);
    else
        return 0;
}

void _serials_send_byte(serials_t *instance, uint8_t data)
{
    instance->fn_send_byte(data);
}
void _serials_send_data(serials_t *instance, uint8_t *data, uint32_t count)
{
    uint32_t i;
    for(i = 0; i < count; i++)
    {
        if(data[i] == SERIALS_BS_BEGIN ||
                data[i] == SERIALS_BS_END ||
                data[i] == SERIALS_BS_ESC)
        {
            _serials_send_byte(instance, SERIALS_BS_ESC);
        }
        _serials_send_byte(instance, data[i]);
    }
}

void _serials_pkt_send(serials_t *instance, serials_pkt_t *pkt)
{
    _serials_send_byte(instance, SERIALS_BS_BEGIN);
    _serials_send_data(instance, (uint8_t*)&(pkt->cmd), 1);
    _serials_send_data(instance, &(pkt->arg), 1);
    if(pkt->data != 0 && pkt->data_count > 0)
        _serials_send_data(instance, pkt->data, pkt->data_count);
    _serials_send_byte(instance, SERIALS_BS_END);
}

void _serials_pkt_init(serials_pkt_t *pkt)
{
    pkt->arg = SERIALS_NO_ARG;
    pkt->data = 0;
    pkt->data_count = 0;
}

void _serials_rxpkt_build(serials_t *instance)
{
    uint8_t *rxdata = instance->rx_buf;
    uint32_t rxdata_count = instance->rx_cnt;
    serials_pkt_t *pkt = &(instance->rx_pkt);
    pkt->cmd = (serials_cmd_t) rxdata[0];
    pkt->arg = rxdata[1];
    pkt->data_count = rxdata_count - 2;
    if(pkt->data_count > 0)
    {
        pkt->data = rxdata + 2;
    }
}

void serials_send_ack(serials_t *instance)
{
    serials_pkt_t pkt;
    _serials_pkt_init(&pkt);
    pkt.cmd = SERIALS_CMD_ACK;
    _serials_pkt_send(instance, &pkt);
}

void serials_send_err(serials_t *instance, uint8_t err_code)
{
    serials_pkt_t pkt;
    _serials_pkt_init(&pkt);
    pkt.cmd = SERIALS_CMD_ERR;
    pkt.arg = err_code;
    _serials_pkt_send(instance, &pkt);
}

void serials_send_bytes(serials_t *instance, uint8_t *data, uint32_t count)
{
    serials_pkt_t pkt;
    _serials_pkt_init(&pkt);
    pkt.cmd = SERIALS_CMD_BYTES;
    pkt.data = data;
    pkt.data_count = count;
    _serials_pkt_send(instance, &pkt);
}

