#ifndef LIGHTSERIAL_H
#define LIGHTSERIAL_H


#include <stdint.h>
#include <stdbool.h>

// Byte stuffing bytes
#define LS_BS_BEGIN	0x0D 	// '\r'
#define LS_BS_END	0x0A 	// '\n'
#define LS_BS_ESC	0x5E	// '^'


// Packet structure (without byte stuffing)
// [CMD] [ARG] [DATA]*
// The argument belongs to the CMD while CODE and DATA are set
// by the user. All fields are 1-byte wide, except DATA which is a
// byte array.


#define LS_NO_ARG	0xFF


typedef enum ls_cmd_e
{
	LS_CMD_ACK 		= 0xAA,
	LS_CMD_ERR 		= 0xFF,
	LS_CMD_BYTES	= 0x01,
} ls_cmd_t;

typedef struct ls_pkt_s
{
	ls_cmd_t 	cmd;
	uint8_t		arg;
	uint8_t		*data;
	uint32_t	data_count;
} ls_pkt_t;

typedef struct ls_init_s
{
	uint8_t *rx_buf;
	uint32_t rx_buf_len;
	void (*fn_send_byte)(uint8_t data);
	void (*cb_rxpkt_ready)(void);
} ls_init_t;

typedef struct ls_s
{
	ls_pkt_t rx_pkt;
	uint8_t *rx_buf;
	uint32_t rx_buf_len;
	uint32_t rx_cnt;
	bool bs_esc;
	bool rxpkt_ready;
	void (*fn_send_byte)(uint8_t data);
	void (*cb_rxpkt_ready)(void);
} ls_t;


void ls_init(ls_t *instance, ls_init_t *init);
void ls_parse_bytes(ls_t *instance, uint8_t *data, uint32_t count);
bool ls_rxpkt_ready(ls_t *instance);
void ls_rxpkt_ready_clear(ls_t *instance);
ls_pkt_t* ls_rxpkt(ls_t *instance);

void ls_send_ack(ls_t *instance);
void ls_send_err(ls_t *instance, uint8_t err_code);
void ls_send_bytes(ls_t *instance, uint8_t *data, uint32_t count);

#endif // LIGHTSERIAL_H
