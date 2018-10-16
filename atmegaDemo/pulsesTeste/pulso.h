#ifndef _PULSO_H
#define _PULSO_H


#define EPPLEY_CTRL_PORT GPIO_C
#define AZ_PULSE_PIN	PC1	/* Arduino A1 */
#define AZ_DIR_PIN		PC2	/* Arduino A1 */
#define ZE_PULSE_PIN	PC3	/* Arduino A1 */
#define ZE_DIR_PIN		PC4	/* Arduino A1 */

void config_timer();
void desliga_pulso();
void gera_pulsos(uint16_t nr_pulsos, uint8_t pino);
volatile uint8_t is_pulsing();

#endif
