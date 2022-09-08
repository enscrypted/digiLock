;
; ras_asm.asm
;
; Created on: Oct 12, 2021
;



; To include names declared in C
                .cdecls "stdint.h","stdbool.h","inc/hw_memmap.h","driverlib/pin_map.h","driverlib/sysctl.h","driverlib/adc.h","ras.h"

                .text
                .global rasRead


RAS_PERIPH   .field  SYSCTL_PERIPH_ADC0
RAS_PORT     .field  ADC0_BASE

			;r0 and r1 are data
rasRead		PUSH 	{LR,r0}	;save return adress, stores space for data[]
			LDR		r0, RAS_PORT
			MOV		r1, #0
			BL		ADCProcessorTrigger	;runs ADCProcessorTrigger(RAS_PORT, 0)


while_loop	LDR r0, RAS_PORT ; sample sequence loop until complete
			MOV r1, #0			;r0 holds base address
			MOV r2, #false		;wait to finish
			BL ADCIntStatus		;run the setup function
			CMP r0, #0			;compare return and 0
			BEQ while_loop		;loop condit.

			;gets the data for a sample sequence | ADCSequenceDataGet(Base, SEQ number, buffer)
			LDR r0, RAS_PORT
			MOV r1, #0		;r1 = sequ.
			POP {r2}		;r2 is taken off stack
			BL ADCSequenceDataGet

			;clear interrupt status | ADCIntClear(ADC0_BASE, 0)
			LDR r0, RAS_PORT ;base address
			MOV r1, #0		 ;r1 = sequ.
			BL ADCIntClear
			POP {PC}		 ;return



