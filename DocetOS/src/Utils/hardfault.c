#include <stdint.h>
#include "cmsis_armclang.h"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

/**
 * HardFaultHandler_C:
 * This is called from the HardFault_Handler (below) with a pointer to the fault stack
 * as the parameter.  We can then unroll the stack and place them into local variables
 * for ease of reading.
 * 
 * The various fault status and address registers help diagnose the cause of the fault.
 * The function ends with a BKPT instruction to pass control back into the debugger.
 * 
 * HardFault_HandlerC is non-static because it must be accessible to the linker
 * (the compiler will not make the association with the B instruction in HardFault_Handler).
 * 
 * -Wunused-variable warnings suppressed for all locals in HardFault_HandlerC.
 * -Wmissing-prototypes warning suppressed for HardFault_Handler (it isn't part of the API
 * but cannot be static).
 */
void HardFault_HandlerC(uint32_t *hardfault_args) {
	volatile uint32_t stacked_r0  = hardfault_args[0];
	volatile uint32_t stacked_r1  = hardfault_args[1];
	volatile uint32_t stacked_r2  = hardfault_args[2];
	volatile uint32_t stacked_r3  = hardfault_args[3];
	volatile uint32_t stacked_r12 = hardfault_args[4];
	volatile uint32_t stacked_lr  = hardfault_args[5];
	volatile uint32_t stacked_pc  = hardfault_args[6];
	volatile uint32_t stacked_psr = hardfault_args[7];
	
	// Configurable Fault Status Register
	// Consists of MMSR, BFSR and UFSR
	volatile uint32_t _CFSR = (*((volatile uint32_t *)(0xE000ED28))) ;   
																																									
	// Hard Fault Status Register
	volatile uint32_t _HFSR = (*((volatile uint32_t *)(0xE000ED2C))) ;

	// Debug Fault Status Register
	volatile uint32_t _DFSR = (*((volatile uint32_t *)(0xE000ED30))) ;

	// Auxiliary Fault Status Register
	volatile uint32_t _AFSR = (*((volatile uint32_t *)(0xE000ED3C))) ;

	// Read the Fault Address Registers. These may not contain valid values.
	// Check BFARVALID/MMARVALID to see if they are valid values
	// MemManage Fault Address Register
	volatile uint32_t _MMAR = (*((volatile uint32_t *)(0xE000ED34))) ;
	// Bus Fault Address Register
	volatile uint32_t _BFAR = (*((volatile uint32_t *)(0xE000ED38))) ;

	__BKPT(0);
}

/**
 * HardFault_Handler:
 * This function is used as the hard fault handler because its name is the same
 * as the default implementation, and the default is subject to a weak export.
 * The LR return code is tested to see which stack was in use when the hard fault
 * occurred, and the relevant stack pointer is put into r0 before the function
 * above is invoked.
 */

__attribute__((naked))
void HardFault_Handler(void) {
	__asm volatile (
		"TST			lr, #4\n\t"
		"ITE			NE\n\t"
		"MRSNE		r0, PSP\n\t"
		"MRSEQ		r0, MSP\n\t"
		"B				HardFault_HandlerC"
	::: "cc" /* Also clobbers r0 but this is used for the "call" */
	);
}
