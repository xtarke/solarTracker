//*
// * pulses.p
// *
// *  Created on: Nov 27, 2017
// *      Author: Charles Borges Lima
// */


.origin 0			// inicio do programa na memória PRU
.entrypoint INICIO	// entrada de programa para o depurador

#define nr_rept_1ms	100000		// 1ms/(5ns*2), 5ns por instrucao e duas por laco de repeticao
#define nr_pulos	500			// essa variavel deve ser passada para o programa, não vai ser uma definicao

//------------------------------------------------------------------
INICIO:
	mov	r1, nr_pulos
	//--------------------------------------------------------------
	GERAL_PULOS:
		set		r30.t5				// pino 105
		call	DELAY_1ms
		clr		r30.t5
		call	DELAY_1ms
		sub		r1, r1, 1
		qbne	GERA_PULSOS, r1, 0	// repete ate que r1=0
	//--------------------------------------------------------------
FIM:
	halt
//------------------------------------------------------------------
//------------------------------------------------------------------
DELAY_1ms:
	mov r0, nr_rept_1ms
	DELAY1ms:
		sub	r0, r0, 1
		qbne	DELAY1ms, r0, 0
ret
//------------------------------------------------------------------
//------------------------------------------------------------------ 
