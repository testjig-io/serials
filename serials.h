#ifndef SERIALS_H
#define SERIALS_H

#include <stdint.h>
#include <stdbool.h>

// Byte-stuffing bytes
#define SERIALS_BS_BEGIN	0x0D 	// '\r'
#define SERIALS_BS_END	    0x0A 	// '\n'
#define SERIALS_BS_ESC	    0x5E	// '^'

// Packet structure (without byte stuffing)
// [CMD] [ARG] [DATA]*
// The argument belongs to the CMD while DATA is set
// by the user. All fields are 1-byte wide, except DATA which is a
// byte array.

#define SERIALS_NO_ARG	0xFF

typedef enum serials_cmd_e
{
	SERIALS_CMD_ACK 		= 0xAA,
	SERIALS_CMD_ERR 		= 0xFF,
	SERIALS_CMD_BYTES	= 0x01,
} serials_cmd_t;

typedef struct serials_pkt_s
{
	serials_cmd_t 	cmd;
	uint8_t		arg;
	uint8_t		*data;
	uint32_t	data_count;
} serials_pkt_t;

typedef struct serials_init_s
{
	uint8_t *rx_buf;
	uint32_t rx_buf_len;
	void (*fn_send_byte)(uint8_t data);
	void (*cb_rxpkt_ready)(void);
} serials_init_t;

typedef struct serials_s
{
	serials_pkt_t rx_pkt;
	uint8_t *rx_buf;
	uint32_t rx_buf_len;
	uint32_t rx_cnt;
	bool bs_esc;
	bool overflow;
	bool rxpkt_ready;
	void (*fn_send_byte)(uint8_t data);
	void (*cb_rxpkt_ready)(void);
} serials_t;


void serials_init(serials_t *instance, serials_init_t *init);
void serials_parse_bytes(serials_t *instance, uint8_t *data, uint32_t count);
bool serials_overflow(serials_t *instance);
bool serials_rxpkt_ready(serials_t *instance);
void serials_rxpkt_ready_clear(serials_t *instance);
serials_pkt_t* serials_rxpkt(serials_t *instance);

void serials_send_ack(serials_t *instance);
void serials_send_err(serials_t *instance, uint8_t err_code);
void serials_send_bytes(serials_t *instance, uint8_t *data, uint32_t count);

#endif // SERIALS_H
