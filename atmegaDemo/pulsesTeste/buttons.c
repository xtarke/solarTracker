/*
 * buttons.c
 *
 *  Created on: Oct 11, 2018
 *      Author: xtarke
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lib/avr_adc.h"
#include "lib/bits.h"
#include "buttons.h"

volatile uint16_t valor_adc = 0;

enum BUTTONS_VALUE {
	RIGHT_ADC = 10,
	UP_ADC = 150,
	NONE_ADC = 1000,
	DOWN_ADC = 350,
	LEFT_ADC = 550,
	SELECT_ADC = 800
};


void buttons_init(){

	/* Ref externa no pino AVCC com capacitor de 100n em VREF.
		 * Habiltiação apenas no Canal 0 */
	ADCS->AD_MUX = SET(REFS0);
	/* Habilita AD:
	 * Conversão contínua
	 * IRQ ativo
	 * Prescaler de 128 */
	ADCS->ADC_SRA = SET(ADEN)  |	//ADC Enable
						SET(ADSC)  | 	// ADC Start conversion
						SET(ADATE) |	// ADC Auto Trigger
						SET(ADPS0) | SET(ADPS1) | SET(ADPS2) | //ADPS[0..2] AD Prescaler selection
						SET(ADIE); 		//AD IRQ ENABLE

	/* Desabilita hardware digital de PC0 */
	ADCS->DIDr0.BITS.ADC0 = 1;
}

uint8_t get_button(uint8_t old_press){

	uint8_t ret = NONE;

	if (valor_adc < RIGHT_ADC)
		ret = RIGHT;
	else if (valor_adc < UP_ADC)
		ret = UP;
	else if (valor_adc < DOWN_ADC)
		ret = DOWN;
	else if (valor_adc < LEFT_ADC)
		ret = LEFT;
	else if (valor_adc < SELECT_ADC)
		ret = SELECT;
	else
		ret = NONE;

	if (ret == old_press)
		ret = old_press;

	return ret;
}


ISR(ADC_vect)
{
	/* Lê o valor do conversor AD na interrupção:
	 * ADC é de 10 bits, logo valor_adc deve ser
	 * de 16 bits
	 */
	valor_adc = ADC;
}
