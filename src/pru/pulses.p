.setcallreg r10.w0  // configura registrador para salvamento do endereço de retorno
.origin 0			// inicio do programa na memória PRU
.entrypoint Inicio	// entrada de programa para o depurador

#define PRU0_R31_VEC_VALID 32    // allows notification of program completion
#define PRU_EVTOUT_0    3        // the event number that is sent back

//#define nr_rept_1msL	0x8740		// 1ms/(5ns*2), 5ns por instrucao e duas por laco de repeticao (parte baixa)
//#define nr_rept_1msH    0x1         // Aproximadamente 1.000.000 (parte alta): constantes máximas de 16-bits para o montador

//Lego:
#define nr_rept_1msL	0x3a00
#define nr_rept_1msH    0xC

//#define PULSE_PIN_A		r30.t5      // BB_pin P9.27

#define PULSE_PIN_A		r30.t14      // BB_pin P9.27

#define PULSE_PIN_Z		r30.t3      // BB_pin P9.28
#define CLOCKWISE_PIN_A r30.t1      // BB_pin P9.29
#define CLOCKWISE_PIN_Z r30.t2      // BB_pin P9.30
#define AZ_ABORT 		r31.t15	    // BB_pin P8.15
#define ZE_ABORT 		r31.t14	    // BB_pin P8.16

//------------------------------------------------------------------
Inicio:
	// Enable the OCP master port -- allows transfer of data to Linux userspace
	lbco    r0, c4, 4, 4     // load SYSCFG reg into r0 (use c4 const addr)
	clr     r0, r0, 4        // clear bit 4 (STANDBY_INIT)
    sbco    r0, C4, 4, 4   // store the modified r0 back at the load addr

	mov   r6, 0x0           // load SRAM init address (0x0000) to r6

    lbbo  r1, r6, 0, 4      // load to r1 the servo ID (0 -> ZENITH_SERVO, 1 -> AZIMUTH_SERVO)
    lbbo  r2, r6, 4, 4      // load to r2 the moviment (0-> CLOCKWISE_Z, 1 -> COUNTERCLOCKWISE_Z)
	lbbo  r3, r6, 8, 4      // load to r3 the moviment (0-> CLOCKWISE_A, 1 -> COUNTERCLOCKWISE_A)
    lbbo  r4, r6, 12, 4     // load to r4 the number of pulses Z
    lbbo  r5, r6, 16, 4     // load to r5 the number of pulses A  

	 
	// * ATENÇÃO NO REUSO DOS REGISTRADORES ! * //


	// qbbs Fim, AZ_ABORT              // halt if P9_28 is set
	 
	// verifica qual servo acionar
	qbeq    Servo_Z, r1, 0  //Se 0 aciona servo Zenite, senao aciona Azimuth
	//--------------------------------------------------------------------------
	Servo_A:
			qbeq    Sent_Horario_A, r3, 0 
            set		CLOCKWISE_PIN_A            
			qba		Gera_pulsos_A

		Sent_Horario_A:		
			clr		CLOCKWISE_PIN_A

		
		Gera_pulsos_A:
		
			qbbs Fim, ZE_ABORT
		
			set		PULSE_PIN_A					
			call	Delay_1ms
			clr		PULSE_PIN_A	
			call	Delay_1ms
			sub		r5, r5, 1
			qbne	Gera_pulsos_A, r5, 0	// repete ate que r4=0
			qba		Fim
	//--------------------------------------------------------------------------
	Servo_Z:	
			qbeq    Sent_Horario_Z, r2, 0 
			set		CLOCKWISE_PIN_Z
			qba		Gera_pulsos_Z

		Sent_Horario_Z:		
			clr		CLOCKWISE_PIN_Z

		
		Gera_pulsos_Z:
			set		PULSE_PIN_Z					
			call	Delay_1ms
			clr		PULSE_PIN_Z	
			call	Delay_1ms
			sub		r4, r4, 1
			qbne	Gera_pulsos_Z, r4, 0	// repete ate que r4=0
	//--------------------------------------------------------------

Fim:
    mov	R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	halt
//------------------------------------------------------------------

//------------------------------------------------------------------
Delay_1ms:  
	ldi r1, nr_rept_1msL
	ldi r2, nr_rept_1msH
	lsl r2, r2, 16
	or r1,r1,r2
	Delay1ms:
		sub	r1, r1, 1
		qbne	Delay1ms, r1, 0
ret
//------------------------------------------------------------------
//------------------------------------------------------------------ 
