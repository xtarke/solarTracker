/*
 * pulso.c
 *
 *  Created on: May 1, 2018
 *      Author: Renan Augusto Starke
 *      Instituto Federal de Santa Catarina
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lib/bits.h"

#include "pulso.h"

#define SET_PULSO(x)		SET_BIT(PORTC, x)
#define CLR_PULSO(x)		CLR_BIT(PORTC, x)
#define TST_PULSO(x)		TST_BIT(PORTC, x)

#define FLAG_TIMER_ON 0

static volatile uint8_t FLAGS=0; 
static volatile uint16_t pulsos, cont_pulsos = 0;
volatile uint8_t pulse_pin = 0;


void desliga_pulso()
{
	TIMSK0 = 0;						// desabilita interrupcao
	cont_pulsos = 0;
	CLR_BIT(FLAGS, FLAG_TIMER_ON);	// libera novamente o timer
}

//-------------------------------------------------------------------------------
void config_timer()							// uso do TC0
{
	TCCR0B = (1<<CS01)|(1<<CS00);			// prescaler = 64 , F=16MHz, estouro a cada 1,024 ms
	TIMSK0 = 1 << TOIE0;					// habilita interrup��o do TC0
	TCNT0 = 0;
	CLR_PULSO(pulse_pin);
}

volatile uint8_t is_pulsing(){
	return FLAGS;
}


//-------------------------------------------------------------------------------
void gera_pulsos(uint16_t nr_pulsos, uint8_t pino)
{
	//while(TST_BIT(FLAGS, FLAG_TIMER_ON));	// espera liberar o uso do timer

	if (TST_BIT(FLAGS, FLAG_TIMER_ON))
		return;
	
	SET_BIT(FLAGS, FLAG_TIMER_ON);			// avisa que vai ligar o timer
	
	pulsos =  nr_pulsos;					// faz a leitura do nr_pulsos
	pulse_pin = pino;
	
	config_timer();
	
}
//-------------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
	if(TST_PULSO(pulse_pin))	// se pino em 1
	{
		CLR_PULSO(pulse_pin);
		cont_pulsos++;
		
		if(cont_pulsos==pulsos)
		{
			desliga_pulso(pulse_pin);
			//CPL_BIT(PORTB,1);
		}
	}
	else
		SET_PULSO(pulse_pin);
}
//-------------------------------------------------------------------------------
