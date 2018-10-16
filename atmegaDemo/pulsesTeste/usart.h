/*
 * usart.h
 *
 *  Created on: Oct 8, 2016
 *      Author: xtarke
 */

#ifndef USART_H_
#define USART_H_

typedef enum {
    _9600,
	_57600,
    _115200,
} BAUD_RATE_T;

void USART_init(BAUD_RATE_T baud_rate, uint8_t *buffer, uint8_t buffer_size);
void USART_tx(unsigned char data);

unsigned char USART_rx_complete (void);


#endif /* USART_H_ */
