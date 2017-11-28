//*
// * pulses.p
// *
// *  Created on: Nov 27, 2017
// *      Author: Charles Borges Lima
// */

.setcallreg r10.w0
.origin 0			// inicio do programa na memória PRU
.entrypoint INICIO	// entrada de programa para o depurador

#define PRU0_R31_VEC_VALID 32    // allows notification of program completion
#define PRU_EVTOUT_0    3        // the event number that is sent back

#define nr_rept_1msL	0x8740		// 1ms/(5ns*2), 5ns por instrucao e duas por laco de repeticao
#define nr_rept_1msH    0x1


#define nr_pulos	#500			// essa variavel deve ser passada para o programa, não vai ser uma definicao

//------------------------------------------------------------------
INICIO:
	// Enable the OCP master port -- allows transfer of data to Linux userspace
	LBCO    r0, C4, 4, 4     // load SYSCFG reg into r0 (use c4 const addr)
	CLR     r0, r0, 4        // clear bit 4 (STANDBY_INIT)
    	SBCO    r0, C4, 4, 4     // store the modified r0 back at the load addr

	//MOV   r3, 0x0   
	//mov   r5, 0x0           // load SRAM init address to r5

	//LBBO    r8, r5, 0, 4     // load to r8 the Linux address that is passed throgh PRU SRAM
    	//LBBO	r9, r5, 4, 4	 // load to r9 the DDR size allocated

	//mov r6, r9

	ldi	r1, 500
	//--------------------------------------------------------------
	GERA_PULSOS:
		set		r30.t5				// pino 105
		call	DELAY_1ms
		clr		r30.t5
		call	DELAY_1ms
		sub		r1, r1, 1
		qbne	GERA_PULSOS, r1, 0	// repete ate que r1=0
	//--------------------------------------------------------------
FIM:
    MOV	R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	halt
//------------------------------------------------------------------
//------------------------------------------------------------------
DELAY_1ms:
	//mov r0, #50000
	ldi r0, nr_rept_1msL
	ldi r2, nr_rept_1msH
	lsl r2, r2, 16
	or r0,r0,r2
	DELAY1ms:
		sub	r0, r0, 1
		qbne	DELAY1ms, r0, 0
ret
//------------------------------------------------------------------
//------------------------------------------------------------------ 
