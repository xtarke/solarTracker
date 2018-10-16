/*
 * usart.c
 *
 *  Created on: Oct 8, 2016
 *      Author: xtarke
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"

#define BR_9600  (F_CPU/16/9600-1)
#define BR_57600  (F_CPU/16/57600-1)
#define BR_115200  (F_CPU/16/115200-1)

struct {
	BAUD_RATE_T baud_rate;
	uint8_t *buffer;
	uint8_t buffer_size;
	uint8_t buffer_index;
	uint8_t cmd_rcv;
} usart_config;

uint8_t Write_USART_buffer(uint8_t u8data);

ISR(USART_RX_vect){
	uint8_t temp = UDR0;

	if ((Write_USART_buffer(temp) == 0) || temp == '.'){
		//disable reception and RX Complete interrupt
		//UCSR0B &= ~((1<<RXEN0)|(1<<RXCIE0));
		Write_USART_buffer('\0');
		usart_config.cmd_rcv = 1;
	}
}

void USART_init(BAUD_RATE_T baud_rate, uint8_t *buffer, uint8_t buffer_size)
{
	usart_config.baud_rate = baud_rate;
	usart_config.buffer = buffer;
	usart_config.buffer_size = buffer_size;
	usart_config.buffer_index = 0;
	usart_config.cmd_rcv = 0;

	switch (baud_rate){
	case _9600:
		UBRR0H = (unsigned char)(BR_9600>>8);
		UBRR0L = (unsigned char) BR_9600;
		break;

	case _57600:
		UBRR0H = (unsigned char)(BR_57600>>8);
		UBRR0L = (unsigned char) BR_57600;
		break;

	case _115200:
		UBRR0H = (unsigned char)(BR_115200>>8);
		UBRR0L = (unsigned char) BR_115200;
		break;
	}

	UCSR0A = 0;//desabilitar velocidade dupla (no Arduino é habilitado por padrão)
	UCSR0B = (1 << 7) | (1<<RXEN0)|(1<<TXEN0); //Habilita a transmissão e a recepção
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);/*modo assíncrono, 8 bits de dados, 1 bit de parada, sem paridade*/
}

void USART_tx(unsigned char data)
{
	while (!( UCSR0A & (1<<UDRE0)) );	//espera o dado ser enviado
	UDR0 = data; 					   //envia o dado
}

unsigned char USART_rx_complete (void)
{
	if (usart_config.cmd_rcv == 1){
		usart_config.buffer_index = 0;
		usart_config.cmd_rcv = 0;
		return 1;
	}

	return 0;
}

uint8_t Write_USART_buffer(uint8_t u8data)
{
	if (usart_config.buffer_index < usart_config.buffer_size){
		usart_config.buffer[usart_config.buffer_index] = u8data;
		usart_config.buffer_index++;

		return 1;
	}

	return 0;
}


